#include "Krystal.Booey/Rendering/Style/StyleMaskBorderData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  Ref<StyleMaskBorderData> StyleMaskBorderData::Copy() const noexcept
  {
    return AdoptRef(*new StyleMaskBorderData(*this));
  }

  StyleMaskBorderData::StyleMaskBorderData() noexcept
      : MaskBorder {
          style::ComputedStyle::InitialMaskBorderSource(), style::ComputedStyle::InitialMaskBorderSlice(),
          style::ComputedStyle::InitialMaskBorderWidth(), style::ComputedStyle::InitialMaskBorderOutset(),
          style::ComputedStyle::InitialMaskBorderRepeat()}
  {
  }

  inline StyleMaskBorderData::StyleMaskBorderData(const StyleMaskBorderData &other) noexcept
      : RefCounted<StyleMaskBorderData>(), MaskBorder {other.MaskBorder}
  {
  }

  bool StyleMaskBorderData::operator==(const StyleMaskBorderData &other) const noexcept
  {
    return MaskBorder == other.MaskBorder;
  }
}
