/*
 * Copyright (C) 2006 Apple Inc. All rights reserved.
 * Copyright (C) 2009 Google, Inc.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGBlock.hpp"

namespace krys::boo::svg
{
  class SVGForeignObjectElement;
}

namespace krys::boo
{
  /// @brief The renderer of `<foreignObject>`: a block laid out as CSS in the element's rectangle, painted
  /// and hit tested in SVG coordinates.
  class LegacyRenderSVGForeignObject final : public RenderSVGBlock
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGForeignObject);

  private:
    AffineTransform _localTransform;
    mutable AffineTransform _localToParentTransform;
    FloatRect _viewport;
    bool _needsTransformUpdate {true};

  public:
    LegacyRenderSVGForeignObject(svg::SVGForeignObjectElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGForeignObject() noexcept override;

    KRYS_NODISCARD svg::SVGForeignObjectElement &ForeignObjectElement() const noexcept;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

    /// @brief Whether it has a layer: not to paint it, which @ref Paint does in SVG coordinates, but to hold
    /// the layers of the positioned boxes in it, which it paints in turn.
    KRYS_NODISCARD bool RequiresLayer() const noexcept override
    {
      return true;
    }
    void Layout() noexcept override;

    KRYS_NODISCARD FloatRect ObjectBoundingBox() const noexcept override
    {
      return FloatRect(FloatPoint(), _viewport.Size());
    }
    KRYS_NODISCARD bool IsObjectBoundingBoxValid() const noexcept
    {
      return !_viewport.IsEmpty();
    }
    KRYS_NODISCARD FloatRect StrokeBoundingBox() const noexcept override
    {
      return FloatRect(FloatPoint(), _viewport.Size());
    }
    KRYS_NODISCARD FloatRect RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept override
    {
      return FloatRect(FloatPoint(), _viewport.Size());
    }
    KRYS_NODISCARD FloatRect DecoratedBoundingBox() const noexcept override
    {
      return FloatRect(FloatPoint(), _viewport.Size());
    }

    bool NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &pointInParent,
                          HitTestAction) noexcept override;

    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }

  private:
    void GraphicsElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGForeignObject";
    }

    void UpdateLogicalWidth() noexcept override;
    KRYS_NODISCARD LogicalExtentComputedValues
      ComputeLogicalHeight(LayoutUnit logicalHeight, LayoutUnit logicalTop) const noexcept override;

    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override;
    KRYS_NODISCARD AffineTransform LocalTransform() const noexcept override
    {
      return _localTransform;
    }

    KRYS_NODISCARD LayoutSize
      OffsetFromContainer(const RenderElement &, const LayoutPoint &,
                          bool *offsetDependsOnPoint = nullptr) const noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGForeignObject)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGForeignObject();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
