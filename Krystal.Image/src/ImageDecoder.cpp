/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2008-2009 Torch Mobile, Inc.
 * Copyright (C) Research In Motion Limited 2009-2010. All rights reserved.
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 *
 */

#include "Krystal.Image/ImageDecoder.hpp"
#include "Krystal.Core/Strings/ASCII.hpp"
#include "Krystal.Image/Decoders/BMPImageDecoder.hpp"
#include "Krystal.Image/Decoders/GIFImageDecoder.hpp"
#include "Krystal.Image/Decoders/ICOImageDecoder.hpp"
#include "Krystal.Image/Decoders/JPEGImageDecoder.hpp"
#include "Krystal.Image/Decoders/PNGImageDecoder.hpp"
#include "Krystal.Image/Decoders/WEBPImageDecoder.hpp"
#include <algorithm>
#include <array>
#include <string_view>

namespace krys::img
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

  RefPtr<ImageDecoder> ImageDecoder::Create(const SharedBuffer &data, AlphaOption alphaOption,
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

  bool ImageDecoder::SupportsMimeType(utf8_stringview mimeType) noexcept
  {
    constexpr utf8_stringview supportedTypes[] = {
      u8"image/png",
      u8"image/apng",
      u8"image/jpeg",
      u8"image/jpg",
      u8"image/pjpeg",
      u8"image/gif",
      u8"image/bmp",
      u8"image/x-bmp",
      u8"image/webp",
      u8"image/x-icon",
      u8"image/vnd.microsoft.icon",
    };
    return std::ranges::any_of(supportedTypes,
                               [&](auto type) { return text::IsASCIICaselessEqual(mimeType, type); });
  }

  bool ImageDecoder::FrameIsCompleteAtIndex(size_t index) const noexcept
  {
    if (index >= _frameBufferCache.size())
    {
      return false;
    }

    auto &frame = _frameBufferCache[index];
    return frame.IsComplete();
  }

  bool ImageDecoder::FrameHasAlphaAtIndex(size_t index) const noexcept
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

  seconds ImageDecoder::FrameDurationAtIndex(size_t index) const noexcept
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

  Maybe<DecodedImage> ImageDecoder::CreateFrameImageAtIndex(size_t index) noexcept
  {
    // Zero-height images can cause problems for some ports. If we have an empty image dimension, just bail.
    if (Size().IsEmpty())
    {
      return null;
    }

    auto *buffer = FrameBufferAtIndex(index);
    if (!buffer || buffer->IsInvalid() || !buffer->HasBackingStore())
    {
      return null;
    }

    return buffer->BackingStore()->Image();
  }
}
