#pragma once

#include "Krystal.Lib/Core/Compiler.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Maths/Conventions.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Quaternion.hpp"
#include "Krystal.Maths/Vector.hpp"
#include <cmath>

namespace Krys::Maths
{
  /// @brief Apply a translation vector to a matrix.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Translate(const Matrix4x4<T> &m, const Vector3<T> &v) noexcept
  {
    Matrix<T, 4, 4> result {m};
    result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
    return result;
  }

  /// @brief Create a matrix with the given translation applied.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Translate(const Vector3<T> &v) noexcept
  {
    return Translate(Identity<Matrix4x4<T>>(), v);
  }

  /// @brief Apply a rotation to a matrix around axis `v`.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Rotate(const Matrix4x4<T> &m, T angle, const Vector3<T> &v) noexcept
  {
    const T c = std::cos(angle);
    const T s = std::sin(angle);

    Vector3<T> axis(Normalize(v));
    Vector3<T> temp((T(1) - c) * axis);

    Matrix4x4<T> rot {};
    rot[0][0] = c + temp[0] * axis[0];
    rot[0][1] = temp[0] * axis[1] + s * axis[2];
    rot[0][2] = temp[0] * axis[2] - s * axis[1];

    rot[1][0] = temp[1] * axis[0] - s * axis[2];
    rot[1][1] = c + temp[1] * axis[1];
    rot[1][2] = temp[1] * axis[2] + s * axis[0];

    rot[2][0] = temp[2] * axis[0] + s * axis[1];
    rot[2][1] = temp[2] * axis[1] - s * axis[0];
    rot[2][2] = c + temp[2] * axis[2];

    Matrix4x4<T> result;
    result[0] = m[0] * rot[0][0] + m[1] * rot[0][1] + m[2] * rot[0][2];
    result[1] = m[0] * rot[1][0] + m[1] * rot[1][1] + m[2] * rot[1][2];
    result[2] = m[0] * rot[2][0] + m[1] * rot[2][1] + m[2] * rot[2][2];
    result[3] = m[3];

    return result;
  }

  /// @brief Create a matrix with the given rotation applied.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Rotate(T angle, const Vector3<T> &v) noexcept
  {
    return Rotate(Identity<Matrix4x4<T>>(), angle, v);
  }

  /// @brief Apply a scale vector to a matrix.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Scale(const Matrix4x4<T> &m, const Vector3<T> &v) noexcept
  {
    Matrix4x4<T> result;
    result[0] = m[0] * v[0];
    result[1] = m[1] * v[1];
    result[2] = m[2] * v[2];
    result[3] = m[3];

    return result;
  }

  /// @brief Create a matrix with the given scale applied.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto Scale(const Vector3<T> &v) noexcept
  {
    return Scale(Identity<Matrix4x4<T>>(), v);
  }

  /// @brief Constructs a right handed look at matrix.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto LookAt_RH(const Vector3<T> &eye, const Vector3<T> &center,
                                      const Vector3<T> &up) noexcept -> Matrix4x4<T>
  {
    const Vector3<T> f(Normalize(center - eye));
    const Vector3<T> s(Normalize(Cross(f, up)));
    const Vector3<T> u(Cross(s, f));

    auto result = Identity<Matrix4x4<T>>();
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] = -f.x;
    result[1][2] = -f.y;
    result[2][2] = -f.z;
    result[3][0] = -Dot(s, eye);
    result[3][1] = -Dot(u, eye);
    result[3][2] = Dot(f, eye);
    return result;
  }

  /// @brief Constructs a left handed look at matrix.
  template <FloatingPoint T>
  KRYS_NODISCARD constexpr auto LookAt_LH(const Vector3<T> &eye, const Vector3<T> &center,
                                      const Vector3<T> &up) noexcept -> Matrix4x4<T>
  {
    const Vector3<T> f(Normalize(center - eye));
    const Vector3<T> s(Normalize(Cross(up, f)));
    const Vector3<T> u(Cross(f, s));

    auto result = Identity<Matrix4x4<T>>();
    result[0][0] = s.x;
    result[1][0] = s.y;
    result[2][0] = s.z;
    result[0][1] = u.x;
    result[1][1] = u.y;
    result[2][1] = u.z;
    result[0][2] = f.x;
    result[1][2] = f.y;
    result[2][2] = f.z;
    result[3][0] = -Dot(s, eye);
    result[3][1] = -Dot(u, eye);
    result[3][2] = -Dot(f, eye);
    return result;
  }

  /// @brief Constructs a look at matrix.
  template <FloatingPoint T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  KRYS_NODISCARD constexpr auto LookAt(const Vector3<T> &eye, const Vector3<T> &center,
                                   const Vector3<T> &up) noexcept -> Matrix4x4<T>
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return LookAt_LH(eye, center, up);
    }
    else
    {
      return LookAt_RH(eye, center, up);
    }
  }

  /// @brief Rotates the given vector by the given quaternion.
  template <Number T>
  KRYS_NODISCARD constexpr Vector3<T> Rotate(const Quaternion<T> &q, const Vector3<T> &v) noexcept
  {
    // Convert the vector to a quaternion with w = 0
    Quaternion<T> quat(0.0f, v.x, v.y, v.z);

    // Perform the rotation: q * v * q^-1
    // The conjugate of a quaternion is the same as the inverse, if the quaternion is normalized.
    Quaternion<T> rotated = q * quat * q.Conjugate();

    // Extract the rotated vector (x, y, z) from the resulting quaternion
    return Vector3<T>(rotated.x, rotated.y, rotated.z);
  }

  /// @brief Rotates the given vector by the given quaternion.
  template <Number T>
  KRYS_NODISCARD constexpr Vector3<T> Rotate(const Quaternion<T> &q, T x, T y, T z) noexcept
  {
    return Rotate(q, Vector3<T>(x, y, z));
  }

  /// @brief Creates a new quaternion that rotates about the positive X axis by the given angle.
  template <Number T>
  KRYS_NODISCARD Quaternion<T> RotateX(T angle) noexcept
  {
    return Quaternion<T>(Vector3<T>(1, 0, 0), angle);
  }

  /// @brief Creates a new quaternion that rotates about the positive Y axis by the given angle.
  template <Number T>
  KRYS_NODISCARD Quaternion<T> RotateY(T angle) noexcept
  {
    return Quaternion<T>(Vector3<T>(0, 1, 0), angle);
  }

  /// @brief Creates a new quaternion that rotates about the positive Z axis by the given angle.
  template <Number T>
  KRYS_NODISCARD Quaternion<T> RotateZ(T angle) noexcept
  {
    return Quaternion<T>(Vector3<T>(0, 0, 1), angle);
  }

  /// @brief Creates a new quaternion that rotates about the given axis by the given angle.
  template <Number T>
  KRYS_NODISCARD Quaternion<T> RotateAxisAngle(const Vector3<T> &axis, T angle) noexcept
  {
    return Quaternion<T>(axis, angle);
  }

  /// @brief Get the world X axis from the given quaternion.
  template <Number T>
  KRYS_NODISCARD constexpr Vector3<T> GetWorldX(const Quaternion<T> &q) noexcept
  {
    return Rotate(q, Vector3<T>(1, 0, 0));
  }

  /// @brief Get the world Y axis from the given quaternion.
  template <Number T>
  KRYS_NODISCARD constexpr Vector3<T> GetWorldY(const Quaternion<T> &q) noexcept
  {
    return Rotate(q, Vector3<T>(0, 1, 0));
  }

  /// @brief Get the world Z axis from the given quaternion.
  template <Number T>
  KRYS_NODISCARD constexpr Vector3<T> GetWorldZ(const Quaternion<T> &q) noexcept
  {
    return Rotate(q, Vector3<T>(0, 0, 1));
  }

  /// @brief Creates a new quaternion that rotates sourceDirection vector (in world space) to coincide with
  /// the targetDirection vector (in world space). Rotation is performed around the origin. The vectors
  /// sourceDirection and targetDirection are assumed to be normalized.
  /// @note There are multiple such rotations - this function returns the rotation that has the shortest angle
  /// (when decomposed to axis-angle notation).
  /// @return the quaternion that rotates sourceDirection to targetDirection.
  template <FloatingPoint T>
  KRYS_NODISCARD Quaternion<T> RotateFromTo(const Vector3<T> &sourceDirection,
                                        const Vector3<T> &targetDirection) noexcept
  {
    // If sourceDirection == targetDirection, the cross product comes out zero, and normalization would fail.
    // In that case, pick an arbitrary axis.
    Vector3<T> axis = Cross(sourceDirection, targetDirection);
    T oldLength = Length(axis);
    axis = Normalize(axis);

    if (oldLength != 0.f)
    {
      T halfCosAngle = 0.5f * sourceDirection.Dot(targetDirection);
      T cosHalfAngle = std::sqrt(0.5f + halfCosAngle);
      T sinHalfAngle = std::sqrt(0.5f - halfCosAngle);
      return Quaternion<T>(axis.x * sinHalfAngle, axis.y * sinHalfAngle, axis.z * sinHalfAngle, cosHalfAngle);
    }
    else
      return Quaternion<T>(1.0f, 0.0f, 0.0f, 0.0f);
  }
}