/*
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Geometry/AffineTransform.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SharedBuffer.hpp"
#include "Krystal.Web/Graphics/Color/Color.hpp"
#include "Krystal.Web/Graphics/Images/ImageObserver.hpp"
#include "Krystal.Web/Graphics/Images/ImagePaintingOptions.hpp"

namespace krys
{
  class TextStream;
  class Timer;
}

namespace krys::web
{
  class GraphicsContext;
  class NativeImage;

  /// @brief Something drawable as an image: a bitmap image, an SVG image or a generated image (a gradient).
  class Image : public RefCounted<Image>, public CanMakeWeakPtr<Image>
  {
    friend class GraphicsContext;

  private:
    RefPtr<SharedBuffer> _encodedImageData;
    WeakPtr<ImageObserver> _imageObserver;
    UniquePtr<Timer> _animationStartTimer;

  public:
    enum TileRule
    {
      StretchTile,
      RoundTile,
      SpaceTile,
      RepeatTile,
    };

    virtual ~Image() noexcept;

    /// @brief The image for @p observer's resource: an SVG image for SVG, a bitmap image for the rest.
    KRYS_NODISCARD static RefPtr<Image> Create(ImageObserver &observer) noexcept;

    KRYS_NODISCARD virtual bool IsBitmapImage() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsGeneratedImage() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsGradientImage() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSVGImage() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSVGImageForContainer() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSVGResourceImage() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool DrawsSVGImage() const noexcept
    {
      return IsSVGImage() || IsSVGImageForContainer();
    }

    KRYS_NODISCARD virtual uint32 FrameCount() const noexcept
    {
      return 1;
    }

    KRYS_NODISCARD virtual bool CurrentFrameKnownToBeOpaque() const noexcept = 0;

    KRYS_NODISCARD virtual bool CurrentFrameIsComplete() const noexcept = 0;

    KRYS_NODISCARD virtual bool IsAnimated() const noexcept
    {
      return false;
    }

    // Derived classes should override this if their rendering could leak
    // cross-origin data (outside of the resource itself, which undergoes
    // a CORS cross-origin check).
    KRYS_NODISCARD virtual bool RenderingTaintsOrigin() const noexcept
    {
      return false;
    }

    /// @brief An empty image, drawn as nothing.
    KRYS_NODISCARD static Image &NullImage() noexcept;

    KRYS_NODISCARD bool IsNull() const noexcept
    {
      return Size().IsEmpty();
    }

    virtual void SetContainerSize(const SizeF &) noexcept
    {
    }

    KRYS_NODISCARD virtual bool UsesContainerSize() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool HasRelativeWidth() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool HasRelativeHeight() const noexcept
    {
      return false;
    }

    virtual void ComputeIntrinsicDimensions(float &intrinsicWidth, float &intrinsicHeight,
                                            SizeF &intrinsicRatio) noexcept;

    KRYS_NODISCARD virtual SizeF
      Size(img::ImageOrientation = img::ImageOrientation::Orientation::FromImage) const noexcept = 0;

    KRYS_NODISCARD virtual SizeF
      SourceSize(img::ImageOrientation = img::ImageOrientation::Orientation::FromImage) const noexcept;

    KRYS_NODISCARD virtual bool HasDensityCorrectedSize() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD RectF Rect() const noexcept
    {
      return RectF(PointF(), Size());
    }

    KRYS_NODISCARD float Width() const noexcept
    {
      return Size().Width();
    }

    KRYS_NODISCARD float Height() const noexcept
    {
      return Size().Height();
    }

    KRYS_NODISCARD virtual Maybe<PointI> HotSpot() const noexcept
    {
      return null;
    }

    KRYS_NODISCARD virtual img::ImageOrientation Orientation() const noexcept
    {
      return img::ImageOrientation::Orientation::FromImage;
    }

    /// @brief Replaces the encoded data and tells the image it changed.
    img::EncodedDataStatus SetData(RefPtr<SharedBuffer> &&data, bool allDataReceived) noexcept;

    virtual img::EncodedDataStatus DataChanged(bool /*allDataReceived*/) noexcept
    {
      return img::EncodedDataStatus::Unknown;
    }

    /// @brief The file extension the data's format uses; empty if unknown.
    KRYS_NODISCARD virtual utf8_string FilenameExtension() const noexcept
    {
      return {};
    }

    /// @brief A description of the image the data carries; empty if unknown.
    KRYS_NODISCARD virtual utf8_string AccessibilityDescription() const noexcept
    {
      return {};
    }

    virtual void DestroyDecodedData(bool /*destroyAll*/ = true) noexcept
    {
    }

    KRYS_NODISCARD SharedBuffer *Data() noexcept
    {
      return _encodedImageData.get();
    }

    KRYS_NODISCARD const SharedBuffer *Data() const noexcept
    {
      return _encodedImageData.get();
    }

    // Animation begins whenever someone draws the image, so startAnimation() is not normally called.
    // It will automatically pause once all observers no longer want to render the image anywhere.
    virtual void StartAnimation() noexcept
    {
    }

    /// @brief Starts the animation the next time the event loop runs its timers.
    void StartAnimationAsynchronously() noexcept;

    virtual void StopAnimation() noexcept
    {
    }

    virtual void ResetAnimation() noexcept
    {
    }

    KRYS_NODISCARD virtual bool IsAnimating() const noexcept
    {
      return false;
    }

    // Typically the CachedImage that owns us.
    KRYS_NODISCARD RefPtr<ImageObserver> GetImageObserver() const noexcept;

    void SetImageObserver(RefPtr<ImageObserver>) noexcept;

    KRYS_NODISCARD net::Url SourceUrl() const noexcept;

    KRYS_NODISCARD utf8_string MimeType() const noexcept;

    KRYS_NODISCARD int64 ExpectedContentLength() const noexcept;

    KRYS_NODISCARD virtual RefPtr<NativeImage> GetNativeImage() noexcept;

    KRYS_NODISCARD virtual RefPtr<NativeImage> NativeImageAtIndex(uint32) noexcept;

    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentNativeImage() noexcept;

    KRYS_NODISCARD virtual RefPtr<NativeImage> CurrentPreTransformedNativeImage(
      img::ImageOrientation = img::ImageOrientation::Orientation::FromImage) noexcept;

    virtual void DrawPattern(GraphicsContext &, const RectF &destRect, const RectF &tileRect,
                             const AffineTransform &patternTransform, const PointF &phase,
                             const SizeF &spacing, ImagePaintingOptions = {}) noexcept;

    virtual void Dump(TextStream &) const noexcept;

  protected:
    explicit Image(ImageObserver * = nullptr) noexcept;

    static void FillWithSolidColor(GraphicsContext &, const RectF &dstRect, const Color &,
                                   CompositeOperator) noexcept;

    virtual ImageDrawResult Draw(GraphicsContext &, const RectF &dstRect, const RectF &srcRect,
                                 ImagePaintingOptions = {}) noexcept = 0;

    ImageDrawResult DrawTiled(GraphicsContext &, const RectF &destinationRect, const PointF &srcPoint,
                              const SizeF &tileSize, const SizeF &spacing,
                              ImagePaintingOptions = {}) noexcept;

    ImageDrawResult DrawTiled(GraphicsContext &, const RectF &dstRect, const RectF &srcRect,
                              const SizeF &tileScaleFactor, TileRule hRule, TileRule vRule,
                              ImagePaintingOptions = {}) noexcept;

    // Supporting tiled drawing
    KRYS_NODISCARD virtual Maybe<Color> SinglePixelSolidColor() const noexcept;
  };

  TextStream &operator<<(TextStream &, const Image &) noexcept;
}
