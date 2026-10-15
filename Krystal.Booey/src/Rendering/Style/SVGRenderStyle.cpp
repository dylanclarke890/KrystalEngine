#include "Krystal.Booey/Rendering/Style/SVGRenderStyle.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

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
    NonInheritedFlags.BufferedRendering =
      static_cast<unsigned>(style::ComputedStyle::InitialBufferedRendering());
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
