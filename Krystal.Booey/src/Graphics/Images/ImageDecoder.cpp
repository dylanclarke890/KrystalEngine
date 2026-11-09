/*
 * Copyright (C) 2017-2024 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Graphics/Images/ImageDecoder.hpp"
#include "Krystal.Booey/Graphics/Images/ImageFrame.hpp"
#include "Krystal.Booey/Platform/ImageDecoders/ScalableImageDecoder.hpp"

namespace krys::boo
{
  RefPtr<ImageDecoder> ImageDecoder::Create(const SharedBuffer &data, utf8_stringview mimeType,
                                            AlphaOption alphaOption,
                                            GammaAndColorProfileOption gammaAndColorProfileOption) noexcept
  {
    (void)mimeType;
    return ScalableImageDecoder::Create(data, alphaOption, gammaAndColorProfileOption);
  }

  ImageDecoder::ImageDecoder() noexcept = default;

  ImageDecoder::~ImageDecoder() noexcept = default;

  bool ImageDecoder::SupportsMediaType(MediaType type) noexcept
  {
    return ScalableImageDecoder::SupportsMediaType(type);
  }

  bool ImageDecoder::FetchFrameMetaDataAtIndex(size_t index, SubsamplingLevel subsamplingLevel,
                                               const DecodingOptions &options,
                                               ImageFrame &frame) const noexcept
  {
    if (options.HasSizeForDrawing())
    {
      krys_debug_assert(frame.HasNativeImage());
      frame._size = frame.GetNativeImage()->Size();
    }
    else
    {
      frame._size = FrameSizeAtIndex(index, subsamplingLevel);
    }

    frame._densityCorrectedSize = FrameDensityCorrectedSizeAtIndex(index);
    frame._subsamplingLevel = subsamplingLevel;
    frame._hasAlpha = FrameHasAlphaAtIndex(index);
    frame._orientation = FrameOrientationAtIndex(index);
    frame._decodingStatus =
      FrameIsCompleteAtIndex(index) ? DecodingStatus::Complete : DecodingStatus::Partial;
    return true;
  }
}
