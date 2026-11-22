#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Maths/Vector.hpp"

namespace krys
{
#pragma region Two Components

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> xx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 1 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> xy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> xz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> xw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> yx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> yy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> yz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> yw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> zx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> zy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> zz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> zw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> wx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> wy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> wz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 2> ww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 2>(v.w, v.w);
  }

#pragma endregion

#pragma region Three Components

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 1 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> xww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.x, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> ywx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> ywy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> ywz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> yww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.y, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> zww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.z, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> wwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 3> www(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 3>(v.w, v.w, v.w);
  }

#pragma endregion

#pragma region Four Components

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 1 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xxww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.x, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xywx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xywy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xywz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xyww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.y, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xzww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.z, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> xwww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.x, v.w, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yxww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.x, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 2 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yywx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yywy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yywz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yyww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.y, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> yzww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.z, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> ywww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.y, v.w, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zxww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.x, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zywx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zywy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zywz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zyww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.y, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 3 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zzww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.z, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> zwww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.z, v.w, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wxww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.x, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wywx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wywy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wywz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wyww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.y, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wzww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.z, v.w, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwxx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.x, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwxy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.x, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwxz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.x, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwxw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.x, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwyx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.y, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwyy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.y, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwyz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.y, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwyw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.y, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwzx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.z, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwzy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.z, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwzz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.z, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwzw(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.z, v.w);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwwx(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.w, v.x);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwwy(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.w, v.y);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwwz(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.w, v.z);
  }

  template <Number T, int N>
  KRYS_NODISCARD constexpr Vector<T, 4> wwww(const Vector<T, N> &v) noexcept
  {
    static_assert(N >= 4 && N <= 4, "Invalid vector length.");
    return Vector<T, 4>(v.w, v.w, v.w, v.w);
  }

#pragma endregion
}
