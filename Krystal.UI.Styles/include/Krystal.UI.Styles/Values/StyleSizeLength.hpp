#pragma once

#include "Krystal.Lib/ComparisonHelpers.hpp"
#include "Krystal.Lib/NullableFloat.hpp"
#include "Krystal.UI.Styles/Enums/Unit.hpp"
#include "Krystal.UI.Styles/Values/UnitValue.hpp"

namespace Krys::UI::Styles
{
  /// @brief Represents a CSS value for sizes (e.g. width, height, min-width, etc.). It may be one of:
  /// 1. Undefined
  /// 2. A keyword (e.g. auto, max-content, stretch, etc.)
  /// 3. A CSS <length-percentage> value:
  ///    a. <length> value (e.g. 10px)
  ///    b. <percentage> value of a reference <length>
  /// References:
  /// 1. https://www.w3.org/TR/css-values-4/#lengths
  /// 2. https://www.w3.org/TR/css-values-4/#percentage-value
  /// 3. https://www.w3.org/TR/css-values-4/#mixed-percentages
  class StyleSizeLength
  {
  private:
    NullableFloat _value {};
    Unit _unit {Unit::Undefined};

    /// @brief Private to avoid invalid combinations of value and unit.
    constexpr StyleSizeLength(NullableFloat value, Unit unit) noexcept : _value(value), _unit(unit)
    {
    }

  public:
    constexpr StyleSizeLength() noexcept = default;

    constexpr static StyleSizeLength Points(float value) noexcept
    {
      return Krys::IsUndefined(value) || IsInf(value) ? Undefined()
                                                      : StyleSizeLength {NullableFloat {value}, Unit::Point};
    }

    constexpr static StyleSizeLength Percent(float value) noexcept
    {
      return Krys::IsUndefined(value) || IsInf(value)
               ? Undefined()
               : StyleSizeLength {NullableFloat {value}, Unit::Percent};
    }

    constexpr static StyleSizeLength Auto() noexcept
    {
      return StyleSizeLength {{}, Unit::Auto};
    }

    constexpr static StyleSizeLength MaxContent() noexcept
    {
      return StyleSizeLength {{}, Unit::MaxContent};
    }

    constexpr static StyleSizeLength FitContent() noexcept
    {
      return StyleSizeLength {{}, Unit::FitContent};
    }

    constexpr static StyleSizeLength Stretch() noexcept
    {
      return StyleSizeLength {{}, Unit::Stretch};
    }

    constexpr static StyleSizeLength Undefined() noexcept
    {
      return StyleSizeLength {{}, Unit::Undefined};
    }

    constexpr bool IsAuto() const noexcept
    {
      return _unit == Unit::Auto;
    }

    constexpr bool IsMaxContent() const noexcept
    {
      return _unit == Unit::MaxContent;
    }

    constexpr bool IsFitContent() const noexcept
    {
      return _unit == Unit::FitContent;
    }

    constexpr bool IsStretch() const noexcept
    {
      return _unit == Unit::Stretch;
    }

    constexpr bool IsUndefined() const noexcept
    {
      return _unit == Unit::Undefined;
    }

    constexpr bool IsDefined() const noexcept
    {
      return !IsUndefined();
    }

    constexpr bool IsPoints() const noexcept
    {
      return _unit == Unit::Point;
    }

    constexpr bool IsPercent() const noexcept
    {
      return _unit == Unit::Percent;
    }

    constexpr NullableFloat Value() const
    {
      return _value;
    }

    constexpr NullableFloat Resolve(float referenceLength)
    {
      switch (_unit)
      {
        case Unit::Point:   return _value;
        case Unit::Percent: return NullableFloat {_value.Value() * referenceLength * 0.01f};
        default:            return NullableFloat {};
      }
    }

    explicit constexpr operator UnitValue() const
    {
      return UnitValue {_value.Value(), _unit};
    }

    constexpr bool operator==(const StyleSizeLength &rhs) const
    {
      return _value == rhs._value && _unit == rhs._unit;
    }

    constexpr bool InexactEquals(const StyleSizeLength &other) const
    {
      return _unit == other._unit && Krys::InexactEquals(_value, other._value);
    }
  };

  inline bool InexactEquals(const StyleSizeLength &a, const StyleSizeLength &b)
  {
    return a.InexactEquals(b);
  }
}