#include "Krystal.Booey/Platform/Decimal.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cfloat>
#include <charconv>
#include <cmath>
#include <limits>
#if defined(_MSC_VER) && !defined(__clang__)
  #include <intrin.h>
#endif

namespace krys::boo
{
  namespace detail
  {
    /// @brief Handles the special values of the operands of an operation.
    class DecimalSpecialValueHandler
    {
      KRYS_NON_COPY_MOVABLE(DecimalSpecialValueHandler);

    public:
      enum HandleResult
      {
        BothFinite,
        BothInfinity,
        EitherNaN,
        LHSIsInfinity,
        RHSIsInfinity,
      };

    private:
      enum Result
      {
        ResultIsLHS,
        ResultIsRHS,
        ResultIsUnknown,
      };

      const Decimal &_lhs;
      const Decimal &_rhs;
      Result _result {ResultIsUnknown};

    public:
      DecimalSpecialValueHandler(const Decimal &lhs, const Decimal &rhs) noexcept : _lhs(lhs), _rhs(rhs)
      {
      }

      KRYS_NODISCARD HandleResult Handle() noexcept
      {
        if (_lhs.IsFinite() && _rhs.IsFinite())
        {
          return BothFinite;
        }

        const auto lhsClass = _lhs.Value().GetFormatClass();
        const auto rhsClass = _rhs.Value().GetFormatClass();
        if (lhsClass == Decimal::EncodedData::ClassNaN)
        {
          _result = ResultIsLHS;
          return EitherNaN;
        }

        if (rhsClass == Decimal::EncodedData::ClassNaN)
        {
          _result = ResultIsRHS;
          return EitherNaN;
        }

        if (lhsClass == Decimal::EncodedData::ClassInfinity)
        {
          return rhsClass == Decimal::EncodedData::ClassInfinity ? BothInfinity : LHSIsInfinity;
        }

        if (rhsClass == Decimal::EncodedData::ClassInfinity)
        {
          return RHSIsInfinity;
        }

        krys_unreachable();
      }

      KRYS_NODISCARD Decimal Value() const noexcept
      {
        switch (_result)
        {
          case ResultIsLHS: return _lhs;
          case ResultIsRHS: return _rhs;
          default:          krys_unreachable();
        }
      }
    };
  }

  namespace
  {
    using detail::DecimalSpecialValueHandler;

    KRYS_NODISCARD int CountDigits(uint64 x) noexcept
    {
      int numberOfDigits = 0;
      for (uint64 powerOfTen = 1; x >= powerOfTen; powerOfTen *= 10)
      {
        ++numberOfDigits;
        if (powerOfTen >= std::numeric_limits<uint64>::max() / 10)
        {
          break;
        }
      }
      return numberOfDigits;
    }

    KRYS_NODISCARD uint64 ScaleDown(uint64 x, int n) noexcept
    {
      krys_debug_assert(n >= 0);
      while (n > 0 && x)
      {
        x /= 10;
        --n;
      }
      return x;
    }

    KRYS_NODISCARD uint64 ScaleUp(uint64 x, int n) noexcept
    {
      krys_debug_assert(n >= 0);
      krys_debug_assert(n < Decimal::Precision);

      uint64 y = 1;
      uint64 z = 10;
      for (;;)
      {
        if (n & 1)
        {
          y = y * z;
        }

        n >>= 1;
        if (!n)
        {
          return x * y;
        }

        z = z * z;
      }
    }

    /// @brief The product of @p lhs and @p rhs, divided by ten until it fits in 64 bits; @p exponent is
    /// increased by the number of divisions.
    KRYS_NODISCARD uint64 MultiplyCoefficients(uint64 lhs, uint64 rhs, int &exponent) noexcept
    {
#if defined(_MSC_VER) && !defined(__clang__)
      uint64 high = 0;
      uint64 low = _umul128(lhs, rhs, &high);
      while (high)
      {
        uint64 remainder = 0;
        uint64 highRemainder = high % 10;
        high /= 10;
        low = _udiv128(highRemainder, low, 10, &remainder);
        ++exponent;
      }
      return low;
#else
      auto work = static_cast<unsigned __int128>(lhs) * static_cast<unsigned __int128>(rhs);
      while (work >> 64)
      {
        work /= 10;
        ++exponent;
      }
      return static_cast<uint64>(work);
#endif
    }

    KRYS_NODISCARD bool IsMultiplePowersOfTen(uint64 coefficient, int n) noexcept
    {
      return !coefficient || !(coefficient % ScaleUp(1, n));
    }
  }

  Decimal &Decimal::operator+=(const Decimal &other) noexcept
  {
    _data = (*this + other)._data;
    return *this;
  }

  Decimal &Decimal::operator-=(const Decimal &other) noexcept
  {
    _data = (*this - other)._data;
    return *this;
  }

  Decimal &Decimal::operator*=(const Decimal &other) noexcept
  {
    _data = (*this * other)._data;
    return *this;
  }

  Decimal &Decimal::operator/=(const Decimal &other) noexcept
  {
    _data = (*this / other)._data;
    return *this;
  }

  Decimal Decimal::operator-() const noexcept
  {
    if (IsNaN())
    {
      return *this;
    }

    Decimal result(*this);
    result._data.SetSign(InvertSign(_data.GetSign()));
    return result;
  }

  Decimal Decimal::operator+(const Decimal &rhs) const noexcept
  {
    const Decimal &lhs = *this;
    const Sign lhsSign = lhs.GetSign();
    const Sign rhsSign = rhs.GetSign();

    DecimalSpecialValueHandler handler(lhs, rhs);
    switch (handler.Handle())
    {
      case DecimalSpecialValueHandler::BothFinite:    break;
      case DecimalSpecialValueHandler::BothInfinity:  return lhsSign == rhsSign ? lhs : NaN();
      case DecimalSpecialValueHandler::EitherNaN:     return handler.Value();
      case DecimalSpecialValueHandler::LHSIsInfinity: return lhs;
      case DecimalSpecialValueHandler::RHSIsInfinity: return rhs;
    }

    const AlignedOperands alignedOperands = AlignOperands(lhs, rhs);

    const uint64 result = lhsSign == rhsSign
                            ? alignedOperands.LhsCoefficient + alignedOperands.RhsCoefficient
                            : alignedOperands.LhsCoefficient - alignedOperands.RhsCoefficient;

    if (lhsSign == Negative && rhsSign == Positive && !result)
    {
      return Decimal(Positive, alignedOperands.Exponent, 0);
    }

    return static_cast<int64>(result) >= 0 ? Decimal(lhsSign, alignedOperands.Exponent, result)
                                           : Decimal(InvertSign(lhsSign), alignedOperands.Exponent,
                                                     static_cast<uint64>(-static_cast<int64>(result)));
  }

  Decimal Decimal::operator-(const Decimal &rhs) const noexcept
  {
    const Decimal &lhs = *this;
    const Sign lhsSign = lhs.GetSign();
    const Sign rhsSign = rhs.GetSign();

    DecimalSpecialValueHandler handler(lhs, rhs);
    switch (handler.Handle())
    {
      case DecimalSpecialValueHandler::BothFinite:    break;
      case DecimalSpecialValueHandler::BothInfinity:  return lhsSign == rhsSign ? NaN() : lhs;
      case DecimalSpecialValueHandler::EitherNaN:     return handler.Value();
      case DecimalSpecialValueHandler::LHSIsInfinity: return lhs;
      case DecimalSpecialValueHandler::RHSIsInfinity: return Infinity(InvertSign(rhsSign));
    }

    const AlignedOperands alignedOperands = AlignOperands(lhs, rhs);

    const uint64 result = lhsSign == rhsSign
                            ? alignedOperands.LhsCoefficient - alignedOperands.RhsCoefficient
                            : alignedOperands.LhsCoefficient + alignedOperands.RhsCoefficient;

    if (lhsSign == Negative && rhsSign == Negative && !result)
    {
      return Decimal(Positive, alignedOperands.Exponent, 0);
    }

    return static_cast<int64>(result) >= 0 ? Decimal(lhsSign, alignedOperands.Exponent, result)
                                           : Decimal(InvertSign(lhsSign), alignedOperands.Exponent,
                                                     static_cast<uint64>(-static_cast<int64>(result)));
  }

  Decimal Decimal::operator*(const Decimal &rhs) const noexcept
  {
    const Decimal &lhs = *this;
    const Sign lhsSign = lhs.GetSign();
    const Sign rhsSign = rhs.GetSign();
    const Sign resultSign = lhsSign == rhsSign ? Positive : Negative;

    DecimalSpecialValueHandler handler(lhs, rhs);
    switch (handler.Handle())
    {
      case DecimalSpecialValueHandler::BothFinite:
      {
        int resultExponent = lhs.Exponent() + rhs.Exponent();
        uint64 work = MultiplyCoefficients(lhs._data.Coefficient(), rhs._data.Coefficient(), resultExponent);
        return Decimal(resultSign, resultExponent, work);
      }
      case DecimalSpecialValueHandler::BothInfinity:  return Infinity(resultSign);
      case DecimalSpecialValueHandler::EitherNaN:     return handler.Value();
      case DecimalSpecialValueHandler::LHSIsInfinity: return rhs.IsZero() ? NaN() : Infinity(resultSign);
      case DecimalSpecialValueHandler::RHSIsInfinity: return lhs.IsZero() ? NaN() : Infinity(resultSign);
    }

    krys_unreachable();
  }

  Decimal Decimal::operator/(const Decimal &rhs) const noexcept
  {
    const Decimal &lhs = *this;
    const Sign lhsSign = lhs.GetSign();
    const Sign rhsSign = rhs.GetSign();
    const Sign resultSign = lhsSign == rhsSign ? Positive : Negative;

    DecimalSpecialValueHandler handler(lhs, rhs);
    switch (handler.Handle())
    {
      case DecimalSpecialValueHandler::BothFinite:    break;
      case DecimalSpecialValueHandler::BothInfinity:  return NaN();
      case DecimalSpecialValueHandler::EitherNaN:     return handler.Value();
      case DecimalSpecialValueHandler::LHSIsInfinity: return Infinity(resultSign);
      case DecimalSpecialValueHandler::RHSIsInfinity: return Zero(resultSign);
    }

    krys_debug_assert(lhs.IsFinite());
    krys_debug_assert(rhs.IsFinite());

    if (rhs.IsZero())
    {
      return lhs.IsZero() ? NaN() : Infinity(resultSign);
    }

    int resultExponent = lhs.Exponent() - rhs.Exponent();

    if (lhs.IsZero())
    {
      return Decimal(resultSign, resultExponent, 0);
    }

    uint64 remainder = lhs._data.Coefficient();
    const uint64 divisor = rhs._data.Coefficient();
    uint64 result = 0;
    for (;;)
    {
      while (remainder < divisor && result < MaxCoefficient / 10)
      {
        remainder *= 10;
        result *= 10;
        --resultExponent;
      }
      if (remainder < divisor)
      {
        break;
      }
      uint64 quotient = remainder / divisor;
      if (result > MaxCoefficient - quotient)
      {
        break;
      }
      result += quotient;
      remainder %= divisor;
      if (!remainder)
      {
        break;
      }
    }

    if (remainder > divisor / 2)
    {
      ++result;
    }

    return Decimal(resultSign, resultExponent, result);
  }

  Decimal Decimal::Abs() const noexcept
  {
    Decimal result(*this);
    result._data.SetSign(Positive);
    return result;
  }

  Decimal::AlignedOperands Decimal::AlignOperands(const Decimal &lhs, const Decimal &rhs) noexcept
  {
    krys_debug_assert(lhs.IsFinite());
    krys_debug_assert(rhs.IsFinite());

    const int lhsExponent = lhs.Exponent();
    const int rhsExponent = rhs.Exponent();
    int exponent = std::min(lhsExponent, rhsExponent);
    uint64 lhsCoefficient = lhs._data.Coefficient();
    uint64 rhsCoefficient = rhs._data.Coefficient();

    if (lhsExponent > rhsExponent)
    {
      const int numberOfLHSDigits = CountDigits(lhsCoefficient);
      if (numberOfLHSDigits)
      {
        const int lhsShiftAmount = lhsExponent - rhsExponent;
        const int overflow = numberOfLHSDigits + lhsShiftAmount - Precision;
        if (overflow <= 0)
        {
          lhsCoefficient = ScaleUp(lhsCoefficient, lhsShiftAmount);
        }
        else
        {
          lhsCoefficient = ScaleUp(lhsCoefficient, lhsShiftAmount - overflow);
          rhsCoefficient = ScaleDown(rhsCoefficient, overflow);
          exponent += overflow;
        }
      }
    }
    else if (lhsExponent < rhsExponent)
    {
      const int numberOfRHSDigits = CountDigits(rhsCoefficient);
      if (numberOfRHSDigits)
      {
        const int rhsShiftAmount = rhsExponent - lhsExponent;
        const int overflow = numberOfRHSDigits + rhsShiftAmount - Precision;
        if (overflow <= 0)
        {
          rhsCoefficient = ScaleUp(rhsCoefficient, rhsShiftAmount);
        }
        else
        {
          rhsCoefficient = ScaleUp(rhsCoefficient, rhsShiftAmount - overflow);
          lhsCoefficient = ScaleDown(lhsCoefficient, overflow);
          exponent += overflow;
        }
      }
    }

    return {.LhsCoefficient = lhsCoefficient, .RhsCoefficient = rhsCoefficient, .Exponent = exponent};
  }

  // Round toward positive infinity.
  Decimal Decimal::Ceil() const noexcept
  {
    if (IsSpecial())
    {
      return *this;
    }

    if (Exponent() >= 0)
    {
      return *this;
    }

    uint64 result = _data.Coefficient();
    const int numberOfDigits = CountDigits(result);
    const int numberOfDropDigits = -Exponent();
    if (numberOfDigits <= numberOfDropDigits)
    {
      return IsPositive() ? Decimal(1) : Zero(Positive);
    }

    result = ScaleDown(result, numberOfDropDigits);
    if (IsPositive() && !IsMultiplePowersOfTen(_data.Coefficient(), numberOfDropDigits))
    {
      ++result;
    }
    return Decimal(GetSign(), 0, result);
  }

  Decimal Decimal::CompareTo(const Decimal &rhs) const noexcept
  {
    const Decimal result(*this - rhs);
    switch (result._data.GetFormatClass())
    {
      case EncodedData::ClassInfinity: return result.IsNegative() ? Decimal(-1) : Decimal(1);
      case EncodedData::ClassNaN:
      case EncodedData::ClassNormal:   return result;
      case EncodedData::ClassZero:     return Zero(Positive);
    }

    krys_unreachable();
  }

  // Round toward negative infinity.
  Decimal Decimal::Floor() const noexcept
  {
    if (IsSpecial())
    {
      return *this;
    }

    if (Exponent() >= 0)
    {
      return *this;
    }

    uint64 result = _data.Coefficient();
    const int numberOfDigits = CountDigits(result);
    const int numberOfDropDigits = -Exponent();
    if (numberOfDigits < numberOfDropDigits)
    {
      return IsPositive() ? Zero(Positive) : Decimal(-1);
    }

    result = ScaleDown(result, numberOfDropDigits);
    if (IsNegative() && !IsMultiplePowersOfTen(_data.Coefficient(), numberOfDropDigits))
    {
      ++result;
    }
    return Decimal(GetSign(), 0, result);
  }

  Decimal Decimal::FromDouble(double doubleValue) noexcept
  {
    if (std::isfinite(doubleValue))
    {
      // The shortest representation that converts back to the same double.
      char buffer[32];
      auto result = std::to_chars(buffer, buffer + sizeof(buffer), doubleValue);
      return FromString(utf8_stringview(reinterpret_cast<const char8 *>(buffer), result.ptr - buffer));
    }

    if (std::isinf(doubleValue))
    {
      return Infinity(doubleValue < 0 ? Negative : Positive);
    }

    return NaN();
  }

  Decimal Decimal::FromString(utf8_stringview str) noexcept
  {
    int exponent = 0;
    Sign exponentSign = Positive;
    int numberOfDigits = 0;
    int numberOfDigitsAfterDot = 0;
    int numberOfExtraDigits = 0;
    Sign sign = Positive;

    enum
    {
      StateDigit,
      StateDot,
      StateDotDigit,
      StateE,
      StateEDigit,
      StateESign,
      StateSign,
      StateStart,
      StateZero,
    } state = StateStart;

    uint64 accumulator = 0;
    for (char8 character : str)
    {
      const int ch = character;
      switch (state)
      {
        case StateDigit:
          if (text::IsASCIIDigit(character))
          {
            if (numberOfDigits < Precision)
            {
              ++numberOfDigits;
              accumulator *= 10;
              accumulator += ch - '0';
            }
            else
            {
              ++numberOfExtraDigits;
            }
            break;
          }

          if (ch == '.')
          {
            state = StateDot;
            break;
          }
          if (ch == 'E' || ch == 'e')
          {
            state = StateE;
            break;
          }
          return NaN();

        case StateDot:
        case StateDotDigit:
          if (text::IsASCIIDigit(character))
          {
            if (numberOfDigits < Precision)
            {
              ++numberOfDigits;
              ++numberOfDigitsAfterDot;
              accumulator *= 10;
              accumulator += ch - '0';
            }
            state = StateDotDigit;
            break;
          }

          if (ch == 'E' || ch == 'e')
          {
            state = StateE;
            break;
          }
          return NaN();

        case StateE:
          if (ch == '+')
          {
            exponentSign = Positive;
            state = StateESign;
            break;
          }

          if (ch == '-')
          {
            exponentSign = Negative;
            state = StateESign;
            break;
          }

          if (text::IsASCIIDigit(character))
          {
            exponent = ch - '0';
            state = StateEDigit;
            break;
          }

          return NaN();

        case StateEDigit:
          if (text::IsASCIIDigit(character))
          {
            exponent *= 10;
            exponent += ch - '0';
            if (exponent > ExponentMax + Precision)
            {
              if (accumulator)
              {
                return exponentSign == Negative ? Zero(Positive) : Infinity(sign);
              }
              return Zero(sign);
            }
            state = StateEDigit;
            break;
          }

          return NaN();

        case StateESign:
          if (text::IsASCIIDigit(character))
          {
            exponent = ch - '0';
            state = StateEDigit;
            break;
          }

          return NaN();

        case StateSign:
          if (ch >= '1' && ch <= '9')
          {
            accumulator = ch - '0';
            numberOfDigits = 1;
            state = StateDigit;
            break;
          }

          if (ch == '0')
          {
            state = StateZero;
            break;
          }
          if (ch == '.')
          {
            state = StateDot;
            break;
          }
          return NaN();

        case StateStart:
          if (ch >= '1' && ch <= '9')
          {
            accumulator = ch - '0';
            numberOfDigits = 1;
            state = StateDigit;
            break;
          }

          if (ch == '-')
          {
            sign = Negative;
            state = StateSign;
            break;
          }

          if (ch == '+')
          {
            sign = Positive;
            state = StateSign;
            break;
          }

          if (ch == '0')
          {
            state = StateZero;
            break;
          }
          if (ch == '.')
          {
            state = StateDot;
            break;
          }
          return NaN();

        case StateZero:
          if (ch == '0')
          {
            break;
          }

          if (ch >= '1' && ch <= '9')
          {
            accumulator = ch - '0';
            numberOfDigits = 1;
            state = StateDigit;
            break;
          }

          if (ch == '.')
          {
            state = StateDot;
            break;
          }
          if (ch == 'E' || ch == 'e')
          {
            state = StateE;
            break;
          }
          return NaN();
      }
    }

    if (state == StateZero)
    {
      return Zero(sign);
    }

    if (state == StateDigit || state == StateDot || state == StateDotDigit || state == StateEDigit)
    {
      int resultExponent =
        exponent * (exponentSign == Negative ? -1 : 1) - numberOfDigitsAfterDot + numberOfExtraDigits;
      if (resultExponent < ExponentMin)
      {
        return Zero(Positive);
      }

      const int overflow = resultExponent - ExponentMax + 1;
      if (overflow > 0)
      {
        if (overflow + numberOfDigits - numberOfDigitsAfterDot > Precision)
        {
          return Infinity(sign);
        }
        accumulator = ScaleUp(accumulator, overflow);
        resultExponent -= overflow;
      }

      return Decimal(sign, resultExponent, accumulator);
    }

    return NaN();
  }

  Decimal Decimal::Remainder(const Decimal &rhs) const noexcept
  {
    const Decimal quotient = *this / rhs;
    return quotient.IsSpecial() ? quotient
                                : *this - (quotient.IsNegative() ? quotient.Ceil() : quotient.Floor()) * rhs;
  }

  Decimal Decimal::Round() const noexcept
  {
    if (IsSpecial())
    {
      return *this;
    }

    if (Exponent() >= 0)
    {
      return *this;
    }

    uint64 result = _data.Coefficient();
    const int numberOfDigits = CountDigits(result);
    const int numberOfDropDigits = -Exponent();
    if (numberOfDigits < numberOfDropDigits)
    {
      return Zero(Positive);
    }

    result = ScaleDown(result, numberOfDropDigits - 1);
    if (result % 10 >= 5)
    {
      result += 10;
    }
    result /= 10;
    return Decimal(GetSign(), 0, result);
  }

  double Decimal::ToDouble() const noexcept
  {
    if (IsFinite())
    {
      auto string = ToString();
      double doubleValue = 0;
      auto first = reinterpret_cast<const char *>(string.data());
      auto last = first + string.size();
      auto result = std::from_chars(first, last, doubleValue);
      return result.ec == std::errc {} && result.ptr == last ? doubleValue
                                                             : std::numeric_limits<double>::quiet_NaN();
    }

    if (IsInfinity())
    {
      return IsNegative() ? -std::numeric_limits<double>::infinity()
                          : std::numeric_limits<double>::infinity();
    }

    return std::numeric_limits<double>::quiet_NaN();
  }

  utf8_string Decimal::ToString() const noexcept
  {
    switch (_data.GetFormatClass())
    {
      case EncodedData::ClassInfinity: return GetSign() ? u8"-Infinity" : u8"Infinity";
      case EncodedData::ClassNaN:      return u8"NaN";
      case EncodedData::ClassNormal:
      case EncodedData::ClassZero:     break;
    }

    StringBuilder builder;
    if (GetSign())
    {
      builder.Append(u8'-');
    }

    int originalExponent = Exponent();
    uint64 coefficient = _data.Coefficient();

    if (originalExponent < 0)
    {
      const int maxDigits = DBL_DIG;
      uint64 lastDigit = 0;
      while (CountDigits(coefficient) > maxDigits)
      {
        lastDigit = coefficient % 10;
        coefficient /= 10;
        ++originalExponent;
      }

      if (lastDigit >= 5)
      {
        ++coefficient;
      }

      while (originalExponent < 0 && coefficient && !(coefficient % 10))
      {
        coefficient /= 10;
        ++originalExponent;
      }
    }

    char digitsBuffer[24];
    auto digitsEnd = std::to_chars(digitsBuffer, digitsBuffer + sizeof(digitsBuffer), coefficient).ptr;
    utf8_stringview digits(reinterpret_cast<const char8 *>(digitsBuffer), digitsEnd - digitsBuffer);
    int coefficientLength = static_cast<int>(digits.size());
    const int adjustedExponent = originalExponent + coefficientLength - 1;
    if (originalExponent <= 0 && adjustedExponent >= -6)
    {
      if (!originalExponent)
      {
        builder.Append(digits);
        return builder.ToString();
      }

      if (adjustedExponent >= 0)
      {
        for (int i = 0; i < coefficientLength; ++i)
        {
          builder.Append(digits[i]);
          if (i == adjustedExponent)
          {
            builder.Append(u8'.');
          }
        }
        return builder.ToString();
      }

      builder.Append(u8"0.");
      for (int i = adjustedExponent + 1; i < 0; ++i)
      {
        builder.Append(u8'0');
      }

      builder.Append(digits);
    }
    else
    {
      builder.Append(digits[0]);
      while (coefficientLength >= 2 && digits[coefficientLength - 1] == '0')
      {
        --coefficientLength;
      }
      if (coefficientLength >= 2)
      {
        builder.Append(u8'.');
        for (int i = 1; i < coefficientLength; ++i)
        {
          builder.Append(digits[i]);
        }
      }

      if (adjustedExponent)
      {
        builder.Append(adjustedExponent < 0 ? u8"e" : u8"e+");
        builder.Append(adjustedExponent);
      }
    }
    return builder.ToString();
  }
}
