#include "Krystal.Booey/Platform/ImageDecoders/JPEGImageDecoder.hpp"
#include <algorithm>
#include <csetjmp>
#include <cstdio>
#include <cstdlib>
#include <cstring>

extern "C"
{
#include <jpeglib.h>
}

// libjpeg reports errors with longjmp to the setjmp in the reader, which owns no objects with destructors.
// clang-format off
#pragma warning(disable : 4611) // interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4324) // structure was padded due to alignment specifier (jmp_buf)
// clang-format on

namespace krys::boo
{
  namespace
  {
    // libjpeg-turbo writes BGRA, which is ARGB on little-endian machines.
    KRYS_NODISCARD J_COLOR_SPACE RGBOutputColorSpace() noexcept
    {
      return JCS_EXT_BGRA;
    }

    KRYS_NODISCARD bool TurboSwizzled(J_COLOR_SPACE colorSpace) noexcept
    {
      return colorSpace == JCS_EXT_RGBA || colorSpace == JCS_EXT_BGRA;
    }

    KRYS_NODISCARD J_DCT_METHOD DctMethod() noexcept
    {
      return JDCT_ISLOW;
    }

    KRYS_NODISCARD J_DITHER_MODE DitherMode() noexcept
    {
      return JDITHER_FS;
    }

    KRYS_NODISCARD bool DoFancyUpsampling() noexcept
    {
      return true;
    }

    constexpr int ExifMarker = JPEG_APP0 + 1;

    struct DecoderErrorManager
    {
      struct jpeg_error_mgr Pub; // "public" fields for IJG library
      jmp_buf SetjmpBuffer; // For handling catastropic errors
    };

    enum class JState : uint8
    {
      Header, // Reading JFIF headers
      StartDecompress,
      DecompressProgressive, // Output progressive pixels
      DecompressSequential,  // Output sequential pixels
      Done,
      Error
    };

    void InitSource(j_decompress_ptr jd);
    boolean FillInputBuffer(j_decompress_ptr jd);
    void SkipInputData(j_decompress_ptr jd, long numBytes);
    void TermSource(j_decompress_ptr jd);
    void ErrorExit(j_common_ptr cinfo);

    KRYS_NODISCARD unsigned ReadUint16(const JOCTET *data, bool isBigEndian) noexcept
    {
      if (isBigEndian)
      {
        return (GETJOCTET(data[0]) << 8) | GETJOCTET(data[1]);
      }
      return (GETJOCTET(data[1]) << 8) | GETJOCTET(data[0]);
    }

    KRYS_NODISCARD unsigned ReadUint32(const JOCTET *data, bool isBigEndian) noexcept
    {
      if (isBigEndian)
      {
        return (GETJOCTET(data[0]) << 24) | (GETJOCTET(data[1]) << 16) | (GETJOCTET(data[2]) << 8)
               | GETJOCTET(data[3]);
      }
      return (GETJOCTET(data[3]) << 24) | (GETJOCTET(data[2]) << 16) | (GETJOCTET(data[1]) << 8)
             | GETJOCTET(data[0]);
    }

    bool CheckExifHeader(jpeg_saved_marker_ptr marker, bool &isBigEndian, unsigned &ifdOffset) noexcept
    {
      // For exif data, the APP1 block is followed by 'E', 'x', 'i', 'f', '\0',
      // then a fill byte, and then a tiff file that contains the metadata.
      // A tiff file starts with 'I', 'I' (intel / little endian byte order) or
      // 'M', 'M' (motorola / big endian byte order), followed by (uint16_t)42,
      // followed by an uint32_t with the offset to the tag block, relative to the
      // tiff file start.
      constexpr unsigned exifHeaderSize = 14;
      const JOCTET *data = marker->data;
      if (!(marker->marker == ExifMarker && marker->data_length >= exifHeaderSize && data[0] == 'E'
            && data[1] == 'x' && data[2] == 'i' && data[3] == 'f'
            && data[4] == '\0'
            // data[5] is a fill byte
            && ((data[6] == 'I' && data[7] == 'I') || (data[6] == 'M' && data[7] == 'M'))))
      {
        return false;
      }

      isBigEndian = data[6] == 'M';
      if (ReadUint16(data + 8, isBigEndian) != 42)
      {
        return false;
      }

      ifdOffset = ReadUint32(data + 10, isBigEndian);
      return true;
    }

    KRYS_NODISCARD ImageOrientation ReadImageOrientation(jpeg_decompress_struct *info) noexcept
    {
      // The JPEG decoder looks at EXIF metadata.
      // FIXME: Possibly implement XMP and IPTC support.
      constexpr unsigned orientationTag = 0x112;
      constexpr unsigned shortType = 3;
      for (jpeg_saved_marker_ptr marker = info->marker_list; marker; marker = marker->next)
      {
        bool isBigEndian;
        unsigned ifdOffset;
        if (!CheckExifHeader(marker, isBigEndian, ifdOffset))
        {
          continue;
        }
        constexpr unsigned offsetToTiffData = 6; // Account for 'Exif\0<fill byte>' header.
        if (marker->data_length < offsetToTiffData || ifdOffset >= marker->data_length - offsetToTiffData)
        {
          continue;
        }
        ifdOffset += offsetToTiffData;

        // The jpeg exif container format contains a tiff block for metadata.
        // A tiff image file directory (ifd) consists of a uint16_t describing
        // the number of ifd entries, followed by that many entries.
        // When touching this code, it's useful to look at the tiff spec:
        // http://partners.adobe.com/public/developer/en/tiff/TIFF6.pdf
        const JOCTET *data = marker->data + ifdOffset;
        size_t remaining = marker->data_length - ifdOffset;
        if (remaining < 2)
        {
          continue;
        }
        unsigned tagCount = ReadUint16(data, isBigEndian);
        data += 2; // Skip over the uint16 that was just read.
        remaining -= 2;

        // Every ifd entry is 2 bytes of tag, 2 bytes of contents datatype,
        // 4 bytes of number-of-elements, and 4 bytes of either offset to the
        // tag data, or if the data is small enough, the inlined data itself.
        constexpr size_t ifdEntrySize = 12;
        for (unsigned i = 0; i < tagCount && remaining >= ifdEntrySize;
             ++i, data += ifdEntrySize, remaining -= ifdEntrySize)
        {
          unsigned tag = ReadUint16(data, isBigEndian);
          unsigned type = ReadUint16(data + 2, isBigEndian);
          unsigned count = ReadUint32(data + 4, isBigEndian);
          if (tag == orientationTag && type == shortType && count == 1)
          {
            return ImageOrientation::FromEXIFValue(static_cast<int>(ReadUint16(data + 8, isBigEndian)));
          }
        }
      }

      return ImageOrientation::Orientation::None;
    }
  }

  // Implementation of a JPEG src object that understands our state machine
  struct DecoderSourceManager
  {
    // public fields; must be first in this struct!
    struct jpeg_source_mgr Pub;

    JPEGImageReader *Decoder;
  };

  class JPEGImageReader
  {
    KRYS_NON_COPY_MOVABLE(JPEGImageReader);

  private:
    JPEGImageDecoder *_decoder;
    unsigned _bufferLength {0};
    long _bytesToSkip {0};
    bool _decodingSizeOnly {false};

    jpeg_decompress_struct _info {};
    DecoderErrorManager _err {};
    JState _state {JState::Header};

    JSAMPARRAY _samples {nullptr};

  public:
    explicit JPEGImageReader(JPEGImageDecoder *decoder) noexcept : _decoder(decoder)
    {
      // We set up the normal JPEG error routines, then override error_exit.
      _info.err = jpeg_std_error(&_err.Pub);
      _err.Pub.error_exit = ErrorExit;

      // Allocate and initialize JPEG decompression object.
      jpeg_create_decompress(&_info);

      auto *src = static_cast<DecoderSourceManager *>(std::calloc(1, sizeof(DecoderSourceManager)));
      if (!src)
      {
        _state = JState::Error;
        return;
      }

      _info.src = reinterpret_cast<jpeg_source_mgr *>(src);

      // Set up callback functions.
      src->Pub.init_source = InitSource;
      src->Pub.fill_input_buffer = FillInputBuffer;
      src->Pub.skip_input_data = SkipInputData;
      src->Pub.resync_to_restart = jpeg_resync_to_restart;
      src->Pub.term_source = TermSource;
      src->Decoder = this;

      // Keep APP1 blocks, for obtaining exif data.
      jpeg_save_markers(&_info, ExifMarker, 0xFFFF);
    }

    ~JPEGImageReader() noexcept
    {
      Close();
    }

    void Close() noexcept
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(_info.src);
      std::free(src);
      _info.src = nullptr;

      jpeg_destroy_decompress(&_info);
    }

    void SkipBytes(long numBytes) noexcept
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(_info.src);
      long bytesToSkip = std::min(numBytes, static_cast<long>(src->Pub.bytes_in_buffer));
      src->Pub.bytes_in_buffer -= static_cast<size_t>(bytesToSkip);
      src->Pub.next_input_byte += bytesToSkip;

      _bytesToSkip = std::max(numBytes - bytesToSkip, 0l);
    }

    // NOTE: not noexcept: libjpeg's errors longjmp back to the setjmp here.
    bool Decode(const SharedBuffer &data, bool onlySize)
    {
      _decodingSizeOnly = onlySize;

      unsigned newByteCount = static_cast<unsigned>(data.Size()) - _bufferLength;
      unsigned readOffset = _bufferLength - static_cast<unsigned>(_info.src->bytes_in_buffer);

      _info.src->bytes_in_buffer += newByteCount;
      _info.src->next_input_byte = data.Data() + readOffset;

      // If we still have bytes to skip, try to skip those now.
      if (_bytesToSkip)
      {
        SkipBytes(_bytesToSkip);
      }

      _bufferLength = static_cast<unsigned>(data.Size());

      // We need to do the setjmp here. Otherwise bad things will happen
      if (setjmp(_err.SetjmpBuffer))
      {
        return _decoder->SetFailed();
      }

      switch (_state)
      {
        case JState::Header:
          // Read file parameters with jpeg_read_header().
          if (jpeg_read_header(&_info, TRUE) == JPEG_SUSPENDED)
          {
            return false; // I/O suspension.
          }

          switch (_info.jpeg_color_space)
          {
            case JCS_GRAYSCALE:
            case JCS_RGB:
            case JCS_YCbCr:
              // libjpeg can convert GRAYSCALE and YCbCr image pixels to RGB.
              _info.out_color_space = RGBOutputColorSpace();
              break;
            case JCS_CMYK:
            case JCS_YCCK:
              // libjpeg can convert YCCK to CMYK, but neither to RGB, so we
              // manually convert CMKY to RGB.
              _info.out_color_space = JCS_CMYK;
              break;
            default: return _decoder->SetFailed();
          }

          _state = JState::StartDecompress;

          // We can fill in the size now that the header is available.
          if (!_decoder->SetSize(
                IntSize(static_cast<int32>(_info.image_width), static_cast<int32>(_info.image_height))))
          {
            return false;
          }

          _decoder->SetOrientation(ReadImageOrientation(Info()));

          // Don't allocate a giant and superfluous memory buffer when the
          // image is a sequential JPEG.
          _info.buffered_image = jpeg_has_multiple_scans(&_info);

          // Used to set up image size so arrays can be allocated.
          jpeg_calc_output_dimensions(&_info);

          // Make a one-row-high sample array that will go away when done with
          // image. Always make it big enough to hold an RGB row. Since this
          // uses the IJG memory manager, it must be allocated before the call
          // to jpeg_start_compress().
          // FIXME: note that some output color spaces do not need the samples
          // buffer. Remove this allocation for those color spaces.
          _samples = (*_info.mem->alloc_sarray)(reinterpret_cast<j_common_ptr>(&_info), JPOOL_IMAGE,
                                                _info.output_width * 4, 1);

          if (_decodingSizeOnly)
          {
            // We can stop here. Reduce our buffer length and available data.
            _bufferLength -= static_cast<unsigned>(_info.src->bytes_in_buffer);
            _info.src->bytes_in_buffer = 0;
            return true;
          }
          [[fallthrough]];

        case JState::StartDecompress:
          // Set parameters for decompression.
          // FIXME -- Should reset dct_method and dither mode for final pass
          // of progressive JPEG.
          _info.dct_method = DctMethod();
          _info.dither_mode = DitherMode();
          _info.do_fancy_upsampling = DoFancyUpsampling() ? TRUE : FALSE;
          _info.enable_2pass_quant = FALSE;
          _info.do_block_smoothing = TRUE;

          // Start decompressor.
          if (!jpeg_start_decompress(&_info))
          {
            return false; // I/O suspension.
          }

          // If this is a progressive JPEG ...
          _state = (_info.buffered_image) ? JState::DecompressProgressive : JState::DecompressSequential;
          [[fallthrough]];

        case JState::DecompressSequential:
          if (_state == JState::DecompressSequential)
          {
            if (!_decoder->OutputScanlines())
            {
              return false; // I/O suspension.
            }

            // If we've completed image output...
            krys_debug_assert(_info.output_scanline == _info.output_height);
            _state = JState::Done;
          }
          [[fallthrough]];

        case JState::DecompressProgressive:
          if (_state == JState::DecompressProgressive)
          {
            int status;
            do
            {
              status = jpeg_consume_input(&_info);
            } while ((status != JPEG_SUSPENDED) && (status != JPEG_REACHED_EOI));

            for (;;)
            {
              if (!_info.output_scanline)
              {
                int scan = _info.input_scan_number;

                // If we haven't displayed anything yet
                // (output_scan_number == 0) and we have enough data for
                // a complete scan, force output of the last full scan.
                if (!_info.output_scan_number && (scan > 1) && (status != JPEG_REACHED_EOI))
                {
                  --scan;
                }

                if (!jpeg_start_output(&_info, scan))
                {
                  return false; // I/O suspension.
                }
              }

              if (_info.output_scanline == 0xffffff)
              {
                _info.output_scanline = 0;
              }

              // If outputScanlines() fails, it deletes |this|. Therefore,
              // copy the decoder pointer and use it to check for failure
              // to avoid member access in the failure case.
              JPEGImageDecoder *decoder = _decoder;
              if (!decoder->OutputScanlines())
              {
                if (decoder->Failed()) // Careful; |this| is deleted.
                {
                  return false;
                }
                if (!_info.output_scanline)
                {
                  // Didn't manage to read any lines - flag so we
                  // don't call jpeg_start_output() multiple times for
                  // the same scan.
                  _info.output_scanline = 0xffffff;
                }
                return false; // I/O suspension.
              }

              if (_info.output_scanline == _info.output_height)
              {
                if (!jpeg_finish_output(&_info))
                {
                  return false; // I/O suspension.
                }

                if (jpeg_input_complete(&_info) && (_info.input_scan_number == _info.output_scan_number))
                {
                  break;
                }

                _info.output_scanline = 0;
              }
            }

            _state = JState::Done;
          }
          [[fallthrough]];

        case JState::Done:
          // Finish decompression.
          return jpeg_finish_decompress(&_info);

        case JState::Error:
          // We can get here if the constructor failed.
          return _decoder->SetFailed();
      }

      return true;
    }

    KRYS_NODISCARD jpeg_decompress_struct *Info() noexcept
    {
      return &_info;
    }
    KRYS_NODISCARD JSAMPARRAY Samples() const noexcept
    {
      return _samples;
    }
    KRYS_NODISCARD JPEGImageDecoder *Decoder() noexcept
    {
      return _decoder;
    }
  };

  namespace
  {
    // Override the standard error method in the IJG JPEG decoder code.
    void ErrorExit(j_common_ptr cinfo)
    {
      // Return control to the setjmp point.
      auto *err = reinterpret_cast<DecoderErrorManager *>(cinfo->err);
      longjmp(err->SetjmpBuffer, -1);
    }

    void InitSource(j_decompress_ptr)
    {
    }

    void SkipInputData(j_decompress_ptr jd, long numBytes)
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(jd->src);
      src->Decoder->SkipBytes(numBytes);
    }

    boolean FillInputBuffer(j_decompress_ptr)
    {
      // Our decode step always sets things up properly, so if this method is ever
      // called, then we have hit the end of the buffer.  A return value of false
      // indicates that we have no data to supply yet.
      return FALSE;
    }

    void TermSource(j_decompress_ptr jd)
    {
      auto *src = reinterpret_cast<DecoderSourceManager *>(jd->src);
      src->Decoder->Decoder()->JpegComplete();
    }

    template <int TColorSpace>
    void SetPixel(ScalableImageDecoderFrame &buffer, uint32 &currentAddress, JSAMPARRAY samples,
                  int column) noexcept
    {
      JSAMPLE *jsample = *samples + column * (TColorSpace == JCS_RGB ? 3 : 4);

      switch (TColorSpace)
      {
        case JCS_RGB:
          buffer.BackingStore()->SetPixel(currentAddress, jsample[0], jsample[1], jsample[2], 0xFF);
          break;
        case JCS_CMYK:
        {
          // Source is 'Inverted CMYK', output is RGB.
          // See: http://www.easyrgb.com/math.php?MATH=M12#text12
          // Or: http://www.ilkeratalay.com/colorspacesfaq.php#rgb
          // From CMYK to CMY:
          // X =   X    * (1 -   K   ) +   K  [for X = C, M, or Y]
          // Thus, from Inverted CMYK to CMY is:
          // X = (1-iX) * (1 - (1-iK)) + (1-iK) => 1 - iX*iK
          // From CMY (0..1) to RGB (0..1):
          // R = 1 - C => 1 - (1 - iC*iK) => iC*iK  [G and B similar]
          unsigned k = jsample[3];
          buffer.BackingStore()->SetPixel(currentAddress, static_cast<uint8>(jsample[0] * k / 255),
                                          static_cast<uint8>(jsample[1] * k / 255),
                                          static_cast<uint8>(jsample[2] * k / 255), 0xFF);
          break;
        }
        default: break;
      }
    }
  }

  JPEGImageDecoder::JPEGImageDecoder(AlphaOption alphaOption,
                                     GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ScalableImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  JPEGImageDecoder::~JPEGImageDecoder() noexcept
  {
    Clear();
  }

  ScalableImageDecoderFrame *JPEGImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index)
    {
      return nullptr;
    }

    if (_frameBufferCache.empty())
    {
      _frameBufferCache.resize(1);
    }

    auto &frame = _frameBufferCache[0];
    if (!frame.IsComplete())
    {
      Decode(false, IsAllDataReceived());
    }
    return &_frameBufferCache[0];
  }

  void JPEGImageDecoder::Clear() noexcept
  {
    _reader = nullptr;
  }

  bool JPEGImageDecoder::SetFailed() noexcept
  {
    Clear();
    return ScalableImageDecoder::SetFailed();
  }

  template <int TColorSpace>
  bool JPEGImageDecoder::OutputScanlines(ScalableImageDecoderFrame &buffer)
  {
    JSAMPARRAY samples = _reader->Samples();
    jpeg_decompress_struct *info = _reader->Info();
    int width = static_cast<int>(info->output_width);

    while (info->output_scanline < info->output_height)
    {
      // jpeg_read_scanlines will increase the scanline counter, so we
      // save the scanline before calling it.
      int sourceY = static_cast<int>(info->output_scanline);
      /* Request one scanline.  Returns 0 or 1 scanlines. */
      if (jpeg_read_scanlines(info, samples, 1) != 1)
      {
        return false;
      }

      auto *currentAddress = buffer.BackingStore()->PixelsStartingAt(0, sourceY);
      for (int x = 0; x < width; ++x, ++currentAddress)
      {
        SetPixel<TColorSpace>(buffer, *currentAddress, samples, x);
      }
    }
    return true;
  }

  bool JPEGImageDecoder::OutputScanlines()
  {
    if (_frameBufferCache.empty())
    {
      return false;
    }

    // Initialize the framebuffer if needed.
    auto &buffer = _frameBufferCache[0];
    if (buffer.IsInvalid())
    {
      if (!buffer.Initialize(Size(), _premultiplyAlpha))
      {
        return SetFailed();
      }
      buffer.SetDecodingStatus(DecodingStatus::Partial);
      // The buffer is transparent outside the decoded area while the image is
      // loading. The completed image will be marked fully opaque in jpegComplete().
      buffer.SetHasAlpha(true);
    }

    jpeg_decompress_struct *info = _reader->Info();

    if (TurboSwizzled(info->out_color_space))
    {
      while (info->output_scanline < info->output_height)
      {
        auto *row = reinterpret_cast<unsigned char *>(
          buffer.BackingStore()->PixelsStartingAt(0, static_cast<int32>(info->output_scanline)));
        if (jpeg_read_scanlines(info, &row, 1) != 1)
        {
          return false;
        }
      }
      return true;
    }

    switch (info->out_color_space)
    {
      // The code inside outputScanlines<int> will be executed
      // for each pixel, so we want to avoid any extra comparisons there.
      // That is why we use template and template specializations here so
      // the proper code will be generated at compile time.
      case JCS_RGB:  return OutputScanlines<JCS_RGB>(buffer);
      case JCS_CMYK: return OutputScanlines<JCS_CMYK>(buffer);
      default:       krys_debug_assert(false); break;
    }

    return SetFailed();
  }

  void JPEGImageDecoder::JpegComplete()
  {
    if (_frameBufferCache.empty())
    {
      return;
    }

    // Hand back an appropriately sized buffer, even if the image ended up being
    // empty.
    auto &buffer = _frameBufferCache[0];
    buffer.SetHasAlpha(false);
    buffer.SetDecodingStatus(DecodingStatus::Complete);
  }

  void JPEGImageDecoder::Decode(bool onlySize, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }

    if (!_reader)
    {
      Clear();
      _reader = CreateUnique<JPEGImageReader>(this);
    }

    // If we couldn't decode the image but we've received all the data, decoding
    // has failed.
    if (!_reader->Decode(*_data, onlySize) && allDataReceived)
    {
      (void)SetFailed();
    }
    // If we're done decoding the image, we don't need the JPEGImageReader
    // anymore.  (If we failed, |m_reader| has already been cleared.)
    else if (!_frameBufferCache.empty() && (_frameBufferCache[0].IsComplete()))
    {
      Clear();
    }
  }
}
