#include "Krystal.Image/ImageDecoder.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Image/Decoders/BMPImageDecoder.hpp"
#include "Krystal.Image/Decoders/GIFImageDecoder.hpp"
#include "Krystal.Image/Decoders/ICOImageDecoder.hpp"
#include "Krystal.Image/Decoders/JPEGImageDecoder.hpp"
#include "Krystal.Image/Decoders/PNGImageDecoder.hpp"
#include "Krystal.Image/Decoders/WEBPImageDecoder.hpp"
#include <algorithm>
#include <string_view>

namespace krys::img
{
  namespace
  {
    using DecoderFactory = RefPtr<ImageDecoder> (*)(AlphaOption, GammaAndColorProfileOption) noexcept;

    // The bytes a format starts with, at an offset into the data.
    struct Signature
    {
      size_t Offset;
      std::string_view Bytes;
    };

    struct Format
    {
      DecoderFactory Factory;
      Signature First;
      Signature Second {0, {}};
    };

    template <typename TDecoder>
    KRYS_NODISCARD RefPtr<ImageDecoder> Make(AlphaOption alphaOption,
                                             GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
    {
      return TDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    using namespace std::string_view_literals;

    const Format Formats[] = {
      {Make<GIFImageDecoder>, {0, "GIF87a"sv}},
      {Make<GIFImageDecoder>, {0, "GIF89a"sv}},
      {Make<PNGImageDecoder>, {0, "\x89PNG\r\n\x1A\n"sv}},
      {Make<ICOImageDecoder>, {0, "\0\0\1\0"sv}}, // An icon.
      {Make<ICOImageDecoder>, {0, "\0\0\2\0"sv}}, // A cursor.
      {Make<JPEGImageDecoder>, {0, "\xFF\xD8\xFF"sv}},
      {Make<BMPImageDecoder>, {0, "BM"sv}},
      // A RIFF container (with its size in bytes 4 to 8) holding WebP.
      {Make<WEBPImageDecoder>, {0, "RIFF"sv}, {8, "WEBPVP"sv}},
    };

    KRYS_NODISCARD bool Matches(Span<const uint8> data, const Signature &signature) noexcept
    {
      if (data.size() < signature.Offset + signature.Bytes.size())
      {
        return false;
      }
      return std::ranges::equal(data.subspan(signature.Offset, signature.Bytes.size()), signature.Bytes,
                                [](uint8 byte, char expected)
                                { return byte == static_cast<uint8>(expected); });
    }
  }

  RefPtr<ImageDecoder> ImageDecoder::Create(const SharedBuffer &data, AlphaOption alphaOption,
                                            GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
  {
    // Wait for enough data to tell every format apart: the WebP signature ends 14 bytes in.
    constexpr size_t sniffLength = 14;
    if (data.Size() < sniffLength)
    {
      return nullptr;
    }

    auto start = data.GetSpan().first(sniffLength);
    for (const auto &format : Formats)
    {
      if (Matches(start, format.First) && Matches(start, format.Second))
      {
        return format.Factory(alphaOption, gammaAndColorProfileOption);
      }
    }
    // FIXME: AVIF and JPEG XL, if they're ever wanted.
    return nullptr;
  }

  bool ImageDecoder::SupportsMimeType(utf8_stringview mimeType) noexcept
  {
    constexpr utf8_stringview types[] = {
      u8"image/apng",
      u8"image/bmp",
      u8"image/gif",
      u8"image/jpeg",
      u8"image/jpg",
      u8"image/pjpeg",
      u8"image/png",
      u8"image/webp",
      u8"image/x-bmp",
      u8"image/x-icon",
      u8"image/vnd.microsoft.icon",
    };
    return std::ranges::any_of(types, [&](utf8_stringview type)
                               { return text::IsASCIICaselessEqual(mimeType, type); });
  }

  bool ImageDecoder::FrameIsCompleteAtIndex(size_t index) const noexcept
  {
    return index < _frameBufferCache.size() && _frameBufferCache[index].IsComplete();
  }

  bool ImageDecoder::FrameHasAlphaAtIndex(size_t index) const noexcept
  {
    // Until a frame is fully decoded, any of it might still be transparent.
    return !FrameIsCompleteAtIndex(index) || _frameBufferCache[index].HasAlpha();
  }

  seconds ImageDecoder::FrameDurationAtIndex(size_t index) const noexcept
  {
    using namespace std::chrono_literals;

    if (!FrameIsCompleteAtIndex(index))
    {
      return {};
    }

    // Browsers show frames shorter than 11 ms for 100 ms, since such short durations are almost always meant
    // as "as fast as possible" rather than as a real rate.
    auto duration = _frameBufferCache[index].Duration();
    return duration < 11ms ? 100ms : duration;
  }

  Maybe<DecodedImage> ImageDecoder::CreateFrameImageAtIndex(size_t index) noexcept
  {
    if (Size().IsEmpty())
    {
      return null;
    }

    auto *frame = FrameBufferAtIndex(index);
    if (!frame || frame->IsInvalid() || !frame->HasBackingStore())
    {
      return null;
    }
    return frame->BackingStore()->Image();
  }
}
