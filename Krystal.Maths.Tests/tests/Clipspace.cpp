#include "pch.hpp"

#include "Krystal.Maths/Clipspace.hpp"
#include "Krystal.Maths/Convert.hpp"
#include <cmath>

namespace Krys::Tests
{
  using namespace Krys::Maths;

  constexpr float left = -1.0f;
  constexpr float right = 1.0f;
  constexpr float bottom = -1.0f;
  constexpr float top = 1.0f;
  constexpr float zNear = 0.1f;
  constexpr float zFar = 100.0f;

  constexpr float fovy = Radians(45.0f);
  constexpr float aspect = 16.0f / 9.0f;

  constexpr float fov = Radians(45.0f);
  constexpr float width = 1920.0f;
  constexpr float height = 1080.0f;

#pragma region Ortho

  TEST_CASE("Ortho_LH_ZO", "[Clipspace]")
  {
    constexpr Mat4 expected {
      1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1.0f / (zFar - zNear), 0, 0, 0, -zNear / (zFar - zNear), 1};
    REQUIRE(Ortho_LH_ZO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Ortho_LH_NO", "[Clipspace]")
  {
    constexpr Mat4 expected {
      1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 2.0f / (zFar - zNear), 0, 0, 0, -(zFar + zNear) / (zFar - zNear), 1};
    REQUIRE(Ortho_LH_NO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Ortho_RH_ZO", "[Clipspace]")
  {
    constexpr Mat4 expected {
      1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -1.0f / (zFar - zNear), 0, 0, 0, -zNear / (zFar - zNear), 1};
    REQUIRE(Ortho_RH_ZO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Ortho_RH_NO", "[Clipspace]")
  {
    constexpr Mat4 expected {
      1, 0, 0, 0, 0, 1, 0, 0, 0, 0, -2.0f / (zFar - zNear), 0, 0, 0, -(zFar + zNear) / (zFar - zNear), 1};
    REQUIRE(Ortho_RH_NO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Ortho", "[Clipspace]")
  {
    constexpr Mat4 expected {1.0f, 0.0f, 0.0f,  0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};

    REQUIRE(Ortho(left, right, bottom, top) == expected);
  }

#pragma endregion Ortho

#pragma region Frustum

  TEST_CASE("Frustum_LH_ZO", "[Clipspace]")
  {
    constexpr Mat4 expected {0.1f, 0.0f, 0.0f,      0.0f, 0.0f, 0.1f, 0.0f,        0.0f,
                             0.0f, 0.0f, 1.001001f, 1.0f, 0.0f, 0.0f, -0.1001001f, 0.0f};

    REQUIRE(Frustum_LH_ZO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Frustum_LH_NO", "[Clipspace]")
  {
    constexpr Mat4 expected {0.1f, 0.0f, 0.0f,      0.0f, 0.0f, 0.1f, 0.0f,        0.0f,
                             0.0f, 0.0f, 1.002002f, 1.0f, 0.0f, 0.0f, -0.2002002f, 0.0f};

    REQUIRE(Frustum_LH_NO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Frustum_RH_ZO", "[Clipspace]")
  {
    constexpr Mat4 expected {0.1f, 0.0f, 0.0f,       0.0f,  0.0f, 0.1f, 0.0f,        0.0f,
                             0.0f, 0.0f, -1.001001f, -1.0f, 0.0f, 0.0f, -0.1001001f, 0.0f};

    REQUIRE(Frustum_RH_ZO(left, right, bottom, top, zNear, zFar) == expected);
  }

  TEST_CASE("Frustum_RH_NO", "[Clipspace]")
  {
    constexpr Mat4 expected {0.1f, 0.0f, 0.0f,       0.0f,  0.0f, 0.1f, 0.0f,        0.0f,
                             0.0f, 0.0f, -1.002002f, -1.0f, 0.0f, 0.0f, -0.2002002f, 0.0f};
    REQUIRE(Frustum_RH_NO(left, right, bottom, top, zNear, zFar) == expected);
  }

#pragma endregion Frustum

#pragma region Perspective

  TEST_CASE("Perspective_RH_ZO", "[Clipspace]")
  {
    float tanHalfFovy = std::tan(fovy / 2.0f);
    Mat4 expected {1 / (aspect * tanHalfFovy),
                   0,
                   0,
                   0,
                   0,
                   1 / tanHalfFovy,
                   0,
                   0,
                   0,
                   0,
                   zFar / (zNear - zFar),
                   -1,
                   0,
                   0,
                   -(zFar * zNear) / (zFar - zNear),
                   0};

    REQUIRE(Perspective_RH_ZO(fovy, aspect, zNear, zFar) == expected);
  }

  TEST_CASE("Perspective_RH_NO", "[Clipspace]")
  {
    float tanHalfFovy = std::tan(fovy / 2.0f);
    Mat4 expected {1.0f / (aspect * tanHalfFovy),
                   0,
                   0,
                   0,
                   0,
                   1.0f / tanHalfFovy,
                   0,
                   0,
                   0,
                   0,
                   -(zFar + zNear) / (zFar - zNear),
                   -1,
                   0,
                   0,
                   -(2.0f * zFar * zNear) / (zFar - zNear),
                   0};

    REQUIRE(Perspective_RH_NO(fovy, aspect, zNear, zFar) == expected);
  }

  TEST_CASE("Perspective_LH_ZO", "[Clipspace]")
  {
    float tanHalfFovy = std::tan(fovy / 2.0f);
    Mat4 expected {1.0f / (aspect * tanHalfFovy),
                   0.0f,
                   0.0f,
                   0.0f,
                   0.0f,
                   1.0f / tanHalfFovy,
                   0.0f,
                   0.0f,
                   0.0f,
                   0.0f,
                   zFar / (zFar - zNear),
                   1.0f,
                   0.0f,
                   0.0f,
                   -(zFar * zNear) / (zFar - zNear),
                   0.0f};

    REQUIRE(Perspective_LH_ZO(fovy, aspect, zNear, zFar) == expected);
  }

  TEST_CASE("Perspective_LH_NO", "[Clipspace]")
  {
    float tanHalfFovy = std::tan(fovy / 2.0f);
    Mat4 expected {1.0f / (aspect * tanHalfFovy),
                   0.0f,
                   0.0f,
                   0.0f,
                   0.0f,
                   1.0f / tanHalfFovy,
                   0.0f,
                   0.0f,
                   0.0f,
                   0.0f,
                   (zFar + zNear) / (zFar - zNear),
                   1.0f,
                   0.0f,
                   0.0f,
                   -(2.0f * zFar * zNear) / (zFar - zNear),
                   0.0f};

    REQUIRE(Perspective_LH_NO(fovy, aspect, zNear, zFar) == expected);
  }

#pragma endregion Perspective

#pragma region PerspectiveFov

  TEST_CASE("PerspectiveFov_RH_ZO", "[Clipspace]")
  {
    float h = std::cos(fov / 2.0f) / std::sin(fov / 2.0f);
    float w = h * height / width;

    Mat4 expected {
      w, 0, 0, 0, 0, h, 0, 0, 0, 0, zFar / (zNear - zFar), -1, 0, 0, -(zFar * zNear) / (zFar - zNear), 0};

    REQUIRE(PerspectiveFov_RH_ZO(fov, width, height, zNear, zFar) == expected);
  }

  TEST_CASE("PerspectiveFov_RH_NO", "[Clipspace]")
  {
    float h = std::cos(fov / 2.0f) / std::sin(fov / 2.0f);
    float w = h * height / width;
    float zDiff = zFar - zNear;

    Mat4 expected {
      w, 0, 0, 0, 0, h, 0, 0, 0, 0, -(zFar + zNear) / zDiff, -1, 0, 0, -(2 * zFar * zNear) / zDiff, 0};

    REQUIRE(PerspectiveFov_RH_NO(fov, width, height, zNear, zFar) == expected);
  }

  TEST_CASE("PerspectiveFov_LH_ZO", "[Clipspace]")
  {
    float h = std::cos(fov / 2.0f) / std::sin(fov / 2.0f);
    float w = h * height / width;

    Mat4 expected {
      w, 0, 0, 0, 0, h, 0, 0, 0, 0, zFar / (zFar - zNear), 1, 0, 0, -(zFar * zNear) / (zFar - zNear), 0};

    REQUIRE(PerspectiveFov_LH_ZO(fov, width, height, zNear, zFar) == expected);
  }

  TEST_CASE("PerspectiveFov_LH_NO", "[Clipspace]")
  {
    float h = std::cos(fov / 2.0f) / std::sin(fov / 2.0f);
    float w = h * height / width;
    float zDiff = zFar - zNear;

    Mat4 expected {w, 0, 0, 0, 0, h, 0, 0, 0, 0, (zFar + zNear) / zDiff, 1, 0, 0, -(2 * zFar * zNear) / zDiff,
                   0};

    REQUIRE(PerspectiveFov_LH_NO(fov, width, height, zNear, zFar) == expected);
  }

#pragma endregion PerspectiveFov

#pragma region InfinitePerspective

  KRYS_DISABLE_WARNING_PUSH()
  KRYS_DISABLE_WARNING(4'459, "-WShadow")

  TEST_CASE("InfinitePerspective_RH_ZO", "[Clipspace]")
  {
    float range = std::tan(fovy / 2.0f) * zNear;
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;
    float zNear2 = 2 * zNear;

    Mat4 expected {
      zNear2 / (right - left), 0, 0, 0, 0, zNear2 / (top - bottom), 0, 0, 0, 0, -1, -1, 0, 0, -zNear, 0};

    REQUIRE(InfinitePerspective_RH_ZO(fovy, aspect, zNear) == expected);
  }

  TEST_CASE("InfinitePerspective_RH_NO", "[Clipspace]")
  {
    float range = std::tan(fovy / 2.0f) * zNear;
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;
    float zNear2 = 2 * zNear;

    Mat4 expected {
      zNear2 / (right - left), 0, 0, 0, 0, zNear2 / (top - bottom), 0, 0, 0, 0, -1, -1, 0, 0, -2 * zNear, 0};

    REQUIRE(InfinitePerspective_RH_NO(fovy, aspect, zNear) == expected);
  }

  TEST_CASE("InfinitePerspective_LH_ZO", "[Clipspace]")
  {
    float range = std::tan(fovy / 2.0f) * zNear;
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;
    float zNear2 = 2 * zNear;

    Mat4 expected {
      zNear2 / (right - left), 0, 0, 0, 0, zNear2 / (top - bottom), 0, 0, 0, 0, 1, 1, 0, 0, -zNear, 0};

    REQUIRE(InfinitePerspective_LH_ZO(fovy, aspect, zNear) == expected);
  }

  TEST_CASE("InfinitePerspective_LH_NO", "[Clipspace]")
  {
    float range = std::tan(fovy / 2.0f) * zNear;
    float left = -range * aspect;
    float right = range * aspect;
    float bottom = -range;
    float top = range;
    float zNear2 = 2 * zNear;

    Mat4 expected {
      zNear2 / (right - left), 0, 0, 0, 0, zNear2 / (top - bottom), 0, 0, 0, 0, 1, 1, 0, 0, -zNear2, 0};

    REQUIRE(InfinitePerspective_LH_NO(fovy, aspect, zNear) == expected);
  }

  KRYS_DISABLE_WARNING_POP()

#pragma endregion InfinitePerspective
}
