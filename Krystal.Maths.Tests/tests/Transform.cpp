#include "pch.hpp"

#include "Krystal.Maths/Convert.hpp"
#include "Krystal.Maths/Transform.hpp"
#include "Krystal.Maths/Maths.hpp"


namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("Translate(Matrix)", "[Matrix][Transform]")
  {
    REQUIRE(Translate(Identity<Mat4x4>(), {1, 3, 4})
            == Mat4x4({1, 0, 0, 0}, {0, 1, 0, 0}, {0, 0, 1, 0}, {1, 3, 4, 1}));
  }

  TEST_CASE("Rotate(Matrix)", "[Matrix][Transform]")
  {
    REQUIRE(Rotate(Identity<Mat4x4>(), Radians(50.0f), {0.0f, 0.0f, 1.0f})
            == Mat4x4({0.64278764f, 0.76604444f, 0, 0}, {-0.76604444f, 0.64278764f, 0, 0}, {0, 0, 1, 0},
                      {0, 0, 0, 1}));
  }

  TEST_CASE("Scale(Matrix)", "[Matrix][Transform]")
  {
    REQUIRE(Scale(Identity<Mat4x4>(), {2, 3, 4})
            == Mat4x4({2, 0, 0, 0}, {0, 3, 0, 0}, {0, 0, 4, 0}, {0, 0, 0, 1}));
  }

  TEST_CASE("LookAt(LH)", "[Matrix][Transform]")
  {
    REQUIRE(LookAt_LH(Vec3(0, 3, 5), Vec3(0), Vec3(0, 0, 1))
            == Mat4x4({1, 0, 0, 0}, {0, -0.857493f, -0.5144958f, 0}, {0, 0.5144958f, -0.857493f, 0},
                      {0, 0, 5.8309526f, 1}));
  }

  TEST_CASE("LookAt(RH)", "[Matrix][Transform]")
  {
    REQUIRE(LookAt_RH(Vec3(0, 3, 5), Vec3(0), Vec3(0, 0, 1))
            == Mat4x4({-1, 0, 0, 0}, {0, -0.857493f, 0.5144958f, 0}, {0, 0.5144958f, 0.857493f, 0},
                      {0, 0, -5.8309526f, 1}));
  }

  TEST_CASE("Rotate(Quaternion(Vector))", "[Matrix][Transform]")
  {
    Quaternion<float> quat(Vec3(0.0f, 1.0f, 0.0f), HalfPi<float>());
    Vec3 forward(1.0f, 0.0f, 0.0f);
    Vec3 rotated = Rotate(quat, forward);
    REQUIRE_THAT(-rotated.z, Catch::Matchers::WithinAbs(1.0f, 1e-6f));
  }
}