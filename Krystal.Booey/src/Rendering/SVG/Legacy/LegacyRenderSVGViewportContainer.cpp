/*
 * Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGViewportContainer.hpp"
#include "Krystal.Booey/Graphics/GraphicsContext.hpp"
#include "Krystal.Booey/Rendering/Painting/PaintInfo.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGRenderSupport.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGViewportContainer::LegacyRenderSVGViewportContainer(svg::SVGSvgElement &element,
                                                                     RenderStyle &&style) noexcept
      : LegacyRenderSVGContainer(Type::LegacySVGViewportContainer, element, krys::move(style))
  {
    krys_debug_assert(IsLegacyRenderSVGViewportContainer());
  }

  LegacyRenderSVGViewportContainer::~LegacyRenderSVGViewportContainer() noexcept = default;

  svg::SVGSvgElement &LegacyRenderSVGViewportContainer::SvgSvgElement() const noexcept
  {
    return Downcast<svg::SVGSvgElement>(LegacyRenderSVGContainer::GetElement());
  }

  void LegacyRenderSVGViewportContainer::DetermineIfLayoutSizeChanged() noexcept
  {
    _isLayoutSizeChanged = SvgSvgElement().HasRelativeLengths() && SelfNeedsLayout();
  }

  void LegacyRenderSVGViewportContainer::ApplyViewportClip(PaintInfo &paintInfo) noexcept
  {
    if (SVGRenderSupport::IsOverflowHidden(*this))
    {
      paintInfo.Context().Clip(_viewport);
    }
  }

  void LegacyRenderSVGViewportContainer::CalcViewport() noexcept
  {
    auto &element = SvgSvgElement();
    svg::SVGLengthContext lengthContext(&element);
    FloatRect newViewport(element.X().Value(lengthContext), element.Y().Value(lengthContext),
                          element.Width().Value(lengthContext), element.Height().Value(lengthContext));

    if (_viewport == newViewport)
    {
      return;
    }

    _viewport = newViewport;

    InvalidateCachedBoundaries();
    SetNeedsTransformUpdate();
  }

  bool LegacyRenderSVGViewportContainer::CalculateLocalTransform() noexcept
  {
    _didTransformToRootUpdate = _needsTransformUpdate || SVGRenderSupport::TransformToRootChanged(Parent());
    if (!_needsTransformUpdate)
    {
      return false;
    }

    _localToParentTransform =
      AffineTransform::MakeTranslation(ToFloatSize(_viewport.Location())) * ViewportTransform();
    _needsTransformUpdate = false;
    return true;
  }

  AffineTransform LegacyRenderSVGViewportContainer::ViewportTransform() const noexcept
  {
    return SvgSvgElement().ViewBoxToViewTransform(_viewport.Width(), _viewport.Height());
  }

  bool LegacyRenderSVGViewportContainer::PointIsInsideViewportClip(const FloatPoint &pointInParent) noexcept
  {
    // Respect the viewport clip (which is in parent coords)
    if (!SVGRenderSupport::IsOverflowHidden(*this))
    {
      return true;
    }

    return _viewport.Contains(pointInParent);
  }

  void LegacyRenderSVGViewportContainer::Paint(PaintInfo &paintInfo, const LayoutPoint &paintOffset) noexcept
  {
    // An empty viewBox disables rendering.
    if (SvgSvgElement().HasEmptyViewBox())
    {
      return;
    }

    LegacyRenderSVGContainer::Paint(paintInfo, paintOffset);
  }
}
