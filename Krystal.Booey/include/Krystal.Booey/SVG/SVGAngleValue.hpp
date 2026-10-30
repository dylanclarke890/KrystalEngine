#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/Properties/SVGPropertyTraits.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  template <typename T>
  class ExceptionOr;
}

namespace krys::boo::svg
{
  class SVGAngleValue
  {
  public:
    enum Type : uint8
    {
      SVG_ANGLETYPE_UNKNOWN = 0,
      SVG_ANGLETYPE_UNSPECIFIED = 1,
      SVG_ANGLETYPE_DEG = 2,
      SVG_ANGLETYPE_RAD = 3,
      SVG_ANGLETYPE_GRAD = 4,
      SVG_ANGLETYPE_TURN = 5
    };

  private:
    Type _unitType {SVG_ANGLETYPE_UNSPECIFIED};
    float _valueInSpecifiedUnits {0};

  public:
    KRYS_NODISCARD Type UnitType() const noexcept
    {
      return _unitType;
    }

    void SetValue(float) noexcept;
    KRYS_NODISCARD float Value() const noexcept;

    void SetValueInSpecifiedUnits(float valueInSpecifiedUnits) noexcept
    {
      _valueInSpecifiedUnits = valueInSpecifiedUnits;
    }
    KRYS_NODISCARD float ValueInSpecifiedUnits() const noexcept
    {
      return _valueInSpecifiedUnits;
    }

    dom::ExceptionOr<void> SetValueAsString(dom::DOMStringView) noexcept;
    KRYS_NODISCARD dom::DOMString ValueAsString() const noexcept;

    dom::ExceptionOr<void> NewValueSpecifiedUnits(uint16 unitType, float valueInSpecifiedUnits) noexcept;
    dom::ExceptionOr<void> ConvertToSpecifiedUnits(uint16 unitType) noexcept;
  };
}
