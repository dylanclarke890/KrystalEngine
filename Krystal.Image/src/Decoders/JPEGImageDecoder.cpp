#include "Krystal.Image/Decoders/JPEGImageDecoder.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Image/EXIFMetadata.hpp"
#include "Krystal.Image/ICCProfileTransform.hpp"
#include <algorithm>
#include <csetjmp>
#include <cstring>
#include <tuple>
#include <utility>

extern "C"
{
#include <jpeglib.h>
}

// libjpeg reports fatal errors through error_exit, which mustn't return: ours longjmps back to the reader. On
// Windows, longjmp unwinds the stack as an exception would, so the functions it passes through aren't
// noexcept.
// clang-format off
#pragma warning(disable : 4611) // interaction between '_setjmp' and C++ object destruction is non-portable
#pragma warning(disable : 4324) // structure was padded due to alignment specifier (jmp_buf)
// clang-format on

namespace krys::img
{
  namespace
  {
    // APP1 holds EXIF metadata, and APP2 ICC profiles.
    constexpr int EXIFMarker = JPEG_APP0 + 1;
    constexpr int ICCMarker = JPEG_APP0 + 2;

    struct ErrorHandler : jpeg_error_mgr
    {
      std::jmp_buf Jump;
    };

    KRYS_NORETURN void ExitOnError(j_common_ptr info)
    {
      std::longjmp(static_cast<ErrorHandler *>(info->err)->Jump, 1); // NOLINT(modernize-avoid-setjmp-longjmp)
    }

    void IgnoreMessage(j_common_ptr)
    {
      // Warnings about recoverable corruption aren't worth printing.
    }

    KRYS_NODISCARD Span<const uint8> MarkerData(const jpeg_marker_struct &marker) noexcept
    {
      return {reinterpret_cast<const uint8 *>(marker.data), marker.data_length};
    }

    KRYS_NODISCARD bool StartsWith(Span<const uint8> data, Span<const char> prefix) noexcept
    {
      return data.size() >= prefix.size() && std::memcmp(data.data(), prefix.data(), prefix.size()) == 0;
    }

    // The EXIF metadata in the first APP1 marker that has it: "Exif", two zero bytes, then a TIFF block.
    KRYS_NODISCARD EXIFMetadata ReadEXIF(const jpeg_decompress_struct &info, const SizeI &size) noexcept
    {
      constexpr char header[] = {'E', 'x', 'i', 'f', 0, 0};
      for (auto *marker = info.marker_list; marker; marker = marker->next)
      {
        auto data = MarkerData(*marker);
        if (marker->marker == EXIFMarker && StartsWith(data, header))
        {
          return EXIFMetadata::Read(data.subspan(sizeof(header)), size);
        }
      }
      return {};
    }

    // The ICC profile, which may be split across APP2 markers: each is "ICC_PROFILE", a zero byte, its
    // 1-based sequence number and the number of markers, then its part of the profile. Empty when there's no
    // profile, or the markers don't make up a whole one.
    KRYS_NODISCARD List<uint8> ReadICCProfile(const jpeg_decompress_struct &info) noexcept
    {
      constexpr char signature[] = "ICC_PROFILE"; // With its terminating zero.
      constexpr size_t headerSize = sizeof(signature) + 2;

      List<Span<const uint8>> parts;
      for (auto *marker = info.marker_list; marker; marker = marker->next)
      {
        auto data = MarkerData(*marker);
        if (marker->marker != ICCMarker || data.size() < headerSize || !StartsWith(data, signature))
        {
          continue;
        }
        auto sequenceNumber = data[sizeof(signature)];
        auto count = data[sizeof(signature) + 1];
        if (!count || sequenceNumber < 1 || sequenceNumber > count
            || (!parts.empty() && parts.size() != count))
        {
          return {};
        }
        parts.resize(count);
        if (!parts[sequenceNumber - 1].empty())
        {
          return {}; // A repeated part.
        }
        parts[sequenceNumber - 1] = data.subspan(headerSize);
      }

      List<uint8> profile;
      for (auto part : parts)
      {
        if (part.empty())
        {
          return {}; // A missing part.
        }
        profile.insert(profile.end(), part.begin(), part.end());
      }
      return profile;
    }

    // Adobe's inverted CMYK stores 255 - C (and so on), so with the ink scaled by the key, each RGB channel
    // is the stored ink value times the stored key.
    KRYS_NODISCARD uint8 InvertedCMYKChannelToRGB(uint8 ink, uint8 key) noexcept
    {
      return static_cast<uint8>(ink * key / 255);
    }
  }

  class JPEGImageDecoder::Reader
  {
    KRYS_NON_COPY_MOVABLE(Reader);

  public:
    enum class Result : uint8
    {
      /// @brief It stopped at the end of the data that's arrived so far.
      NeedsMoreData,
      /// @brief It decoded as much as was asked for.
      Done,
      Failed,
    };

  private:
    enum class Stage : uint8
    {
      Header,
      StartDecompressing,
      SequentialRows,
      ProgressiveScans,
      Finishing,
      Complete,
    };

    struct Source : jpeg_source_mgr
    {
      Reader *Owner;
    };

    JPEGImageDecoder &_decoder;
    jpeg_decompress_struct _info {};
    ErrorHandler _errors {};
    Source _source {};
    Stage _stage {Stage::Header};
    // How much of the data libjpeg has been given; it may not have consumed all of it.
    size_t _bytesGiven {0};
    // Bytes libjpeg asked to skip beyond the end of what it was given.
    size_t _bytesToSkip {0};
    // Whether a progressive output pass has started, and so is resumed rather than started again.
    bool _outputPassStarted {false};
    bool _isCMYK {false};
    ICCProfileTransform _profileTransform;
    // A CMYK row as libjpeg decodes it, and as the profile converts it to RGB.
    List<uint8> _cmykRow;
    List<uint8> _rgbRow;

  public:
    explicit Reader(JPEGImageDecoder &decoder) noexcept : _decoder(decoder)
    {
      _info.err = jpeg_std_error(&_errors);
      _errors.error_exit = ExitOnError;
      _errors.output_message = IgnoreMessage;
      jpeg_create_decompress(&_info);

      _source.init_source = [](j_decompress_ptr) {};
      // The reader gives libjpeg all the data it has, so when libjpeg wants more, it has to wait.
      _source.fill_input_buffer = [](j_decompress_ptr) -> boolean
      {
        return FALSE;
      };
      _source.skip_input_data = [](j_decompress_ptr info, long count)
      {
        static_cast<Source *>(info->src)->Owner->Skip(static_cast<size_t>(std::max(count, 0l)));
      };
      _source.resync_to_restart = jpeg_resync_to_restart;
      _source.term_source = [](j_decompress_ptr) {};
      _source.Owner = this;
      _info.src = &_source;

      jpeg_save_markers(&_info, EXIFMarker, 0xFFFF);
      if (!_decoder.IgnoresGammaAndColorProfile())
      {
        jpeg_save_markers(&_info, ICCMarker, 0xFFFF);
      }
    }

    ~Reader() noexcept
    {
      jpeg_destroy_decompress(&_info);
    }

    /// @brief Decodes what's arrived of @p data (the image's data so far): up to its size if @p sizeOnly,
    /// or else as much of the image as it can.
    Result Decode(const SharedBuffer &data, bool sizeOnly)
    {
      GiveData(data);
      if (setjmp(_errors.Jump)) // NOLINT(modernize-avoid-setjmp-longjmp): libjpeg's errors land here.
      {
        return Result::Failed;
      }
      return Run(sizeOnly);
    }

  private:
    void GiveData(const SharedBuffer &data) noexcept
    {
      // The data may have moved as it grew, so point libjpeg at where it had got to in the new copy.
      auto unreadStart = _bytesGiven - _source.bytes_in_buffer;
      _source.next_input_byte = data.Data() + unreadStart;
      _source.bytes_in_buffer = data.Size() - unreadStart;
      _bytesGiven = data.Size();
      Skip(std::exchange(_bytesToSkip, 0));
    }

    void Skip(size_t count) noexcept
    {
      auto skipped = std::min(count, _source.bytes_in_buffer);
      _source.next_input_byte += skipped;
      _source.bytes_in_buffer -= skipped;
      _bytesToSkip += count - skipped;
    }

    Result Run(bool sizeOnly)
    {
      while (true)
      {
        switch (_stage)
        {
          case Stage::Header:
            if (jpeg_read_header(&_info, TRUE) == JPEG_SUSPENDED)
            {
              return Result::NeedsMoreData;
            }
            if (!ReadHeader())
            {
              return Result::Failed;
            }
            _stage = Stage::StartDecompressing;
            break;
          case Stage::StartDecompressing:
            if (sizeOnly)
            {
              return Result::Done;
            }
            _info.dct_method = JDCT_ISLOW;
            _info.do_fancy_upsampling = TRUE;
            _info.do_block_smoothing = TRUE;
            if (!jpeg_start_decompress(&_info))
            {
              return Result::NeedsMoreData;
            }
            _stage = _info.buffered_image ? Stage::ProgressiveScans : Stage::SequentialRows;
            break;
          case Stage::SequentialRows:
            if (sizeOnly)
            {
              return Result::Done;
            }
            if (!WriteRows())
            {
              return Result::NeedsMoreData;
            }
            _stage = Stage::Finishing;
            break;
          case Stage::ProgressiveScans:
            if (sizeOnly)
            {
              return Result::Done;
            }
            if (!WriteScans())
            {
              return Result::NeedsMoreData;
            }
            _stage = Stage::Finishing;
            break;
          case Stage::Finishing:
            if (sizeOnly)
            {
              return Result::Done;
            }
            // Reads up to the end of the image.
            if (!jpeg_finish_decompress(&_info))
            {
              return Result::NeedsMoreData;
            }
            _stage = Stage::Complete;
            Frame().SetHasAlpha(false);
            Frame().SetDecodingStatus(DecodingStatus::Complete);
            break;
          case Stage::Complete: return Result::Done;
        }
      }
    }

    // Takes what's needed from the header: the size, the colour space to decode to, the EXIF metadata and the
    // ICC profile.
    bool ReadHeader()
    {
      switch (_info.jpeg_color_space)
      {
        case JCS_GRAYSCALE:
        case JCS_RGB:
        case JCS_YCbCr:
          // libjpeg-turbo writes BGRA straight into the image's pixels.
          _info.out_color_space = JCS_EXT_BGRA;
          break;
        case JCS_CMYK:
        case JCS_YCCK:
          // libjpeg can't convert CMYK to RGB, so it decodes to (inverted) CMYK, and the reader converts
          // that.
          _info.out_color_space = JCS_CMYK;
          _isCMYK = true;
          break;
        default: return false;
      }

      SizeI size {static_cast<int32>(_info.image_width), static_cast<int32>(_info.image_height)};
      if (!_decoder.SetSize(size))
      {
        return false;
      }
      _decoder.SetEXIFMetadata(ReadEXIF(_info, size));
      if (!_decoder.IgnoresGammaAndColorProfile())
      {
        auto profile = ReadICCProfile(_info);
        _profileTransform =
          _isCMYK ? ICCProfileTransform::Create(profile, ICCProfileTransform::PixelLayout::InvertedCMYK,
                                                ICCProfileTransform::PixelLayout::RGB)
                  : ICCProfileTransform::Create(profile, ICCProfileTransform::PixelLayout::BGRA,
                                                ICCProfileTransform::PixelLayout::BGRA);
      }

      // Progressive images are decoded a scan at a time, which needs their coefficients buffered; sequential
      // ones are decoded straight to rows.
      _info.buffered_image = jpeg_has_multiple_scans(&_info);
      return true;
    }

    KRYS_NODISCARD ImageDecoderFrame &Frame() noexcept
    {
      return _decoder._frameBufferCache[0];
    }

    // Writes rows until the output pass is complete, or libjpeg needs more data.
    bool WriteRows()
    {
      auto &frame = Frame();
      if (frame.IsInvalid())
      {
        if (!frame.Initialize(_decoder.Size(), _decoder._premultiplyAlpha))
        {
          // Fail as libjpeg's own errors do.
          ExitOnError(reinterpret_cast<j_common_ptr>(&_info));
        }
        // What isn't decoded yet is transparent.
        frame.SetDecodingStatus(DecodingStatus::Partial);
        frame.SetHasAlpha(true);
      }

      auto &pixels = *frame.BackingStore();
      auto width = _info.output_width;
      while (_info.output_scanline < _info.output_height)
      {
        auto y = static_cast<int32>(_info.output_scanline);
        if (!_isCMYK)
        {
          auto *row = reinterpret_cast<JSAMPLE *>(pixels.PixelsStartingAt(0, y));
          if (jpeg_read_scanlines(&_info, &row, 1) != 1)
          {
            return false;
          }
          _profileTransform.Transform(row, row, width);
          continue;
        }

        _cmykRow.resize(size_t {width} * 4);
        auto *row = _cmykRow.data();
        if (jpeg_read_scanlines(&_info, &row, 1) != 1)
        {
          return false;
        }
        auto *destination = pixels.PixelsStartingAt(0, y);
        if (_profileTransform)
        {
          _rgbRow.resize(size_t {width} * 3);
          _profileTransform.Transform(_cmykRow.data(), _rgbRow.data(), width);
          for (uint32 x = 0; x < width; ++x)
          {
            const auto *rgb = &_rgbRow[x * 3];
            pixels.SetPixel(destination[x], rgb[0], rgb[1], rgb[2], 0xFF);
          }
          continue;
        }
        for (uint32 x = 0; x < width; ++x)
        {
          const auto *cmyk = &_cmykRow[x * 4];
          pixels.SetPixel(destination[x], InvertedCMYKChannelToRGB(cmyk[0], cmyk[3]),
                          InvertedCMYKChannelToRGB(cmyk[1], cmyk[3]),
                          InvertedCMYKChannelToRGB(cmyk[2], cmyk[3]), 0xFF);
        }
      }
      return true;
    }

    // Shows each scan of a progressive image once enough has arrived, until the last one is shown.
    bool WriteScans()
    {
      int status;
      do
      {
        status = jpeg_consume_input(&_info);
      } while (status != JPEG_SUSPENDED && status != JPEG_REACHED_EOI);

      while (true)
      {
        if (!_outputPassStarted)
        {
          // Before anything is shown, show the last scan that's fully arrived rather than one still arriving.
          auto scan = _info.input_scan_number;
          if (!_info.output_scan_number && scan > 1 && status != JPEG_REACHED_EOI)
          {
            --scan;
          }
          if (!jpeg_start_output(&_info, scan))
          {
            return false;
          }
          _outputPassStarted = true;
        }
        if (!WriteRows() || !jpeg_finish_output(&_info))
        {
          return false;
        }
        _outputPassStarted = false;
        if (jpeg_input_complete(&_info) && _info.input_scan_number == _info.output_scan_number)
        {
          return true;
        }
      }
    }
  };

  JPEGImageDecoder::JPEGImageDecoder(AlphaOption alphaOption,
                                     GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
      : ImageDecoder(alphaOption, gammaAndColorProfileOption)
  {
  }

  JPEGImageDecoder::~JPEGImageDecoder() noexcept = default;

  ImageDecoderFrame *JPEGImageDecoder::FrameBufferAtIndex(size_t index) noexcept
  {
    if (index)
    {
      return nullptr;
    }
    if (_frameBufferCache.empty())
    {
      _frameBufferCache.resize(1);
    }
    if (!_frameBufferCache[0].IsComplete())
    {
      Decode(false, IsAllDataReceived());
    }
    return &_frameBufferCache[0];
  }

  void JPEGImageDecoder::Decode(bool sizeOnly, bool allDataReceived) noexcept
  {
    if (Failed())
    {
      return;
    }
    if (!sizeOnly && _frameBufferCache.empty())
    {
      _frameBufferCache.resize(1);
    }
    if (!_reader)
    {
      _reader = CreateUnique<Reader>(*this);
    }

    auto result = _reader->Decode(*_data, sizeOnly);
    if (result == Reader::Result::Failed || Failed()
        || (result == Reader::Result::NeedsMoreData && allDataReceived))
    {
      _reader = nullptr;
      std::ignore = SetFailed();
    }
    else if (!_frameBufferCache.empty() && _frameBufferCache[0].IsComplete())
    {
      _reader = nullptr;
    }
  }
}
