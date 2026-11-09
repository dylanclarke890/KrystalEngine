#include "Krystal.Core/Maths/Project.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::tests
{
  namespace
  {
    using Vec3 = Vector<float, 3>;
    using Vec4 = Vector<float, 4>;

    constexpr auto identity = Identity<Mat4>();
    constexpr Vec4 viewport {0.f, 0.f, 100.f, 200.f};
  }

  TEST_CASE("Project_ZO: depth is kept as it is", "[Project]")
  {
    STATIC_CHECK(Project_ZO(Vec3 {0.5f, -0.5f, 0.5f}, identity, identity, viewport)
                 == Vec3 {75.f, 50.f, 0.5f});
  }

  TEST_CASE("Project_NO: depth is mapped from [-1, 1] to [0, 1]", "[Project]")
  {
    STATIC_CHECK(Project_NO(Vec3 {0.5f, -0.5f, 0.5f}, identity, identity, viewport)
                 == Vec3 {75.f, 50.f, 0.75f});
    STATIC_CHECK(Project_NO(Vec3 {0.f, 0.f, -1.f}, identity, identity, viewport) == Vec3 {50.f, 100.f, 0.f});
  }

  TEST_CASE("Project_ZO: the perspective divide", "[Project]")
  {
    auto projection = identity;
    projection[3][3] = 2.f;
    CHECK(Project_ZO(Vec3 {0.5f, -0.5f, 0.5f}, identity, projection, viewport) == Vec3 {62.5f, 75.f, 0.25f});
  }

  TEST_CASE("Project: the depth range is chosen at compile time", "[Project]")
  {
    constexpr Vec3 obj {0.5f, -0.5f, 0.5f};

    // The default depth range is the default convention's.
    STATIC_CHECK(Conventions::DefaultDepthRange == Conventions::DepthRange::NegativeOneToOne);
    STATIC_CHECK(Project(obj, identity, identity, viewport) == Project_NO(obj, identity, identity, viewport));

    STATIC_CHECK(Project<float, float, Conventions::DepthRange::ZeroToOne>(obj, identity, identity, viewport)
                 == Project_ZO(obj, identity, identity, viewport));
    STATIC_CHECK(
      Project<float, float, Conventions::DepthRange::NegativeOneToOne>(obj, identity, identity, viewport)
      == Project_NO(obj, identity, identity, viewport));
  }
}
