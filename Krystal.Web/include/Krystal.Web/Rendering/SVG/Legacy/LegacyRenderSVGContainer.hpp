/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2009-2015 Google, Inc. All rights reserved.
 * Copyright (C) 2009-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Types/Markable.hpp"
#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGModelObject.hpp"

namespace krys::web
{
  /// @brief An SVG renderer whose children are SVG renderers, painted in its coordinates.
  class LegacyRenderSVGContainer : public LegacyRenderSVGModelObject
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGContainer);

  private:
    Markable<RectF> _objectBoundingBox;
    mutable Markable<RectF> _strokeBoundingBox;
    RectF _repaintBoundingBox {};
    mutable Markable<RectF> _accurateRepaintBoundingBox;

    bool _needsBoundariesUpdate {true};
    bool _repaintIsSuspendedForChildrenDuringLayout {false};

  public:
    ~LegacyRenderSVGContainer() noexcept override;

    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

    void SetNeedsBoundariesUpdate() noexcept final
    {
      _needsBoundariesUpdate = true;
    }

    KRYS_NODISCARD virtual bool DidTransformToRootUpdate() noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool IsObjectBoundingBoxValid() const noexcept
    {
      return static_cast<bool>(_objectBoundingBox);
    }

    KRYS_NODISCARD bool IsRepaintSuspendedForChildren() const noexcept
    {
      return _repaintIsSuspendedForChildrenDuringLayout;
    }

    KRYS_NODISCARD RectF ObjectBoundingBox() const noexcept final
    {
      return _objectBoundingBox.value_or(RectF());
    }

  protected:
    LegacyRenderSVGContainer(Type, SVGElement &, RenderStyle &&, EnumSet<SVGModelObjectFlag> = {}) noexcept;

    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGContainer";
    }

    KRYS_NODISCARD bool CanHaveChildren() const noexcept final
    {
      return true;
    }

    void Layout() noexcept override;

    void AddFocusRingRects(List<LayoutRect> &, const LayoutPoint &additionalOffset,
                           const RenderLayerModelObject *paintContainer = nullptr) const noexcept final;

    KRYS_NODISCARD RectF StrokeBoundingBox() const noexcept final;

    KRYS_NODISCARD RectF RepaintRectInLocalCoordinates(
      RepaintRectCalculation = RepaintRectCalculation::Fast) const noexcept final;

    KRYS_NODISCARD RectF DecoratedBoundingBox() const noexcept final;

    bool NodeAtPointF(const HitTestRequest &, HitTestResult &, const PointF &pointInParent,
                      HitTestAction) noexcept override;

    // Allow LegacyRenderSVGTransformableContainer to hook in at the right time in layout()
    virtual bool CalculateLocalTransform() noexcept
    {
      return false;
    }

    // Allow RenderSVGViewportContainer to hook in at the right times in layout(), paint() and
    // nodeAtPointF()
    virtual void CalcViewport() noexcept
    {
    }

    virtual void ApplyViewportClip(PaintInfo &) noexcept
    {
    }

    KRYS_NODISCARD virtual bool PointIsInsideViewportClip(const PointF & /*pointInParent*/) noexcept
    {
      return true;
    }

    virtual void DetermineIfLayoutSizeChanged() noexcept
    {
    }

    KRYS_NODISCARD bool SelfWillPaint() noexcept;

    void UpdateCachedBoundaries() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::LegacyRenderSVGContainer)
  KRYS_NODISCARD static bool IsType(const krys::web::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
