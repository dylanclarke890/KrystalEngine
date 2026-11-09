/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2009 Google, Inc.
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGContainer.hpp"

namespace krys::boo::svg
{
  class SVGSvgElement;
}

namespace krys::boo
{
  // This is used for non-root <svg> elements and <marker> elements, neither of which are SVGTransformable
  // thus we inherit from LegacyRenderSVGContainer instead of LegacyRenderSVGTransformableContainer
  class LegacyRenderSVGViewportContainer final : public LegacyRenderSVGContainer
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(LegacyRenderSVGViewportContainer);

  private:
    bool _didTransformToRootUpdate : 1 {false};
    bool _isLayoutSizeChanged : 1 {false};
    bool _needsTransformUpdate : 1 {true};
    FloatRect _viewport {};
    mutable AffineTransform _localToParentTransform {};

  public:
    LegacyRenderSVGViewportContainer(svg::SVGSvgElement &, RenderStyle &&) noexcept;
    ~LegacyRenderSVGViewportContainer() noexcept override;

    KRYS_NODISCARD svg::SVGSvgElement &SvgSvgElement() const noexcept;

    KRYS_NODISCARD FloatRect Viewport() const noexcept
    {
      return _viewport;
    }

    KRYS_NODISCARD bool IsLayoutSizeChanged() const noexcept
    {
      return _isLayoutSizeChanged;
    }
    KRYS_NODISCARD bool DidTransformToRootUpdate() noexcept override
    {
      return _didTransformToRootUpdate;
    }
    void DetermineIfLayoutSizeChanged() noexcept override;
    void SetNeedsTransformUpdate() noexcept override
    {
      _needsTransformUpdate = true;
    }
    void Paint(PaintInfo &, const LayoutPoint &) noexcept override;

  private:
    void GetElement() const = delete;
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGViewportContainer";
    }

    KRYS_NODISCARD AffineTransform ViewportTransform() const noexcept;
    KRYS_NODISCARD const AffineTransform &LocalToParentTransform() const noexcept override
    {
      return _localToParentTransform;
    }

    void CalcViewport() noexcept override;
    bool CalculateLocalTransform() noexcept override;

    void ApplyViewportClip(PaintInfo &) noexcept override;
    KRYS_NODISCARD bool PointIsInsideViewportClip(const FloatPoint &pointInParent) noexcept override;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::LegacyRenderSVGViewportContainer)
  KRYS_NODISCARD static bool IsType(const krys::boo::RenderObject &renderer) noexcept
  {
    return renderer.IsLegacyRenderSVGViewportContainer();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
