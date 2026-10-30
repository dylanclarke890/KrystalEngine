#include "Krystal.Booey/Rendering/Style/StyleFlexibleBoxData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleFlexibleBoxData::StyleFlexibleBoxData() noexcept
      : FlexGrow(style::ComputedStyle::InitialFlexGrow()),
        FlexShrink(style::ComputedStyle::InitialFlexShrink()),
        FlexBasis(style::ComputedStyle::InitialFlexBasis()),
        FlexDirection(static_cast<unsigned>(style::ComputedStyle::InitialFlexDirection())),
        FlexWrap(static_cast<unsigned>(style::ComputedStyle::InitialFlexWrap()))
  {
  }

  inline StyleFlexibleBoxData::StyleFlexibleBoxData(const StyleFlexibleBoxData &other) noexcept
      : RefCounted<StyleFlexibleBoxData>(), FlexGrow(other.FlexGrow), FlexShrink(other.FlexShrink),
        FlexBasis(other.FlexBasis), FlexDirection(other.FlexDirection), FlexWrap(other.FlexWrap)
  {
  }

  Ref<StyleFlexibleBoxData> StyleFlexibleBoxData::Copy() const noexcept
  {
    return AdoptRef(*new StyleFlexibleBoxData(*this));
  }

  bool StyleFlexibleBoxData::operator==(const StyleFlexibleBoxData &other) const noexcept
  {
    return FlexGrow == other.FlexGrow && FlexShrink == other.FlexShrink && FlexBasis == other.FlexBasis
           && FlexDirection == other.FlexDirection && FlexWrap == other.FlexWrap;
  }
}
