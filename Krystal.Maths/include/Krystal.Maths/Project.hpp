#pragma once

#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Maths/Matrix.hpp"
#include "Krystal.Maths/Vector.hpp"

namespace Krys::Maths
{
  /// Map the specified object coordinates (obj.x, obj.y, obj.z) into window coordinates.
  /// The near and far clip planes correspond to z normalized device coordinates of 0 and +1 respectively.
  template <FloatingPoint T, Number U>
  KRYS_NODISCARD constexpr auto Project_ZO(const Vector<T, 3> &obj, const Matrix<T, 4, 4> &model,
                                       const Matrix<T, 4, 4> &proj, const Vector<U, 4> &viewport) noexcept
  {
    Vector<T, 4> tmp(obj.x, obj.y, obj.z, T(1));
    tmp = model * tmp;
    tmp = proj * tmp;

    tmp /= tmp.w;
    tmp.x = tmp.x * T(0.5) + T(0.5);
    tmp.y = tmp.y * T(0.5) + T(0.5);

    tmp[0] = tmp[0] * T(viewport[2]) + T(viewport[0]);
    tmp[1] = tmp[1] * T(viewport[3]) + T(viewport[1]);

    return Vector<T, 3>(tmp);
  }

  /// Map the specified object coordinates (obj.x, obj.y, obj.z) into window coordinates.
  /// The near and far clip planes correspond to z normalized device coordinates of -1 and +1 respectively.
  template <FloatingPoint T, Number U>
  KRYS_NODISCARD constexpr auto Project_NO(const Vector<T, 3> &obj, const Matrix<T, 4, 4> &model,
                                       const Matrix<T, 4, 4> &proj, const Vector<U, 4> &viewport)
  {
    Vector<T, 4> tmp(obj.x, obj.y, obj.z, T(1));
    tmp = model * tmp;
    tmp = proj * tmp;

    tmp /= tmp.w;
    tmp = tmp * T(0.5) + T(0.5);
    tmp[0] = tmp[0] * T(viewport[2]) + T(viewport[0]);
    tmp[1] = tmp[1] * T(viewport[3]) + T(viewport[1]);

    return Vector<T, 3>(tmp);
  }

  // TODO(fix): use policy based design to select between ZO and NO at compile time
  /// Map the specified object coordinates (obj.x, obj.y, obj.z) into window coordinates.
  template <FloatingPoint T, Number U>
  KRYS_NODISCARD constexpr Vector<T, 3> Project(const Vector<T, 3> &obj, const Matrix<T, 4, 4> &model,
                                            const Matrix<T, 4, 4> &proj, const Vector<U, 4> &viewport)
  {
#if KRYS_MATRIX_DEPTH_RANGE == KRYS_MATRIX_DEPTH_RANGE_ZERO_TO_ONE
    return Project_ZO(obj, model, proj, viewport);
#else
    return Project_NO(obj, model, proj, viewport);
#endif
  }
}