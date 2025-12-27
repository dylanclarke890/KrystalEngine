#pragma once

#include <cmath>
#include <limits>

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
#pragma region Constants

  /// @returns 0.
  template <Number T>
  NO_DISCARD constexpr T Zero() noexcept
  {
    return T(0);
  }

  /// @returns 1.
  template <Number T>
  NO_DISCARD constexpr T One() noexcept
  {
    return T(1);
  }

  /// @returns 1/2.
  template <Number T>
  NO_DISCARD constexpr T Half() noexcept
  {
    return T(0.5);
  }

  /// @returns The epsilon constant.
  template <FloatingPoint T>
  NO_DISCARD constexpr T Epsilon() noexcept
  {
    return std::numeric_limits<T>::epsilon();
  }

  /// @returns pi.
  template <Number T>
  NO_DISCARD constexpr T Pi() noexcept
  {
    return T(3.14159265358979323846264338327950288);
  }

  /// @returns pi * 2.
  template <Number T>
  NO_DISCARD constexpr T TwoPi() noexcept
  {
    return T(6.28318530717958647692528676655900576);
  }

  /// @returns sqrt(pi).
  template <Number T>
  NO_DISCARD constexpr T RootPi() noexcept
  {
    return T(1.772453850905516027);
  }

  /// @returns pi / 2.
  template <Number T>
  NO_DISCARD constexpr T HalfPi() noexcept
  {
    return T(1.57079632679489661923132169163975144);
  }

  /// @returns 3/2 * pi.
  template <Number T>
  NO_DISCARD constexpr T ThreeOverTwoPi() noexcept
  {
    return T(4.71238898038468985769396507491925432);
  }

  /// @returns 1/4 * pi.
  template <Number T>
  NO_DISCARD constexpr T QuarterPi() noexcept
  {
    return T(0.785398163397448309615660845819875721);
  }

  /// @returns 1 / pi.
  template <Number T>
  NO_DISCARD constexpr T OneOverPi() noexcept
  {
    return T(0.318309886183790671537767526745028724);
  }

  /// @returns 1 / (2 * pi).
  template <Number T>
  NO_DISCARD constexpr T OneOverTwoPi() noexcept
  {
    return T(0.159154943091895335768883763372514362);
  }

  /// @returns 2 / pi.
  template <Number T>
  NO_DISCARD constexpr T TwoOverPi() noexcept
  {
    return T(0.636619772367581343075535053490057448);
  }

  /// @returns 4 / pi.
  template <Number T>
  NO_DISCARD constexpr T FourOverPi() noexcept
  {
    return T(1.273239544735162686151070106980114898);
  }

  /// @returns 2 / sqrt(pi).
  template <Number T>
  NO_DISCARD constexpr T TwoOverRootPi() noexcept
  {
    return T(1.12837916709551257389615890312154517);
  }

  /// @returns 1 / sqrt(2).
  template <Number T>
  NO_DISCARD constexpr T OneOverRootTwo() noexcept
  {
    return T(0.707106781186547524400844362104849039);
  }

  /// @returns sqrt(1/2 * pi).
  template <Number T>
  NO_DISCARD constexpr T RootHalfPi() noexcept
  {
    return T(1.253314137315500251);
  }

  /// @returns sqrt(2 * pi).
  template <Number T>
  NO_DISCARD constexpr T RootTwoPi() noexcept
  {
    return T(2.506628274631000502);
  }

  /// @returns sqrt(ln(4)).
  template <Number T>
  NO_DISCARD constexpr T RootLnFour() noexcept
  {
    return T(1.17741002251547469);
  }

  /// @returns e constant.
  template <Number T>
  NO_DISCARD constexpr T e() noexcept
  {
    return T(2.71828182845904523536);
  }

  /// @returns log10(e), the base-10 logarithm of e.
  template <Number T>
  NO_DISCARD constexpr T Log10E() noexcept
  {
    return T(0.434294481903251827651128918916605082);
  }

  /// @returns sqrt(e), the square root of e.
  template <Number T>
  NO_DISCARD constexpr T RootE() noexcept
  {
    return T(1.64872127070012814684865078781416357);
  }

  /// @returns Euler's constant.
  template <Number T>
  NO_DISCARD constexpr T Euler() noexcept
  {
    return T(0.577215664901532860606);
  }

  /// @returns sqrt(2).
  template <Number T>
  NO_DISCARD constexpr T RootTwo() noexcept
  {
    return T(1.41421356237309504880168872420969808);
  }

  /// @returns sqrt(3).
  template <Number T>
  NO_DISCARD constexpr T RootThree() noexcept
  {
    return T(1.73205080756887729352744634150587236);
  }

  /// @returns sqrt(5).
  template <Number T>
  NO_DISCARD constexpr T RootFive() noexcept
  {
    return T(2.23606797749978969640917366873127623);
  }

  /// @returns ln(2).
  template <Number T>
  NO_DISCARD constexpr T LnTwo() noexcept
  {
    return T(0.693147180559945309417232121458176568);
  }

  /// @returns ln(10).
  template <Number T>
  NO_DISCARD constexpr T LnTen() noexcept
  {
    return T(2.30258509299404568401799145468436421);
  }

  /// @returns ln(ln(2)).
  template <Number T>
  NO_DISCARD constexpr T LnLnTwo() noexcept
  {
    return T(-0.3665129205816643);
  }

  /// @returns 1/3.
  template <Number T>
  NO_DISCARD constexpr T OneThird() noexcept
  {
    return T(0.3333333333333333333333333333333333333333);
  }

  /// @returns 2/3.
  template <Number T>
  NO_DISCARD constexpr T TwoThirds() noexcept
  {
    return T(0.666666666666666666666666666666666666667);
  }

  /// @returns The golden ratio constant.
  template <Number T>
  NO_DISCARD constexpr T GoldenRatio() noexcept
  {
    return T(1.61803398874989484820458683436563811);
  }

  /// @returns cos(1 / 2).
  template <Number T>
  NO_DISCARD constexpr T CosOneHalf()
  {
    return T(0.877582561890372716130286068203503191);
  }

#pragma endregion

#pragma region FloatingPoint Classify

  template <FloatingPoint T>
  NO_DISCARD constexpr bool IsNaN(T x) noexcept
  {
    return std::isnan(x);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr bool IsInfinite(T x) noexcept
  {
    return std::isinf(x);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr bool IsFinite(T x) noexcept
  {
    return std::isfinite(x);
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr bool IsDenormal(T x) noexcept
  {
    return std::fpclassify(x) == FP_SUBNORMAL;
  }

  template <FloatingPoint T>
  NO_DISCARD constexpr bool IsNormal(T x) noexcept
  {
    return std::isnormal(x);
  }

#pragma endregion

#pragma region Abs

  template <Number T>
  NO_DISCARD constexpr T Abs(T x) noexcept
  {
    if (IS_COMPILE_TIME)
    {
      if (UnsignedIntegral<T>)
      {
        return x;
      }
      else
      {
        return (x < T(0)) ? -x : x;
      }
    }
    else if constexpr (UnsignedIntegral<T>)
    {
      return x;
    }
    else
    {
      return std::abs(x);
    }
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Abs(const VECTOR_TYPE &v) noexcept
  {
    return MapEach(v, [](auto x) { return Abs(x); });
  }

  template <MATRIX_TEMPLATE_PARAMS>
  NO_DISCARD constexpr auto Abs(const MATRIX_TYPE &m) noexcept
  {
    return MapEach(m, [](auto x) { return Abs(x); });
  }

#pragma endregion

#pragma region Trig

  template <FloatingPoint T>
  NO_DISCARD inline T Cos(T angle) noexcept
  {
    return std::cos(angle);
  }

  template <FloatingPoint T>
  NO_DISCARD inline T ACos(T value) noexcept
  {
    return std::acos(value);
  }

  template <FloatingPoint T>
  NO_DISCARD inline T Sin(T angle) noexcept
  {
    return std::sin(angle);
  }

  template <FloatingPoint T>
  NO_DISCARD inline T Asin(T value) noexcept
  {
    return std::asin(value);
  }

  template <FloatingPoint T>
  NO_DISCARD inline T Tan(T angle) noexcept
  {
    return std::tan(angle);
  }

  template <FloatingPoint T>
  NO_DISCARD inline T Atan2(T y, T x) noexcept
  {
    return std::atan2(y, x);
  }

  template <FloatingPoint T>
  NO_DISCARD inline T Exp(T value) noexcept
  {
    return std::exp(value);
  }

#pragma endregion
}
