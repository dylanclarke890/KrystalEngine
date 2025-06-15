#include "pch.hpp"

#include <Krystal.Core/Core.hpp>
#include <Krystal.Maths/Matrix.hpp>
#include <Krystal.Maths/Vector.hpp>

#include <catch_all.hpp>
#include <utility>

using namespace Krys::Maths;

namespace Krys::Tests
{
#pragma region Test Helpers

#define TEST_COPY_AND_MOVE()                                                                                 \
  constexpr Mat copy(m);                                                                                     \
  REQUIRE(copy == m);                                                                                        \
  constexpr Mat move(std::move(Mat(m)));                                                                     \
  REQUIRE(move == m);

#define TEST_COMMON_OPERATORS()                                                                              \
  REQUIRE(-m == Mat(-1));                                                                                    \
  REQUIRE(+m == m);                                                                                          \
  REQUIRE(~m == Mat(-2));                                                                                    \
                                                                                                             \
  REQUIRE(m + m == Mat(2));                                                                                  \
  REQUIRE(m + 1 == Mat(2));                                                                                  \
                                                                                                             \
  REQUIRE(m - Mat(1) == Mat(0));                                                                             \
  REQUIRE(m - 1 == Mat(0));                                                                                  \
                                                                                                             \
  REQUIRE(Mat(2) / Mat(2) == m);                                                                             \
  REQUIRE(Mat(2) / 2 == m);                                                                                  \
                                                                                                             \
  REQUIRE(m << Mat(1) == Mat(2));                                                                            \
  REQUIRE(m << 1 == Mat(2));                                                                                 \
                                                                                                             \
  REQUIRE(Mat(2) >> Mat(1) == Mat(1));                                                                       \
  REQUIRE(Mat(2) >> 1 == Mat(1));                                                                            \
                                                                                                             \
  REQUIRE((m | Mat(2)) == Mat(3));                                                                           \
  REQUIRE((m | 2) == Mat(3));                                                                                \
                                                                                                             \
  REQUIRE((Mat(3) & m) == Mat(1));                                                                           \
  REQUIRE((Mat(3) & 1) == Mat(1));                                                                           \
                                                                                                             \
  REQUIRE((m ^ Mat(2)) == Mat(3));                                                                           \
  REQUIRE((m ^ 2) == Mat(3));                                                                                \
  REQUIRE(m * 2 == Mat(2));                                                                                  \
                                                                                                             \
  REQUIRE(Transpose(Transpose(m)) == m);

#pragma endregion

  TEST_CASE("Mat2x2", "[Matrix]")
  {
    using Mat = Mat2x2i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);

    REQUIRE(m == Mat(1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat(1, 2, 3, 4);
    REQUIRE(mat * Mat({5, 6}, {7, 8}) == Mat({19, 22}, {43, 50}));
    REQUIRE(mat * Mat2x3i({5, 6, 7}, {8, 9, 10}) == Mat2x3i({21, 24, 27}, {47, 54, 61}));
    REQUIRE(mat * Mat2x4i({5, 6, 7, 8}, {9, 10, 11, 12}) == Mat2x4i({23, 26, 29, 32}, {51, 58, 65, 72}));
    REQUIRE(mat * Vec2i(5, 6) == Vec2i(17, 39));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5)) == Mat(2, 6, 12, 20));

    REQUIRE(Determinant(Mat(4, 3, 6, 8)) == 14);

    REQUIRE(Inverse(Mat2x2(4, 2, 7, 6)) == Mat2x2({0.6f, -0.2f}, {-0.7f, 0.4f}));

    REQUIRE(OuterProduct(Vec2i(1, 2), Vec2i(3, 4)) == Mat2x2i({3, 4}, {6, 8}));
  }

  TEST_CASE("Mat2x3", "[Matrix]")
  {
    using Mat = Mat2x3i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[0].z == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[1].z == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2, 3}, {4, 5, 6});
    REQUIRE(mat * Mat3x2i({7, 8}, {9, 10}, {11, 12}) == Mat2x2i({58, 64}, {139, 154}));
    REQUIRE(mat * Mat3x3i({7, 8, 9}, {10, 11, 12}, {13, 14, 15}) == Mat2x3i({66, 72, 78}, {156, 171, 186}));
    REQUIRE(mat * Mat3x4i({7, 8, 9, 10}, {11, 12, 13, 14}, {15, 16, 17, 18})
            == Mat2x4i({74, 80, 86, 92}, {173, 188, 203, 218}));
    REQUIRE(mat * Vec3i(7, 8, 9) == Vec2i(50, 122));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7)) == Mat(2, 6, 12, 20, 30, 42));

    REQUIRE(OuterProduct(Vec2i(1, 2), Vec3i(3, 4, 5)) == Mat2x3i({3, 4, 5}, {6, 8, 10}));
  }

  TEST_CASE("Mat2x4", "[Matrix]")
  {
    using Mat = Mat2x4i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[0].z == 1);
    REQUIRE(m[0].w == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[1].z == 1);
    REQUIRE(m[1].w == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2, 3, 4}, {5, 6, 7, 8});
    REQUIRE(mat * Mat4x2i({9, 10}, {11, 12}, {13, 14}, {15, 16}) == Mat2x2i({130, 140}, {322, 348}));
    REQUIRE(mat * Mat4x3i({9, 10, 11}, {12, 13, 14}, {15, 16, 17}, {18, 19, 20})
            == Mat2x3i({150, 160, 170}, {366, 392, 418}));
    REQUIRE(mat * Mat4x4i({9, 10, 11, 12}, {13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24})
            == Mat2x4i({170, 180, 190, 200}, {410, 436, 462, 488}));
    REQUIRE(mat * Vec4i(9, 10, 11, 12) == Vec2i(110, 278));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9)) == Mat(2, 6, 12, 20, 30, 42, 56, 72));

    REQUIRE(OuterProduct(Vec2i(1, 2), Vec4i(3, 4, 5, 6)) == Mat2x4i({3, 4, 5, 6}, {6, 8, 10, 12}));
  }

  TEST_CASE("Mat3x2", "[Matrix]")
  {
    using Mat = Mat3x2i;

    constexpr Mat m(1);
    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[2].x == 1);
    REQUIRE(m[2].y == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2}, {3, 4}, {5, 6});
    REQUIRE(mat * Mat2x2i({7, 8}, {9, 10}) == Mat3x2i({25, 28}, {57, 64}, {89, 100}));
    REQUIRE(mat * Mat2x3i({7, 8, 9}, {10, 11, 12}) == Mat3x3i({27, 30, 33}, {61, 68, 75}, {95, 106, 117}));
    REQUIRE(mat * Mat2x4i({7, 8, 9, 10}, {11, 12, 13, 14})
            == Mat3x4i({29, 32, 35, 38}, {65, 72, 79, 86}, {101, 112, 123, 134}));
    REQUIRE(mat * Vec2i(7, 8) == Vec3i(23, 53, 83));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7)) == Mat(2, 6, 12, 20, 30, 42));

    REQUIRE(OuterProduct(Vec3i(1, 2, 3), Vec2i(4, 5)) == Mat3x2i({4, 5}, {8, 10}, {12, 15}));
  }

  TEST_CASE("Mat3x3", "[Matrix]")
  {
    using Mat = Mat3x3i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[0].z == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[1].z == 1);
    REQUIRE(m[2].x == 1);
    REQUIRE(m[2].y == 1);
    REQUIRE(m[2].z == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2, 3}, {4, 5, 6}, {7, 8, 9});
    REQUIRE(mat * Mat3x2i({10, 11}, {12, 13}, {14, 15}) == Mat3x2i({76, 82}, {184, 199}, {292, 316}));
    REQUIRE(mat * Mat3x3i({10, 11, 12}, {13, 14, 15}, {16, 17, 18})
            == Mat3x3i({84, 90, 96}, {201, 216, 231}, {318, 342, 366}));
    REQUIRE(mat * Mat3x4i({10, 11, 12, 13}, {14, 15, 16, 17}, {18, 19, 20, 21})
            == Mat3x4i({92, 98, 104, 110}, {218, 233, 248, 263}, {344, 368, 392, 416}));
    REQUIRE(mat * Vec3i(10, 11, 12) == Vec3i(68, 167, 266));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10)) == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90));

    REQUIRE(Determinant(Mat {{6, 4, 2}, {1, -2, 8}, {1, 5, 7}}) == -306);

    REQUIRE(Inverse(Mat3({1, -1, 1}, {2, 0, -1}, {-1, 1, 0}))
            == Mat3({0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 1.5f}, {1.0f, 0.0f, 1.0f}));

    REQUIRE(OuterProduct(Vec3i(1, 2, 3), Vec3i(4, 5, 6)) == Mat3x3i({4, 5, 6}, {8, 10, 12}, {12, 15, 18}));
  }

  TEST_CASE("Mat3x4", "[Matrix]")
  {
    using Mat = Mat3x4i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[0].z == 1);
    REQUIRE(m[0].w == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[1].z == 1);
    REQUIRE(m[1].w == 1);
    REQUIRE(m[2].x == 1);
    REQUIRE(m[2].y == 1);
    REQUIRE(m[2].z == 1);
    REQUIRE(m[2].w == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12});
    REQUIRE(mat * Mat4x2i({13, 14}, {15, 16}, {17, 18}, {19, 20})
            == Mat3x2i({170, 180}, {426, 452}, {682, 724}));
    REQUIRE(mat * Mat4x3i({13, 14, 15}, {16, 17, 18}, {19, 20, 21}, {22, 23, 24})
            == Mat3x3i({190, 200, 210}, {470, 496, 522}, {750, 792, 834}));
    REQUIRE(mat * Mat4x4i({13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}, {25, 26, 27, 28})
            == Mat3x4i({210, 220, 230, 240}, {514, 540, 566, 592}, {818, 860, 902, 944}));
    REQUIRE(mat * Vec4i(13, 14, 15, 16) == Vec3i(150, 382, 614));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13))
            == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132, 156));

    REQUIRE(OuterProduct(Vec3i(1, 2, 3), Vec4i(4, 5, 6, 7))
            == Mat3x4i({4, 5, 6, 7}, {8, 10, 12, 14}, {12, 15, 18, 21}));
  }

  TEST_CASE("Mat4x2", "[Matrix]")
  {
    using Mat = Mat4x2i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[2].x == 1);
    REQUIRE(m[2].y == 1);
    REQUIRE(m[3].x == 1);
    REQUIRE(m[3].y == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2}, {3, 4}, {5, 6}, {7, 8});
    REQUIRE(mat * Mat2x2i({9, 10}, {11, 12}) == Mat4x2i({31, 34}, {71, 78}, {111, 122}, {151, 166}));
    REQUIRE(mat * Mat2x3i({9, 10, 11}, {12, 13, 14})
            == Mat4x3i({33, 36, 39}, {75, 82, 89}, {117, 128, 139}, {159, 174, 189}));
    REQUIRE(mat * Mat2x4i({9, 10, 11, 12}, {13, 14, 15, 16})
            == Mat4x4i({35, 38, 41, 44}, {79, 86, 93, 100}, {123, 134, 145, 156}, {167, 182, 197, 212}));
    REQUIRE(mat * Vec2i(9, 10) == Vec4i(29, 67, 105, 143));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9)) == Mat(2, 6, 12, 20, 30, 42, 56, 72));

    REQUIRE(OuterProduct(Vec4i(1, 2, 3, 4), Vec2i(5, 6)) == Mat4x2i({5, 6}, {10, 12}, {15, 18}, {20, 24}));
  }

  TEST_CASE("Mat4x3", "[Matrix]")
  {
    using Mat = Mat4x3i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[0].z == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[1].z == 1);
    REQUIRE(m[2].x == 1);
    REQUIRE(m[2].y == 1);
    REQUIRE(m[2].z == 1);
    REQUIRE(m[3].x == 1);
    REQUIRE(m[3].y == 1);
    REQUIRE(m[3].z == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {10, 11, 12});
    REQUIRE(mat * Mat3x2i({13, 14}, {15, 16}, {17, 18})
            == Mat4x2i({94, 100}, {229, 244}, {364, 388}, {499, 532}));
    REQUIRE(mat * Mat3x3i({13, 14, 15}, {16, 17, 18}, {19, 20, 21})
            == Mat4x3i({102, 108, 114}, {246, 261, 276}, {390, 414, 438}, {534, 567, 600}));
    REQUIRE(
      mat * Mat3x4i({13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24})
      == Mat4x4i({110, 116, 122, 128}, {263, 278, 293, 308}, {416, 440, 464, 488}, {569, 602, 635, 668}));
    REQUIRE(mat * Vec3i(13, 14, 15) == Vec4i(86, 212, 338, 464));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13))
            == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132, 156));

    REQUIRE(OuterProduct(Vec4i(1, 2, 3, 4), Vec3i(5, 6, 7))
            == Mat4x3i({5, 6, 7}, {10, 12, 14}, {15, 18, 21}, {20, 24, 28}));
  }

  TEST_CASE("Mat4x4", "[Matrix]")
  {
    using Mat = Mat4x4i;

    constexpr Mat m(1);

    REQUIRE(m[0].x == 1);
    REQUIRE(m[0].y == 1);
    REQUIRE(m[0].z == 1);
    REQUIRE(m[0].w == 1);
    REQUIRE(m[1].x == 1);
    REQUIRE(m[1].y == 1);
    REQUIRE(m[1].z == 1);
    REQUIRE(m[1].w == 1);
    REQUIRE(m[2].x == 1);
    REQUIRE(m[2].y == 1);
    REQUIRE(m[2].z == 1);
    REQUIRE(m[2].w == 1);
    REQUIRE(m[3].x == 1);
    REQUIRE(m[3].y == 1);
    REQUIRE(m[3].z == 1);
    REQUIRE(m[3].w == 1);

    REQUIRE(m == Mat(1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));
    REQUIRE(m != Mat(2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1));

    TEST_COPY_AND_MOVE();
    TEST_COMMON_OPERATORS();

    constexpr Mat mat({1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16});
    REQUIRE(mat * Mat4x2i({17, 18}, {19, 20}, {21, 22}, {23, 24})
            == Mat4x2i({210, 220}, {530, 556}, {850, 892}, {1'170, 1'228}));
    REQUIRE(mat * Mat4x3i({17, 18, 19}, {20, 21, 22}, {23, 24, 25}, {26, 27, 28})
            == Mat4x3i({230, 240, 250}, {574, 600, 626}, {918, 960, 1'002}, {1'262, 1'320, 1'378}));
    REQUIRE(mat * Mat4x4i({17, 18, 19, 20}, {21, 22, 23, 24}, {25, 26, 27, 28}, {29, 30, 31, 32})
            == Mat4x4i({250, 260, 270, 280}, {618, 644, 670, 696}, {986, 1'028, 1'070, 1'112},
                       {1'354, 1'412, 1'470, 1'528}));
    REQUIRE(mat * Vec4i(17, 18, 19, 20) == Vec4i(190, 486, 782, 1'078));
    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17))
            == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132, 156, 182, 210, 240, 272));

    REQUIRE(Determinant(Mat {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}}) == 0);
    REQUIRE(Determinant(Mat {4, 2, 3, 1, 6, 8, 7, 5, 1, -2, 8, 3, 5, 1, 4, 7}) == 1'066);

    REQUIRE(Inverse(Mat4({1, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 3, 0}, {0, 0, 0, 4}))
            == Mat4({1, 0, 0, 0}, {0, 0.5f, 0, 0}, {0, 0, 0.33333333333333f, 0}, {0, 0, 0, 0.25f}));

    REQUIRE(OuterProduct(Vec4i(1, 2, 3, 4), Vec4i(5, 6, 7, 8))
            == Mat4x4i({5, 6, 7, 8}, {10, 12, 14, 16}, {15, 18, 21, 24}, {20, 24, 28, 32}));
  }

  TEST_CASE("ForEach(Matrix)", "[Matrix][ForEach]")
  {
#define DO_TEST(MatType, rows, cols)                                                                         \
  SECTION(STRINGIFY(MatType))                                                                                \
  {                                                                                                          \
    constexpr MatType mat(1);                                                                                \
    MatType result(0);                                                                                       \
    ForEach(mat, [&result](auto v) { result += v; });                                                        \
    REQUIRE(result == MatType(rows * cols));                                                                 \
  }

    DO_TEST(Mat2x2i, 2, 2);
    DO_TEST(Mat2x3i, 2, 3);
    DO_TEST(Mat2x4i, 2, 4);
    DO_TEST(Mat3x2i, 3, 2);
    DO_TEST(Mat3x3i, 3, 3);
    DO_TEST(Mat3x4i, 3, 4);
    DO_TEST(Mat4x2i, 4, 2);
    DO_TEST(Mat4x3i, 4, 3);
    DO_TEST(Mat4x4i, 4, 4);

#undef DO_TEST
  }

  TEST_CASE("MapEach(Matrix)", "[Matrix][MapEach]")
  {
#define DO_TEST(MatType)                                                                                     \
  SECTION(STRINGIFY(MatType))                                                                                \
  {                                                                                                          \
    constexpr MatType mat(1);                                                                                \
    MatType result = MapEach(mat, [](auto v) { return v + 1; });                                             \
    REQUIRE(result == MatType(2));                                                                           \
  }
    DO_TEST(Mat2x2i);
    DO_TEST(Mat2x3i);
    DO_TEST(Mat2x4i);
    DO_TEST(Mat3x2i);
    DO_TEST(Mat3x3i);
    DO_TEST(Mat3x4i);
    DO_TEST(Mat4x2i);
    DO_TEST(Mat4x3i);
    DO_TEST(Mat4x4i);

#undef DO_TEST
  }

  TEST_CASE("ZipTwo(Matrix)", "[Matrix][Zip]")
  {
#define DO_TEST(MatType)                                                                                     \
  SECTION(STRINGIFY(MatType))                                                                                \
  {                                                                                                          \
    constexpr MatType a(1);                                                                                  \
    constexpr MatType b(1);                                                                                  \
    MatType result = Zip(a, b, [](auto c, auto d) { return c + d; });                                        \
    REQUIRE(result == MatType(2));                                                                           \
  }
    DO_TEST(Mat2x2i);
    DO_TEST(Mat2x3i);
    DO_TEST(Mat2x4i);
    DO_TEST(Mat3x2i);
    DO_TEST(Mat3x3i);
    DO_TEST(Mat3x4i);
    DO_TEST(Mat4x2i);
    DO_TEST(Mat4x3i);
    DO_TEST(Mat4x4i);

#undef DO_TEST
  }

  TEST_CASE("ZipThree(Matrix)", "[Matrix][Zip]")
  {
#define DO_TEST(MatType)                                                                                     \
  SECTION(STRINGIFY(MatType))                                                                                \
  {                                                                                                          \
    constexpr MatType a(1);                                                                                  \
    constexpr MatType b(1);                                                                                  \
    constexpr MatType c(1);                                                                                  \
    MatType result = Zip(a, b, c, [](auto d, auto e, auto f) { return d + e + f; });                         \
    REQUIRE(result == MatType(3));                                                                           \
  }
    DO_TEST(Mat2x2i);
    DO_TEST(Mat2x3i);
    DO_TEST(Mat2x4i);
    DO_TEST(Mat3x2i);
    DO_TEST(Mat3x3i);
    DO_TEST(Mat3x4i);
    DO_TEST(Mat4x2i);
    DO_TEST(Mat4x3i);
    DO_TEST(Mat4x4i);

#undef DO_TEST
  }

  TEST_CASE("Sum(Matrix)", "[Matrix]")
  {
    REQUIRE(Sum(Mat4(1.f)) == 16.f);
    REQUIRE(Sum(Mat4(1.f), [](auto x) { return x * 2; }) == 32.f);
  }

  TEST_CASE("NoneOf(Matrix)", "[Matrix]")
  {
#define DO_TEST(MatType)                                                                                     \
  REQUIRE(NoneOf(MatType(1), [](auto x) { return x == 0; }));                                                \
  REQUIRE_FALSE(NoneOf(MatType(1), [](auto x) { return x == 1; }));

    DO_TEST(Mat2x2i);
    DO_TEST(Mat2x3i);
    DO_TEST(Mat2x4i);
    DO_TEST(Mat3x2i);
    DO_TEST(Mat3x3i);
    DO_TEST(Mat3x4i);
    DO_TEST(Mat4x2i);
    DO_TEST(Mat4x3i);
    DO_TEST(Mat4x4i);

#undef DO_TEST
  }

  TEST_CASE("AnyOf(Matrix)", "[Matrix]")
  {
#define DO_TEST(MatType)                                                                                     \
  REQUIRE(AnyOf(MatType(1), [](auto x) { return x == 1; }));                                                 \
  REQUIRE_FALSE(AnyOf(MatType(1), [](auto x) { return x == 0; }));

    DO_TEST(Mat2x2i);
    DO_TEST(Mat2x3i);
    DO_TEST(Mat2x4i);
    DO_TEST(Mat3x2i);
    DO_TEST(Mat3x3i);
    DO_TEST(Mat3x4i);
    DO_TEST(Mat4x2i);
    DO_TEST(Mat4x3i);
    DO_TEST(Mat4x4i);

#undef DO_TEST
  }

  TEST_CASE("AllOf(Matrix)", "[Matrix]")
  {
#define DO_TEST(MatType)                                                                                     \
  REQUIRE(AllOf(MatType(1), [](auto x) { return x == 1; }));                                                 \
  REQUIRE_FALSE(AllOf(MatType(1), [](auto x) { return x == 0; }));

    DO_TEST(Mat2x2i);
    DO_TEST(Mat2x3i);
    DO_TEST(Mat2x4i);
    DO_TEST(Mat3x2i);
    DO_TEST(Mat3x3i);
    DO_TEST(Mat3x4i);
    DO_TEST(Mat4x2i);
    DO_TEST(Mat4x3i);
    DO_TEST(Mat4x4i);

#undef DO_TEST
  }
}