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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Booey/Graphics/Images/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Graphics/Images/PlatformImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Time/Duration.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys::boo
{
  class ImageFrame;
  class SharedBuffer;

  struct ImageDecoderFrameInfo
  {
    bool HasAlpha;
    seconds Duration;
  };

  /// @brief Decodes an image's encoded data, as it arrives, into its size, frames and frame metadata.
  class ImageDecoder : public RefCounted<ImageDecoder>, public CanMakeWeakPtr<ImageDecoder>
  {
    KRYS_NON_COPY_MOVABLE(ImageDecoder);

  public:
    using FrameInfo = ImageDecoderFrameInfo;

    enum class MediaType : uint8
    {
      Image,
      Video,
    };

    /// @brief A decoder for @p data, sniffed from its first bytes, or null if no decoder supports it (or not
    /// enough has arrived to tell).
    /// @param mimeType The type the data came with, which decoders don't rely on.
    // FIXME: Video decoders, once there's video.
    KRYS_NODISCARD static RefPtr<ImageDecoder>
      Create(const SharedBuffer &data, utf8_stringview mimeType, AlphaOption alphaOption,
             GammaAndColorProfileOption gammaAndColorProfileOption) noexcept;

    virtual ~ImageDecoder() noexcept;

    KRYS_NODISCARD static bool SupportsMediaType(MediaType mediaType) noexcept;

    KRYS_NODISCARD virtual size_t BytesDecodedToDetermineProperties() const noexcept = 0;

    KRYS_NODISCARD virtual EncodedDataStatus GetEncodedDataStatus() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsSizeAvailable() const noexcept
    {
      return GetEncodedDataStatus() >= EncodedDataStatus::SizeAvailable;
    }

    KRYS_NODISCARD virtual IntSize Size() const noexcept = 0;
    KRYS_NODISCARD virtual size_t FrameCount() const noexcept = 0;
    KRYS_NODISCARD virtual size_t PrimaryFrameIndex() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual RepetitionCount GetRepetitionCount() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept = 0;
    KRYS_NODISCARD virtual utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual Maybe<IntPoint> HotSpot() const noexcept = 0;

    KRYS_NODISCARD virtual IntSize
      FrameSizeAtIndex(size_t index, SubsamplingLevel = SubsamplingLevel::Default) const noexcept = 0;
    KRYS_NODISCARD virtual bool FrameIsCompleteAtIndex(size_t index) const noexcept = 0;
    KRYS_NODISCARD virtual ImageOrientation FrameOrientationAtIndex(size_t) const noexcept
    {
      return ImageOrientation::Orientation::None;
    }
    KRYS_NODISCARD virtual Maybe<IntSize> FrameDensityCorrectedSizeAtIndex(size_t) const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual seconds FrameDurationAtIndex(size_t index) const noexcept = 0;
    KRYS_NODISCARD virtual bool FrameHasAlphaAtIndex(size_t index) const noexcept = 0;

    virtual bool FetchFrameMetaDataAtIndex(size_t index, SubsamplingLevel subsamplingLevel,
                                           const DecodingOptions &options, ImageFrame &frame) const noexcept;

    KRYS_NODISCARD virtual PlatformImagePtr CreateFrameImageAtIndex(
      size_t index, SubsamplingLevel = SubsamplingLevel::Default,
      const DecodingOptions & = DecodingOptions(DecodingMode::Synchronous)) noexcept = 0;

    virtual void SetExpectedContentSize(int64) noexcept
    {
    }
    virtual void SetData(const SharedBuffer &data, bool allDataReceived) noexcept = 0;
    KRYS_NODISCARD virtual bool IsAllDataReceived() const noexcept = 0;
    virtual void ClearFrameBufferCache(size_t index) noexcept = 0;

  protected:
    ImageDecoder() noexcept;
  };
}
