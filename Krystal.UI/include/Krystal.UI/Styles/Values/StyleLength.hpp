#pragma once

#include "Krystal.Lib/Types/NullableFloat.hpp"
#include "Krystal.Lib/Types/Numeric.hpp"
#include "Krystal.UI/Styles/Enums/Unit.hpp"
#include "Krystal.UI/Styles/Values/UnitValue.hpp"

namespace Krys::UI
{
  /// @brief Represents a CSS Value which may be one of:
  /// 1. Undefined
  /// 2. A keyword (e.g. auto)
  /// 3. A CSS <length-percentage> value:
  ///    a. <length> value (e.g. 10px)
  ///    b. <percentage> value of a reference <length>
  /// References:
  /// 1. https://www.w3.org/TR/css-values-4/#lengths
  /// 2. https://www.w3.org/TR/css-values-4/#percentage-value
  /// 3. https://www.w3.org/TR/css-values-4/#mixed-percentages
  class StyleLength
  {
  private:
    NullableFloat _value {};
    Unit _unit {Unit::Undefined};

    /// @brief Private to avoid invalid combinations of value and unit.
    constexpr StyleLength(NullableFloat value, Unit unit) : _value(value), _unit(unit)
    {
    }

  public:
    constexpr StyleLength() = default;

    constexpr static StyleLength Points(float value) noexcept
    {
      return Krys::IsUndefined(value) || Krys::IsInf(value)
               ? Undefined()
               : StyleLength {NullableFloat {value}, Unit::Point};
    }

    constexpr static StyleLength Percent(float value) noexcept
    {
      return Krys::IsUndefined(value) || Krys::IsInf(value)
               ? Undefined()
               : StyleLength {NullableFloat {value}, Unit::Percent};
    }

    constexpr static StyleLength Auto() noexcept
    {
      return StyleLength {{}, Unit::Auto};
    }

    constexpr static StyleLength Undefined() noexcept
    {
      return StyleLength {{}, Unit::Undefined};
    }

    constexpr bool IsAuto() const noexcept
    {
      return _unit == Unit::Auto;
    }

    constexpr bool IsUndefined() const noexcept
    {
      return _unit == Unit::Undefined;
    }

    constexpr bool IsPoints() const noexcept
    {
      return _unit == Unit::Point;
    }

    constexpr bool IsPercent() const noexcept
    {
      return _unit == Unit::Percent;
    }

    constexpr bool IsDefined() const noexcept
    {
      return !IsUndefined();
    }

    constexpr NullableFloat Value() const noexcept
    {
      return _value;
    }

    constexpr NullableFloat Resolve(float referenceLength) const noexcept
    {
      switch (_unit)
      {
        case Unit::Point:   return _value;
        case Unit::Percent: return NullableFloat {_value.Value() * referenceLength * 0.01f};
        default:            return NullableFloat {};
      }
    }

    explicit constexpr operator UnitValue() const noexcept
    {
      return UnitValue {_value.Value(), _unit};
    }

    constexpr bool operator==(const StyleLength &rhs) const noexcept
    {
      return _value == rhs._value && _unit == rhs._unit;
    }

    constexpr bool InexactEquals(const StyleLength &other) const noexcept
    {
      return _unit == other._unit && Krys::InexactEquals(_value, other._value);
    }
  };

  inline bool InexactEquals(const StyleLength &a, const StyleLength &b) noexcept
  {
    return a.InexactEquals(b);
  }
}