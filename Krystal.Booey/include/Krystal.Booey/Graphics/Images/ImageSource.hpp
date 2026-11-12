/*
 * Copyright (C) 2016-2025 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Graphics/Images/DecodingOptions.hpp"
#include "Krystal.Booey/Graphics/Images/ImageFrame.hpp"
#include "Krystal.Booey/Graphics/Images/ImageOrientation.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/Expected.hpp"
#include "Krystal.Core/Types/Func.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/String.hpp"

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  class FloatSize;
  class GraphicsContext;
  class SharedBuffer;

  /// @brief Where an image's frames and metadata come from: its decoder, or a native image it was made from.
  // FIXME: Colour spaces and HDR, once there are any.
  class ImageSource : public RefCounted<ImageSource>, public CanMakeWeakPtr<ImageSource>
  {
  public:
    virtual ~ImageSource() noexcept = default;

    // Encoded and decoded data
    virtual EncodedDataStatus DataChanged(SharedBuffer *, bool) noexcept
    {
      krys_assert_msg(false, "Not reached");
      return EncodedDataStatus::Unknown;
    }
    virtual void DestroyDecodedData(bool) noexcept
    {
      krys_assert_msg(false, "Not reached");
    }

    // Animation
    virtual void StartAnimation() noexcept
    {
    }
    virtual void StopAnimation() noexcept
    {
    }
    virtual void ResetAnimation() noexcept
    {
    }
    KRYS_NODISCARD virtual bool IsAnimated() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool IsAnimating() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool HasEverAnimated() const noexcept
    {
      return false;
    }

    // Decoding
    KRYS_NODISCARD virtual bool IsLargeForDecoding() const noexcept
    {
      return false;
    }
    virtual void StopDecodingWorkQueue() noexcept
    {
      krys_assert_msg(false, "Not reached");
    }
    virtual void Decode(Func<void(DecodingStatus)> &&) noexcept
    {
      krys_assert_msg(false, "Not reached");
    }

    // ImageFrame
    KRYS_NODISCARD virtual uint32 CurrentFrameIndex() const noexcept
    {
      return PrimaryFrameIndex();
    }

    virtual const ImageFrame &PrimaryImageFrame(const Maybe<SubsamplingLevel> & = null) noexcept = 0;
    virtual const ImageFrame &
      CurrentImageFrame(const Maybe<SubsamplingLevel> &subsamplingLevel = null) noexcept
    {
      return PrimaryImageFrame(subsamplingLevel);
    }

    // NativeImage
    KRYS_NODISCARD virtual RefPtr<NativeImage> PrimaryNativeImage() noexcept = 0;
    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentNativeImage() noexcept
    {
      return PrimaryNativeImage();
    }
    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentPreTransformedNativeImage(ImageOrientation) noexcept
    {
      return CurrentNativeImage();
    }
    /// @brief The current frame laid out as drawn with the orientation, at its full resolution: unlike the
    /// pre-transformed image, not scaled to a density-corrected size.
    KRYS_NODISCARD virtual RefPtr<NativeImage>
      CurrentOrientedNativeImage(ImageOrientation orientation) noexcept
    {
      return CurrentPreTransformedNativeImage(orientation);
    }

    KRYS_NODISCARD virtual RefPtr<NativeImage> NativeImageAtIndex(uint32) noexcept
    {
      return PrimaryNativeImage();
    }

    KRYS_NODISCARD virtual Expected<Ref<NativeImage>, DecodingStatus>
      PrimaryNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept;
    KRYS_NODISCARD virtual Expected<Ref<NativeImage>, DecodingStatus>
      CurrentNativeImageForDrawing(SubsamplingLevel, const DecodingOptions &) noexcept;

    // Image Metadata
    KRYS_NODISCARD virtual IntSize
      Size(ImageOrientation = ImageOrientation::Orientation::FromImage) const noexcept = 0;
    KRYS_NODISCARD virtual IntSize
      SourceSize(ImageOrientation orientation = ImageOrientation::Orientation::FromImage) const noexcept
    {
      return Size(orientation);
    }
    KRYS_NODISCARD virtual bool HasDensityCorrectedSize() const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual ImageOrientation Orientation() const noexcept
    {
      return ImageOrientation::Orientation::None;
    }
    KRYS_NODISCARD virtual uint32 PrimaryFrameIndex() const noexcept
    {
      return 0;
    }
    KRYS_NODISCARD virtual uint32 FrameCount() const noexcept
    {
      return 1;
    }
    KRYS_NODISCARD virtual Maybe<Color> SinglePixelSolidColor() const noexcept = 0;

    KRYS_NODISCARD bool HasSolidColor() const noexcept;

    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }
    KRYS_NODISCARD virtual Maybe<IntPoint> HotSpot() const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual SubsamplingLevel
      SubsamplingLevelForScaleFactor(GraphicsContext &, const FloatSize &, AllowImageSubsampling) noexcept
    {
      return SubsamplingLevel::Default;
    }

    // ImageFrame Metadata
    KRYS_NODISCARD virtual seconds FrameDurationAtIndex(uint32) const noexcept
    {
      krys_assert_msg(false, "Not reached");
      return {};
    }
    KRYS_NODISCARD virtual ImageOrientation FrameOrientationAtIndex(uint32) const noexcept
    {
      krys_assert_msg(false, "Not reached");
      return ImageOrientation::Orientation::None;
    }
    KRYS_NODISCARD virtual DecodingStatus FrameDecodingStatusAtIndex(uint32) const noexcept
    {
      krys_assert_msg(false, "Not reached");
      return DecodingStatus::Invalid;
    }

    virtual void Dump(TextStream &) const noexcept
    {
    }
  };
}
