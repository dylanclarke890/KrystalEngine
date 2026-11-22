#include "Krystal.Image/Decoders/PNGImageDecoder.hpp"
#include "Krystal.Core/Containers/Array.hpp"
#include <algorithm>
#include <cstring>
#include <png.h>
#include <tuple>

// libpng reports errors with longjmp, back to the setjmp in FrameReader::Feed, which owns no objects with
// destructors.
// clang-format off
#pragma warning(disable : 4611) // interaction between '_setjmp' and C++ object destruction is non-portable
// clang-format on

namespace krys::img
{
  namespace
  {
    constexpr uint8 Signature[] = {137, 80, 78, 71, 13, 10, 26, 10};
    constexpr uint32 MaxChunkLength = 0x7F'FF'FF'FF;
    // Enough frames for over an hour at 60 frames a second, while keeping the frame cache's size in check.
    constexpr uint32 MaxFrameCount = (1u << 18) - 1;
    // The display's gamma, and the largest file gamma libpng accepts.
    constexpr double DisplayGamma = 2.2;
    constexpr double MaxGamma = 21'474.83;

    KRYS_NODISCARD uint32 ReadUInt32(const uint8 *bytes) noexcept
    {
      return uint32 {bytes[0]} << 24 | uint32 {bytes[1]} << 16 | uint32 {bytes[2]} << 8 | bytes[3];
    }

    KRYS_NODISCARD uint16 ReadUInt16(const uint8 *bytes) noexcept
    {
      return static_cast<uint16>(bytes[0] << 8 | bytes[1]);
    }

    void WriteUInt32(uint8 *bytes, uint32 value) noexcept
    {
      bytes[0] = static_cast<uint8>(value >> 24);
      bytes[1] = static_cast<uint8>(value >> 16);
      bytes[2] = static_cast<uint8>(value >> 8);
      bytes[3] = static_cast<uint8>(value);
    }

    KRYS_NODISCARD bool IsChunk(const uint8 *type, const char *name) noexcept
    {
      return !std::memcmp(type, name, 4);
    }

    // PNG's CRC-32, over a chunk's type and data.
    constexpr auto CRCTable = []
    {
      Array<uint32, 256> table {};
      for (uint32 n = 0; n < 256; ++n)
      {
        auto c = n;
        for (int32 bit = 0; bit < 8; ++bit)
        {
          c = c & 1 ? 0xED'B8'83'20u ^ (c >> 1) : c >> 1;
        }
        table[n] = c;
      }
      return table;
    }();

    constexpr uint32 CRCStart = 0xFF'FF'FF'FF;

    KRYS_NODISCARD constexpr uint32 UpdateCRC(uint32 crc, Span<const uint8> bytes) noexcept
    {
      for (auto byte : bytes)
      {
        crc = CRCTable[(crc ^ byte) & 0xFF] ^ (crc >> 8);
      }
      return crc;
    }

    KRYS_NODISCARD constexpr uint32 FinishCRC(uint32 crc) noexcept
    {
      return crc ^ CRCStart;
    }

    // Whether the IHDR's bit depth, colour type and methods are a combination PNG allows.
    KRYS_NODISCARD bool IsValidHeader(Span<const uint8> header) noexcept
    {
      auto width = ReadUInt32(header.data());
      auto height = ReadUInt32(header.data() + 4);
      auto bitDepth = header[8];
      auto colorType = header[9];
      if (!width || !height || width > MaxChunkLength || height > MaxChunkLength || header[10] || header[11]
          || header[12] > 1)
      {
        return false;
      }
      switch (colorType)
      {
        case 0:  return bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8 || bitDepth == 16;
        case 3:  return bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8;
        case 2:
        case 4:
        case 6:  return bitDepth == 8 || bitDepth == 16;
        default: return false;
      }
    }

    KRYS_NORETURN void PNGCBAPI ExitOnError(png_structp png, png_const_charp)
    {
      png_longjmp(png, 1);
    }

    void PNGCBAPI IgnoreWarning(png_structp, png_const_charp)
    {
    }
  }

  struct PNGImageDecoder::FrameInfo
  {
    // Where a chunk's image data is in the file, past an fdAT's sequence number.
    struct DataChunk
    {
      size_t Offset;
      uint32 Size;
    };

    RectI Rect;
    milliseconds Duration {};
    ImageDecoderFrame::DisposalMethod Disposal {ImageDecoderFrame::DisposalMethod::Unspecified};
    bool BlendOver {false};
    // Whether the frame's data is in IDAT chunks (the default image) rather than fdAT ones.
    bool IsDefaultImage {false};
    // The frame's data chunks so far, and whether there are any more. The last may not have fully arrived.
    List<DataChunk> Chunks;
    bool IsDataComplete {false};
  };

  /// @brief Reads the structure of a PNG as its data arrives: a resumable walk over its chunks.
  class PNGImageDecoder::Parser
  {
  public:
    enum class Result : uint8
    {
      NeedsMoreData,
      Complete,
      Failed,
    };

    // Where a whole chunk is in the file.
    struct ChunkRange
    {
      size_t Offset;
      size_t Size;
    };

  private:
    enum class Stage : uint8
    {
      Signature,
      Chunk,
      Done,
    };

    // The image whose data chunks are arriving.
    enum class Receiving : uint8
    {
      Nothing,
      HiddenImage,
      LastFrame,
    };

    Stage _stage {Stage::Signature};
    size_t _position {0};
    Array<uint8, 13> _header {};
    bool _hasHeader {false};
    // The chunks between IHDR and the image data that libpng reads (palette, transparency, colour space),
    // which each frame's reader is given.
    List<ChunkRange> _headerChunks;
    bool _seenImageData {false};
    bool _isAnimated {false};
    uint32 _frameLimit {0};
    uint32 _plays {0};
    uint32 _nextSequenceNumber {0};
    // The frame control chunk before the image data, which makes the default image the first frame.
    Maybe<FrameInfo> _defaultImageControl;
    // The default image of an animation it isn't part of: shown only if the animation turns out broken.
    Maybe<FrameInfo> _hiddenImage;
    Receiving _receiving {Receiving::Nothing};
    List<FrameInfo> _frames;

  public:
    KRYS_NODISCARD bool HasHeader() const noexcept
    {
      return _hasHeader;
    }

    KRYS_NODISCARD SizeI Size() const noexcept
    {
      return {static_cast<int32>(ReadUInt32(_header.data())),
              static_cast<int32>(ReadUInt32(_header.data() + 4))};
    }

    /// @brief The IHDR chunk's data.
    KRYS_NODISCARD Span<const uint8> Header() const noexcept
    {
      return _header;
    }

    KRYS_NODISCARD const List<ChunkRange> &HeaderChunks() const noexcept
    {
      return _headerChunks;
    }

    KRYS_NODISCARD bool IsAnimated() const noexcept
    {
      return _isAnimated;
    }

    /// @brief How many times an animation plays, 0 for forever.
    KRYS_NODISCARD uint32 Plays() const noexcept
    {
      return _plays;
    }

    /// @brief The frames whose frame control chunk (or, in a still image, first data chunk) has arrived.
    KRYS_NODISCARD const List<FrameInfo> &Frames() const noexcept
    {
      return _frames;
    }

    Result Parse(Span<const uint8> data) noexcept
    {
      while (true)
      {
        // Image data is skipped before it arrives, so the position can be past the end of the data.
        if (_stage != Stage::Done && _position > data.size())
        {
          return Result::NeedsMoreData;
        }
        auto available = data.size() - _position;
        const auto *bytes = data.data() + _position;
        switch (_stage)
        {
          case Stage::Signature:
            if (available < sizeof(Signature))
            {
              return Result::NeedsMoreData;
            }
            if (std::memcmp(bytes, Signature, sizeof(Signature)))
            {
              return Result::Failed;
            }
            _position += sizeof(Signature);
            _stage = Stage::Chunk;
            break;
          case Stage::Chunk:
          {
            if (available < 8)
            {
              return Result::NeedsMoreData;
            }
            auto length = ReadUInt32(bytes);
            const auto *type = bytes + 4;
            if (length > MaxChunkLength || (!_hasHeader && !IsChunk(type, "IHDR")))
            {
              return Result::Failed;
            }
            auto size = size_t {length} + 12;

            // Any other chunk ends the image data that was arriving.
            if (auto *frame = ReceivingFrame();
                frame && !IsChunk(type, frame->IsDefaultImage ? "IDAT" : "fdAT"))
            {
              frame->IsDataComplete = true;
              _receiving = Receiving::Nothing;
              if (frame->Chunks.empty() && !StopAnimation())
              {
                return Result::Failed;
              }
              if (_stage == Stage::Done)
              {
                break;
              }
            }

            // Image data is read by the frame readers as it arrives, so only its chunk header is needed.
            if (IsChunk(type, "IDAT"))
            {
              ReadImageData(_position + 8, length);
              _position += size;
              break;
            }
            if (IsChunk(type, "fdAT"))
            {
              if (_isAnimated)
              {
                if (available < 12)
                {
                  return Result::NeedsMoreData;
                }
                if (!ReadFrameData(_position + 12, length, ReadUInt32(bytes + 8)))
                {
                  if (!StopAnimation())
                  {
                    return Result::Failed;
                  }
                  break;
                }
              }
              _position += size;
              break;
            }
            if (IsChunk(type, "IEND"))
            {
              _stage = Stage::Done;
              break;
            }

            // Chunks after the image data are skipped, except for frame control chunks.
            auto isFrameControl = IsChunk(type, "fcTL");
            if (_seenImageData && !isFrameControl)
            {
              _position += size;
              break;
            }
            if (available < size)
            {
              return Result::NeedsMoreData;
            }
            auto body = Span<const uint8>(bytes + 8, length);
            auto crcMatches = FinishCRC(UpdateCRC(CRCStart, Span<const uint8>(type, 4 + size_t {length})))
                              == ReadUInt32(bytes + 8 + length);
            if (IsChunk(type, "IHDR"))
            {
              if (_hasHeader || length != _header.size() || !crcMatches || !IsValidHeader(body))
              {
                return Result::Failed;
              }
              std::copy(body.begin(), body.end(), _header.begin());
              _hasHeader = true;
            }
            else if (IsChunk(type, "acTL"))
            {
              if (crcMatches)
              {
                ReadAnimationControl(body);
              }
            }
            else if (isFrameControl)
            {
              if (_isAnimated && !(crcMatches && ReadFrameControl(body)))
              {
                if (!StopAnimation())
                {
                  return Result::Failed;
                }
                if (_stage == Stage::Done)
                {
                  break;
                }
              }
            }
            else
            {
              _headerChunks.push_back({_position, size});
            }
            _position += size;
            break;
          }
          case Stage::Done: return Result::Complete;
        }
      }
    }

    /// @brief Ends the data: the image data that was arriving is all there is.
    Result EndOfData() noexcept
    {
      if (_stage != Stage::Done)
      {
        if (!_hasHeader)
        {
          return Result::Failed;
        }
        if (auto *frame = ReceivingFrame())
        {
          frame->IsDataComplete = true;
          _receiving = Receiving::Nothing;
        }
        if (_isAnimated && _seenImageData && !StopAnimation())
        {
          return Result::Failed;
        }
        _stage = Stage::Done;
      }
      return _frames.empty() ? Result::Failed : Result::Complete;
    }

  private:
    KRYS_NODISCARD FrameInfo *ReceivingFrame() noexcept
    {
      switch (_receiving)
      {
        case Receiving::HiddenImage: return &*_hiddenImage;
        case Receiving::LastFrame:   return &_frames.back();
        default:                     return nullptr;
      }
    }

    void ReadImageData(size_t offset, uint32 length) noexcept
    {
      if (!_seenImageData)
      {
        _seenImageData = true;
        FrameInfo image;
        image.Rect = {PointI {}, Size()};
        if (_isAnimated && _defaultImageControl)
        {
          image = *_defaultImageControl;
        }
        image.IsDefaultImage = true;
        if (_isAnimated && !_defaultImageControl)
        {
          _hiddenImage = krys::move(image);
          _receiving = Receiving::HiddenImage;
        }
        else
        {
          _frames.push_back(krys::move(image));
          _receiving = Receiving::LastFrame;
        }
      }
      // An IDAT after the image data has ended is ignored.
      if (auto *frame = ReceivingFrame(); frame && frame->IsDefaultImage)
      {
        frame->Chunks.push_back({offset, length});
      }
    }

    bool ReadFrameData(size_t offset, uint32 length, uint32 sequenceNumber) noexcept
    {
      auto *frame = ReceivingFrame();
      if (length < 4 || sequenceNumber != _nextSequenceNumber++ || !frame || frame->IsDefaultImage)
      {
        return false;
      }
      frame->Chunks.push_back({offset, length - 4});
      return true;
    }

    void ReadAnimationControl(Span<const uint8> body) noexcept
    {
      if (_seenImageData || _isAnimated || body.size() != 8)
      {
        return;
      }
      auto frames = ReadUInt32(body.data());
      auto plays = ReadUInt32(body.data() + 4);
      if (!frames || frames > MaxFrameCount || plays > MaxChunkLength)
      {
        return;
      }
      _isAnimated = true;
      _frameLimit = frames;
      _plays = plays;
    }

    bool ReadFrameControl(Span<const uint8> body) noexcept
    {
      if (body.size() != 26 || ReadUInt32(body.data()) != _nextSequenceNumber++)
      {
        return false;
      }
      auto width = ReadUInt32(body.data() + 4);
      auto height = ReadUInt32(body.data() + 8);
      auto x = ReadUInt32(body.data() + 12);
      auto y = ReadUInt32(body.data() + 16);
      auto delayNumerator = ReadUInt16(body.data() + 20);
      auto delayDenominator = ReadUInt16(body.data() + 22);
      auto disposal = body[24];
      auto blend = body[25];
      auto size = Size();
      if (!width || !height || uint64 {x} + width > static_cast<uint64>(size.Width())
          || uint64 {y} + height > static_cast<uint64>(size.Height()) || disposal > 2 || blend > 1)
      {
        return false;
      }

      FrameInfo frame;
      frame.Rect = {static_cast<int32>(x), static_cast<int32>(y), static_cast<int32>(width),
                    static_cast<int32>(height)};
      // The delay is a fraction of a second, whose denominator 0 means hundredths.
      frame.Duration = milliseconds(delayNumerator * 1000.0 / (delayDenominator ? delayDenominator : 100));
      constexpr ImageDecoderFrame::DisposalMethod disposals[] = {
        ImageDecoderFrame::DisposalMethod::DoNotDispose,
        ImageDecoderFrame::DisposalMethod::RestoreToBackground,
        ImageDecoderFrame::DisposalMethod::RestoreToPrevious,
      };
      frame.Disposal = disposals[disposal];
      frame.BlendOver = blend == 1;

      // Before the image data, the frame is the default image, which must fill the image.
      if (!_seenImageData)
      {
        if (_defaultImageControl || frame.Rect != RectI {PointI {}, size})
        {
          return false;
        }
        _defaultImageControl = krys::move(frame);
        return true;
      }
      if (_frames.size() >= _frameLimit)
      {
        return false;
      }
      _frames.push_back(krys::move(frame));
      _receiving = Receiving::LastFrame;
      return true;
    }

    /// @brief Gives up on a broken animation. Before the image data, the image is shown as a still one; after
    /// it, the frames whose data has all arrived are kept and the rest of the file ignored. False if that
    /// leaves nothing to show.
    bool StopAnimation() noexcept
    {
      if (!_seenImageData)
      {
        _isAnimated = false;
        _defaultImageControl = null;
        return true;
      }
      while (!_frames.empty() && (!_frames.back().IsDataComplete || _frames.back().Chunks.empty()))
      {
        _frames.pop_back();
      }
      _receiving = Receiving::Nothing;
      if (_frames.empty())
      {
        if (!_hiddenImage || !_hiddenImage->IsDataComplete)
        {
          return false;
        }
        _frames.push_back(krys::move(*_hiddenImage));
        _hiddenImage = null;
      }
      _isAnimated = _frames.size() > 1;
      _stage = Stage::Done;
      return true;
    }
  };

  /// @brief Decodes one frame with libpng as its data arrives. libpng is given a PNG of the frame alone: the
  /// image's header resized to the frame, the chunks describing the image, and the frame's data as IDAT
  /// chunks.
  /// @note libpng reports errors with longjmp to the setjmp in Feed, through the callbacks, so neither they
  /// nor Feed can be noexcept.
  class PNGImageDecoder::FrameReader
  {
    KRYS_NON_COPY_MOVABLE(FrameReader);

  public:
    enum class Result : uint8
    {
      NeedsMoreData,
      Complete,
      Failed,
    };

  private:
    PNGImageDecoder &_decoder;
    size_t _index;
    png_structp _png {nullptr};
    png_infop _info {nullptr};
    bool _prologueGiven {false};
    // The data chunk being given to libpng, and how much of it has been.
    size_t _chunk {0};
    bool _chunkStarted {false};
    size_t _chunkBytesGiven {0};
    uint32 _crc {CRCStart};
    bool _endGiven {false};
    bool _complete {false};
    bool _failed {false};
    uint32 _width {0};
    uint32 _height {0};
    uint32 _channels {0};
    bool _interlaced {false};
    // Whether the frame is drawn once it's complete, rather than as rows decode: an interlaced frame blended
    // over the one before can't be blended a pass at a time.
    bool _deferred {false};
    // The frame's pixels so far, which interlaced passes combine into.
    List<uint8> _pixels;

  public:
    FrameReader(PNGImageDecoder &decoder, size_t index) noexcept
        : _decoder(decoder), _index(index),
          _png(png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, ExitOnError, IgnoreWarning)),
          _info(_png ? png_create_info_struct(_png) : nullptr)
    {
      if (_png)
      {
        png_set_progressive_read_fn(_png, this, HeaderAvailable, RowAvailable, Ended);
      }
    }

    ~FrameReader() noexcept
    {
      if (_png)
      {
        png_destroy_read_struct(&_png, _info ? &_info : nullptr, nullptr);
      }
    }

    KRYS_NODISCARD size_t FrameIndex() const noexcept
    {
      return _index;
    }

    /// @brief Gives libpng what's arrived of the frame since the last call.
    Result Feed(Span<const uint8> data)
    {
      if (!_png || !_info || _failed)
      {
        return Result::Failed;
      }
      if (setjmp(png_jmpbuf(_png))) // NOLINT(modernize-avoid-setjmp-longjmp): libpng
      {
        return Result::Failed;
      }
      return Give(data);
    }

  private:
    Result Give(Span<const uint8> data)
    {
      const auto &parser = *_decoder._parser;
      const auto &frame = parser.Frames()[_index];
      if (!_prologueGiven)
      {
        Process(Signature);
        Array<uint8, 25> header {};
        WriteUInt32(header.data(), 13);
        std::memcpy(header.data() + 4, "IHDR", 4);
        std::copy(parser.Header().begin(), parser.Header().end(), header.begin() + 8);
        WriteUInt32(header.data() + 8, static_cast<uint32>(frame.Rect.Width()));
        WriteUInt32(header.data() + 12, static_cast<uint32>(frame.Rect.Height()));
        WriteUInt32(header.data() + 21,
                    FinishCRC(UpdateCRC(CRCStart, Span<const uint8>(header.data() + 4, 17))));
        Process(header);
        for (auto chunk : parser.HeaderChunks())
        {
          Process(data.subspan(chunk.Offset, chunk.Size));
        }
        _prologueGiven = true;
      }

      for (; _chunk < frame.Chunks.size(); ++_chunk)
      {
        auto chunk = frame.Chunks[_chunk];
        if (!_chunkStarted)
        {
          Array<uint8, 8> header {};
          WriteUInt32(header.data(), chunk.Size);
          std::memcpy(header.data() + 4, "IDAT", 4);
          _crc = UpdateCRC(CRCStart, Span<const uint8>(header.data() + 4, 4));
          Process(header);
          _chunkStarted = true;
          _chunkBytesGiven = 0;
        }
        auto end = std::min(data.size(), chunk.Offset + chunk.Size);
        auto start = chunk.Offset + _chunkBytesGiven;
        if (end > start)
        {
          auto bytes = data.subspan(start, end - start);
          _crc = UpdateCRC(_crc, bytes);
          Process(bytes);
          _chunkBytesGiven += bytes.size();
        }
        if (_failed)
        {
          return Result::Failed;
        }
        if (_chunkBytesGiven < chunk.Size)
        {
          // The data ended part way through the chunk.
          return frame.IsDataComplete ? Result::Failed : Result::NeedsMoreData;
        }
        Array<uint8, 4> crc {};
        WriteUInt32(crc.data(), FinishCRC(_crc));
        Process(crc);
        _chunkStarted = false;
      }
      if (!frame.IsDataComplete)
      {
        return Result::NeedsMoreData;
      }

      if (!_endGiven)
      {
        constexpr uint8 end[] = {0, 0, 0, 0, 'I', 'E', 'N', 'D', 0xAE, 0x42, 0x60, 0x82};
        Process(end);
        _endGiven = true;
      }
      if (_failed || !_complete)
      {
        return Result::Failed;
      }
      if (_deferred)
      {
        auto stride = size_t {_width} * _channels;
        for (uint32 y = 0; y < _height; ++y)
        {
          if (!_decoder.WriteRow(_index, _pixels.data() + y * stride, static_cast<int32>(y), _channels == 4))
          {
            return Result::Failed;
          }
        }
      }
      return Result::Complete;
    }

    void Process(Span<const uint8> bytes)
    {
      png_process_data(_png, _info, const_cast<png_bytep>(bytes.data()), bytes.size());
    }

    static void PNGCBAPI HeaderAvailable(png_structp png, png_infop)
    {
      static_cast<FrameReader *>(png_get_progressive_ptr(png))->ReadHeader();
    }

    static void PNGCBAPI RowAvailable(png_structp png, png_bytep row, png_uint_32 y, int)
    {
      static_cast<FrameReader *>(png_get_progressive_ptr(png))->ReadRow(row, y);
    }

    static void PNGCBAPI Ended(png_structp png, png_infop)
    {
      static_cast<FrameReader *>(png_get_progressive_ptr(png))->_complete = true;
    }

    // Sets libpng up to give rows of 8-bit RGB or RGBA, gamma corrected unless they're colour managed.
    void ReadHeader()
    {
      int bitDepth = 0;
      int colorType = 0;
      int interlaceType = 0;
      png_get_IHDR(_png, _info, &_width, &_height, &bitDepth, &colorType, &interlaceType, nullptr, nullptr);
      if (colorType == PNG_COLOR_TYPE_PALETTE || (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8)
          || png_get_valid(_png, _info, PNG_INFO_tRNS))
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

      // An ICC profile describing the (RGB) pixels takes the place of the gamma, which its curves include.
      Span<const uint8> profile;
      if (!_decoder._ignoreGammaAndColorProfile)
      {
        png_charp name = nullptr;
        int compression = 0;
        png_bytep profileData = nullptr;
        png_uint_32 profileSize = 0;
        if (png_get_iCCP(_png, _info, &name, &compression, &profileData, &profileSize)
            && ICCProfileTransform::DescribesPixels(Span<const uint8>(profileData, profileSize),
                                                    ICCProfileTransform::PixelLayout::RGB))
        {
          profile = Span<const uint8>(profileData, profileSize);
        }
        double gamma = 0;
        if (profile.empty() && png_get_gAMA(_png, _info, &gamma) && gamma > 0 && gamma <= MaxGamma)
        {
          png_set_gamma(_png, DisplayGamma, gamma);
        }
      }

      _interlaced = interlaceType == PNG_INTERLACE_ADAM7;
      if (_interlaced)
      {
        png_set_interlace_handling(_png);
      }
      png_read_update_info(_png, _info);
      _channels = png_get_channels(_png, _info);
      krys_debug_assert(_channels == 3 || _channels == 4);

      // The profile is the same for every frame, so its transform is made once.
      if (!profile.empty() && !_decoder._colorTransform)
      {
        auto layout =
          _channels == 4 ? ICCProfileTransform::PixelLayout::RGBA : ICCProfileTransform::PixelLayout::RGB;
        _decoder._colorTransform = ICCProfileTransform::Create(profile, layout, layout);
      }

      const auto &frame = _decoder._parser->Frames()[_index];
      _deferred = _interlaced && frame.BlendOver && _index > 0;
      if (_interlaced)
      {
        _pixels.resize(size_t {_width} * _height * _channels);
      }
    }

    void ReadRow(png_bytep row, png_uint_32 y)
    {
      // Rows an interlaced pass leaves unchanged come without pixels.
      if (!row || _failed || y >= _height)
      {
        return;
      }
      if (_interlaced)
      {
        auto *combined = _pixels.data() + size_t {y} * _width * _channels;
        png_progressive_combine_row(_png, combined, row);
        row = combined;
        if (_deferred)
        {
          return;
        }
      }
      if (!_decoder.WriteRow(_index, row, static_cast<int32>(y), _channels == 4))
      {
        _failed = true;
      }
    }
  };

  PNGImageDecoder::PNGImageDecoder(AlphaOption alphaOption,
                                   GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  PNGImageDecoder::~PNGImageDecoder() noexcept = default;

  bool PNGImageDecoder::SetSize(const SizeI &size) noexcept
  {
    // The parser reports the size again whenever it parses more.
    if (IsSizeAvailable() && Size() == size)
    {
      return true;
    }
    return ImageDecoder::SetSize(size);
  }

  void PNGImageDecoder::TryDecodeSize(bool allDataReceived) noexcept
  {
    std::ignore = Parse(allDataReceived);
  }

  bool PNGImageDecoder::Parse(bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return false;
    }
    if (!_parser)
    {
      _parser = CreateUnique<Parser>();
    }
    auto result = _parser->Parse(_data->GetSpan());
    if (result == Parser::Result::NeedsMoreData && allDataReceived)
    {
      result = _parser->EndOfData();
    }
    if (result == Parser::Result::Failed)
    {
      _reader = nullptr;
      return SetFailed();
    }
    if (_parser->HasHeader() && !SetSize(_parser->Size()))
    {
      return false;
    }
    // A frame is counted once its frame control chunk has arrived. A broken animation can lose frames.
    auto frameCount = _parser->Frames().size();
    if (_reader && _reader->FrameIndex() >= frameCount)
    {
      _reader = nullptr;
    }
    if (_frameBufferCache.size() != frameCount)
    {
      _frameBufferCache.resize(frameCount);
    }
    return true;
  }

  size_t PNGImageDecoder::FrameCount() const noexcept
  {
    std::ignore = const_cast<PNGImageDecoder *>(this)->Parse(IsAllDataReceived());
    return _frameBufferCache.size();
  }

  RepetitionCount PNGImageDecoder::GetRepetitionCount() const noexcept
  {
    if (Failed() || !_parser || !_parser->IsAnimated())
    {
      return RepetitionCountNone;
    }
    auto plays = _parser->Plays();
    return plays ? static_cast<RepetitionCount>(plays) : RepetitionCountInfinite;
  }

  ImageDecoderFrame *PNGImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index >= FrameCount())
    {
      return nullptr;
    }
    if (!_frameBufferCache[index].IsComplete())
    {
      for (auto frame = FirstFrameToDecode(index); frame <= index; ++frame)
      {
        if (!DecodeFrame(frame))
        {
          break;
        }
      }
    }
    return index < _frameBufferCache.size() ? &_frameBufferCache[index] : nullptr;
  }

  size_t PNGImageDecoder::FirstFrameToDecode(size_t index) const noexcept
  {
    auto canvas = RectI {PointI {}, Size()};
    for (auto frame = index; frame > 0; --frame)
    {
      const auto &previous = _frameBufferCache[frame - 1];
      const auto &previousInfo = _parser->Frames()[frame - 1];
      // A frame restored to what preceded it leaves no trace, so look past it.
      if (previousInfo.Disposal == ImageDecoderFrame::DisposalMethod::RestoreToPrevious)
      {
        continue;
      }
      // Decoding can start after a complete frame, or after one that's cleared to transparent everywhere.
      if (previous.IsComplete()
          || (previousInfo.Disposal == ImageDecoderFrame::DisposalMethod::RestoreToBackground
              && previousInfo.Rect.Contains(canvas)))
      {
        return frame;
      }
    }
    return 0;
  }

  bool PNGImageDecoder::DecodeFrame(size_t index) noexcept
  {
    if (_frameBufferCache[index].IsComplete())
    {
      return true;
    }
    if (_parser->Frames()[index].Chunks.empty())
    {
      return false;
    }
    if (!_reader || _reader->FrameIndex() != index)
    {
      // A partly decoded frame is only ever the one the decoder is working on; anything else starts afresh.
      _frameBufferCache[index].Clear();
      _reader = CreateUnique<FrameReader>(*this, index);
    }

    switch (_reader->Feed(_data->GetSpan()))
    {
      case FrameReader::Result::Complete: _reader = nullptr; return CompleteFrame(index);
      case FrameReader::Result::NeedsMoreData:
        if (IsAllDataReceived())
        {
          _reader = nullptr;
          std::ignore = SetFailed();
        }
        return false;
      case FrameReader::Result::Failed:
        _reader = nullptr;
        std::ignore = SetFailed();
        return false;
    }
    return false;
  }

  bool PNGImageDecoder::InitializeFrame(size_t index) noexcept
  {
    auto &frame = _frameBufferCache[index];
    const auto &info = _parser->Frames()[index];
    auto canvas = RectI {PointI {}, Size()};

    // The frame starts from the most recent frame not restored to what preceded it.
    auto base = index;
    while (base > 0
           && _parser->Frames()[base - 1].Disposal == ImageDecoderFrame::DisposalMethod::RestoreToPrevious)
    {
      --base;
    }
    if (!base)
    {
      if (!frame.Initialize(Size(), _premultiplyAlpha))
      {
        return SetFailed();
      }
    }
    else
    {
      const auto &previous = _frameBufferCache[base - 1];
      const auto &previousInfo = _parser->Frames()[base - 1];
      krys_debug_assert(previous.IsComplete());
      if (previousInfo.Disposal == ImageDecoderFrame::DisposalMethod::RestoreToBackground
          && previousInfo.Rect.Contains(canvas))
      {
        // Clearing the whole image leaves nothing to copy.
        if (!frame.Initialize(Size(), _premultiplyAlpha))
        {
          return SetFailed();
        }
      }
      else
      {
        if (!previous.HasBackingStore() || !frame.Initialize(*previous.BackingStore()))
        {
          return SetFailed();
        }
        frame.SetHasAlpha(previous.HasAlpha());
        if (previousInfo.Disposal == ImageDecoderFrame::DisposalMethod::RestoreToBackground)
        {
          frame.BackingStore()->ClearRect(previousInfo.Rect);
          frame.SetHasAlpha(true);
        }
      }
    }

    frame.BackingStore()->SetFrameRect(info.Rect);
    frame.SetDecodingStatus(DecodingStatus::Partial);
    _frameSawAlpha = false;
    return true;
  }

  bool PNGImageDecoder::WriteRow(size_t index, const uint8 *pixels, int32 y, bool hasAlphaChannel) noexcept
  {
    const auto &info = _parser->Frames()[index];
    auto &frame = _frameBufferCache[index];
    if (frame.IsInvalid() && !InitializeFrame(index))
    {
      return false;
    }
    if (y >= info.Rect.Height())
    {
      return true;
    }

    auto width = static_cast<uint32>(info.Rect.Width());
    if (_colorTransform)
    {
      auto size = size_t {width} * (hasAlphaChannel ? 4 : 3);
      if (_transformedRow.size() < size)
      {
        _transformedRow.resize(size);
      }
      _colorTransform.Transform(pixels, _transformedRow.data(), width);
      pixels = _transformedRow.data();
    }

    auto &store = *frame.BackingStore();
    auto *destination = store.PixelsStartingAt(info.Rect.X(), info.Rect.Y() + y);
    if (!hasAlphaChannel)
    {
      for (uint32 x = 0; x < width; ++x, pixels += 3)
      {
        destination[x] = 0xFF'00'00'00u | uint32 {pixels[0]} << 16 | uint32 {pixels[1]} << 8 | pixels[2];
      }
      return true;
    }
    // The first frame has nothing beneath it to blend over.
    auto blendOver = info.BlendOver && index > 0;
    for (uint32 x = 0; x < width; ++x, pixels += 4)
    {
      _frameSawAlpha |= pixels[3] != 0xFF;
      if (blendOver)
      {
        store.BlendPixel(destination[x], pixels[0], pixels[1], pixels[2], pixels[3]);
      }
      else
      {
        store.SetPixel(destination[x], pixels[0], pixels[1], pixels[2], pixels[3]);
      }
    }
    return true;
  }

  bool PNGImageDecoder::CompleteFrame(size_t index) noexcept
  {
    auto &frame = _frameBufferCache[index];
    const auto &info = _parser->Frames()[index];
    if (frame.IsInvalid() && !InitializeFrame(index))
    {
      return false;
    }
    frame.SetDecodingStatus(DecodingStatus::Complete);
    frame.SetDuration(info.Duration);
    frame.SetDisposalMethod(info.Disposal);

    // An opaque frame covering the whole image has no alpha; a frame with transparent pixels that replace
    // what's beneath has. Otherwise the frame keeps the alpha of what it started from.
    if (!_frameSawAlpha && info.Rect.Contains(RectI {PointI {}, Size()}))
    {
      frame.SetHasAlpha(false);
    }
    else if (_frameSawAlpha && !(info.BlendOver && index > 0))
    {
      frame.SetHasAlpha(true);
    }
    return true;
  }

  void PNGImageDecoder::ClearFrameBufferCache(size_t clearBeforeFrame) noexcept
  {
    if (_frameBufferCache.empty())
    {
      return;
    }

    // The last frame is never cleared: it's empty, being decoded, or the next frame may build on it.
    auto end = std::min(clearBeforeFrame, _frameBufferCache.size() - 1);

    // Keep the frame the next frame to decode will start from: the latest decoded frame at or before `end`
    // that isn't restored to what preceded it. Decoded frames in between that are restored can go.
    auto keep = end;
    while (keep > 0
           && (_frameBufferCache[keep].IsInvalid()
               || _frameBufferCache[keep].GetDisposalMethod()
                    == ImageDecoderFrame::DisposalMethod::RestoreToPrevious))
    {
      if (_frameBufferCache[keep].IsComplete() && keep != end)
      {
        _frameBufferCache[keep].Clear();
      }
      --keep;
    }
    for (size_t index = 0; index < keep; ++index)
    {
      krys_debug_assert(!_frameBufferCache[index].IsPartial());
      if (!_frameBufferCache[index].IsInvalid())
      {
        _frameBufferCache[index].Clear();
      }
    }
  }
}
