#include "Krystal.Maths/Matrix.hpp"
#include <catch_all.hpp>

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

    REQUIRE(mat * Mat({5, 6}, {7, 8}) == Mat({23, 34}, {31, 46}));
    REQUIRE(mat * Mat3x2i({5, 6}, {7, 8}, {9, 10}) == Mat3x2i({23, 34}, {31, 46}, {39, 58}));
    REQUIRE(mat * Mat4x2i({5, 6}, {7, 8}, {9, 10}, {11, 12})
            == Mat4x2i({23, 34}, {31, 46}, {39, 58}, {47, 70}));
    REQUIRE(mat * Vec2i(5, 6) == Vec2i(23, 34));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5)) == Mat(2, 6, 12, 20));
    REQUIRE(Determinant(Mat(4, 3, 6, 8)) == 14);
    REQUIRE(Inverse(Mat2x2(4, 2, 7, 6)) == Mat2x2({0.6f, -0.2f}, {-0.7f, 0.4f}));
    REQUIRE(OuterProduct(Vec2i(1, 2), Vec2i(3, 4)) == Mat2x2i({3, 6}, {4, 8}));
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

    REQUIRE(mat * Mat2x2i({7, 8}, {9, 10}) == Mat2x3i({39, 54, 69}, {49, 68, 87}));
    REQUIRE(mat * Mat3x2i({7, 8}, {9, 10}, {11, 12}) == Mat3x3i({39, 54, 69}, {49, 68, 87}, {59, 82, 105}));
    REQUIRE(mat * Mat4x2i({7, 8}, {9, 10}, {11, 12}, {13, 14})
            == Mat4x3i({39, 54, 69}, {49, 68, 87}, {59, 82, 105}, {69, 96, 123}));
    REQUIRE(mat * Vec2i(7, 8) == Vec3i(39, 54, 69));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7)) == Mat(2, 6, 12, 20, 30, 42));
    REQUIRE(OuterProduct(Vec2i(1, 2), Vec3i(3, 4, 5)) == Mat3x2i({3, 6}, {4, 8}, {5, 10}));
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

    REQUIRE(mat * Mat2x2i({9, 10}, {11, 12}) == Mat2x4i({59, 78, 97, 116}, {71, 94, 117, 140}));
    REQUIRE(mat * Mat3x2i({9, 10}, {11, 12}, {13, 14})
            == Mat3x4i({59, 78, 97, 116}, {71, 94, 117, 140}, {83, 110, 137, 164}));
    REQUIRE(mat * Mat4x2i({9, 10}, {11, 12}, {13, 14}, {15, 16})
            == Mat4x4i({59, 78, 97, 116}, {71, 94, 117, 140}, {83, 110, 137, 164}, {95, 126, 157, 188}));
    REQUIRE(mat * Vec2i(9, 10) == Vec4i(59, 78, 97, 116));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9)) == Mat(2, 6, 12, 20, 30, 42, 56, 72));
    REQUIRE(OuterProduct(Vec2i(1, 2), Vec4i(3, 4, 5, 6)) == Mat4x2i({3, 6}, {4, 8}, {5, 10}, {6, 12}));
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
    REQUIRE(mat * Mat2x3i({7, 8, 9}, {10, 11, 12}) == Mat2x2i({76, 100}, {103, 136}));
    REQUIRE(mat * Mat3x3i({7, 8, 9}, {10, 11, 12}, {13, 14, 15})
            == Mat3x2i({76, 100}, {103, 136}, {130, 172}));
    REQUIRE(mat * Mat4x3i({7, 8, 9}, {10, 11, 12}, {13, 14, 15}, {16, 17, 18})
            == Mat4x2i({76, 100}, {103, 136}, {130, 172}, {157, 208}));
    REQUIRE(mat * Vec3i(7, 8, 9) == Vec2i(76, 100));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7)) == Mat(2, 6, 12, 20, 30, 42));
    REQUIRE(OuterProduct(Vec3i(1, 2, 3), Vec2i(4, 5)) == Mat2x3i({4, 8, 12}, {5, 10, 15}));
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
    REQUIRE(mat * Mat2x3i({10, 11, 12}, {13, 14, 15}) == Mat2x3i({138, 171, 204}, {174, 216, 258}));
    REQUIRE(mat * Mat3x3i({10, 11, 12}, {13, 14, 15}, {16, 17, 18})
            == Mat3x3i({138, 171, 204}, {174, 216, 258}, {210, 261, 312}));
    REQUIRE(mat * Mat4x3i({10, 11, 12}, {13, 14, 15}, {16, 17, 18}, {19, 20, 21})
            == Mat4x3i({138, 171, 204}, {174, 216, 258}, {210, 261, 312}, {246, 306, 366}));
    REQUIRE(mat * Vec3i(10, 11, 12) == Vec3i(138, 171, 204));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10)) == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90));
    REQUIRE(Determinant(Mat {{6, 4, 2}, {1, -2, 8}, {1, 5, 7}}) == -306);
    REQUIRE(Inverse(Mat3({1, -1, 1}, {2, 0, -1}, {-1, 1, 0}))
            == Mat3({0.5f, 0.5f, 0.5f}, {0.5f, 0.5f, 1.5f}, {1.0f, 0.0f, 1.0f}));
    REQUIRE(OuterProduct(Vec3i(1, 2, 3), Vec3i(4, 5, 6)) == Mat3x3i({4, 8, 12}, {5, 10, 15}, {6, 12, 18}));
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
    REQUIRE(mat * Mat2x3i({13, 14, 15}, {16, 17, 18}) == Mat2x4i({218, 260, 302, 344}, {263, 314, 365, 416}));
    REQUIRE(mat * Mat3x3i({13, 14, 15}, {16, 17, 18}, {19, 20, 21})
            == Mat3x4i({218, 260, 302, 344}, {263, 314, 365, 416}, {308, 368, 428, 488}));
    REQUIRE(
      mat * Mat4x3i({13, 14, 15}, {16, 17, 18}, {19, 20, 21}, {22, 23, 24})
      == Mat4x4i({218, 260, 302, 344}, {263, 314, 365, 416}, {308, 368, 428, 488}, {353, 422, 491, 560}));
    REQUIRE(mat * Vec3i(13, 14, 15) == Vec4i(218, 260, 302, 344));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13))
            == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132, 156));
    REQUIRE(OuterProduct(Vec3i(1, 2, 3), Vec4i(4, 5, 6, 7))
            == Mat4x3i({4, 8, 12}, {5, 10, 15}, {6, 12, 18}, {7, 14, 21}));
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
    REQUIRE(mat * Mat2x4i({9, 10, 11, 12}, {13, 14, 15, 16}) == Mat2x2i({178, 220}, {242, 300}));
    REQUIRE(mat * Mat3x4i({9, 10, 11, 12}, {13, 14, 15, 16}, {17, 18, 19, 20})
            == Mat3x2i({178, 220}, {242, 300}, {306, 380}));
    REQUIRE(mat * Mat4x4i({9, 10, 11, 12}, {13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24})
            == Mat4x2i({178, 220}, {242, 300}, {306, 380}, {370, 460}));
    REQUIRE(mat * Vec4i(9, 10, 11, 12) == Vec2i(178, 220));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9)) == Mat(2, 6, 12, 20, 30, 42, 56, 72));
    REQUIRE(OuterProduct(Vec4i(1, 2, 3, 4), Vec2i(5, 6)) == Mat2x4i({5, 10, 15, 20}, {6, 12, 18, 24}));
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
    REQUIRE(mat * Mat2x4i({13, 14, 15, 16}, {17, 18, 19, 20}) == Mat2x3i({334, 392, 450}, {422, 496, 570}));
    REQUIRE(mat * Mat3x4i({13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24})
            == Mat3x3i({334, 392, 450}, {422, 496, 570}, {510, 600, 690}));
    REQUIRE(mat * Mat4x4i({13, 14, 15, 16}, {17, 18, 19, 20}, {21, 22, 23, 24}, {25, 26, 27, 28})
            == Mat4x3i({334, 392, 450}, {422, 496, 570}, {510, 600, 690}, {598, 704, 810}));
    REQUIRE(mat * Vec4i(13, 14, 15, 16) == Vec3i(334, 392, 450));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13))
            == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132, 156));
    REQUIRE(OuterProduct(Vec4i(1, 2, 3, 4), Vec3i(5, 6, 7))
            == Mat3x4i({5, 10, 15, 20}, {6, 12, 18, 24}, {7, 14, 21, 28}));
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
    REQUIRE(mat * Mat2x4i({17, 18, 19, 20}, {21, 22, 23, 24})
            == Mat2x4i({538, 612, 686, 760}, {650, 740, 830, 920}));
    REQUIRE(mat * Mat3x4i({17, 18, 19, 20}, {21, 22, 23, 24}, {25, 26, 27, 28})
            == Mat3x4i({538, 612, 686, 760}, {650, 740, 830, 920}, {762, 868, 974, 1'080}));
    REQUIRE(mat * Mat4x4i({17, 18, 19, 20}, {21, 22, 23, 24}, {25, 26, 27, 28}, {29, 30, 31, 32})
            == Mat4x4i({538, 612, 686, 760}, {650, 740, 830, 920}, {762, 868, 974, 1'080},
                       {874, 996, 1'118, 1'240}));
    REQUIRE(mat * Vec4i(17, 18, 19, 20) == Vec4i(538, 612, 686, 760));

    REQUIRE(Hadamard(mat, Mat(2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17))
            == Mat(2, 6, 12, 20, 30, 42, 56, 72, 90, 110, 132, 156, 182, 210, 240, 272));
    REQUIRE(Determinant(Mat {{1, 2, 3, 4}, {5, 6, 7, 8}, {9, 10, 11, 12}, {13, 14, 15, 16}}) == 0);
    REQUIRE(Determinant(Mat {4, 2, 3, 1, 6, 8, 7, 5, 1, -2, 8, 3, 5, 1, 4, 7}) == 1'066);
    REQUIRE(Inverse(Mat4({1, 0, 0, 0}, {0, 2, 0, 0}, {0, 0, 3, 0}, {0, 0, 0, 4}))
            == Mat4({1, 0, 0, 0}, {0, 0.5f, 0, 0}, {0, 0, 0.33333333333333f, 0}, {0, 0, 0, 0.25f}));
    REQUIRE(OuterProduct(Vec4i(1, 2, 3, 4), Vec4i(5, 6, 7, 8))
            == Mat4x4i({5, 10, 15, 20}, {6, 12, 18, 24}, {7, 14, 21, 28}, {8, 16, 24, 32}));
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