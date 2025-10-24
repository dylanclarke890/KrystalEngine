#pragma once

#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI.Styles/Enums/Unit.hpp"

namespace Krys::UI::Styles
{
  struct UnitValue
  {
    float Value;
    Unit Type;

    bool operator==(const UnitValue &rhs) const noexcept;
    bool operator!=(const UnitValue &rhs) const noexcept;
    UnitValue operator-() const noexcept;

    static const UnitValue Auto;
    static const UnitValue Undefined;
    static const UnitValue Zero;
  };
}