#pragma once

#include "Krystal.Booey/Graphics/WindRule.hpp"
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
      unsigned BufferedRendering : 2;
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
