#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/Markable.hpp"
#include <cmath>
#include <compare>

namespace krys
{
  class TextStream;
}

namespace krys::boo
{
  constexpr int32 LayoutUnitFractionalBits = 6;
  constexpr int32 FixedPointDenominator = 1 << LayoutUnitFractionalBits;
  constexpr int32 IntMaxForLayoutUnit = Limits<int32>::max() / FixedPointDenominator;
  constexpr int32 IntMinForLayoutUnit = Limits<int32>::min() / FixedPointDenominator;

  /// @brief A fixed-point number with 6 fractional bits (1/64th of a pixel), used for layout. Arithmetic
  /// saturates at `Min()`/`Max()` instead of overflowing.
  class LayoutUnit
  {
  private:
    int32 _value {0};

  public:
    constexpr LayoutUnit() noexcept = default;
    constexpr LayoutUnit(const LayoutUnit &) noexcept = default;

    constexpr LayoutUnit(int32 value) noexcept
    {
      SetValue(value);
    }

    constexpr LayoutUnit(uint16 value) noexcept
    {
      SetValue(static_cast<uint32>(value));
    }

    constexpr LayoutUnit(uint32 value) noexcept
    {
      SetValue(value);
    }

    constexpr explicit LayoutUnit(uint64 value) noexcept
    {
      if (value > static_cast<uint64>(IntMaxForLayoutUnit))
      {
        _value = Limits<int32>::max();
      }
      else
      {
        _value = static_cast<int32>(value * FixedPointDenominator);
      }
    }

    constexpr explicit LayoutUnit(double value) noexcept
        : _value(ClampToInteger(value * FixedPointDenominator))
    {
    }

    constexpr LayoutUnit &operator=(const LayoutUnit &) noexcept = default;

    constexpr LayoutUnit &operator=(const float &other) noexcept
    {
      return *this = LayoutUnit(other);
    }

    // Without this, assigning an integer would convert it to float and lose precision.
    constexpr LayoutUnit &operator=(int32 other) noexcept
    {
      return *this = LayoutUnit(other);
    }

    KRYS_NODISCARD friend constexpr bool operator==(LayoutUnit, LayoutUnit) noexcept = default;
    KRYS_NODISCARD friend constexpr auto operator<=>(LayoutUnit, LayoutUnit) noexcept = default;

    KRYS_NODISCARD static LayoutUnit FromFloatCeil(float value) noexcept
    {
      return FromRawValue(ClampToInteger(std::ceil(value * FixedPointDenominator)));
    }

    KRYS_NODISCARD static LayoutUnit FromFloatFloor(float value) noexcept
    {
      return FromRawValue(ClampToInteger(std::floor(value * FixedPointDenominator)));
    }

    KRYS_NODISCARD constexpr static LayoutUnit FromFloatRound(float value) noexcept
    {
      if (value >= 0)
      {
        return Clamp(value + Epsilon() / 2.0f);
      }

      return Clamp(value - Epsilon() / 2.0f);
    }

    KRYS_NODISCARD constexpr static LayoutUnit FromRawValue(int32 value) noexcept
    {
      LayoutUnit v;
      v._value = value;
      return v;
    }

    KRYS_NODISCARD constexpr int32 ToInt() const noexcept
    {
      return _value / FixedPointDenominator;
    }

    KRYS_NODISCARD constexpr float ToFloat() const noexcept
    {
      return static_cast<float>(_value) / FixedPointDenominator;
    }

    KRYS_NODISCARD constexpr double ToDouble() const noexcept
    {
      return static_cast<double>(_value) / FixedPointDenominator;
    }

    KRYS_NODISCARD constexpr uint32 ToUnsigned() const noexcept
    {
      return static_cast<uint32>(ToInt());
    }

    constexpr operator int32() const noexcept
    {
      return ToInt();
    }

    constexpr operator float() const noexcept
    {
      return ToFloat();
    }

    constexpr operator double() const noexcept
    {
      return ToDouble();
    }

    constexpr explicit operator bool() const noexcept
    {
      return _value != 0;
    }

    constexpr LayoutUnit &operator++() noexcept
    {
      _value = SaturatedSum<int32>(_value, FixedPointDenominator);
      return *this;
    }

    KRYS_NODISCARD constexpr int32 RawValue() const noexcept
    {
      return _value;
    }

    constexpr void SetRawValue(int32 value) noexcept
    {
      _value = value;
    }

    constexpr void SetRawValue(int64 value) noexcept
    {
      _value = static_cast<int32>(value);
    }

    KRYS_NODISCARD constexpr LayoutUnit Abs() const noexcept
    {
      if (_value == Limits<int32>::min())
      {
        return Max();
      }

      return FromRawValue(_value < 0 ? -_value : _value);
    }

    KRYS_NODISCARD constexpr int32 Ceil() const noexcept
    {
      if (_value >= Limits<int32>::max() - FixedPointDenominator + 1) KRYS_UNLIKELY
      {
        return IntMaxForLayoutUnit;
      }

      if (_value >= 0)
      {
        return (_value + FixedPointDenominator - 1) / FixedPointDenominator;
      }

      return ToInt();
    }

    KRYS_NODISCARD constexpr int32 Round() const noexcept
    {
      return ToInt() + ((Fraction().RawValue() + (FixedPointDenominator / 2)) >> LayoutUnitFractionalBits);
    }

    KRYS_NODISCARD constexpr int32 Floor() const noexcept
    {
      if (_value <= Limits<int32>::min() + FixedPointDenominator - 1) KRYS_UNLIKELY
      {
        return IntMinForLayoutUnit;
      }

      return _value >> LayoutUnitFractionalBits;
    }

    KRYS_NODISCARD float CeilToFloat() const noexcept
    {
      float floatValue = ToFloat();
      if (static_cast<int32>(floatValue * FixedPointDenominator) == _value)
      {
        return floatValue;
      }

      if (floatValue > 0)
      {
        return std::nextafter(floatValue, Limits<float>::max());
      }

      return std::nextafter(floatValue, Limits<float>::min());
    }

    KRYS_NODISCARD constexpr LayoutUnit Fraction() const noexcept
    {
      // Add the fraction to the size (as opposed to the full location) to avoid overflows.
      // Compute fraction using the mod operator to preserve the sign of the value as it may affect rounding.
      return FromRawValue(RawValue() % FixedPointDenominator);
    }

    KRYS_NODISCARD constexpr bool MightBeSaturated() const noexcept
    {
      return RawValue() == Limits<int32>::max() || RawValue() == Limits<int32>::min();
    }

    KRYS_NODISCARD constexpr static float Epsilon() noexcept
    {
      return 1.0f / FixedPointDenominator;
    }

    KRYS_NODISCARD constexpr static LayoutUnit Max() noexcept
    {
      return FromRawValue(Limits<int32>::max());
    }

    KRYS_NODISCARD constexpr static LayoutUnit Min() noexcept
    {
      return FromRawValue(Limits<int32>::min());
    }

    // Versions of max/min that are slightly smaller/larger than max/min() to allow for rounding without
    // overflowing.
    KRYS_NODISCARD constexpr static LayoutUnit NearlyMax() noexcept
    {
      return FromRawValue(Limits<int32>::max() - FixedPointDenominator / 2);
    }

    KRYS_NODISCARD constexpr static LayoutUnit NearlyMin() noexcept
    {
      return FromRawValue(Limits<int32>::min() + FixedPointDenominator / 2);
    }

    KRYS_NODISCARD constexpr static LayoutUnit Clamp(double value) noexcept
    {
      return ClampTo<LayoutUnit>(value, Min(), Max());
    }

  private:
    constexpr void SetValue(int32 value) noexcept
    {
      if (value > IntMaxForLayoutUnit)
      {
        _value = Limits<int32>::max();
      }
      else if (value < IntMinForLayoutUnit)
      {
        _value = Limits<int32>::min();
      }
      else
      {
        _value = value * FixedPointDenominator;
      }
    }

    constexpr void SetValue(uint32 value) noexcept
    {
      if (value >= static_cast<uint32>(IntMaxForLayoutUnit))
      {
        _value = Limits<int32>::max();
      }
      else
      {
        _value = static_cast<int32>(value) * FixedPointDenominator;
      }
    }
  };

  KRYS_NODISCARD constexpr bool operator==(const LayoutUnit &a, int32 b) noexcept
  {
    return a == LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr auto operator<=>(const LayoutUnit &a, int32 b) noexcept
  {
    return a <=> LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr bool operator==(const LayoutUnit &a, float b) noexcept
  {
    return a.ToFloat() == b;
  }

  KRYS_NODISCARD constexpr auto operator<=>(const LayoutUnit &a, float b) noexcept
  {
    return a.ToFloat() <=> b;
  }

  KRYS_NODISCARD constexpr auto operator<=>(const LayoutUnit &a, double b) noexcept
  {
    return a.ToDouble() <=> b;
  }

  // For multiplication that's prone to overflow, this bounds it to LayoutUnit::Max() and ::Min()
  KRYS_NODISCARD constexpr LayoutUnit BoundedMultiply(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    int64 result =
      static_cast<int64>(a.RawValue()) * static_cast<int64>(b.RawValue()) / FixedPointDenominator;
    int32 high = static_cast<int32>(result >> 32);
    int32 low = static_cast<int32>(result);
    uint32 saturated =
      (static_cast<uint32>(a.RawValue() ^ b.RawValue()) >> 31) + static_cast<uint32>(Limits<int32>::max());
    // If the higher 32 bits does not match the lower 32 with sign extension the operation overflowed.
    if (high != low >> 31)
    {
      result = saturated;
    }

    return LayoutUnit::FromRawValue(static_cast<int32>(result));
  }

#pragma region Multiplication

  KRYS_NODISCARD constexpr LayoutUnit operator*(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    return BoundedMultiply(a, b);
  }

  KRYS_NODISCARD constexpr double operator*(const LayoutUnit &a, double b) noexcept
  {
    return a.ToDouble() * b;
  }

  KRYS_NODISCARD constexpr float operator*(const LayoutUnit &a, float b) noexcept
  {
    return a.ToFloat() * b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(const LayoutUnit &a, int32 b) noexcept
  {
    return a * LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(const LayoutUnit &a, uint16 b) noexcept
  {
    return a * LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(const LayoutUnit &a, uint32 b) noexcept
  {
    return a * LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(const LayoutUnit &a, uint64 b) noexcept
  {
    return a * LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(uint16 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) * b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(uint32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) * b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(uint64 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) * b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator*(const int32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) * b;
  }

  KRYS_NODISCARD constexpr float operator*(const float a, const LayoutUnit &b) noexcept
  {
    return a * b.ToFloat();
  }

  KRYS_NODISCARD constexpr double operator*(const double a, const LayoutUnit &b) noexcept
  {
    return a * b.ToDouble();
  }

#pragma endregion

#pragma region Division

  KRYS_NODISCARD constexpr LayoutUnit operator/(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    int64 rawVal = static_cast<int64>(FixedPointDenominator) * a.RawValue() / b.RawValue();
    return LayoutUnit::FromRawValue(ClampTo<int32>(rawVal));
  }

  KRYS_NODISCARD constexpr float operator/(const LayoutUnit &a, float b) noexcept
  {
    return a.ToFloat() / b;
  }

  KRYS_NODISCARD constexpr double operator/(const LayoutUnit &a, double b) noexcept
  {
    return a.ToDouble() / b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(const LayoutUnit &a, int32 b) noexcept
  {
    return a / LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(const LayoutUnit &a, uint16 b) noexcept
  {
    return a / LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(const LayoutUnit &a, uint32 b) noexcept
  {
    return a / LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(const LayoutUnit &a, uint64 b) noexcept
  {
    return a / LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr float operator/(const float a, const LayoutUnit &b) noexcept
  {
    return a / b.ToFloat();
  }

  KRYS_NODISCARD constexpr double operator/(const double a, const LayoutUnit &b) noexcept
  {
    return a / b.ToDouble();
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(const int32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) / b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(uint16 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) / b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(uint32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) / b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator/(uint64 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) / b;
  }

#pragma endregion

#pragma region Addition and Subtraction

  KRYS_NODISCARD constexpr LayoutUnit operator+(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit::FromRawValue(SaturatedSum<int32>(a.RawValue(), b.RawValue()));
  }

  KRYS_NODISCARD constexpr LayoutUnit operator+(const LayoutUnit &a, int32 b) noexcept
  {
    return a + LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr float operator+(const LayoutUnit &a, float b) noexcept
  {
    return a.ToFloat() + b;
  }

  KRYS_NODISCARD constexpr double operator+(const LayoutUnit &a, double b) noexcept
  {
    return a.ToDouble() + b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator+(const int32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) + b;
  }

  KRYS_NODISCARD constexpr float operator+(const float a, const LayoutUnit &b) noexcept
  {
    return a + b.ToFloat();
  }

  KRYS_NODISCARD constexpr double operator+(const double a, const LayoutUnit &b) noexcept
  {
    return a + b.ToDouble();
  }

  KRYS_NODISCARD constexpr LayoutUnit operator-(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit::FromRawValue(SaturatedDifference<int32>(a.RawValue(), b.RawValue()));
  }

  KRYS_NODISCARD constexpr LayoutUnit operator-(const LayoutUnit &a, int32 b) noexcept
  {
    return a - LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator-(const LayoutUnit &a, uint32 b) noexcept
  {
    return a - LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr float operator-(const LayoutUnit &a, float b) noexcept
  {
    return a.ToFloat() - b;
  }

  KRYS_NODISCARD constexpr LayoutUnit operator-(const int32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) - b;
  }

  KRYS_NODISCARD constexpr float operator-(const float a, const LayoutUnit &b) noexcept
  {
    return a - b.ToFloat();
  }

  KRYS_NODISCARD constexpr LayoutUnit operator-(const LayoutUnit &a) noexcept
  {
    // -Min() is saturated to Max().
    if (a == LayoutUnit::Min())
    {
      return LayoutUnit::Max();
    }

    return LayoutUnit::FromRawValue(-a.RawValue());
  }

#pragma endregion

#pragma region Modulo

  /// @brief For returning the remainder after a division with integer results.
  KRYS_NODISCARD constexpr LayoutUnit IntMod(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    // This calculates the modulo so that: a = static_cast<int32>(a / b) * b + IntMod(a, b).
    return LayoutUnit::FromRawValue(a.RawValue() % b.RawValue());
  }

  KRYS_NODISCARD constexpr LayoutUnit operator%(const LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    // This calculates the modulo so that: a = (a / b) * b + a % b.
    int64 rawVal = (static_cast<int64>(FixedPointDenominator) * a.RawValue()) % b.RawValue();
    return LayoutUnit::FromRawValue(static_cast<int32>(rawVal / FixedPointDenominator));
  }

  KRYS_NODISCARD constexpr LayoutUnit operator%(const LayoutUnit &a, int32 b) noexcept
  {
    return a % LayoutUnit(b);
  }

  KRYS_NODISCARD constexpr LayoutUnit operator%(int32 a, const LayoutUnit &b) noexcept
  {
    return LayoutUnit(a) % b;
  }

#pragma endregion

#pragma region Compound Assignment

  constexpr LayoutUnit &operator+=(LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    a.SetRawValue(SaturatedSum<int32>(a.RawValue(), b.RawValue()));
    return a;
  }

  constexpr LayoutUnit &operator+=(LayoutUnit &a, int32 b) noexcept
  {
    a = a + b;
    return a;
  }

  constexpr LayoutUnit &operator+=(LayoutUnit &a, float b) noexcept
  {
    a = a + b;
    return a;
  }

  constexpr float &operator+=(float &a, const LayoutUnit &b) noexcept
  {
    a = a + b;
    return a;
  }

  constexpr LayoutUnit &operator-=(LayoutUnit &a, int32 b) noexcept
  {
    a = a - b;
    return a;
  }

  constexpr LayoutUnit &operator-=(LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    a.SetRawValue(SaturatedDifference<int32>(a.RawValue(), b.RawValue()));
    return a;
  }

  constexpr LayoutUnit &operator-=(LayoutUnit &a, float b) noexcept
  {
    a = a - b;
    return a;
  }

  constexpr float &operator-=(float &a, const LayoutUnit &b) noexcept
  {
    a = a - b;
    return a;
  }

  constexpr LayoutUnit &operator*=(LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    a = a * b;
    return a;
  }

  // operator*=(LayoutUnit &a, int32 b) is supported by the operator above plus LayoutUnit(int32).

  constexpr LayoutUnit &operator*=(LayoutUnit &a, float b) noexcept
  {
    a = a * b;
    return a;
  }

  constexpr float &operator*=(float &a, const LayoutUnit &b) noexcept
  {
    a = a * b;
    return a;
  }

  constexpr LayoutUnit &operator/=(LayoutUnit &a, const LayoutUnit &b) noexcept
  {
    a = a / b;
    return a;
  }

  // operator/=(LayoutUnit &a, int32 b) is supported by the operator above plus LayoutUnit(int32).

  constexpr LayoutUnit &operator/=(LayoutUnit &a, float b) noexcept
  {
    a = a / b;
    return a;
  }

  constexpr float &operator/=(float &a, const LayoutUnit &b) noexcept
  {
    a = a / b;
    return a;
  }

#pragma endregion

#pragma region Rounding

  KRYS_NODISCARD constexpr int32 RoundToInt(LayoutUnit value) noexcept
  {
    return value.Round();
  }

  KRYS_NODISCARD constexpr int32 FloorToInt(LayoutUnit value) noexcept
  {
    return value.Floor();
  }

  KRYS_NODISCARD inline float RoundToDevicePixel(LayoutUnit value, float pixelSnappingFactor,
                                                 bool needsDirectionalRounding = false) noexcept
  {
    double valueToRound = value.ToDouble();
    if (needsDirectionalRounding)
    {
      valueToRound -= LayoutUnit::Epsilon() / (2 * FixedPointDenominator);
    }

    if (valueToRound >= 0)
    {
      return static_cast<float>(std::round(valueToRound * pixelSnappingFactor) / pixelSnappingFactor);
    }

    // This adjusts directional rounding on negative halfway values. It produces the same direction for both
    // negative and positive values. Instead of rounding negative halfway cases away from zero, we translate
    // them to positive values before rounding. It helps snapping relative negative coordinates to the same
    // position as if they were positive absolute coordinates.
    uint32 translateOrigin = ~static_cast<uint32>(value.RawValue()) + 1u;
    return static_cast<float>(
      (std::round((valueToRound + translateOrigin) * pixelSnappingFactor) / pixelSnappingFactor)
      - translateOrigin);
  }

  KRYS_NODISCARD inline float FloorToDevicePixel(LayoutUnit value, float pixelSnappingFactor) noexcept
  {
    return std::floor((value.RawValue() * pixelSnappingFactor) / FixedPointDenominator) / pixelSnappingFactor;
  }

  KRYS_NODISCARD inline float CeilToDevicePixel(LayoutUnit value, float pixelSnappingFactor) noexcept
  {
    return std::ceil((value.RawValue() * pixelSnappingFactor) / FixedPointDenominator) / pixelSnappingFactor;
  }

  KRYS_NODISCARD constexpr int32 RoundToInt(float value) noexcept
  {
    return RoundToInt(LayoutUnit(value));
  }

  KRYS_NODISCARD inline float RoundToDevicePixel(float value, float pixelSnappingFactor,
                                                 bool needsDirectionalRounding = false) noexcept
  {
    return RoundToDevicePixel(LayoutUnit(value), pixelSnappingFactor, needsDirectionalRounding);
  }

  KRYS_NODISCARD inline float FloorToDevicePixel(float value, float pixelSnappingFactor) noexcept
  {
    return FloorToDevicePixel(LayoutUnit(value), pixelSnappingFactor);
  }

  KRYS_NODISCARD inline float CeilToDevicePixel(float value, float pixelSnappingFactor) noexcept
  {
    return CeilToDevicePixel(LayoutUnit(value), pixelSnappingFactor);
  }

#pragma endregion

  KRYS_NODISCARD constexpr LayoutUnit AbsoluteValue(const LayoutUnit &value) noexcept
  {
    return value.Abs();
  }

  KRYS_NODISCARD constexpr bool IsIntegerValue(const LayoutUnit value) noexcept
  {
    return value.ToInt() == value;
  }

  inline namespace Literals
  {
    KRYS_NODISCARD constexpr LayoutUnit operator""_lu(unsigned long long value) noexcept
    {
      return LayoutUnit(static_cast<uint64>(value));
    }
  }

  TextStream &operator<<(TextStream &ts, const LayoutUnit &value) noexcept;
}

namespace krys
{
  template <>
  struct DefaultHash<boo::LayoutUnit>
  {
    constexpr static bool IsSafeToCompareToEmptyOrDeleted = true;

    KRYS_NODISCARD static size_t Hash(const boo::LayoutUnit &p) noexcept
    {
      return DefaultHash<int32>::Hash(p.RawValue());
    }

    KRYS_NODISCARD static bool Equal(const boo::LayoutUnit &a, const boo::LayoutUnit &b) noexcept
    {
      return a == b;
    }
  };

  // The empty value is INT_MIN, the deleted value is INT_MAX. During the course of layout
  // these values are typically only used to represent uninitialized values, so they are
  // good candidates to represent the deleted and empty values in HashMaps as well.
  template <>
  struct HashTraits<boo::LayoutUnit> : public GenericHashTraits<boo::LayoutUnit>
  {
    constexpr static bool EmptyValueIsZero = false;

    KRYS_NODISCARD constexpr static boo::LayoutUnit EmptyValue() noexcept
    {
      return boo::LayoutUnit::FromRawValue(Limits<int32>::min());
    }

    constexpr static void ConstructDeletedValue(boo::LayoutUnit &slot) noexcept
    {
      slot.SetRawValue(Limits<int32>::max());
    }

    KRYS_NODISCARD constexpr static bool IsDeletedValue(boo::LayoutUnit value) noexcept
    {
      return value.RawValue() == Limits<int32>::max();
    }
  };

  template <>
  struct MarkableTraits<boo::LayoutUnit>
  {
    KRYS_NODISCARD constexpr static bool IsEmptyValue(boo::LayoutUnit value) noexcept
    {
      return value == boo::LayoutUnit(-1);
    }

    KRYS_NODISCARD constexpr static boo::LayoutUnit EmptyValue() noexcept
    {
      return boo::LayoutUnit(-1);
    }
  };
}
