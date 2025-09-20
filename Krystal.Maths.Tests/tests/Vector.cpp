#include "Krystal.Maths/Vector.hpp"
#include "Krystal.Maths/Maths.hpp"
#include <catch_all.hpp>

using namespace Krys::Maths;

namespace Krys::Tests
{
#pragma region Test Helpers

#define TEST_COPY_AND_MOVE()                                                                                 \
  constexpr Vec copy(v);                                                                                     \
  REQUIRE(copy == v);                                                                                        \
  constexpr Vec move(std::move(Vec(v)));                                                                     \
  REQUIRE(move == v);

#define TEST_COMMON_OPERATORS()                                                                              \
  REQUIRE(-v == Vec(-1));                                                                                    \
  REQUIRE(+v == v);                                                                                          \
  REQUIRE(~v == Vec(-2));                                                                                    \
                                                                                                             \
  REQUIRE(v + v == Vec(2));                                                                                  \
  REQUIRE(v + 1 == Vec(2));                                                                                  \
                                                                                                             \
  REQUIRE(v - Vec(1) == Vec(0));                                                                             \
  REQUIRE(v - 1 == Vec(0));                                                                                  \
                                                                                                             \
  REQUIRE(Vec(2) / Vec(2) == v);                                                                             \
  REQUIRE(Vec(2) / 2 == v);                                                                                  \
                                                                                                             \
  REQUIRE(v << Vec(1) == Vec(2));                                                                            \
  REQUIRE(v << 1 == Vec(2));                                                                                 \
                                                                                                             \
  REQUIRE(Vec(2) >> Vec(1) == Vec(1));                                                                       \
  REQUIRE(Vec(2) >> 1 == Vec(1));                                                                            \
                                                                                                             \
  REQUIRE((v | Vec(2)) == Vec(3));                                                                           \
  REQUIRE((v | 2) == Vec(3));                                                                                \
                                                                                                             \
  REQUIRE((Vec(3) & v) == Vec(1));                                                                           \
  REQUIRE((Vec(3) & 1) == Vec(1));                                                                           \
                                                                                                             \
  REQUIRE((v ^ Vec(2)) == Vec(3));                                                                           \
  REQUIRE((v ^ 2) == Vec(3));                                                                                \
  REQUIRE(v * 2 == Vec(2));

#pragma endregion

  TEST_CASE("Vec1", "[Vector]")
  {
    using Vec = Vec1i;

    constexpr Vec v(1);

    REQUIRE(v.x == 1);

    REQUIRE(v == Vec(1));
    REQUIRE(v != Vec(2));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();
  }

  TEST_CASE("Vec2", "[Vector]")
  {
    using Vec = Vec2i;

    constexpr Vec v(1);

    REQUIRE(v.x == 1);
    REQUIRE(v.y == 1);

    REQUIRE(v == Vec(1, 1));
    REQUIRE(v != Vec(2, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();
  }

  TEST_CASE("Vec3", "[Vector]")
  {
    using Vec = Vec3i;

    constexpr Vec v(1);

    REQUIRE(v.x == 1);
    REQUIRE(v.y == 1);
    REQUIRE(v.z == 1);

    REQUIRE(v == Vec(1, 1, 1));
    REQUIRE(v != Vec(2, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();
  }

  TEST_CASE("Vec4", "[Vector]")
  {
    using Vec = Vec4i;

    constexpr Vec v(1);

    REQUIRE(v.x == 1);
    REQUIRE(v.y == 1);
    REQUIRE(v.z == 1);
    REQUIRE(v.w == 1);

    REQUIRE(v == Vec(1, 1, 1, 1));
    REQUIRE(v != Vec(2, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();
  }

  TEST_CASE("ForEach(Vector)", "[Vector][ForEach]")
  {
#define DO_TEST(VecType, length)                                                                             \
  SECTION(STRINGIFY(VecType))                                                                                \
  {                                                                                                          \
    constexpr VecType vec(1);                                                                                \
    VecType result(0);                                                                                       \
    ForEach(vec, [&result](auto v) { result += v; });                                                        \
    REQUIRE(result == VecType(length));                                                                      \
  }

    DO_TEST(Vec1i, 1);
    DO_TEST(Vec2i, 2);
    DO_TEST(Vec3i, 3);
    DO_TEST(Vec4i, 4);

#undef DO_TEST
  }

  TEST_CASE("MapEach(Vector)", "[Vector][MapEach]")
  {
#define DO_TEST(VecType)                                                                                     \
  SECTION(STRINGIFY(VecType))                                                                                \
  {                                                                                                          \
    constexpr VecType vec(1);                                                                                \
    VecType result = MapEach(vec, [](auto v) { return v + 1; });                                             \
    REQUIRE(result == VecType(2));                                                                           \
  }

    DO_TEST(Vec1i);
    DO_TEST(Vec2i);
    DO_TEST(Vec3i);
    DO_TEST(Vec4i);

#undef DO_TEST
  }

  TEST_CASE("ZipTwo(Vector)", "[Vector][Zip]")
  {
#define DO_TEST(VecType)                                                                                     \
  SECTION(STRINGIFY(VecType))                                                                                \
  {                                                                                                          \
    constexpr VecType a(1);                                                                                  \
    constexpr VecType b(1);                                                                                  \
    VecType result = Zip(a, b, [](auto c, auto d) { return c + d; });                                        \
    REQUIRE(result == VecType(2));                                                                           \
  }

    DO_TEST(Vec1i);
    DO_TEST(Vec2i);
    DO_TEST(Vec3i);
    DO_TEST(Vec4i);

#undef DO_TEST
  }

  TEST_CASE("ZipThree(Vector)", "[Vector][Zip]")
  {
#define DO_TEST(VecType)                                                                                     \
  SECTION(STRINGIFY(VecType))                                                                                \
  {                                                                                                          \
    constexpr VecType a(1);                                                                                  \
    constexpr VecType b(1);                                                                                  \
    constexpr VecType c(1);                                                                                  \
    VecType result = Zip(a, b, c, [](auto d, auto e, auto f) { return d + e + f; });                         \
    REQUIRE(result == VecType(3));                                                                           \
  }

    DO_TEST(Vec1i);
    DO_TEST(Vec2i);
    DO_TEST(Vec3i);
    DO_TEST(Vec4i);

#undef DO_TEST
  }

  TEST_CASE("Sum(Vector)", "[Vector]")
  {
    REQUIRE(Sum(Vec4(1.f)) == 4.f);
    REQUIRE(Sum(Vec4(1.f), [](auto x) { return x * 2; }) == 8.f);
  }

  TEST_CASE("NoneOf(Vector)", "[Vector]")
  {
    REQUIRE(NoneOf(Vec1i {0}, [](auto x) { return x == 1; }));
    REQUIRE(NoneOf(Vec2i {0, 0}, [](auto x) { return x == 1; }));
    REQUIRE(NoneOf(Vec3i {0, 0, 0}, [](auto x) { return x == 1; }));
    REQUIRE(NoneOf(Vec4i {0, 0, 0, 0}, [](auto x) { return x == 1; }));
    REQUIRE(!NoneOf(Vec4i {1, 0, 0, 0}, [](auto x) { return x == 1; }));
  }

  TEST_CASE("AnyOf(Vector)", "[Vector]")
  {
    REQUIRE(AnyOf(Vec1i {1}, [](auto x) { return x == 1; }));
    REQUIRE(AnyOf(Vec2i {1, 0}, [](auto x) { return x == 1; }));
    REQUIRE(AnyOf(Vec3i {1, 0, 0}, [](auto x) { return x == 1; }));
    REQUIRE(AnyOf(Vec4i {1, 0, 0, 0}, [](auto x) { return x == 1; }));
    REQUIRE(!AnyOf(Vec4i {0, 0, 0, 0}, [](auto x) { return x == 1; }));
  }

  TEST_CASE("AllOf(Vector)", "[Vector]")
  {
    REQUIRE(AllOf(Vec1i {1}, [](auto x) { return x == 1; }));
    REQUIRE(AllOf(Vec2i {1, 1}, [](auto x) { return x == 1; }));
    REQUIRE(AllOf(Vec3i {1, 1, 1}, [](auto x) { return x == 1; }));
    REQUIRE(AllOf(Vec4i {1, 1, 1, 1}, [](auto x) { return x == 1; }));
    REQUIRE(!AllOf(Vec4i {0, 0, 0, 0}, [](auto x) { return x == 1; }));
  }

  TEST_CASE("Length(Vector)", "[Vector]")
  {
    REQUIRE_THAT(Length(Vec1i {1}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(Length(Vec2i {1, 0}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(Length(Vec3i {1, 0, 0}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(Length(Vec4i {1, 0, 0, 0}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
  }

  TEST_CASE("LengthSquared(Vector)", "[Vector]")
  {
    REQUIRE(LengthSquared(Vec1i {2}) == 4);
    REQUIRE(LengthSquared(Vec2i {2, 0}) == 4);
    REQUIRE(LengthSquared(Vec3i {2, 0, 0}) == 4);
    REQUIRE(LengthSquared(Vec4i {2, 0, 0, 0}) == 4);
  }

  TEST_CASE("Distance(Vector)", "[Vector]")
  {
    REQUIRE_THAT(Distance(Vec1i {1}, Vec1i {1}), Catch::Matchers::WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(Distance(Vec2i {1, 0}, Vec2i {1, 0}), Catch::Matchers::WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(Distance(Vec3i {1, 0, 0}, Vec3i {1, 0, 0}), Catch::Matchers::WithinAbs(0.0f, 1e-6f));
    REQUIRE_THAT(Distance(Vec4i {1, 0, 0, 0}, Vec4i {1, 0, 0, 0}), Catch::Matchers::WithinAbs(0.0f, 1e-6f));
  }

  TEST_CASE("DistanceSquared(Vector)", "[Vector]")
  {
    REQUIRE(DistanceSquared(Vec1i {2}, Vec1i {4}) == 4);
    REQUIRE(DistanceSquared(Vec2i {2}, Vec2i {4}) == 8);
    REQUIRE(DistanceSquared(Vec3i {2}, Vec3i {4}) == 12);
    REQUIRE(DistanceSquared(Vec4i {2}, Vec4i {4}) == 16);
  }

  TEST_CASE("Dot(Vector)", "[Vector]")
  {
    REQUIRE_THAT(Dot(Vec1i {1}, Vec1i {1}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(Dot(Vec2i {1, 0}, Vec2i {1, 0}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(Dot(Vec3i {1, 0, 0}, Vec3i {1, 0, 0}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
    REQUIRE_THAT(Dot(Vec4i {1, 0, 0, 0}, Vec4i {1, 0, 0, 0}), Catch::Matchers::WithinAbs(1.0f, 1e-6f));
  }

  TEST_CASE("Cross(Vector)", "[Vector]")
  {
    constexpr auto IsTrue = [](float v)
    {
      return v < std::numeric_limits<float>::epsilon();
    };

    Vec3 c1 = Cross(Vec3(1, 0, 0), Vec3(0, 1, 0));
    Vec3 c2 = Cross(Vec3(0, 1, 0), Vec3(1, 0, 0));

    REQUIRE(AllOf(Abs(c1 - Vec3(0, 0, 1)), IsTrue));
    REQUIRE(AllOf(Abs(c2 - Vec3(0, 0, -1)), IsTrue));
  }

  TEST_CASE("Normalize(Vector)", "[Vector]")
  {
    constexpr auto IsTrue = [](float v)
    {
      return v < std::numeric_limits<float>::epsilon();
    };

    Vec3 n1 = Normalize(Vec3(1, 0, 0));
    Vec3 n2 = Normalize(Vec3(2, 0, 0));

    REQUIRE(AllOf(Abs(n1 - Vec3(1, 0, 0)), IsTrue));
    REQUIRE(AllOf(Abs(n2 - Vec3(1, 0, 0)), IsTrue));

    Vec3 ro = Vec3(std::cos(5.0f) * 3.0f, 2.0f, std::sin(5.0f) * 3.0f);
    Vec3 w = Normalize(Vec3(0, -0.2f, 0) - ro);
    Vec3 u = Normalize(Cross(w, Vec3(0, 1, 0)));
    Vec3 v = Cross(u, w);
    Vec3 x = Cross(w, u);

    REQUIRE(AllOf(Abs(x + v), IsTrue));
  }

  TEST_CASE("Reflect", "[Vector]")
  {
    REQUIRE(Reflect(Vec2(1, -1), Vec2(0, 1)) == Vec2(1, 1));
  }

  TEST_CASE("Refract", "[Vector]")
  {
    REQUIRE(Refract(Vec2(0, -1), Vec2(0, 1), 0.5f) == Vec2(0, -1));
  }

  TEST_CASE("FaceForward", "[Vector]")
  {
    constexpr Vec3 n(0, 0, 1);
    constexpr Vec3 i(1, 0, 1);
    constexpr Vec3 nRef(0, 0, 1);
    REQUIRE(FaceForward(n, i, nRef) == Vec3(0, 0, -1));
  }

  TEST_CASE("TriangleNormal", "[Vector]")
  {
    REQUIRE(TriangleNormal(Vec3(0, 0, 0), Vec3(1, 0, 0), Vec3(0, 1, 0))
            == Vec3(0, 0, 1)); // Normal points along +Z axis
  }
}