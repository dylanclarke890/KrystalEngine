/*
 * Copyright (C) 2006 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
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

#include "Krystal.Web/Rendering/SVG/Legacy/LegacyRenderSVGResourceRadialGradient.hpp"
#include "Krystal.Web/Svg/Elements/SVGRadialGradientElement.hpp"
#include "Krystal.Web/Svg/SVGLengthContext.hpp"

namespace krys::web
{
  LegacyRenderSVGResourceRadialGradient::LegacyRenderSVGResourceRadialGradient(
    SVGRadialGradientElement &element, RenderStyle &&style) noexcept
      : LegacyRenderSVGResourceGradient(Type::LegacySVGResourceRadialGradient, element, krys::move(style))
  {
  }

  LegacyRenderSVGResourceRadialGradient::~LegacyRenderSVGResourceRadialGradient() noexcept = default;

  SVGRadialGradientElement &LegacyRenderSVGResourceRadialGradient::RadialGradientElement() const noexcept
  {
    return Downcast<SVGRadialGradientElement>(LegacyRenderSVGResourceGradient::GradientElement());
  }

  bool LegacyRenderSVGResourceRadialGradient::CollectGradientAttributes() noexcept
  {
    _attributes = svg::RadialGradientAttributes();
    return RadialGradientElement().CollectGradientAttributes(_attributes);
  }

  PointF LegacyRenderSVGResourceRadialGradient::CenterPoint(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return SVGLengthContext::ResolvePoint(&RadialGradientElement(), attributes.GradientUnits(),
                                          attributes.Cx(), attributes.Cy());
  }

  PointF LegacyRenderSVGResourceRadialGradient::FocalPoint(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return SVGLengthContext::ResolvePoint(&RadialGradientElement(), attributes.GradientUnits(),
                                          attributes.Fx(), attributes.Fy());
  }

  float LegacyRenderSVGResourceRadialGradient::Radius(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return SVGLengthContext::ResolveLength(&RadialGradientElement(), attributes.GradientUnits(),
                                           attributes.R());
  }

  float LegacyRenderSVGResourceRadialGradient::FocalRadius(
    const svg::RadialGradientAttributes &attributes) const noexcept
  {
    return SVGLengthContext::ResolveLength(&RadialGradientElement(), attributes.GradientUnits(),
                                           attributes.Fr());
  }

  Ref<Gradient> LegacyRenderSVGResourceRadialGradient::BuildGradient() const noexcept
  {
    return Gradient::Create(Gradient::RadialData {FocalPoint(_attributes), CenterPoint(_attributes),
                                                  FocalRadius(_attributes), Radius(_attributes), 1},
                            {ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Unpremultiplied},
                            PlatformSpreadMethodFromSVGType(_attributes.SpreadMethod()),
                            GradientColorStops {_attributes.Stops()}, false);
  }
}
