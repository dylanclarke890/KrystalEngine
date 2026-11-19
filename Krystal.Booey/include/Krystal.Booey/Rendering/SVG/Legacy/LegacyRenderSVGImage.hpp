/*
 * Copyright (C) 2006 Alexander Kellett <lypanov@kde.org>
 * Copyright (C) 2006, 2009 Apple Inc. All rights reserved.
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"
#include "Krystal.Core/Geometry/AffineTransform.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class RenderImageResource;
  class SVGImageElement;

  /// @brief The renderer of `<image>`: draws its image into its rectangle, fitted by `preserveAspectRatio`.
  class LegacyRenderSVGImage final : public LegacyRenderSVGModelObject
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGImage);

  private:
    bool _needsBoundariesUpdate : 1 {true};
    bool _needsTransformUpdate : 1 {true};
    AffineTransform _localTransform;
    RectF _objectBoundingBox;
    RectF _repaintBoundingBox;
    const UniquePtr<RenderImageResource> _imageResource;

  public:
    LegacyRenderSVGImage(SVGImageElement &, RenderStyle &&) noexcept;

    ~LegacyRenderSVGImage() noexcept override;

    KRYS_NODISCARD SVGImageElement &ImageElement() const noexcept;

    bool UpdateImageViewport() noexcept;

    void SetNeedsBoundariesUpdate() noexcept override
    {
      _needsBoundariesUpdate = true;
    }

    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }

    KRYS_NODISCARD RenderImageResource &ImageResource() noexcept
    {
      return *_imageResource;
    }

    KRYS_NODISCARD const RenderImageResource &ImageResource() const noexcept
    {
      return *_imageResource;
    }

    // Note: Assumes the PaintInfo context has had all local transforms applied.
    void PaintForeground(PaintInfo &) noexcept;

    KRYS_NODISCARD bool IsObjectBoundingBoxValid() const noexcept
    {
      return !_objectBoundingBox.IsEmpty();
    }

    void GetElement() const = delete;

  private:
    void WillBeDestroyed() noexcept override;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGImage";
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localTransform;
    }

    void NotifyFinished(CachedResource &newImage) noexcept override;

    KRYS_NODISCARD RectF CalculateObjectBoundingBox() const noexcept;

    KRYS_NODISCARD RectF ObjectBoundingBox() const noexcept override
    {
      return _objectBoundingBox;
    }

    KRYS_NODISCARD RectF StrokeBoundingBox() const noexcept override
    {
      return _objectBoundingBox;
    }

    KRYS_NODISCARD RectF RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept override
    {
      return _repaintBoundingBox;
    }

    KRYS_NODISCARD RectF DecoratedBoundingBox() const noexcept override
    {
      return _objectBoundingBox;
    }

    void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                           const RenderLayerModelObject *paintContainer = nullptr) const noexcept override;

    void ImageChanged(WrappedImagePtr, const RectI * = nullptr) noexcept override;

    void Layout() noexcept override;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

    bool NodeAtPointF(const HitTestRequest &, HitTestResult &, const PointF &pointInParent,
                      HitTestAction) noexcept override;

    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept override
    {
      return _localTransform;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGImage)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGImage();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
