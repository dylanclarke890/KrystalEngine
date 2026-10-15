#include "Krystal.Booey/Rendering/Style/StyleVisitedLinkColorData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleVisitedLinkColorData::StyleVisitedLinkColorData() noexcept
      : VisitedLinkBackgroundColor(style::ComputedStyle::InitialBackgroundColor())
  // VisitedLinkBorderColors
  // VisitedLinkTextDecorationColor
  // VisitedLinkOutlineColor
  {
  }

  StyleVisitedLinkColorData::StyleVisitedLinkColorData(const StyleVisitedLinkColorData &o) noexcept
      : RefCounted<StyleVisitedLinkColorData>(), VisitedLinkBackgroundColor(o.VisitedLinkBackgroundColor),
        VisitedLinkBorderColors(o.VisitedLinkBorderColors),
        VisitedLinkTextDecorationColor(o.VisitedLinkTextDecorationColor),
        VisitedLinkOutlineColor(o.VisitedLinkOutlineColor)
  {
  }

  StyleVisitedLinkColorData::~StyleVisitedLinkColorData() noexcept = default;

  Ref<StyleVisitedLinkColorData> StyleVisitedLinkColorData::Copy() const noexcept
  {
    return AdoptRef(*new StyleVisitedLinkColorData(*this));
  }

  bool StyleVisitedLinkColorData::operator==(const StyleVisitedLinkColorData &o) const noexcept
  {
    return VisitedLinkBackgroundColor == o.VisitedLinkBackgroundColor
           && VisitedLinkBorderColors == o.VisitedLinkBorderColors
           && VisitedLinkTextDecorationColor == o.VisitedLinkTextDecorationColor
           && VisitedLinkOutlineColor == o.VisitedLinkOutlineColor;
  }
}
