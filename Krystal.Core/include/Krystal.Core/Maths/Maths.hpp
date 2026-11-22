#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Matrix.hpp"
#include "Krystal.Core/Maths/Vector.hpp"
#include <cmath>
#include <limits>
#include <numbers>

namespace krys
{
#pragma region Constants

  /// @returns 0.
  template <Number T>
  KRYS_NODISCARD constexpr T Zero() noexcept
  {
    return T(0);
  }

  /// @returns 1.
  template <Number T>
  KRYS_NODISCARD constexpr T One() noexcept
  {
    return T(1);
  }

  /// @returns 1/2.
  template <Number T>
  KRYS_NODISCARD constexpr T Half() noexcept
  {
    return T(0.5);
  }

  /// @returns The epsilon constant.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr T Epsilon() noexcept
  {
    return std::numeric_limits<T>::epsilon();
  }

  /// @returns pi.
  template <Number T>
  KRYS_NODISCARD constexpr T Pi() noexcept
  {
    return T(std::numbers::pi);
  }

  /// @returns pi * 2.
  template <Number T>
  KRYS_NODISCARD constexpr T TwoPi() noexcept
  {
    return T(6.28318530717958647692528676655900576);
  }

  /// @returns sqrt(pi).
  template <Number T>
  KRYS_NODISCARD constexpr T RootPi() noexcept
  {
    return T(1.772453850905516027);
  }

  /// @returns pi / 2.
  template <Number T>
  KRYS_NODISCARD constexpr T HalfPi() noexcept
  {
    return T(1.57079632679489661923132169163975144);
  }

  /// @returns 3/2 * pi.
  template <Number T>
  KRYS_NODISCARD constexpr T ThreeOverTwoPi() noexcept
  {
    return T(4.71238898038468985769396507491925432);
  }

  /// @returns 1/4 * pi.
  template <Number T>
  KRYS_NODISCARD constexpr T QuarterPi() noexcept
  {
    return T(0.785398163397448309615660845819875721);
  }

  /// @returns 1 / pi.
  template <Number T>
  KRYS_NODISCARD constexpr T OneOverPi() noexcept
  {
    return T(std::numbers::inv_pi);
  }

  /// @returns 1 / (2 * pi).
  template <Number T>
  KRYS_NODISCARD constexpr T OneOverTwoPi() noexcept
  {
    return T(0.159154943091895335768883763372514362);
  }

  /// @returns 2 / pi.
  template <Number T>
  KRYS_NODISCARD constexpr T TwoOverPi() noexcept
  {
    return T(0.636619772367581343075535053490057448);
  }

  /// @returns 4 / pi.
  template <Number T>
  KRYS_NODISCARD constexpr T FourOverPi() noexcept
  {
    return T(1.273239544735162686151070106980114898);
  }

  /// @returns 2 / sqrt(pi).
  template <Number T>
  KRYS_NODISCARD constexpr T TwoOverRootPi() noexcept
  {
    return T(1.12837916709551257389615890312154517);
  }

  /// @returns 1 / sqrt(2).
  template <Number T>
  KRYS_NODISCARD constexpr T OneOverRootTwo() noexcept
  {
    return T(0.707106781186547524400844362104849039);
  }

  /// @returns sqrt(1/2 * pi).
  template <Number T>
  KRYS_NODISCARD constexpr T RootHalfPi() noexcept
  {
    return T(1.253314137315500251);
  }

  /// @returns sqrt(2 * pi).
  template <Number T>
  KRYS_NODISCARD constexpr T RootTwoPi() noexcept
  {
    return T(2.506628274631000502);
  }

  /// @returns sqrt(ln(4)).
  template <Number T>
  KRYS_NODISCARD constexpr T RootLnFour() noexcept
  {
    return T(1.17741002251547469);
  }

  /// @returns e constant.
  template <Number T>
  KRYS_NODISCARD constexpr T e() noexcept
  {
    return T(std::numbers::e);
  }

  /// @returns log10(e), the base-10 logarithm of e.
  template <Number T>
  KRYS_NODISCARD constexpr T Log10E() noexcept
  {
    return T(std::numbers::log10e);
  }

  /// @returns sqrt(e), the square root of e.
  template <Number T>
  KRYS_NODISCARD constexpr T RootE() noexcept
  {
    return T(1.64872127070012814684865078781416357);
  }

  /// @returns Euler's constant.
  template <Number T>
  KRYS_NODISCARD constexpr T Euler() noexcept
  {
    return T(std::numbers::egamma);
  }

  /// @returns sqrt(2).
  template <Number T>
  KRYS_NODISCARD constexpr T RootTwo() noexcept
  {
    return T(std::numbers::sqrt2);
  }

  /// @returns sqrt(3).
  template <Number T>
  KRYS_NODISCARD constexpr T RootThree() noexcept
  {
    return T(std::numbers::sqrt3);
  }

  /// @returns sqrt(5).
  template <Number T>
  KRYS_NODISCARD constexpr T RootFive() noexcept
  {
    return T(2.23606797749978969640917366873127623);
  }

  /// @returns ln(2).
  template <Number T>
  KRYS_NODISCARD constexpr T LnTwo() noexcept
  {
    return T(std::numbers::ln2);
  }

  /// @returns ln(10).
  template <Number T>
  KRYS_NODISCARD constexpr T LnTen() noexcept
  {
    return T(std::numbers::ln10);
  }

  /// @returns ln(ln(2)).
  template <Number T>
  KRYS_NODISCARD constexpr T LnLnTwo() noexcept
  {
    return T(-0.3665129205816643);
  }

  /// @returns 1/3.
  template <Number T>
  KRYS_NODISCARD constexpr T OneThird() noexcept
  {
    return T(0.3333333333333333333333333333333333333333);
  }

  /// @returns 2/3.
  template <Number T>
  KRYS_NODISCARD constexpr T TwoThirds() noexcept
  {
    return T(0.666666666666666666666666666666666666667);
  }

  /// @returns The golden ratio constant.
  template <Number T>
  KRYS_NODISCARD constexpr T GoldenRatio() noexcept
  {
    return T(std::numbers::phi);
  }

  /// @returns cos(1 / 2).
  template <Number T>
  KRYS_NODISCARD constexpr T CosOneHalf()
  {
    return T(0.877582561890372716130286068203503191);
  }

#pragma endregion

#pragma region FloatingPoint Classify

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsNaN(T x) noexcept
  {
    return std::isnan(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsInfinite(T x) noexcept
  {
    return std::isinf(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsFinite(T x) noexcept
  {
    return std::isfinite(x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsDenormal(T x) noexcept
  {
    return std::fpclassify(x) == FP_SUBNORMAL;
  }

  template <FloatingPoint T>
  KRYS_NODISCARD constexpr bool IsNormal(T x) noexcept
  {
    return std::isnormal(x);
  }

#pragma endregion

#pragma region Abs

  template <Number T>
  KRYS_NODISCARD constexpr T Abs(T x) noexcept
  {
    if (std::is_constant_evaluated())
    {
      if (UnsignedIntegral<T>)
      {
        return x;
      }

      return (x < T(0)) ? -x : x;
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

  template <Number T, int N>
  KRYS_NODISCARD constexpr auto Abs(const Vector<T, N> &v) noexcept
  {
    return MapEach(v, [](auto x) { return Abs(x); });
  }

  template <Number T, int C, int R>
  KRYS_NODISCARD constexpr auto Abs(const Matrix<T, C, R> &m) noexcept
  {
    return MapEach(m, [](auto x) { return Abs(x); });
  }

#pragma endregion

#pragma region Trig

  template <FloatingPoint T>
  KRYS_NODISCARD inline T Cos(T angle) noexcept
  {
    return std::cos(angle);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD inline T ACos(T value) noexcept
  {
    return std::acos(value);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD inline T Sin(T angle) noexcept
  {
    return std::sin(angle);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD inline T Asin(T value) noexcept
  {
    return std::asin(value);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD inline T Tan(T angle) noexcept
  {
    return std::tan(angle);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD inline T Atan2(T y, T x) noexcept
  {
    return std::atan2(y, x);
  }

  template <FloatingPoint T>
  KRYS_NODISCARD inline T Exp(T value) noexcept
  {
    return std::exp(value);
  }

#pragma endregion
}
