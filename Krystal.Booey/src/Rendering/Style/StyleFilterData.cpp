#include "Krystal.Booey/Rendering/Style/StyleFilterData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{
  StyleFilterData::StyleFilterData() noexcept : Filter(style::ComputedStyle::InitialFilter())
  {
  }

  StyleFilterData::StyleFilterData(const StyleFilterData &other) noexcept
      : RefCounted<StyleFilterData>(), Filter(other.Filter)
  {
  }

  Ref<StyleFilterData> StyleFilterData::Copy() const noexcept
  {
    return AdoptRef(*new StyleFilterData(*this));
  }

  bool StyleFilterData::operator==(const StyleFilterData &other) const noexcept
  {
    return Filter == other.Filter;
  }
}
