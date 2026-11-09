/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Booey/Rendering/SVG/Legacy/LegacyRenderSVGResourceLinearGradient.hpp"
#include "Krystal.Booey/SVG/Elements/SVGLinearGradientElement.hpp"
#include "Krystal.Booey/SVG/SVGLengthContext.hpp"

namespace krys::boo
{
  LegacyRenderSVGResourceLinearGradient::LegacyRenderSVGResourceLinearGradient(
    svg::SVGLinearGradientElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceGradient(Type::LegacySVGResourceLinearGradient, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceLinearGradient::~LegacyRenderSVGResourceLinearGradient() noexcept = default;

  svg::SVGLinearGradientElement &LegacyRenderSVGResourceLinearGradient::LinearGradientElement() const noexcept
  {
    return Downcast<svg::SVGLinearGradientElement>(LegacyRenderSVGResourceGradient::GradientElement());
  }

  bool LegacyRenderSVGResourceLinearGradient::CollectGradientAttributes() noexcept
  {
    _attributes = svg::LinearGradientAttributes();
    return LinearGradientElement().CollectGradientAttributes(_attributes);
  }

  FloatPoint LegacyRenderSVGResourceLinearGradient::StartPoint(
    const svg::LinearGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolvePoint(&LinearGradientElement(), attributes.GradientUnits(),
                                               attributes.X1(), attributes.Y1());
  }

  FloatPoint LegacyRenderSVGResourceLinearGradient::EndPoint(
    const svg::LinearGradientAttributes &attributes) const noexcept
  {
    return svg::SVGLengthContext::ResolvePoint(&LinearGradientElement(), attributes.GradientUnits(),
                                               attributes.X2(), attributes.Y2());
  }

  Ref<Gradient> LegacyRenderSVGResourceLinearGradient::BuildGradient() const noexcept
  {
    return Gradient::Create(Gradient::LinearData {StartPoint(_attributes), EndPoint(_attributes)},
                            {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied},
                            PlatformSpreadMethodFromSVGType(_attributes.SpreadMethod()),
                            GradientColorStops {_attributes.Stops()}, false);
  }
}
