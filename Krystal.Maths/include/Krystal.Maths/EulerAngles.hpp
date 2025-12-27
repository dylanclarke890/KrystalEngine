#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Quaternion.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <cmath>

namespace Krys::Impl
{
  using namespace Krys::Maths;

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerXYX(const Mat &m) noexcept
  {
    if (m[0][0] < One<T>())
    {
      if (m[0][0] > -One<T>())
        return {std::acos(m[0][0]), std::atan2(m[1][0], -m[2][0]), std::atan2(m[0][1], m[0][2])};
      else
        // Not a unique solution.
        return {Pi<T>(), -std::atan2(-m[1][2], m[1][1]), Zero<T>()};
    }

    return {Zero<T>(), std::atan2(-m[1][2], m[1][1]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerXZX(const Mat &m) noexcept
  {
    if (m[0][0] < One<T>())
    {
      if (m[0][0] > -One<T>())
        return {std::acos(m[0][0]), std::atan2(m[2][0], m[1][0]), std::atan2(m[0][2], -m[0][1])};
      else
        return {Pi<T>(), -std::atan2(m[2][1], m[2][2]), Zero<T>()};
    }

    return {Zero<T>(), std::atan2(m[2][1], m[2][2]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerXYZ(const Mat &m) noexcept
  {
    if (m[0][2] < One<T>())
    {
      if (m[0][2] > -One<T>())
        return {std::std::asin(m[0][2]), std::atan2(-m[1][2], m[2][2]), std::atan2(-m[0][1], m[0][0])};
      else
        // Not a unique solution: z - x = atan2(m[1][0], m[1][1]);
        return {-HalfPi<T>(), -std::atan2(m[1][0], m[1][1]), Zero<T>()};
    }

    return {HalfPi<T>(), std::atan2(m[1][0], m[1][1]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerYXY(const Mat &m) noexcept
  {
    if (m[1][1] < One<T>())
    {
      if (m[1][1] > -One<T>())
        return {std::acos(m[1][1]), std::atan2(m[0][1], m[2][1]), std::atan2(m[1][0], -m[1][2])};
      else
        // Not a unique solution.
        return {Pi<T>(), -std::atan2(m[0][2], m[0][0]), Zero<T>()};
    }

    // Not a unique solution.
    return {Zero<T>(), std::atan2(m[0][2], m[0][0]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerYZY(const Mat &m) noexcept
  {
    if (m[1][1] < One<T>())
    {
      if (m[1][1] > -One<T>())
        return {std::acos(m[1][1]), std::atan2(m[2][1], -m[0][1]), std::atan2(m[1][2], m[1][0])};
      else
        return {Pi<T>(), -std::atan2(-m[2][0], m[2][2]), Zero<T>()};
    }

    // Not a unique solution.
    return {Zero<T>(), std::atan2(-m[2][0], m[2][2]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerZXZ(const Mat &m) noexcept
  {
    if (m[2][2] < One<T>())
    {
      if (m[2][2] > -One<T>())
        return {std::acos(m[2][2]), std::atan2(m[0][2], -m[1][2]), std::atan2(m[2][0], m[2][1])};
      else
        return {Pi<T>(), -std::atan2(-m[0][1], m[0][0]), Zero<T>()};
    }

    return {Zero<T>(), std::atan2(-m[0][1], m[0][0]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerZYZ(const Mat &m) noexcept
  {
    if (m[2][2] < One<T>())
    {
      if (m[2][2] > -One<T>())
        return {std::acos(m[2][2]), std::atan2(m[1][2], m[0][2]), std::atan2(m[2][1], -m[2][0])};
      else
        return {Pi<T>(), -std::atan2(m[1][0], m[1][1]), Zero<T>()};
    }

    return {Zero<T>(), std::atan2(m[1][0], m[1][1]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerXZY(const Mat &m) noexcept
  {
    if (m[0][1] < One<T>())
    {
      if (m[0][1] > -One<T>())
        return {std::asin(-m[0][1]), std::atan2(m[2][1], m[1][1]), std::atan2(m[0][2], m[0][0])};
      else
        // Not a unique solution: y - x = atan2(-m[2][0], m[2][2]);
        return {HalfPi<T>(), -std::atan2(-m[2][0], m[2][2]), Zero<T>()};
    }

    // Not a unique solution: y + x = atan2(-m[2][0], m[2][2]);
    return {-HalfPi<T>(), std::atan2(-m[2][0], m[2][2]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerYXZ(const Mat &m) noexcept
  {
    if (m[1][2] < One<T>())
    {
      if (m[1][2] > -One<T>())
      {
        return {std::asin(-m[1][2]), std::atan2(m[0][2], m[2][2]), std::atan2(m[1][0], m[1][1])};
      }
      else
        return {HalfPi<T>(), -std::atan2(-m[0][1], m[0][0]), Zero<T>()};
    }

    return {-HalfPi<T>(), std::atan2(-m[0][1], m[0][0]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerYZX(const Mat &m) noexcept
  {
    if (m[1][0] < One<T>())
    {
      if (m[1][0] > -One<T>())
        return {std::asin(m[1][0]), std::atan2(-m[2][0], m[0][0]), std::atan2(-m[1][2], m[1][1])};
      else
        return {-HalfPi<T>(), -std::atan2(m[2][1], m[2][2]), Zero<T>()};
    }

    return {HalfPi<T>(), std::atan2(m[2][1], m[2][2]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerZXY(const Mat &m) noexcept
  {
    if (m[2][1] < One<T>())
    {
      if (m[2][1] > -One<T>())
        return {std::asin(m[2][1]), std::atan2(-m[0][1], m[1][1]), std::atan2(-m[2][0], m[2][2])};
      else
        return {-HalfPi<T>(), -std::atan2(m[0][2], m[0][0]), Zero<T>()};
    }

    return {HalfPi<T>(), std::atan2(m[0][2], m[0][0]), Zero<T>()};
  }

  template <typename Mat, Number T>
  KRYS_NODISCARD constexpr Vector3<T> ExtractEulerZYX(const Mat &m) noexcept
  {
    if (m[2][0] < One<T>())
    {
      if (m[2][0] > -One<T>())
        return {std::asin(-m[2][0]), std::atan2(m[1][0], m[0][0]), std::atan2(m[2][1], m[2][2])};
      else
        return {HalfPi<T>(), -std::atan2(-m[1][2], m[1][1]), Zero<T>()};
    }

    return {-HalfPi<T>(), std::atan2(-m[1][2], m[1][1]), Zero<T>()};
  }
}

namespace Krys::Maths
{
#pragma region Helper Macros

#define TO_EULER_ANGLE_FUNC(Convention, MatType)                                                             \
  template <Number T>                                                                                        \
  KRYS_NODISCARD constexpr Vector3<T> ToEuler##Convention(const MatType<T> &m) noexcept                          \
  {                                                                                                          \
    return Impl::ExtractEuler##Convention<MatType<T>, T>(m);                                                 \
  }

#define QUATERNION_TO_EULER_ANGLE_FUNC(Convention)                                                           \
  template <Number T>                                                                                        \
  KRYS_NODISCARD constexpr Vector3<T> ToEuler##Convention(const Quaternion<T> &q) noexcept                       \
  {                                                                                                          \
    return ToEuler##Convention(q.ToMat3x3());                                                                \
  }

#define TO_EULER_ANGLE_FUNCS(Convention)                                                                     \
  TO_EULER_ANGLE_FUNC(Convention, Matrix3x3)                                                                 \
  TO_EULER_ANGLE_FUNC(Convention, Matrix3x4)                                                                 \
  TO_EULER_ANGLE_FUNC(Convention, Matrix4x4)                                                                 \
  QUATERNION_TO_EULER_ANGLE_FUNC(Convention)

#pragma endregion

  TO_EULER_ANGLE_FUNCS(XYX)
  TO_EULER_ANGLE_FUNCS(XZX)
  TO_EULER_ANGLE_FUNCS(XYZ)
  TO_EULER_ANGLE_FUNCS(YXY)
  TO_EULER_ANGLE_FUNCS(YZY)
  TO_EULER_ANGLE_FUNCS(ZXZ)
  TO_EULER_ANGLE_FUNCS(ZYZ)
  TO_EULER_ANGLE_FUNCS(XZY)
  TO_EULER_ANGLE_FUNCS(YXZ)
  TO_EULER_ANGLE_FUNCS(YZX)
  TO_EULER_ANGLE_FUNCS(ZXY)
  TO_EULER_ANGLE_FUNCS(ZYX)

#undef QUATERNION_TO_EULER_ANGLE_FUNC
#undef TO_EULER_ANGLE_FUNC
#undef TO_EULER_ANGLE_FUNCS

  /// @brief Creates a quaternion from Euler angles using the XYX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXYX(T x2, T y, T x) noexcept
  {
    return (RotateX(x2) * RotateY(y) * RotateX(x)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the XYX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXYX(const Vector3<T> &v) noexcept
  {
    return FromEulerXYX(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the XZX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXZX(T x2, T z, T x) noexcept
  {
    return (RotateX(x2) * RotateZ(z) * RotateX(x)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the XZX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXZX(const Vector3<T> &v) noexcept
  {
    return FromEulerXZX(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the YXY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYXY(T y2, T x, T y) noexcept
  {
    return (RotateY(y2) * RotateX(x) * RotateY(y)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the YXY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYXY(const Vector3<T> &v) noexcept
  {
    return FromEulerYXY(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the YZY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYZY(T y2, T z, T y) noexcept
  {
    return (RotateY(y2) * RotateZ(z) * RotateY(y)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the YZY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYZY(const Vector3<T> &v) noexcept
  {
    return FromEulerYZY(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the ZXZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZXZ(T z2, T x, T z) noexcept
  {
    return (RotateZ(z2) * RotateX(x) * RotateZ(z)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the ZXZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZXZ(const Vector3<T> &v) noexcept
  {
    return FromEulerZXZ(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the ZYZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZYZ(T z2, T y, T z) noexcept
  {
    return (RotateZ(z2) * RotateY(y) * RotateZ(z)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the ZYZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZYZ(const Vector3<T> &v) noexcept
  {
    return FromEulerZYZ(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the XYZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXYZ(T x, T y, T z) noexcept
  {
    return (RotateX(x) * RotateY(y) * RotateZ(z)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the XYZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXYZ(const Vector3<T> &v) noexcept
  {
    return FromEulerXYZ(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the XZY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXZY(T x, T z, T y) noexcept
  {
    return (RotateX(x) * RotateZ(z) * RotateY(y)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the XZY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerXZY(const Vector3<T> &v) noexcept
  {
    return FromEulerXZY(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the YXZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYXZ(T y, T x, T z) noexcept
  {
    return (RotateY(y) * RotateX(x) * RotateZ(z)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the YXZ convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYXZ(const Vector3<T> &v) noexcept
  {
    return FromEulerYXZ(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the YZX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYZX(T y, T z, T x) noexcept
  {
    return (RotateY(y) * RotateZ(z) * RotateX(x)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the YZX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerYZX(const Vector3<T> &v) noexcept
  {
    return FromEulerYZX(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the ZXY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZXY(T z, T x, T y) noexcept
  {
    return (RotateZ(z) * RotateX(x) * RotateY(y)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the ZXY convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZXY(const Vector3<T> &v) noexcept
  {
    return FromEulerZXY(v.x, v.y, v.z);
  }

  /// @brief Creates a quaternion from Euler angles using the ZYX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZYX(T z, T y, T x) noexcept
  {
    return (RotateZ(z) * RotateY(y) * RotateX(x)).Normalize();
  }

  /// @brief Creates a quaternion from Euler angles using the ZYX convention.
  template <Number T>
  KRYS_NODISCARD constexpr Quaternion<T> FromEulerZYX(const Vector3<T> &v) noexcept
  {
    return FromEulerZYX(v.x, v.y, v.z);
  }
}