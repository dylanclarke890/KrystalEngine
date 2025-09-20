#pragma once

#include "Krystal.Lib/Attributes.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Maths/Conventions.hpp"
#include "Krystal.Maths/Maths.hpp"
#include "Krystal.Maths/Matrix.hpp"

#include <cassert>
#include <cmath>
#include <limits>

namespace Krys::Maths
{
#pragma region Ortho

  template <Arithmetic T>
  NO_DISCARD constexpr auto Ortho_LH_ZO(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    auto result = Identity<Matrix4x4<T>>();

    result[0][0] = T(2) / (right - left);
    result[1][1] = T(2) / (top - bottom);
    result[2][2] = T(1) / (zFar - zNear);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -zNear / (zFar - zNear);

    return result;
  }

  template <Arithmetic T>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_LH_NO(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    auto result = Identity<Matrix4x4<T>>();

    result[0][0] = T(2) / (right - left);
    result[1][1] = T(2) / (top - bottom);
    result[2][2] = T(2) / (zFar - zNear);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(zFar + zNear) / (zFar - zNear);

    return result;
  }

  template <Arithmetic T>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_RH_ZO(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    auto result = Identity<Matrix4x4<T>>();

    result[0][0] = T(2) / (right - left);
    result[1][1] = T(2) / (top - bottom);
    result[2][2] = -T(1) / (zFar - zNear);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -zNear / (zFar - zNear);

    return result;
  }

  template <Arithmetic T>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_RH_NO(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    auto result = Identity<Matrix4x4<T>>();

    result[0][0] = T(2) / (right - left);
    result[1][1] = T(2) / (top - bottom);
    result[2][2] = -T(2) / (zFar - zNear);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);
    result[3][2] = -(zFar + zNear) / (zFar - zNear);

    return result;
  }

  template <Arithmetic T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_LH(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return Ortho_LH_ZO(left, right, bottom, top, zNear, zFar);
    }
    else
    {
      return Ortho_LH_NO(left, right, bottom, top, zNear, zFar);
    }
  }

  template <Arithmetic T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_RH(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return Ortho_RH_ZO(left, right, bottom, top, zNear, zFar);
    }
    else
    {
      return Ortho_RH_NO(left, right, bottom, top, zNear, zFar);
    }
  }

  template <Arithmetic T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_NO(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Ortho_LH_NO(left, right, bottom, top, zNear, zFar);
    }
    else
    {
      return Ortho_RH_NO(left, right, bottom, top, zNear, zFar);
    }
  }

  template <Arithmetic T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> Ortho_ZO(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Ortho_LH_ZO(left, right, bottom, top, zNear, zFar);
    }
    else
    {
      return Ortho_RH_ZO(left, right, bottom, top, zNear, zFar);
    }
  }

  template <Arithmetic T, Conventions::Handedness Handedness = Conventions::DefaultHandedness,
            Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Ortho(T left, T right, T bottom, T top, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Ortho_LH<T, DepthRange>(left, right, bottom, top, zNear, zFar);
    }
    else
    {
      return Ortho_RH<T, DepthRange>(left, right, bottom, top, zNear, zFar);
    }
  }

  template <Arithmetic T, Conventions::Handedness Handedness = Conventions::DefaultHandedness,
            Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Ortho(T width, T height, T depth)
  {
    const auto right = width;
    const auto left = -right;
    const auto top = height;
    const auto bottom = -top;
    const auto farPlane = depth;
    const auto nearPlane = -farPlane;

    return Ortho<T, Handedness, DepthRange>(left, right, bottom, top, nearPlane, farPlane);
  }

  template <Arithmetic T>
  NO_DISCARD constexpr Matrix4x4<T> Ortho(T left, T right, T bottom, T top)
  {
    auto result = Identity<Matrix4x4<T>>();

    result[0][0] = T(2) / (right - left);
    result[1][1] = T(2) / (top - bottom);
    result[2][2] = -T(1);
    result[3][0] = -(right + left) / (right - left);
    result[3][1] = -(top + bottom) / (top - bottom);

    return result;
  }

#pragma endregion

#pragma region Frustum

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_LH_ZO(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    Matrix4x4<T> result(0);
    result[0][0] = (T(2) * nearVal) / (right - left);
    result[1][1] = (T(2) * nearVal) / (top - bottom);
    result[2][0] = -(right + left) / (right - left);
    result[2][1] = -(top + bottom) / (top - bottom);
    result[2][2] = farVal / (farVal - nearVal);
    result[2][3] = T(1);
    result[3][2] = -(farVal * nearVal) / (farVal - nearVal);
    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_LH_NO(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    Matrix4x4<T> result(0);
    result[0][0] = (T(2) * nearVal) / (right - left);
    result[1][1] = (T(2) * nearVal) / (top - bottom);
    result[2][0] = -(right + left) / (right - left);
    result[2][1] = -(top + bottom) / (top - bottom);
    result[2][2] = (farVal + nearVal) / (farVal - nearVal);
    result[2][3] = T(1);
    result[3][2] = -(T(2) * farVal * nearVal) / (farVal - nearVal);
    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_RH_ZO(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    Matrix4x4<T> result(0);
    result[0][0] = (T(2) * nearVal) / (right - left);
    result[1][1] = (T(2) * nearVal) / (top - bottom);
    result[2][0] = (right + left) / (right - left);
    result[2][1] = (top + bottom) / (top - bottom);
    result[2][2] = farVal / (nearVal - farVal);
    result[2][3] = T(-1);
    result[3][2] = -(farVal * nearVal) / (farVal - nearVal);
    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_RH_NO(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    Matrix4x4<T> result(0);
    result[0][0] = (T(2) * nearVal) / (right - left);
    result[1][1] = (T(2) * nearVal) / (top - bottom);
    result[2][0] = (right + left) / (right - left);
    result[2][1] = (top + bottom) / (top - bottom);
    result[2][2] = -(farVal + nearVal) / (farVal - nearVal);
    result[2][3] = T(-1);
    result[3][2] = -(T(2) * farVal * nearVal) / (farVal - nearVal);
    return result;
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_ZO(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Frustum_LH_ZO(left, right, bottom, top, nearVal, farVal);
    }
    else
    {
      return Frustum_RH_ZO(left, right, bottom, top, nearVal, farVal);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_NO(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Frustum_LH_NO(left, right, bottom, top, nearVal, farVal);
    }
    else
    {
      return Frustum_RH_NO(left, right, bottom, top, nearVal, farVal);
    }
  }

  template <typename T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_LH(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return Frustum_LH_ZO(left, right, bottom, top, nearVal, farVal);
    }
    else
    {
      return Frustum_LH_NO(left, right, bottom, top, nearVal, farVal);
    }
  }

  template <typename T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Frustum_RH(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return Frustum_RH_ZO(left, right, bottom, top, nearVal, farVal);
    }
    else
    {
      return Frustum_RH_NO(left, right, bottom, top, nearVal, farVal);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness,
            Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Frustum(T left, T right, T bottom, T top, T nearVal, T farVal)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Frustum_LH<T, DepthRange>(left, right, bottom, top, nearVal, farVal);
    }
    else
    {
      return Frustum_RH<T, DepthRange>(left, right, bottom, top, nearVal, farVal);
    }
  }

#pragma endregion

#pragma region Perspective

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_RH_ZO(T fovy, T aspect, T zNear, T zFar)
  {
    assert(Abs(aspect - std::numeric_limits<T>::epsilon()) > Zero<T>());

    const T tanHalfFovy = std::tan(fovy / T(2));

    Matrix4x4<T> result(Zero<T>());
    result[0][0] = One<T>() / (aspect * tanHalfFovy);
    result[1][1] = One<T>() / (tanHalfFovy);
    result[2][2] = zFar / (zNear - zFar);
    result[2][3] = -One<T>();
    result[3][2] = -(zFar * zNear) / (zFar - zNear);
    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_RH_NO(T fovy, T aspect, T zNear, T zFar)
  {
    assert(Abs(aspect - std::numeric_limits<T>::epsilon()) > Zero<T>());

    const T tanHalfFovy = std::tan(fovy / T(2));

    Matrix4x4<T> result(Zero<T>());
    result[0][0] = One<T>() / (aspect * tanHalfFovy);
    result[1][1] = One<T>() / (tanHalfFovy);
    result[2][2] = -(zFar + zNear) / (zFar - zNear);
    result[2][3] = -One<T>();
    result[3][2] = -(T(2) * zFar * zNear) / (zFar - zNear);
    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_LH_ZO(T fovy, T aspect, T zNear, T zFar)
  {
    assert(Abs(aspect - std::numeric_limits<T>::epsilon()) > Zero<T>());

    const T tanHalfFovy = std::tan(fovy / T(2));

    Matrix4x4<T> result(Zero<T>());
    result[0][0] = One<T>() / (aspect * tanHalfFovy);
    result[1][1] = One<T>() / (tanHalfFovy);
    result[2][2] = zFar / (zFar - zNear);
    result[2][3] = One<T>();
    result[3][2] = -(zFar * zNear) / (zFar - zNear);
    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_LH_NO(T fovy, T aspect, T zNear, T zFar)
  {
    assert(Abs(aspect - std::numeric_limits<T>::epsilon()) > Zero<T>());

    const T tanHalfFovy = std::tan(fovy / T(2));

    Matrix4x4<T> result(Zero<T>());
    result[0][0] = One<T>() / (aspect * tanHalfFovy);
    result[1][1] = One<T>() / (tanHalfFovy);
    result[2][2] = (zFar + zNear) / (zFar - zNear);
    result[2][3] = One<T>();
    result[3][2] = -(T(2) * zFar * zNear) / (zFar - zNear);
    return result;
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_ZO(T fovy, T aspect, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Perspective_LH_ZO(fovy, aspect, zNear, zFar);
    }
    else
    {
      return Perspective_RH_ZO(fovy, aspect, zNear, zFar);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_NO(T fovy, T aspect, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Perspective_LH_NO(fovy, aspect, zNear, zFar);
    }
    else
    {
      return Perspective_RH_NO(fovy, aspect, zNear, zFar);
    }
  }

  template <typename T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_LH(T fovy, T aspect, T zNear, T zFar)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return Perspective_LH_ZO(fovy, aspect, zNear, zFar);
    }
    else
    {
      return Perspective_LH_NO(fovy, aspect, zNear, zFar);
    }
  }

  template <typename T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Perspective_RH(T fovy, T aspect, T zNear, T zFar)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return Perspective_RH_ZO(fovy, aspect, zNear, zFar);
    }
    else
    {
      return Perspective_RH_NO(fovy, aspect, zNear, zFar);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness,
            Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Perspective(T fovy, T aspect, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return Perspective_LH<T, DepthRange>(fovy, aspect, zNear, zFar);
    }
    else
    {
      return Perspective_RH<T, DepthRange>(fovy, aspect, zNear, zFar);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness,
            Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> Perspective(T fovy, T width, T height, T zNear, T zFar)
  {
    return Perspective<T, Handedness, DepthRange>(fovy, width / height, zNear, zFar);
  }

#pragma endregion

#pragma region PerspectiveFov

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_RH_ZO(T fov, T width, T height, T zNear, T zFar)
  {
    assert(width > Zero<T>());
    assert(height > Zero<T>());
    assert(fov > Zero<T>());

    const T rad = fov;
    const T h = std::cos(T(0.5) * rad) / std::sin(T(0.5) * rad);
    const T w = h * height / width; /// todo max(width , Height) / min(width , Height)?

    Matrix4x4<T> result(Zero<T>());

    result[0][0] = w;
    result[1][1] = h;
    result[2][2] = zFar / (zNear - zFar);
    result[2][3] = -One<T>();
    result[3][2] = -(zFar * zNear) / (zFar - zNear);

    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_RH_NO(T fov, T width, T height, T zNear, T zFar)
  {
    assert(width > Zero<T>());
    assert(height > Zero<T>());
    assert(fov > Zero<T>());

    const T rad = fov;
    const T h = std::cos(Half<T>() * rad) / std::sin(Half<T>() * rad);
    const T w = h * height / width; /// todo max(width , Height) / min(width , Height)?

    Matrix4x4<T> result(Zero<T>());

    result[0][0] = w;
    result[1][1] = h;
    result[2][2] = -(zFar + zNear) / (zFar - zNear);
    result[2][3] = -One<T>();
    result[3][2] = -(T(2) * zFar * zNear) / (zFar - zNear);

    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_LH_ZO(T fov, T width, T height, T zNear, T zFar)
  {
    assert(width > Zero<T>());
    assert(height > Zero<T>());
    assert(fov > Zero<T>());

    const T rad = fov;
    const T h = std::cos(T(0.5) * rad) / std::sin(T(0.5) * rad);
    const T w = h * height / width; /// todo max(width , Height) / min(width , Height)?

    Matrix4x4<T> result(Zero<T>());

    result[0][0] = w;
    result[1][1] = h;
    result[2][2] = zFar / (zFar - zNear);
    result[2][3] = One<T>();
    result[3][2] = -(zFar * zNear) / (zFar - zNear);

    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_LH_NO(T fov, T width, T height, T zNear, T zFar)
  {
    assert(width > Zero<T>());
    assert(height > Zero<T>());
    assert(fov > Zero<T>());

    const T rad = fov;
    const T h = std::cos(T(0.5) * rad) / std::sin(T(0.5) * rad);
    const T w = h * height / width; /// todo max(width , Height) / min(width , Height)?

    Matrix4x4<T> result(Zero<T>());

    result[0][0] = w;
    result[1][1] = h;
    result[2][2] = (zFar + zNear) / (zFar - zNear);
    result[2][3] = One<T>();
    result[3][2] = -(T(2) * zFar * zNear) / (zFar - zNear);

    return result;
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_ZO(T fov, T width, T height, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return PerspectiveFov_LH_ZO(fov, width, height, zNear, zFar);
    }
    else
    {
      return PerspectiveFov_RH_ZO(fov, width, height, zNear, zFar);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_NO(T fov, T width, T height, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return PerspectiveFov_LH_NO(fov, width, height, zNear, zFar);
    }
    else
    {
      return PerspectiveFov_RH_NO(fov, width, height, zNear, zFar);
    }
  }

  template <typename T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_LH(T fov, T width, T height, T zNear, T zFar)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return PerspectiveFov_LH_ZO(fov, width, height, zNear, zFar);
    }
    else
    {
      return PerspectiveFov_LH_NO(fov, width, height, zNear, zFar);
    }
  }

  template <typename T, Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov_RH(T fov, T width, T height, T zNear, T zFar)
  {
    if constexpr (DepthRange == Conventions::DepthRange::ZeroToOne)
    {
      return PerspectiveFov_RH_ZO(fov, width, height, zNear, zFar);
    }
    else
    {
      return PerspectiveFov_RH_NO(fov, width, height, zNear, zFar);
    }
  }

  template <typename T, Conventions::Handedness Handedness = Conventions::DefaultHandedness,
            Conventions::DepthRange DepthRange = Conventions::DefaultDepthRange>
  NO_DISCARD constexpr Matrix4x4<T> PerspectiveFov(T fov, T width, T height, T zNear, T zFar)
  {
    if constexpr (Handedness == Conventions::Handedness::Left)
    {
      return PerspectiveFov_LH<T, DepthRange>(fov, width, height, zNear, zFar);
    }
    else
    {
      return PerspectiveFov_RH<T, DepthRange>(fov, width, height, zNear, zFar);
    }
  }

#pragma endregion

#pragma region InfinitePerspective

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> InfinitePerspective_RH_NO(T fovy, T aspect, T zNear)
  {
    const T range = std::tan(fovy / T(2)) * zNear;
    const T left = -range * aspect;
    const T right = range * aspect;
    const T bottom = -range;
    const T top = range;

    Matrix4x4<T> result(T(0));

    result[0][0] = (T(2) * zNear) / (right - left);
    result[1][1] = (T(2) * zNear) / (top - bottom);
    result[2][2] = -T(1);
    result[2][3] = -T(1);
    result[3][2] = -T(2) * zNear;

    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> InfinitePerspective_RH_ZO(T fovy, T aspect, T zNear)
  {
    const T range = std::tan(fovy / T(2)) * zNear;
    const T left = -range * aspect;
    const T right = range * aspect;
    const T bottom = -range;
    const T top = range;

    Matrix4x4<T> result(T(0));

    result[0][0] = (T(2) * zNear) / (right - left);
    result[1][1] = (T(2) * zNear) / (top - bottom);
    result[2][2] = -T(1);
    result[2][3] = -T(1);
    result[3][2] = -zNear;

    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> InfinitePerspective_LH_NO(T fovy, T aspect, T zNear)
  {
    const T range = std::tan(fovy / T(2)) * zNear;
    const T left = -range * aspect;
    const T right = range * aspect;
    const T bottom = -range;
    const T top = range;

    Matrix4x4<T> result(T(0));

    result[0][0] = (T(2) * zNear) / (right - left);
    result[1][1] = (T(2) * zNear) / (top - bottom);
    result[2][2] = T(1);
    result[2][3] = T(1);
    result[3][2] = -T(2) * zNear;

    return result;
  }

  template <typename T>
  NO_DISCARD constexpr Matrix4x4<T> InfinitePerspective_LH_ZO(T fovy, T aspect, T zNear)
  {
    const T range = std::tan(fovy / T(2)) * zNear;
    const T left = -range * aspect;
    const T right = range * aspect;
    const T bottom = -range;
    const T top = range;

    Matrix4x4<T> result(T(0));

    result[0][0] = (T(2) * zNear) / (right - left);
    result[1][1] = (T(2) * zNear) / (top - bottom);
    result[2][2] = T(1);
    result[2][3] = T(1);
    result[3][2] = -zNear;

    return result;
  }

#pragma endregion
}