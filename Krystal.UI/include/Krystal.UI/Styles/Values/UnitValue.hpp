#pragma once

#include "Krystal.Core/Numeric.hpp"
#include "Krystal.UI/Styles/Enums/Unit.hpp"

namespace krys::UI
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