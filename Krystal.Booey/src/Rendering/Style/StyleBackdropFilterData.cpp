#include "Krystal.Booey/Rendering/Style/StyleBackdropFilterData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{
  StyleBackdropFilterData::StyleBackdropFilterData() noexcept
      : BackdropFilter(style::ComputedStyle::InitialBackdropFilter())
  {
  }

  StyleBackdropFilterData::StyleBackdropFilterData(const StyleBackdropFilterData &other) noexcept
      : RefCounted<StyleBackdropFilterData>(), BackdropFilter(other.BackdropFilter)
  {
  }

  Ref<StyleBackdropFilterData> StyleBackdropFilterData::Copy() const noexcept
  {
    return AdoptRef(*new StyleBackdropFilterData(*this));
  }

  bool StyleBackdropFilterData::operator==(const StyleBackdropFilterData &other) const noexcept
  {
    return BackdropFilter == other.BackdropFilter;
  }
}
