/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
    Copyright (C) Research In Motion Limited 2010. All rights reserved.
    Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
    Copyright (C) 2025 Samuel Weinig <sam@webkit.org>

    Based on khtml code by:
    Copyright (C) 2000-2003 Lars Knoll (knoll@kde.org)
              (C) 2000 Antti Koivisto (koivisto@kde.org)
              (C) 2000-2003 Dirk Mueller (mueller@kde.org)
              (C) 2002-2003 Apple Inc. All rights reserved.

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Color/Opacity.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGBaselineShift.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGCenterCoordinateComponent.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGCoordinateComponent.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGMarkerResource.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPaint.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGPathData.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGRadius.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGRadiusComponent.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDasharray.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDashoffset.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  // Inherited/Non-Inherited Style Datastructures
  class StyleFillData : public RefCounted<StyleFillData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleFillData> Create() noexcept
    {
      return AdoptRef(*new StyleFillData);
    }
    KRYS_NODISCARD Ref<StyleFillData> Copy() const noexcept;

    bool operator==(const StyleFillData &) const noexcept;

    style::Opacity FillOpacity;
    style::SVGPaint Fill;

  private:
    StyleFillData() noexcept;
    StyleFillData(const StyleFillData &) noexcept;
  };
  class StyleStrokeData : public RefCounted<StyleStrokeData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleStrokeData> Create() noexcept
    {
      return AdoptRef(*new StyleStrokeData);
    }
    KRYS_NODISCARD Ref<StyleStrokeData> Copy() const noexcept;

    bool operator==(const StyleStrokeData &) const noexcept;

    style::Opacity StrokeOpacity;
    style::SVGPaint Stroke;
    style::SVGStrokeDashoffset StrokeDashOffset;
    style::SVGStrokeDasharray StrokeDashArray;

  private:
    StyleStrokeData() noexcept;
    StyleStrokeData(const StyleStrokeData &) noexcept;
  };
  class StyleStopData : public RefCounted<StyleStopData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleStopData> Create() noexcept
    {
      return AdoptRef(*new StyleStopData);
    }
    KRYS_NODISCARD Ref<StyleStopData> Copy() const noexcept;

    bool operator==(const StyleStopData &) const noexcept;

    style::Opacity StopOpacity;
    style::Color StopColor;

  private:
    StyleStopData() noexcept;
    StyleStopData(const StyleStopData &) noexcept;
  };

  // Note: the rule for this class is, *no inheritance* of these props
  class StyleMiscData : public RefCounted<StyleMiscData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleMiscData> Create() noexcept
    {
      return AdoptRef(*new StyleMiscData);
    }
    KRYS_NODISCARD Ref<StyleMiscData> Copy() const noexcept;

    bool operator==(const StyleMiscData &) const noexcept;

    style::Opacity FloodOpacity;
    style::Color FloodColor;
    style::Color LightingColor;

    style::SVGBaselineShift BaselineShift;

  private:
    StyleMiscData() noexcept;
    StyleMiscData(const StyleMiscData &) noexcept;
  };

  // Inherited resources
  class StyleInheritedResourceData : public RefCounted<StyleInheritedResourceData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleInheritedResourceData> Create() noexcept
    {
      return AdoptRef(*new StyleInheritedResourceData);
    }
    KRYS_NODISCARD Ref<StyleInheritedResourceData> Copy() const noexcept;

    bool operator==(const StyleInheritedResourceData &) const noexcept;

    style::SVGMarkerResource MarkerStart;
    style::SVGMarkerResource MarkerMid;
    style::SVGMarkerResource MarkerEnd;

  private:
    StyleInheritedResourceData() noexcept;
    StyleInheritedResourceData(const StyleInheritedResourceData &) noexcept;
  };

  // Positioning and sizing properties.
  class StyleLayoutData : public RefCounted<StyleLayoutData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleLayoutData> Create() noexcept
    {
      return AdoptRef(*new StyleLayoutData);
    }
    KRYS_NODISCARD Ref<StyleLayoutData> Copy() const noexcept;

    bool operator==(const StyleLayoutData &) const noexcept;

    style::SVGCenterCoordinateComponent Cx;
    style::SVGCenterCoordinateComponent Cy;
    style::SVGRadius R;
    style::SVGRadiusComponent Rx;
    style::SVGRadiusComponent Ry;
    style::SVGCoordinateComponent X;
    style::SVGCoordinateComponent Y;
    style::SVGPathData D;

  private:
    StyleLayoutData() noexcept;
    StyleLayoutData(const StyleLayoutData &) noexcept;
  };
}
