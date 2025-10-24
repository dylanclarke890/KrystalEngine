#include "Krystal.UI.Styles/Values/UnitValue.hpp"

namespace Krys::UI::Styles
{
  const UnitValue UnitValue::Auto {0.0f, Unit::Auto};
  const UnitValue UnitValue::Undefined {0.0f, Unit::Undefined};
  const UnitValue UnitValue::Zero {0.0f, Unit::Point};

  bool UnitValue::operator==(const UnitValue &rhs) const noexcept
  {
    if (Type != rhs.Type)
    {
      return false;
    }

    switch (Type)
    {
      case Unit::Undefined:
      case Unit::Auto:
      case Unit::FitContent:
      case Unit::MaxContent:
      case Unit::Stretch:    return true;
      case Unit::Point:
      case Unit::Percent:    return Value == rhs.Value;
      default:               return false;
    }
  }

  bool UnitValue::operator!=(const UnitValue &rhs) const noexcept
  {
    return !(*this == rhs);
  }

  UnitValue UnitValue::operator-() const noexcept
  {
    return {-Value, Type};
  }
}