#pragma once

#include "Krystal.Core/Core.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
#define SWIZZLE_BOUNDS_CHECK(lo, hi) static_assert(N >= lo && N <= hi, "Invalid vector length.");

#pragma region Two Components

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> xx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(1, 4);
    return Vector<T, 2>(v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> xy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 2>(v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> xz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 2>(v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> xw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> yx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 2>(v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> yy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 2>(v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> yz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 2>(v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> yw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> zx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 2>(v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> zy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 2>(v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> zz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 2>(v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> zw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> wx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> wy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> wz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 2> ww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 2>(v.w, v.w);
  }

#pragma endregion

#pragma region Three Components

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(1, 4);
    return Vector<T, 3>(v.x, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.x, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.x, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.x, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.x, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.x, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.x, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.x, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.x, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> xww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.x, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.y, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.y, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.y, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.y, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 3>(v.y, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.y, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.y, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.y, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.y, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> ywx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> ywy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> ywz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> yww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.y, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 3>(v.z, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> zww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.z, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> wwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 3> www(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 3>(v.w, v.w, v.w);
  }

#pragma endregion

#pragma region Four Components

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(1, 4);
    return Vector<T, 4>(v.x, v.x, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.x, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.x, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.x, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.x, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.x, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.x, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.x, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.x, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xxww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.x, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.y, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.y, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.y, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.y, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.x, v.y, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.y, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.y, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.y, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.y, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xywx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xywy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xywz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xyww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.y, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.x, v.z, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xzww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.z, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> xwww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.x, v.w, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.x, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.x, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.x, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.x, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.x, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.x, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.x, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.x, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.x, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yxww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.x, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.y, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.y, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.y, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.y, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(2, 4);
    return Vector<T, 4>(v.y, v.y, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.y, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.y, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.y, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.y, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yywx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yywy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yywz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yyww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.y, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.y, v.z, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> yzww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.z, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> ywww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.y, v.w, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.x, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zxww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.x, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.y, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zywx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zywy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zywz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zyww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.y, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(3, 4);
    return Vector<T, 4>(v.z, v.z, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zzww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.z, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> zwww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.z, v.w, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wxww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.x, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wywx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wywy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wywz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wyww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.y, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wzww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.z, v.w, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwxx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.x, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwxy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.x, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwxz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.x, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwxw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.x, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwyx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.y, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwyy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.y, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwyz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.y, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwyw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.y, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwzx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.z, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwzy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.z, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwzz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.z, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwzw(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.z, v.w);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwwx(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.w, v.x);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwwy(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.w, v.y);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwwz(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.w, v.z);
  }

  template <VECTOR_TEMPLATE_PARAMS>
  NO_DISCARD constexpr Vector<T, 4> wwww(const VECTOR_TYPE &v) noexcept
  {
    SWIZZLE_BOUNDS_CHECK(4, 4);
    return Vector<T, 4>(v.w, v.w, v.w, v.w);
  }

#pragma endregion

#undef SWIZZLE_BOUNDS_CHECK
}