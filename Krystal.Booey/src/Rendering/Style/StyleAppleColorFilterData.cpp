#include "Krystal.Booey/Rendering/Style/StyleAppleColorFilterData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{
  StyleAppleColorFilterData::StyleAppleColorFilterData() noexcept
      : AppleColorFilter(style::ComputedStyle::InitialAppleColorFilter())
  {
  }

  StyleAppleColorFilterData::StyleAppleColorFilterData(const StyleAppleColorFilterData &other) noexcept
      : RefCounted<StyleAppleColorFilterData>(), AppleColorFilter(other.AppleColorFilter)
  {
  }

  Ref<StyleAppleColorFilterData> StyleAppleColorFilterData::Copy() const noexcept
  {
    return AdoptRef(*new StyleAppleColorFilterData(*this));
  }

  bool StyleAppleColorFilterData::operator==(const StyleAppleColorFilterData &other) const noexcept
  {
    return AppleColorFilter == other.AppleColorFilter;
  }
}
