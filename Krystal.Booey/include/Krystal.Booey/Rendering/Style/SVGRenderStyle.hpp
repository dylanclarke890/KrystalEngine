/*
    Copyright (C) 2004, 2005, 2007 Nikolas Zimmermann <zimmermann@kde.org>
                  2004, 2005 Rob Buis <buis@kde.org>
    Copyright (C) 2005-2017 Apple Inc. All rights reserved.
    Copyright (C) Research In Motion Limited 2010. All rights reserved.
    Copyright (C) 2014 Adobe Systems Incorporated. All rights reserved.
    Copyright (C) 2025 Samuel Weinig <sam@webkit.org>

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

#include "Krystal.Booey/Graphics/Paths/WindRule.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/SVGRenderStyleDefs.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGGlyphOrientationHorizontal.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGGlyphOrientationVertical.hpp"

namespace krys::boo
{
  class SVGRenderStyle : public RefCounted<SVGRenderStyle>
  {
  public:
    KRYS_NODISCARD static Ref<SVGRenderStyle> CreateDefaultStyle() noexcept;
    KRYS_NODISCARD static Ref<SVGRenderStyle> Create() noexcept
    {
      return AdoptRef(*new SVGRenderStyle);
    }
    KRYS_NODISCARD Ref<SVGRenderStyle> Copy() const noexcept;
    ~SVGRenderStyle() noexcept;

    KRYS_NODISCARD bool InheritedEqual(const SVGRenderStyle &) const noexcept;
    KRYS_NODISCARD bool NonInheritedEqual(const SVGRenderStyle &) const noexcept;

    void InheritFrom(const SVGRenderStyle &) noexcept;
    void CopyNonInheritedFrom(const SVGRenderStyle &) noexcept;

    bool operator==(const SVGRenderStyle &) const noexcept;

    struct InheritedFlags
    {
      KRYS_NODISCARD bool operator==(const InheritedFlags &) const noexcept = default;

      unsigned ShapeRendering : 2;
      unsigned ClipRule : 1;
      unsigned FillRule : 1;
      unsigned TextAnchor : 2;
      unsigned ColorInterpolation : 2;
      unsigned ColorInterpolationFilters : 2;
      unsigned GlyphOrientationHorizontal : 2;
      unsigned GlyphOrientationVertical : 3;
    };

    struct NonInheritedFlags
    {
      KRYS_NODISCARD bool operator==(const NonInheritedFlags &) const noexcept = default;

      unsigned AlignmentBaseline : 4;
      unsigned DominantBaseline : 4;
      unsigned VectorEffect : 1;
      unsigned MaskType : 1;
    };

    InheritedFlags InheritedFlags;
    NonInheritedFlags NonInheritedFlags;

    // inherited attributes
    DataRef<StyleFillData> FillData;
    DataRef<StyleStrokeData> StrokeData;
    DataRef<StyleInheritedResourceData> InheritedResourceData;

    // non-inherited attributes
    DataRef<StyleStopData> StopData;
    DataRef<StyleMiscData> MiscData;
    DataRef<StyleLayoutData> LayoutData;

  private:
    SVGRenderStyle() noexcept;
    SVGRenderStyle(const SVGRenderStyle &) noexcept;

    enum CreateDefaultType
    {
      CreateDefault
    };
    SVGRenderStyle(CreateDefaultType) noexcept; // Used to create the default style.

    void SetBitDefaults() noexcept;
  };
}
