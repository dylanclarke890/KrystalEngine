#include "Krystal.Booey/Rendering/Style/StyleGridItemData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleGridItemData::StyleGridItemData() noexcept
      : GridItemColumnStart(style::ComputedStyle::InitialGridItemColumnStart()),
        GridItemColumnEnd(style::ComputedStyle::InitialGridItemColumnEnd()),
        GridItemRowStart(style::ComputedStyle::InitialGridItemRowStart()),
        GridItemRowEnd(style::ComputedStyle::InitialGridItemRowEnd())
  {
  }

  inline StyleGridItemData::StyleGridItemData(const StyleGridItemData &o) noexcept
      : RefCounted<StyleGridItemData>(), GridItemColumnStart(o.GridItemColumnStart),
        GridItemColumnEnd(o.GridItemColumnEnd), GridItemRowStart(o.GridItemRowStart),
        GridItemRowEnd(o.GridItemRowEnd)
  {
  }

  Ref<StyleGridItemData> StyleGridItemData::Copy() const noexcept
  {
    return AdoptRef(*new StyleGridItemData(*this));
  }
}
