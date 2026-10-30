#include "Krystal.Booey/Rendering/Style/StyleInheritedData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleInheritedData::StyleInheritedData() noexcept
      : BorderHorizontalSpacing(style::ComputedStyle::InitialBorderHorizontalSpacing()),
        BorderVerticalSpacing(style::ComputedStyle::InitialBorderVerticalSpacing()),
        LineHeight(style::ComputedStyle::InitialLineHeight()), FontData(StyleFontData::Create()),
        Color(style::ComputedStyle::InitialColor()), VisitedLinkColor(style::ComputedStyle::InitialColor())
  {
  }

  inline StyleInheritedData::StyleInheritedData(const StyleInheritedData &o) noexcept
      : RefCounted<StyleInheritedData>(), BorderHorizontalSpacing(o.BorderHorizontalSpacing),
        BorderVerticalSpacing(o.BorderVerticalSpacing), LineHeight(o.LineHeight), FontData(o.FontData),
        Color(o.Color), VisitedLinkColor(o.VisitedLinkColor)
  {
    krys_debug_assert_msg(o == *this, "StyleInheritedData should be properly copied.");
  }

  Ref<StyleInheritedData> StyleInheritedData::Copy() const noexcept
  {
    return AdoptRef(*new StyleInheritedData(*this));
  }

  bool StyleInheritedData::operator==(const StyleInheritedData &other) const noexcept
  {
    return FastPathInheritedEqual(other) && NonFastPathInheritedEqual(other);
  }

  bool StyleInheritedData::FastPathInheritedEqual(const StyleInheritedData &other) const noexcept
  {
    // These properties also need to have "fast-path-inherited" codegen property set.
    // Cases where other properties depend on these values need to disallow the fast path (via
    // RenderStyle::setDisallowsFastPathInheritance).
    return Color == other.Color && VisitedLinkColor == other.VisitedLinkColor;
  }

  bool StyleInheritedData::NonFastPathInheritedEqual(const StyleInheritedData &other) const noexcept
  {
    return LineHeight == other.LineHeight && FontData == other.FontData
           && BorderHorizontalSpacing == other.BorderHorizontalSpacing
           && BorderVerticalSpacing == other.BorderVerticalSpacing;
  }

  void StyleInheritedData::FastPathInheritFrom(const StyleInheritedData &inheritParent) noexcept
  {
    Color = inheritParent.Color;
    VisitedLinkColor = inheritParent.VisitedLinkColor;
  }
}
