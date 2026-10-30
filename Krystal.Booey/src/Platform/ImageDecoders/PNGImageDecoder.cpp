#include "Krystal.Booey/Platform/ImageDecoders/PNGImageDecoder.hpp"
#include <algorithm>
#include <cstring>
#include <png.h>

// libpng reports errors with longjmp to the setjmp in the reader, which owns no objects with destructors.
// clang-format off
#pragma warning(disable : 4611) // interaction between '_setjmp' and C++ object destruction is non-portable
// clang-format on

namespace krys::boo
{
  namespace
  {
    // Gamma constants.
    constexpr double MaxGamma = 21474.83;
    constexpr double DefaultGamma = 2.2;
    constexpr double InverseGamma = 0.45455;

    // Protect against APNGs with huge amounts of frames, and PNGs with large amounts
    // of pixel data in general. See bug #302220 and Mozilla's bug #251381 for more info.
    //
    // The maximum frame count limits the memory used for array of ScalableImageDecoderFrames
    // to take at most 10MiB on 64-bit platforms, and it is large enough to cover ~70 minutes
    // of animation at 60 FPS.
    constexpr uint32 MaxFrameCount = (1 << 18) - 1;
    constexpr uint32 MaxPNGSize = 1'000'000;
    constexpr uint64 MaxDecodedPixels = static_cast<uint64>(MaxPNGSize) * MaxPNGSize;

    // Called if the decoding of the image fails.
    void PNGAPI DecodingFailed(png_structp png, png_const_charp)
    {
      longjmp(png_jmpbuf(png), 1);
    }

    // Callbacks given to the read struct.  The first is for warnings (we want to
    // treat a particular warning as an error, which is why we have to register this
    // callback).
    void PNGAPI DecodingWarning(png_structp png, png_const_charp warningMsg)
    {
      // Mozilla did this, so we will too.
      // Convert a tRNS warning to be an error (see
      // http://bugzilla.mozilla.org/show_bug.cgi?id=251381 )
      constexpr const char *tRNSWarning = "Missing PLTE before tRNS";
      if (!std::strncmp(warningMsg, tRNSWarning, std::strlen(tRNSWarning)))
      {
        png_error(png, warningMsg);
      }
    }

    // Called when we have obtained the header information (including the size).
    void PNGAPI HeaderAvailableCallback(png_structp png, png_infop)
    {
      static_cast<PNGImageDecoder *>(png_get_progressive_ptr(png))->HeaderAvailable();
    }

    // Called when a row is ready.
    void PNGAPI RowAvailableCallback(png_structp png, png_bytep rowBuffer, png_uint_32 rowIndex,
                                     int interlacePass)
    {
      static_cast<PNGImageDecoder *>(png_get_progressive_ptr(png))
        ->RowAvailable(rowBuffer, rowIndex, interlacePass);
    }

    // Called when we have completely finished decoding the image.
    void PNGAPI PngCompleteCallback(png_structp png, png_infop)
    {
      static_cast<PNGImageDecoder *>(png_get_progressive_ptr(png))->PngComplete();
    }

    // Called when we have the frame header.
    void PNGAPI FrameHeaderCallback(png_structp png, png_infop)
    {
      static_cast<PNGImageDecoder *>(png_get_progressive_ptr(png))->FrameHeader();
    }

    // Called when we found user chunks.
    int PNGAPI ReadChunksCallback(png_structp png, png_unknown_chunkp chunk)
    {
      static_cast<PNGImageDecoder *>(png_get_user_chunk_ptr(png))->ReadChunks(chunk);
      return 1;
    }

    KRYS_NODISCARD bool ChunkNameIs(const png_unknown_chunk &chunk, const char *name) noexcept
    {
      return !std::memcmp(chunk.name, name, 4);
    }
  }

  class PNGImageReader
  {
    KRYS_NON_COPY_MOVABLE(PNGImageReader);

  private:
    png_structp _png;
    png_infop _info;
    uint32 _readOffset {0};
    uint32 _currentBufferSize {0};
    bool _decodingSizeOnly {false};
    bool _hasAlpha {false};
    UniquePtr<png_byte[]> _interlaceBuffer;

  public:
    explicit PNGImageReader(PNGImageDecoder *decoder) noexcept
        : _png(png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, DecodingFailed, DecodingWarning)),
          _info(png_create_info_struct(_png))
    {
      png_set_progressive_read_fn(_png, decoder, HeaderAvailableCallback, RowAvailableCallback,
                                  PngCompleteCallback);
      png_byte apngChunks[] = {"acTL\0fcTL\0fdAT\0"};
      png_set_keep_unknown_chunks(_png, 1, apngChunks, 3);
      png_set_read_user_chunk_fn(_png, static_cast<png_voidp>(decoder), ReadChunksCallback);
      decoder->Init();
    }

    ~PNGImageReader() noexcept
    {
      Close();
    }

    void Close() noexcept
    {
      if (_png && _info)
      {
        // This will zero the pointers.
        png_destroy_read_struct(&_png, &_info, nullptr);
      }
      _interlaceBuffer.reset();
      _readOffset = 0;
    }

    // NOTE: not noexcept: libpng's errors longjmp back to the setjmp here, through its callbacks.
    bool Decode(const SharedBuffer &data, bool sizeOnly, unsigned haltAtFrame)
    {
      _decodingSizeOnly = sizeOnly;
      auto *decoder = static_cast<PNGImageDecoder *>(png_get_progressive_ptr(_png));

      // We need to do the setjmp here. Otherwise bad things will happen.
      if (setjmp(png_jmpbuf(_png)))
      {
        return decoder->SetFailed();
      }

      auto bytesToSkip = _readOffset;
      auto bytesToUse = static_cast<uint32>(data.Size()) - bytesToSkip;
      _readOffset += bytesToUse;
      _currentBufferSize = _readOffset;
      png_process_data(_png, _info, const_cast<png_bytep>(data.Data() + bytesToSkip), bytesToUse);
      // We explicitly specify the superclass encodedDataStatus() because we
      // merely want to check if we've managed to set the size, not
      // (recursively) trigger additional decoding if we haven't.
      if (sizeOnly ? decoder->ScalableImageDecoder::GetEncodedDataStatus() >= EncodedDataStatus::SizeAvailable
                   : decoder->IsCompleteAtIndex(haltAtFrame))
      {
        return true;
      }

      return false;
    }

    KRYS_NODISCARD png_structp PngPtr() const noexcept
    {
      return _png;
    }
    KRYS_NODISCARD png_infop InfoPtr() const noexcept
    {
      return _info;
    }

    void SetReadOffset(uint32 offset) noexcept
    {
      _readOffset = offset;
    }
    KRYS_NODISCARD uint32 CurrentBufferSize() const noexcept
    {
      return _currentBufferSize;
    }
    KRYS_NODISCARD bool DecodingSizeOnly() const noexcept
    {
      return _decodingSizeOnly;
    }
    void SetHasAlpha(bool hasAlpha) noexcept
    {
      _hasAlpha = hasAlpha;
    }
    KRYS_NODISCARD bool HasAlpha() const noexcept
    {
      return _hasAlpha;
    }

    KRYS_NODISCARD png_bytep InterlaceBuffer() const noexcept
    {
      return _interlaceBuffer.get();
    }
    void CreateInterlaceBuffer(size_t size) noexcept
    {
      _interlaceBuffer = UniquePtr<png_byte[]>(new png_byte[size]);
    }
  };

  PNGImageDecoder::PNGImageDecoder(AlphaOption alphaOption,
                                   GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  PNGImageDecoder::~PNGImageDecoder() noexcept
  {
    Clear();
  }

  RepetitionCount PNGImageDecoder::GetRepetitionCount() const noexcept
  {
    // Signal no repetition if the PNG image is not animated.
    if (!_isAnimated)
    {
      return RepetitionCountNone;
    }

    // APNG format uses 0 to indicate that an animation must play indefinitely. But
    // the RepetitionCount enumeration uses RepetitionCountInfinite, so we need to adapt this.
    if (!_playCount)
    {
      return RepetitionCountInfinite;
    }

    return static_cast<RepetitionCount>(_playCount);
  }

  ScalableImageDecoderFrame *PNGImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (ScalableImageDecoder::GetEncodedDataStatus() < EncodedDataStatus::SizeAvailable)
    {
      return nullptr;
    }

    if (index >= FrameCount())
    {
      index = FrameCount() - 1;
    }

    if (_frameBufferCache.empty())
    {
      _frameBufferCache.resize(1);
    }

    auto &frame = _frameBufferCache[index];
    if (!frame.IsComplete())
    {
      Decode(false, static_cast<unsigned>(index), IsAllDataReceived());
    }
    return &_frameBufferCache[index];
  }

  void PNGImageDecoder::Clear() noexcept
  {
    _reader = nullptr;
  }

  bool PNGImageDecoder::SetFailed() noexcept
  {
    if (_doNothingOnFailure)
    {
      return false;
    }
    Clear();
    return ScalableImageDecoder::SetFailed();
  }

  void PNGImageDecoder::HeaderAvailable()
  {
    png_structp png = _reader->PngPtr();
    png_infop info = _reader->InfoPtr();
    png_uint_32 width = png_get_image_width(png, info);
    png_uint_32 height = png_get_image_height(png, info);

    // Protect against large images.
    uint64 pixelCount = static_cast<uint64>(width) * height + _decodedPixelCount;
    if (pixelCount > MaxDecodedPixels)
    {
      longjmp(png_jmpbuf(png), 1);
    }
    _decodedPixelCount = static_cast<size_t>(pixelCount);

    // We can fill in the size now that the header is available.  Avoid memory
    // corruption issues by returning early from setFailed() during this call; if we don't
    // do this, failures will cause |m_reader| to be deleted, and our jmpbuf
    // will cease to exist.  Note that we'll still properly set the failure flag
    // in this case as soon as we longjmp().
    _doNothingOnFailure = true;
    bool result = SetSize(IntSize(static_cast<int32>(width), static_cast<int32>(height)));
    _doNothingOnFailure = false;
    if (!result)
    {
      longjmp(png_jmpbuf(png), 1);
    }

    int bitDepth, colorType, interlaceType, compressionType, filterType, channels;
    png_get_IHDR(png, info, &width, &height, &bitDepth, &colorType, &interlaceType, &compressionType,
                 &filterType);

    // The options we set here match what Mozilla does.

    _hasInfo = true;
    if (_isAnimated)
    {
      png_save_uint_32(_dataIHDR.data(), 13);
      std::memcpy(_dataIHDR.data() + 4, "IHDR", 4);
      png_save_uint_32(&_dataIHDR[8], width);
      png_save_uint_32(&_dataIHDR[12], height);
      _dataIHDR[16] = static_cast<png_byte>(bitDepth);
      _dataIHDR[17] = static_cast<png_byte>(colorType);
      _dataIHDR[18] = static_cast<png_byte>(compressionType);
      _dataIHDR[19] = static_cast<png_byte>(filterType);
      _dataIHDR[20] = static_cast<png_byte>(interlaceType);
    }

    // Expand to ensure we use 24-bit for RGB and 32-bit for RGBA.
    if (colorType == PNG_COLOR_TYPE_PALETTE)
    {
      if (_isAnimated)
      {
        png_colorp palette;
        int paletteSize = 0;
        png_get_PLTE(png, info, &palette, &paletteSize);
        paletteSize *= 3;
        png_save_uint_32(_dataPLTE.data(), static_cast<png_uint_32>(paletteSize));
        std::memcpy(_dataPLTE.data() + 4, "PLTE", 4);
        std::memcpy(_dataPLTE.data() + 8, palette, static_cast<size_t>(paletteSize));
        _sizePLTE = static_cast<uint32>(paletteSize) + 12;
      }
      png_set_expand(png);
    }

    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    {
      png_set_expand(png);
    }

    png_bytep trns = nullptr;
    int trnsCount = 0;
    png_color_16p transValues;
    if (png_get_valid(png, info, PNG_INFO_tRNS))
    {
      png_get_tRNS(png, info, &trns, &trnsCount, &transValues);
      if (_isAnimated)
      {
        if (colorType == PNG_COLOR_TYPE_RGB)
        {
          png_save_uint_16(&_datatRNS[8], transValues->red);
          png_save_uint_16(&_datatRNS[10], transValues->green);
          png_save_uint_16(&_datatRNS[12], transValues->blue);
          trnsCount = 6;
        }
        else if (colorType == PNG_COLOR_TYPE_GRAY)
        {
          png_save_uint_16(&_datatRNS[8], transValues->gray);
          trnsCount = 2;
        }
        else if (colorType == PNG_COLOR_TYPE_PALETTE)
        {
          std::memcpy(_datatRNS.data() + 8, trns, static_cast<size_t>(trnsCount));
        }

        png_save_uint_32(_datatRNS.data(), static_cast<png_uint_32>(trnsCount));
        std::memcpy(_datatRNS.data() + 4, "tRNS", 4);
        _sizetRNS = static_cast<uint32>(trnsCount) + 12;
      }
      png_set_expand(png);
    }

    if (bitDepth == 16)
    {
      png_set_strip_16(png);
    }

    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
      png_set_gray_to_rgb(png);
    }

    // Deal with gamma and keep it under our control.
    double gamma;
    if (!_ignoreGammaAndColorProfile && png_get_gAMA(png, info, &gamma))
    {
      if ((gamma <= 0.0) || (gamma > MaxGamma))
      {
        gamma = InverseGamma;
        png_set_gAMA(png, info, gamma);
      }
      png_set_gamma(png, DefaultGamma, gamma);
      _gamma = static_cast<int32>(gamma * 100'000);
    }
    else
    {
      png_set_gamma(png, DefaultGamma, InverseGamma);
    }

    // FIXME: ICC colour profiles (LCMS), once there's colour management.

    // Tell libpng to send us rows for interlaced pngs.
    if (interlaceType == PNG_INTERLACE_ADAM7)
    {
      png_set_interlace_handling(png);
    }

    // Update our info now.
    png_read_update_info(png, info);
    channels = png_get_channels(png, info);
    krys_debug_assert(channels == 3 || channels == 4);

    _reader->SetHasAlpha(channels == 4);

    if (_reader->DecodingSizeOnly())
    {
      // If we only needed the size, halt the reader.
      // '0' argument to png_process_data_pause means: Do not cache unprocessed data.
      _reader->SetReadOffset(_reader->CurrentBufferSize()
                             - static_cast<uint32>(png_process_data_pause(png, 0)));
    }
  }

  void PNGImageDecoder::RowAvailable(unsigned char *rowBuffer, unsigned rowIndex, int)
  {
    if (_frameBufferCache.empty())
    {
      return;
    }

    // Initialize the framebuffer if needed.
    if (_currentFrame >= FrameCount())
    {
      return;
    }
    auto &buffer = _frameBufferCache[_currentFrame];
    if (buffer.IsInvalid())
    {
      png_structp png = _reader->PngPtr();
      if (!buffer.Initialize(Size(), _premultiplyAlpha))
      {
        longjmp(png_jmpbuf(png), 1);
      }

      unsigned colorChannels = _reader->HasAlpha() ? 4 : 3;
      if (PNG_INTERLACE_ADAM7 == png_get_interlace_type(png, _reader->InfoPtr()) || _currentFrame)
      {
        if (!_reader->InterlaceBuffer())
        {
          _reader->CreateInterlaceBuffer(static_cast<size_t>(colorChannels) * Size().Width()
                                         * Size().Height());
        }
        if (!_reader->InterlaceBuffer())
        {
          longjmp(png_jmpbuf(png), 1);
        }
      }

      buffer.SetDecodingStatus(DecodingStatus::Partial);
      buffer.SetHasAlpha(false);

      if (_currentFrame)
      {
        InitFrameBuffer(_currentFrame);
      }
    }

    /* libpng comments (here to explain what follows).
     *
     * this function is called for every row in the image. If the
     * image is interlacing, and you turned on the interlace handler,
     * this function will be called for every row in every pass.
     * Some of these rows will not be changed from the previous pass.
     * When the row is not changed, the new_row variable will be NULL.
     * The rows and passes are called in order, so you don't really
     * need the row_num and pass, but I'm supplying them because it
     * may make your life easier.
     */

    // Nothing to do if the row is unchanged, or the row is outside
    // the image bounds: libpng may send extra rows, ignore them to
    // make our lives easier.
    if (!rowBuffer)
    {
      return;
    }
    if (rowIndex >= static_cast<unsigned>(Size().Height()))
    {
      return;
    }

    /* libpng comments (continued).
     *
     * For the non-NULL rows of interlaced images, you must call
     * png_progressive_combine_row() passing in the row and the
     * old row.  You can call this function for NULL rows (it will
     * just return) and for non-interlaced images (it just does the
     * memcpy for you) if it will make the code easier. Thus, you
     * can just do this for all cases:
     *
     *    png_progressive_combine_row(png_ptr, old_row, new_row);
     *
     * where old_row is what was displayed for previous rows. Note
     * that the first pass (pass == 0 really) will completely cover
     * the old row, so the rows do not have to be initialized. After
     * the first pass (and only for interlaced images), you will have
     * to pass the current row, and the function will combine the
     * old row and the new row.
     */

    bool hasAlpha = _reader->HasAlpha();
    png_bytep row = rowBuffer;

    if (png_bytep interlaceBuffer = _reader->InterlaceBuffer())
    {
      unsigned colorChannels = hasAlpha ? 4 : 3;
      row = interlaceBuffer + (static_cast<size_t>(rowIndex) * colorChannels * Size().Width());
      if (_currentFrame)
      {
        png_progressive_combine_row(_png, row, rowBuffer);
        return; // Only do incremental image display for the first frame.
      }
      png_progressive_combine_row(_reader->PngPtr(), row, rowBuffer);
    }

    // Write the decoded row pixels to the frame buffer.
    auto *address = buffer.BackingStore()->PixelsStartingAt(0, static_cast<int32>(rowIndex));
    int32 width = Size().Width();
    unsigned char nonTrivialAlphaMask = 0;

    png_bytep pixel = row;
    if (hasAlpha)
    {
      for (int32 x = 0; x < width; ++x, pixel += 4, ++address)
      {
        unsigned alpha = pixel[3];
        buffer.BackingStore()->SetPixel(*address, pixel[0], pixel[1], pixel[2], static_cast<uint8>(alpha));
        nonTrivialAlphaMask |= static_cast<unsigned char>(255 - alpha);
      }
    }
    else
    {
      for (int32 x = 0; x < width; ++x, pixel += 3, ++address)
      {
        *address = 0xFF'00'00'00u | static_cast<uint32>(pixel[0]) << 16 | static_cast<uint32>(pixel[1]) << 8
                   | pixel[2];
      }
    }

    if (nonTrivialAlphaMask && !buffer.HasAlpha())
    {
      buffer.SetHasAlpha(true);
    }
  }

  void PNGImageDecoder::PngComplete()
  {
    if (_isAnimated)
    {
      if (!ProcessingFinish() && _frameCount == _currentFrame)
      {
        return;
      }

      FallbackNotAnimated();
    }
    if (!_frameBufferCache.empty())
    {
      _frameBufferCache.front().SetDecodingStatus(DecodingStatus::Complete);
    }
  }

  void PNGImageDecoder::Decode(bool onlySize, unsigned haltAtFrame, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    if (!_reader)
    {
      Clear();
      _reader = CreateUnique<PNGImageReader>(this);
    }

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if (!_reader->Decode(*_data, onlySize, haltAtFrame) && allDataReceived)
    {
      (void)SetFailed();
    }
    // If we're done decoding the image, we don't need the PNGImageReader
    // anymore.  (If we failed, |m_reader| has already been cleared.)
    else if (IsComplete())
    {
      Clear();
    }
  }

  void PNGImageDecoder::ReadChunks(png_unknown_chunkp chunk)
  {
    if (chunk->size == 8 && ChunkNameIs(*chunk, "acTL"))
    {
      if (_hasInfo || _isAnimated)
      {
        return;
      }

      _frameCount = png_get_uint_32(chunk->data);
      _playCount = png_get_uint_32(chunk->data + 4);

      if (!_frameCount || _frameCount > MaxFrameCount || _playCount > PNG_UINT_31_MAX)
      {
        FallbackNotAnimated();
        return;
      }

      _isAnimated = true;
      if (!_frameInfo)
      {
        _frameIsHidden = true;
      }

      if (_frameBufferCache.size() == _frameCount)
      {
        return;
      }

      _frameBufferCache.resize(_frameCount);
    }
    else if (chunk->size == 26 && ChunkNameIs(*chunk, "fcTL"))
    {
      if (_hasInfo && !_isAnimated)
      {
        return;
      }

      _frameInfo = false;

      if (ProcessingFinish())
      {
        FallbackNotAnimated();
        return;
      }

      // At this point the old frame is done. Let's start a new one.
      unsigned sequenceNumber = png_get_uint_32(chunk->data);
      if (sequenceNumber != _sequenceNumber++)
      {
        FallbackNotAnimated();
        return;
      }

      _width = png_get_uint_32(chunk->data + 4);
      _height = png_get_uint_32(chunk->data + 8);
      _xOffset = png_get_uint_32(chunk->data + 12);
      _yOffset = png_get_uint_32(chunk->data + 16);
      _delayNumerator = png_get_uint_16(chunk->data + 20);
      _delayDenominator = png_get_uint_16(chunk->data + 22);
      _dispose = chunk->data[24];
      _blend = chunk->data[25];

      png_structp png = _reader->PngPtr();
      png_infop info = _reader->InfoPtr();
      png_uint_32 width = png_get_image_width(png, info);
      png_uint_32 height = png_get_image_height(png, info);

      // Protect against large images.
      uint64 pixelCount = static_cast<uint64>(width) * height + _decodedPixelCount;
      if (pixelCount > MaxDecodedPixels || _xOffset > MaxPNGSize || _yOffset > MaxPNGSize
          || _xOffset + _width > width || _yOffset + _height > height || _dispose > 2 || _blend > 1)
      {
        FallbackNotAnimated();
        return;
      }
      _decodedPixelCount = static_cast<size_t>(pixelCount);

      if (_frameBufferCache.empty())
      {
        _frameBufferCache.resize(1);
      }

      if (_currentFrame < _frameBufferCache.size())
      {
        auto &buffer = _frameBufferCache[_currentFrame];

        if (!_delayDenominator)
        {
          buffer.SetDuration(milliseconds(_delayNumerator * 10.0));
        }
        else
        {
          buffer.SetDuration(milliseconds(_delayNumerator * 1000.0 / _delayDenominator));
        }

        if (_dispose == 2)
        {
          buffer.SetDisposalMethod(ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious);
        }
        else if (_dispose == 1)
        {
          buffer.SetDisposalMethod(ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground);
        }
        else
        {
          buffer.SetDisposalMethod(ScalableImageDecoderFrame::DisposalMethod::DoNotDispose);
        }
      }

      _frameInfo = true;
      _frameIsHidden = false;

      if (ProcessingStart(chunk))
      {
        FallbackNotAnimated();
        return;
      }
    }
    else if (chunk->size >= 4 && ChunkNameIs(*chunk, "fdAT"))
    {
      if (!_frameInfo || !_isAnimated)
      {
        return;
      }

      unsigned sequenceNumber = png_get_uint_32(chunk->data);
      if (sequenceNumber != _sequenceNumber++)
      {
        FallbackNotAnimated();
        return;
      }

      if (setjmp(png_jmpbuf(_png)))
      {
        FallbackNotAnimated();
        return;
      }

      png_save_uint_32(chunk->data, static_cast<png_uint_32>(chunk->size - 4));
      png_process_data(_png, _info, chunk->data, 4);
      std::memcpy(chunk->data, "IDAT", 4);
      png_process_data(_png, _info, chunk->data, chunk->size);
      png_process_data(_png, _info, chunk->data, 4);
    }
  }

  void PNGImageDecoder::FrameHeader()
  {
    int colorType = png_get_color_type(_png, _info);

    if (colorType == PNG_COLOR_TYPE_PALETTE)
    {
      png_set_expand(_png);
    }

    int bitDepth = png_get_bit_depth(_png, _info);
    if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
    {
      png_set_expand(_png);
    }

    if (png_get_valid(_png, _info, PNG_INFO_tRNS))
    {
      png_set_expand(_png);
    }

    if (bitDepth == 16)
    {
      png_set_strip_16(_png);
    }

    if (colorType == PNG_COLOR_TYPE_GRAY || colorType == PNG_COLOR_TYPE_GRAY_ALPHA)
    {
      png_set_gray_to_rgb(_png);
    }

    double gamma;
    if (png_get_gAMA(_png, _info, &gamma))
    {
      png_set_gamma(_png, DefaultGamma, gamma);
    }

    png_set_interlace_handling(_png);

    png_read_update_info(_png, _info);
  }

  void PNGImageDecoder::Init() noexcept
  {
    _isAnimated = false;
    _frameInfo = false;
    _frameIsHidden = false;
    _hasInfo = false;
    _currentFrame = 0;
    _totalFrames = 0;
    _sequenceNumber = 0;
  }

  void PNGImageDecoder::ClearFrameBufferCache(size_t clearBeforeFrame) noexcept
  {
    if (_frameBufferCache.empty())
    {
      return;
    }

    // See GIFImageDecoder for full explanation.
    clearBeforeFrame = std::min(clearBeforeFrame, _frameBufferCache.size() - 1);
    const auto end = _frameBufferCache.begin() + static_cast<ptrdiff_t>(clearBeforeFrame);

    auto i = end;
    for (; (i != _frameBufferCache.begin())
           && (i->IsInvalid()
               || (i->GetDisposalMethod() == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious));
         --i)
    {
      if (i->IsComplete() && (i != end))
      {
        i->Clear();
      }
    }

    // Now |i| holds the last frame we need to preserve; clear prior frames.
    for (auto j = _frameBufferCache.begin(); j != i; ++j)
    {
      krys_debug_assert(!j->IsPartial());
      if (j->IsInvalid())
      {
        j->Clear();
      }
    }
  }

  void PNGImageDecoder::InitFrameBuffer(size_t frameIndex)
  {
    if (frameIndex >= FrameCount())
    {
      return;
    }

    auto &buffer = _frameBufferCache[frameIndex];

    // The starting state for this frame depends on the previous frame's
    // disposal method.
    //
    // Frames that use the DisposalMethod::RestoreToPrevious method are effectively
    // no-ops in terms of changing the starting state of a frame compared to
    // the starting state of the previous frame, so skip over them.  (If the
    // first frame specifies this method, it will get treated like
    // DisposeOverwriteBgcolor below and reset to a completely empty image.)
    const auto *prevBuffer = &_frameBufferCache[--frameIndex];
    auto prevMethod = prevBuffer->GetDisposalMethod();
    while (frameIndex && (prevMethod == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious))
    {
      prevBuffer = &_frameBufferCache[--frameIndex];
      prevMethod = prevBuffer->GetDisposalMethod();
    }

    png_structp png = _reader->PngPtr();
    krys_debug_assert(prevBuffer->IsComplete());

    if (prevMethod == ScalableImageDecoderFrame::DisposalMethod::DoNotDispose)
    {
      // Preserve the last frame as the starting state for this frame.
      if (!prevBuffer->BackingStore() || !buffer.Initialize(*prevBuffer->BackingStore()))
      {
        longjmp(png_jmpbuf(png), 1);
      }
    }
    else
    {
      // We want to clear the previous frame to transparent, without
      // affecting pixels in the image outside of the frame.
      IntRect prevRect = prevBuffer->BackingStore()->FrameRect();
      if (!frameIndex || prevRect.Contains(IntRect(IntPoint(), Size())))
      {
        // Clearing the first frame, or a frame the size of the whole
        // image, results in a completely empty image.
        buffer.BackingStore()->Clear();
        buffer.SetHasAlpha(true);
      }
      else
      {
        // Copy the whole previous buffer, then clear just its frame.
        if (!prevBuffer->BackingStore() || !buffer.Initialize(*prevBuffer->BackingStore()))
        {
          longjmp(png_jmpbuf(png), 1);
        }
        buffer.BackingStore()->ClearRect(prevRect);
        buffer.SetHasAlpha(true);
      }
    }

    IntRect frameRect(static_cast<int32>(_xOffset), static_cast<int32>(_yOffset), static_cast<int32>(_width),
                      static_cast<int32>(_height));

    // Make sure the frameRect doesn't extend outside the buffer.
    if (frameRect.MaxX() > Size().Width())
    {
      frameRect.SetWidth(Size().Width() - static_cast<int32>(_xOffset));
    }
    if (frameRect.MaxY() > Size().Height())
    {
      frameRect.SetHeight(Size().Height() - static_cast<int32>(_yOffset));
    }

    buffer.BackingStore()->SetFrameRect(frameRect);
  }

  void PNGImageDecoder::FrameComplete()
  {
    if (_frameIsHidden || _currentFrame >= FrameCount())
    {
      return;
    }

    auto &buffer = _frameBufferCache[_currentFrame];
    buffer.SetDecodingStatus(DecodingStatus::Complete);

    png_bytep interlaceBuffer = _reader->InterlaceBuffer();

    if (_currentFrame && interlaceBuffer)
    {
      IntRect rect = buffer.BackingStore()->FrameRect();
      bool hasAlpha = _reader->HasAlpha();
      bool nonTrivialAlpha = false;
      if (_blend && !hasAlpha)
      {
        _blend = 0;
      }

      png_bytep row = interlaceBuffer;
      unsigned colorChannels = hasAlpha ? 4 : 3;
      for (int32 y = rect.Y(); y < rect.MaxY();
           ++y, row += static_cast<size_t>(colorChannels) * Size().Width())
      {
        png_bytep pixel = row;
        auto *address = buffer.BackingStore()->PixelsStartingAt(rect.X(), y);
        for (int32 x = rect.X(); x < rect.MaxX(); ++x, pixel += colorChannels)
        {
          unsigned alpha = hasAlpha ? pixel[3] : 255;
          nonTrivialAlpha |= alpha < 255;
          if (!_blend)
          {
            buffer.BackingStore()->SetPixel(*address, pixel[0], pixel[1], pixel[2],
                                            static_cast<uint8>(alpha));
          }
          else
          {
            buffer.BackingStore()->BlendPixel(*address, pixel[0], pixel[1], pixel[2],
                                              static_cast<uint8>(alpha));
          }
          ++address;
        }
      }

      if (!nonTrivialAlpha)
      {
        IntRect frameRect = buffer.BackingStore()->FrameRect();
        if (frameRect.Contains(IntRect(IntPoint(), Size())))
        {
          buffer.SetHasAlpha(false);
        }
        else
        {
          size_t frameIndex = _currentFrame;
          const auto *prevBuffer = &_frameBufferCache[--frameIndex];
          while (frameIndex
                 && (prevBuffer->GetDisposalMethod()
                     == ScalableImageDecoderFrame::DisposalMethod::RestoreToPrevious))
          {
            prevBuffer = &_frameBufferCache[--frameIndex];
          }

          IntRect prevRect = prevBuffer->BackingStore()->FrameRect();
          if ((prevBuffer->GetDisposalMethod()
               == ScalableImageDecoderFrame::DisposalMethod::RestoreToBackground)
              && !prevBuffer->HasAlpha() && frameRect.Contains(prevRect))
          {
            buffer.SetHasAlpha(false);
          }
        }
      }
      else if (!_blend && !buffer.HasAlpha())
      {
        buffer.SetHasAlpha(nonTrivialAlpha);
      }
    }
    _currentFrame++;
  }

  int PNGImageDecoder::ProcessingStart(png_unknown_chunkp chunk)
  {
    static png_byte dataPNG[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    static png_byte datagAMA[16] = {0, 0, 0, 4, 103, 65, 77, 65};

    if (!_hasInfo)
    {
      return 0;
    }

    _totalFrames++;

    _png = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, DecodingFailed, nullptr);
    _info = png_create_info_struct(_png);
    if (setjmp(png_jmpbuf(_png)))
    {
      return 1;
    }

    png_set_crc_action(_png, PNG_CRC_QUIET_USE, PNG_CRC_QUIET_USE);
    png_set_progressive_read_fn(_png, static_cast<png_voidp>(this), FrameHeaderCallback, RowAvailableCallback,
                                nullptr);

    std::memcpy(_dataIHDR.data() + 8, chunk->data + 4, 8);
    png_save_uint_32(datagAMA + 8, static_cast<png_uint_32>(_gamma));

    png_process_data(_png, _info, dataPNG, 8);
    png_process_data(_png, _info, _dataIHDR.data(), 25);
    png_process_data(_png, _info, datagAMA, 16);
    if (_sizePLTE > 0)
    {
      png_process_data(_png, _info, _dataPLTE.data(), _sizePLTE);
    }
    if (_sizetRNS > 0)
    {
      png_process_data(_png, _info, _datatRNS.data(), _sizetRNS);
    }

    return 0;
  }

  int PNGImageDecoder::ProcessingFinish()
  {
    static png_byte dataIEND[12] = {0, 0, 0, 0, 73, 69, 78, 68, 174, 66, 96, 130};

    if (!_hasInfo)
    {
      return 0;
    }

    if (_totalFrames)
    {
      if (setjmp(png_jmpbuf(_png)))
      {
        return 1;
      }

      png_process_data(_png, _info, dataIEND, 12);
      png_destroy_read_struct(&_png, &_info, nullptr);
    }

    FrameComplete();
    return 0;
  }

  void PNGImageDecoder::FallbackNotAnimated() noexcept
  {
    _isAnimated = false;
    _playCount = 0;
    _currentFrame = 0;
  }
}
