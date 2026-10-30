#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/BMPImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/GIFImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ICOImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/JPEGImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/PNGImageDecoder.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/WEBPImageDecoder.hpp"
#include <algorithm>
#include <array>
#include <string_view>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool HasPrefix(Span<const uint8> contents, std::string_view prefix) noexcept
    {
      return contents.size() >= prefix.size()
             && std::equal(prefix.begin(), prefix.end(), contents.begin(),
                           [](char a, uint8 b) { return static_cast<uint8>(a) == b; });
    }

    KRYS_NODISCARD bool MatchesGIFSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, "GIF87a") || HasPrefix(contents, "GIF89a");
    }

    KRYS_NODISCARD bool MatchesPNGSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, std::string_view("\x89\x50\x4E\x47\x0D\x0A\x1A\x0A", 8));
    }

    KRYS_NODISCARD bool MatchesJPEGSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, std::string_view("\xFF\xD8\xFF", 3));
    }

    KRYS_NODISCARD bool MatchesBMPSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, "BM");
    }

    KRYS_NODISCARD bool MatchesICOSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, std::string_view("\x00\x00\x01\x00", 4));
    }

    KRYS_NODISCARD bool MatchesCURSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, std::string_view("\x00\x00\x02\x00", 4));
    }

    KRYS_NODISCARD bool MatchesWebPSignature(Span<const uint8> contents) noexcept
    {
      return HasPrefix(contents, "RIFF") && HasPrefix(contents.subspan(8), "WEBPVP");
    }
  }

  RefPtr<ScalableImageDecoder>
    ScalableImageDecoder::Create(const SharedBuffer &data, AlphaOption alphaOption,
                                 GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
  {
    constexpr size_t lengthOfLongestSignature = 14; // To wit: "RIFF????WEBPVP"
    if (data.Size() < lengthOfLongestSignature)
    {
      return nullptr;
    }

    auto contents = data.GetSpan().first(lengthOfLongestSignature);

    if (MatchesGIFSignature(contents))
    {
      return GIFImageDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    if (MatchesPNGSignature(contents))
    {
      return PNGImageDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    if (MatchesICOSignature(contents) || MatchesCURSignature(contents))
    {
      return ICOImageDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    if (MatchesJPEGSignature(contents))
    {
      return JPEGImageDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    if (MatchesBMPSignature(contents))
    {
      return BMPImageDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    if (MatchesWebPSignature(contents))
    {
      return WEBPImageDecoder::Create(alphaOption, gammaAndColorProfileOption);
    }

    // FIXME: AVIF and JPEG XL, if they're ever wanted.
    return nullptr;
  }

  bool ScalableImageDecoder::FrameIsCompleteAtIndex(size_t index) const noexcept
  {
    if (index >= _frameBufferCache.size())
    {
      return false;
    }

    auto &frame = _frameBufferCache[index];
    return frame.IsComplete();
  }

  bool ScalableImageDecoder::FrameHasAlphaAtIndex(size_t index) const noexcept
  {
    if (_frameBufferCache.size() <= index)
    {
      return true;
    }

    auto &frame = _frameBufferCache[index];
    if (!frame.IsComplete())
    {
      return true;
    }
    return frame.HasAlpha();
  }

  seconds ScalableImageDecoder::FrameDurationAtIndex(size_t index) const noexcept
  {
    using namespace std::chrono_literals;

    if (index >= _frameBufferCache.size())
    {
      return {};
    }

    auto &frame = _frameBufferCache[index];
    if (!frame.IsComplete())
    {
      return {};
    }

    // Many annoying ads specify a 0 duration to make an image flash as quickly as possible.
    // We follow Firefox's behavior and use a duration of 100 ms for any frames that specify
    // a duration of <= 10 ms. See <rdar://problem/7689300> and <http://webkit.org/b/36082>
    // for more information.
    seconds duration = frame.Duration();
    if (duration < 11ms)
    {
      return 100ms;
    }
    return duration;
  }

  PlatformImagePtr ScalableImageDecoder::CreateFrameImageAtIndex(size_t index, SubsamplingLevel,
                                                                 const DecodingOptions &) noexcept
  {
    // Zero-height images can cause problems for some ports. If we have an empty image dimension, just bail.
    if (Size().IsEmpty())
    {
      return nullptr;
    }

    auto *buffer = FrameBufferAtIndex(index);
    if (!buffer || buffer->IsInvalid() || !buffer->HasBackingStore())
    {
      return nullptr;
    }

    // Return the buffer contents as a native image. For some ports, the data
    // is already in a native container, and this just increments its refcount.
    return buffer->BackingStore()->Image();
  }
}
