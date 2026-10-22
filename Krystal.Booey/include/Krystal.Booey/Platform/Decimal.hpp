#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/String.hpp"
#include <compare>

namespace krys::boo
{
  namespace detail
  {
    class DecimalSpecialValueHandler;
  }

  /// @brief A decimal floating point number: an 18 digit coefficient and a base 10 exponent.
  // FIXME: Once all C++ compiler support decimal type, we should replace this class to compiler supported
  // one.
  class Decimal
  {
  public:
    static constexpr int ExponentMax = 1'023;
    static constexpr int ExponentMin = -1'023;
    static constexpr int Precision = 18;
    static constexpr uint64 MaxCoefficient = 0xD'E0'B6'B3'A7'63'FF'FFull; // 999999999999999999 == 18 9's

    enum Sign
    {
      Positive,
      Negative,
    };

    /// @brief The encoded number; only tests should use it.
    class EncodedData
    {
      friend class Decimal;
      friend class detail::DecimalSpecialValueHandler;

    private:
      enum FormatClass
      {
        ClassInfinity,
        ClassNormal,
        ClassNaN,
        ClassZero,
      };

      uint64 _coefficient {};
      int16 _exponent {};
      FormatClass _formatClass {};
      Sign _sign {};

    public:
      constexpr EncodedData(Sign sign, int exponent, uint64 coefficient) noexcept;

      KRYS_NODISCARD friend bool operator==(const EncodedData &, const EncodedData &) noexcept = default;

      KRYS_NODISCARD uint64 Coefficient() const noexcept
      {
        return _coefficient;
      }

      KRYS_NODISCARD int Exponent() const noexcept
      {
        return _exponent;
      }

      KRYS_NODISCARD bool IsFinite() const noexcept
      {
        return !IsSpecial();
      }

      KRYS_NODISCARD bool IsInfinity() const noexcept
      {
        return _formatClass == ClassInfinity;
      }

      KRYS_NODISCARD bool IsNaN() const noexcept
      {
        return _formatClass == ClassNaN;
      }

      KRYS_NODISCARD bool IsSpecial() const noexcept
      {
        return _formatClass == ClassInfinity || _formatClass == ClassNaN;
      }

      KRYS_NODISCARD bool IsZero() const noexcept
      {
        return _formatClass == ClassZero;
      }

      KRYS_NODISCARD Sign GetSign() const noexcept
      {
        return _sign;
      }

      void SetSign(Sign sign) noexcept
      {
        _sign = sign;
      }

    private:
      constexpr EncodedData(Sign sign, FormatClass formatClass) noexcept;

      KRYS_NODISCARD FormatClass GetFormatClass() const noexcept
      {
        return _formatClass;
      }
    };

  private:
    struct AlignedOperands
    {
      uint64 LhsCoefficient;
      uint64 RhsCoefficient;
      int Exponent;
    };

    EncodedData _data;

  public:
    constexpr Decimal(int32 i32 = 0) noexcept;
    constexpr Decimal(Sign sign, int exponent, uint64 coefficient) noexcept;

    /// @brief Only tests should use this.
    explicit constexpr Decimal(const EncodedData &data) noexcept : _data(data)
    {
    }

    Decimal &operator+=(const Decimal &other) noexcept;
    Decimal &operator-=(const Decimal &other) noexcept;
    Decimal &operator*=(const Decimal &other) noexcept;
    Decimal &operator/=(const Decimal &other) noexcept;

    KRYS_NODISCARD Decimal operator-() const noexcept;
    KRYS_NODISCARD Decimal operator+(const Decimal &rhs) const noexcept;
    KRYS_NODISCARD Decimal operator-(const Decimal &rhs) const noexcept;
    KRYS_NODISCARD Decimal operator*(const Decimal &rhs) const noexcept;
    KRYS_NODISCARD Decimal operator/(const Decimal &rhs) const noexcept;

    KRYS_NODISCARD friend bool operator==(const Decimal &a, const Decimal &b) noexcept;
    /// @note False when either is NaN, as is `==`.
    KRYS_NODISCARD friend bool operator!=(const Decimal &a, const Decimal &b) noexcept;
    KRYS_NODISCARD friend std::partial_ordering operator<=>(const Decimal &a, const Decimal &b) noexcept;

    KRYS_NODISCARD int Exponent() const noexcept
    {
      krys_debug_assert(IsFinite());
      return _data.Exponent();
    }

    KRYS_NODISCARD bool IsFinite() const noexcept
    {
      return _data.IsFinite();
    }

    KRYS_NODISCARD bool IsInfinity() const noexcept
    {
      return _data.IsInfinity();
    }

    KRYS_NODISCARD bool IsNaN() const noexcept
    {
      return _data.IsNaN();
    }

    KRYS_NODISCARD bool IsNegative() const noexcept
    {
      return GetSign() == Negative;
    }

    KRYS_NODISCARD bool IsPositive() const noexcept
    {
      return GetSign() == Positive;
    }

    KRYS_NODISCARD bool IsSpecial() const noexcept
    {
      return _data.IsSpecial();
    }

    KRYS_NODISCARD bool IsZero() const noexcept
    {
      return _data.IsZero();
    }

    KRYS_NODISCARD Decimal Abs() const noexcept;
    KRYS_NODISCARD Decimal Ceil() const noexcept;
    KRYS_NODISCARD Decimal Floor() const noexcept;
    KRYS_NODISCARD Decimal Remainder(const Decimal &rhs) const noexcept;
    KRYS_NODISCARD Decimal Round() const noexcept;

    KRYS_NODISCARD double ToDouble() const noexcept;

    /// @note Supports infinity and NaN, though `FromString` doesn't.
    KRYS_NODISCARD utf8_string ToString() const noexcept;

    KRYS_NODISCARD static Decimal FromDouble(double doubleValue) noexcept;

    /// @brief Parses the syntax below (EBNF), but not "infinity" or "nan":
    /// @code
    ///  number ::= sign? digit+ ('.' digit*) (exponent-marker sign? digit+)?
    ///          | sign? '.' digit+ (exponent-marker sign? digit+)?
    ///  sign ::= '+' | '-'
    ///  exponent-marker ::= 'e' | 'E'
    ///  digit ::= '0' | '1' | ... | '9'
    /// @endcode
    KRYS_NODISCARD static Decimal FromString(utf8_stringview string) noexcept;

    KRYS_NODISCARD static constexpr Decimal Infinity(Sign sign) noexcept;
    KRYS_NODISCARD static constexpr Decimal NaN() noexcept;
    KRYS_NODISCARD static constexpr Decimal Zero(Sign sign) noexcept;
    KRYS_NODISCARD static constexpr Decimal DoubleMax() noexcept;

    /// @brief Only tests should use this.
    KRYS_NODISCARD const EncodedData &Value() const noexcept
    {
      return _data;
    }

  private:
    KRYS_NODISCARD Decimal CompareTo(const Decimal &rhs) const noexcept;
    KRYS_NODISCARD static AlignedOperands AlignOperands(const Decimal &lhs, const Decimal &rhs) noexcept;

    KRYS_NODISCARD static Sign InvertSign(Sign sign) noexcept
    {
      return sign == Negative ? Positive : Negative;
    }

    KRYS_NODISCARD Sign GetSign() const noexcept
    {
      return _data.GetSign();
    }
  };

  inline constexpr Decimal::EncodedData::EncodedData(Sign sign, FormatClass formatClass) noexcept
      : _coefficient(0), _exponent(0), _formatClass(formatClass), _sign(sign)
  {
  }

  inline constexpr Decimal::EncodedData::EncodedData(Sign sign, int exponent, uint64 coefficient) noexcept
      : _formatClass(coefficient ? ClassNormal : ClassZero), _sign(sign)
  {
    if (exponent >= ExponentMin && exponent <= ExponentMax)
    {
      while (coefficient > MaxCoefficient)
      {
        coefficient /= 10;
        ++exponent;
      }
    }

    if (exponent > ExponentMax)
    {
      _coefficient = 0;
      _exponent = 0;
      _formatClass = ClassInfinity;
      return;
    }

    if (exponent < ExponentMin)
    {
      _coefficient = 0;
      _exponent = 0;
      _formatClass = ClassZero;
      return;
    }

    _coefficient = coefficient;
    _exponent = static_cast<int16>(exponent);
  }

  inline constexpr Decimal::Decimal(int32 i32) noexcept
      : _data(i32 < 0 ? Negative : Positive, 0,
              i32 < 0 ? static_cast<uint64>(-static_cast<int64>(i32)) : static_cast<uint64>(i32))
  {
  }

  inline constexpr Decimal::Decimal(Sign sign, int exponent, uint64 coefficient) noexcept
      : _data(sign, exponent, coefficient)
  {
  }

  inline bool operator==(const Decimal &a, const Decimal &b) noexcept
  {
    return a._data == b._data || a.CompareTo(b).IsZero();
  }

  inline bool operator!=(const Decimal &a, const Decimal &b) noexcept
  {
    if (a._data == b._data)
    {
      return false;
    }
    auto result = a.CompareTo(b);
    return !result.IsNaN() && !result.IsZero();
  }

  inline std::partial_ordering operator<=>(const Decimal &a, const Decimal &b) noexcept
  {
    if (a._data == b._data)
    {
      return std::partial_ordering::equivalent;
    }
    auto result = a.CompareTo(b);
    if (result.IsZero())
    {
      return std::partial_ordering::equivalent;
    }
    if (result.IsNaN())
    {
      return std::partial_ordering::unordered;
    }
    return result.IsNegative() ? std::partial_ordering::less : std::partial_ordering::greater;
  }

  inline constexpr Decimal Decimal::Infinity(Sign sign) noexcept
  {
    return Decimal(EncodedData(sign, EncodedData::ClassInfinity));
  }

  inline constexpr Decimal Decimal::NaN() noexcept
  {
    return Decimal(EncodedData(Positive, EncodedData::ClassNaN));
  }

  inline constexpr Decimal Decimal::Zero(Sign sign) noexcept
  {
    return Decimal(EncodedData(sign, EncodedData::ClassZero));
  }

  inline constexpr Decimal Decimal::DoubleMax() noexcept
  {
    return Decimal(EncodedData(Sign::Positive, 292, 17'976'931'348'623'157));
  }
}
