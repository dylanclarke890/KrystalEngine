#include "pch.hpp"

#include "Krystal.Maths/Traits.hpp"

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("Traits(NonArithmetic)", "[Traits]")
  {
    STATIC_REQUIRE_FALSE(Traits<void>::IsArithmetic);
    STATIC_REQUIRE_FALSE(Traits<std::string>::IsArithmetic);
    STATIC_REQUIRE_FALSE(Traits<std::vector<int>>::IsArithmetic);
    STATIC_REQUIRE_FALSE(Traits<std::array<int, 3>>::IsArithmetic);
    STATIC_REQUIRE_FALSE(Traits<std::tuple<int, float>>::IsArithmetic);
  }

  TEST_CASE("Traits(Arithmetic)", "[Traits]")
  {
    STATIC_REQUIRE(Traits<int>::IsArithmetic);
    STATIC_REQUIRE(Traits<float>::IsArithmetic);
    STATIC_REQUIRE(Traits<double>::IsArithmetic);
    STATIC_REQUIRE(Traits<long double>::IsArithmetic);
    STATIC_REQUIRE(Traits<unsigned int>::IsArithmetic);
    STATIC_REQUIRE(Traits<unsigned long>::IsArithmetic);
  }

  TEST_CASE("Traits(Vector)", "[Traits]")
  {
    STATIC_REQUIRE(Traits<Vec1>::IsVector);
    STATIC_REQUIRE_FALSE(Traits<Vec1>::IsMatrix);

    STATIC_REQUIRE(Traits<Vec1>::Rows == 1);
    STATIC_REQUIRE(Traits<Vec1>::Columns == 1);
    STATIC_REQUIRE(Traits<Vec1>::TotalComponents == 1);

    STATIC_REQUIRE(Traits<Vec2>::IsVector);
    STATIC_REQUIRE_FALSE(Traits<Vec2>::IsMatrix);

    STATIC_REQUIRE(Traits<Vec2>::Rows == 2);
    STATIC_REQUIRE(Traits<Vec2>::Columns == 1);
    STATIC_REQUIRE(Traits<Vec2>::TotalComponents == 2);

    STATIC_REQUIRE(Traits<Vec3>::IsVector);
    STATIC_REQUIRE_FALSE(Traits<Vec3>::IsMatrix);

    STATIC_REQUIRE(Traits<Vec3>::Rows == 3);
    STATIC_REQUIRE(Traits<Vec3>::Columns == 1);
    STATIC_REQUIRE(Traits<Vec3>::TotalComponents == 3);

    STATIC_REQUIRE(Traits<Vec4>::IsVector);
    STATIC_REQUIRE_FALSE(Traits<Vec4>::IsMatrix);

    STATIC_REQUIRE(Traits<Vec4>::Rows == 4);
    STATIC_REQUIRE(Traits<Vec4>::Columns == 1);
    STATIC_REQUIRE(Traits<Vec4>::TotalComponents == 4);
  }

  TEST_CASE("Traits(Matrix)", "[Traits]")
  {
    STATIC_REQUIRE(Traits<Mat2x2>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat2x2>::IsVector);
    STATIC_REQUIRE(Traits<Mat2x2>::Rows == 2);
    STATIC_REQUIRE(Traits<Mat2x2>::Columns == 2);
    STATIC_REQUIRE(Traits<Mat2x2>::TotalComponents == 4);

    STATIC_REQUIRE(Traits<Mat2x3>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat2x3>::IsVector);

    STATIC_REQUIRE(Traits<Mat2x3>::Rows == 2);
    STATIC_REQUIRE(Traits<Mat2x3>::Columns == 3);
    STATIC_REQUIRE(Traits<Mat2x3>::TotalComponents == 6);

    STATIC_REQUIRE(Traits<Mat2x4>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat2x4>::IsVector);

    STATIC_REQUIRE(Traits<Mat2x4>::Rows == 2);
    STATIC_REQUIRE(Traits<Mat2x4>::Columns == 4);
    STATIC_REQUIRE(Traits<Mat2x4>::TotalComponents == 8);

    STATIC_REQUIRE(Traits<Mat3x2>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat3x2>::IsVector);

    STATIC_REQUIRE(Traits<Mat3x2>::Rows == 3);
    STATIC_REQUIRE(Traits<Mat3x2>::Columns == 2);
    STATIC_REQUIRE(Traits<Mat3x2>::TotalComponents == 6);

    STATIC_REQUIRE(Traits<Mat3x3>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat3x3>::IsVector);

    STATIC_REQUIRE(Traits<Mat3x3>::Rows == 3);
    STATIC_REQUIRE(Traits<Mat3x3>::Columns == 3);
    STATIC_REQUIRE(Traits<Mat3x3>::TotalComponents == 9);

    STATIC_REQUIRE(Traits<Mat3x4>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat3x4>::IsVector);

    STATIC_REQUIRE(Traits<Mat3x4>::Rows == 3);
    STATIC_REQUIRE(Traits<Mat3x4>::Columns == 4);
    STATIC_REQUIRE(Traits<Mat3x4>::TotalComponents == 12);

    STATIC_REQUIRE(Traits<Mat4x2>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat4x2>::IsVector);

    STATIC_REQUIRE(Traits<Mat4x2>::Rows == 4);
    STATIC_REQUIRE(Traits<Mat4x2>::Columns == 2);
    STATIC_REQUIRE(Traits<Mat4x2>::TotalComponents == 8);

    STATIC_REQUIRE(Traits<Mat4x3>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat4x3>::IsVector);

    STATIC_REQUIRE(Traits<Mat4x3>::Rows == 4);
    STATIC_REQUIRE(Traits<Mat4x3>::Columns == 3);
    STATIC_REQUIRE(Traits<Mat4x3>::TotalComponents == 12);

    STATIC_REQUIRE(Traits<Mat4x4>::IsMatrix);
    STATIC_REQUIRE_FALSE(Traits<Mat4x4>::IsVector);

    STATIC_REQUIRE(Traits<Mat4x4>::Rows == 4);
    STATIC_REQUIRE(Traits<Mat4x4>::Columns == 4);
    STATIC_REQUIRE(Traits<Mat4x4>::TotalComponents == 16);
  }
}