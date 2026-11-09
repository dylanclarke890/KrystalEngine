/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005, 2010 Rob Buis <buis@kde.org>
    Copyright (C) Research In Motion Limited 2010. All rights reserved.
    Copyright (C) 2025 Samuel Weinig <sam@webkit.org>

    Based on khtml code by:
    Copyright (C) 1999 Antti Koivisto (koivisto@kde.org)
    Copyright (C) 1999-2003 Lars Knoll (knoll@kde.org)
    Copyright (C) 2002-2003 Dirk Mueller (mueller@kde.org)
    Copyright (C) 2002 Apple Inc. All rights reserved.

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

#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Core/Threading/NeverDestroyed.hpp"

namespace krys::boo
{
  static const SVGRenderStyle &DefaultSVGStyle() noexcept
  {
    static NeverDestroyed<DataRef<SVGRenderStyle>> style {SVGRenderStyle::CreateDefaultStyle()};
    return style->Get();
  }

  Ref<SVGRenderStyle> SVGRenderStyle::CreateDefaultStyle() noexcept
  {
    return AdoptRef(*new SVGRenderStyle(CreateDefault));
  }

  SVGRenderStyle::SVGRenderStyle() noexcept
      : FillData(DefaultSVGStyle().FillData), StrokeData(DefaultSVGStyle().StrokeData),
        InheritedResourceData(DefaultSVGStyle().InheritedResourceData), StopData(DefaultSVGStyle().StopData),
        MiscData(DefaultSVGStyle().MiscData), LayoutData(DefaultSVGStyle().LayoutData)
  {
    SetBitDefaults();
  }

  SVGRenderStyle::SVGRenderStyle(CreateDefaultType) noexcept
      : FillData(StyleFillData::Create()), StrokeData(StyleStrokeData::Create()),
        InheritedResourceData(StyleInheritedResourceData::Create()), StopData(StyleStopData::Create()),
        MiscData(StyleMiscData::Create()), LayoutData(StyleLayoutData::Create())
  {
    SetBitDefaults();
  }

  inline SVGRenderStyle::SVGRenderStyle(const SVGRenderStyle &other) noexcept
      : RefCounted<SVGRenderStyle>(), InheritedFlags(other.InheritedFlags),
        NonInheritedFlags(other.NonInheritedFlags), FillData(other.FillData), StrokeData(other.StrokeData),
        InheritedResourceData(other.InheritedResourceData), StopData(other.StopData),
        MiscData(other.MiscData), LayoutData(other.LayoutData)
  {
    krys_debug_assert_msg(other == *this, "SVGRenderStyle should be properly copied.");
  }

  Ref<SVGRenderStyle> SVGRenderStyle::Copy() const noexcept
  {
    return AdoptRef(*new SVGRenderStyle(*this));
  }

  SVGRenderStyle::~SVGRenderStyle() noexcept = default;

  bool SVGRenderStyle::operator==(const SVGRenderStyle &other) const noexcept
  {
    return InheritedEqual(other) && NonInheritedEqual(other);
  }

  void SVGRenderStyle::SetBitDefaults() noexcept
  {
    InheritedFlags.ClipRule = static_cast<unsigned>(style::ComputedStyle::InitialClipRule());
    InheritedFlags.FillRule = static_cast<unsigned>(style::ComputedStyle::InitialFillRule());
    InheritedFlags.ShapeRendering = static_cast<unsigned>(style::ComputedStyle::InitialShapeRendering());
    InheritedFlags.TextAnchor = static_cast<unsigned>(style::ComputedStyle::InitialTextAnchor());
    InheritedFlags.ColorInterpolation =
      static_cast<unsigned>(style::ComputedStyle::InitialColorInterpolation());
    InheritedFlags.ColorInterpolationFilters =
      static_cast<unsigned>(style::ComputedStyle::InitialColorInterpolationFilters());
    InheritedFlags.GlyphOrientationHorizontal =
      static_cast<unsigned>(style::ComputedStyle::InitialGlyphOrientationHorizontal());
    InheritedFlags.GlyphOrientationVertical =
      static_cast<unsigned>(style::ComputedStyle::InitialGlyphOrientationVertical());

    NonInheritedFlags.AlignmentBaseline =
      static_cast<unsigned>(style::ComputedStyle::InitialAlignmentBaseline());
    NonInheritedFlags.DominantBaseline =
      static_cast<unsigned>(style::ComputedStyle::InitialDominantBaseline());
    NonInheritedFlags.VectorEffect = static_cast<unsigned>(style::ComputedStyle::InitialVectorEffect());
    NonInheritedFlags.MaskType = static_cast<unsigned>(style::ComputedStyle::InitialMaskType());
  }

  bool SVGRenderStyle::InheritedEqual(const SVGRenderStyle &other) const noexcept
  {
    return FillData == other.FillData && StrokeData == other.StrokeData
           && InheritedResourceData == other.InheritedResourceData && InheritedFlags == other.InheritedFlags;
  }

  bool SVGRenderStyle::NonInheritedEqual(const SVGRenderStyle &other) const noexcept
  {
    return StopData == other.StopData && MiscData == other.MiscData && LayoutData == other.LayoutData
           && NonInheritedFlags == other.NonInheritedFlags;
  }

  void SVGRenderStyle::InheritFrom(const SVGRenderStyle &other) noexcept
  {
    FillData = other.FillData;
    StrokeData = other.StrokeData;
    InheritedResourceData = other.InheritedResourceData;

    InheritedFlags = other.InheritedFlags;
  }

  void SVGRenderStyle::CopyNonInheritedFrom(const SVGRenderStyle &other) noexcept
  {
    NonInheritedFlags = other.NonInheritedFlags;
    StopData = other.StopData;
    MiscData = other.MiscData;
    LayoutData = other.LayoutData;
  }
}
