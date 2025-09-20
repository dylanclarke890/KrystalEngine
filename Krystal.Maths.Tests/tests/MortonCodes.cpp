#include "Krystal.Maths/MortonCodes.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::Maths;

#pragma warning(push)
#pragma warning(disable : 4'309)

#pragma region uint8

  TEST_CASE("Morton::Encode(2xu8)", "[MortonCodes]")
  {
    constexpr uint8 x = 0b11'111'111;
    constexpr uint8 y = 0b00'000'000;

    constexpr uint16 expected = 0b0'101'010'101'010'101;
    constexpr uint16 result = Morton::Encode(x, y);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec2u8)", "[MortonCodes]")
  {
    constexpr uint8 x = 0b11'111'111;
    constexpr uint8 y = 0b00'000'000;
    constexpr Vector2<uint8> v(x, y);

    constexpr uint16 expected = 0b0'101'010'101'010'101;
    constexpr uint16 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(3xu8)", "[MortonCodes]")
  {
    constexpr uint8 x = 5;
    constexpr uint8 y = 9;
    constexpr uint8 z = 1;

    constexpr uint32 expected = 0b010'001'000'111;
    constexpr uint32 result = Morton::Encode(x, y, z);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec3u8)", "[MortonCodes]")
  {
    constexpr uint8 x = 5;
    constexpr uint8 y = 9;
    constexpr uint8 z = 1;
    constexpr Vector3<uint8> v(x, y, z);

    constexpr uint32 expected = 0b010'001'000'111;
    constexpr uint32 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(4xu8)", "[MortonCodes]")
  {
    constexpr uint8 x = 0b11'111'111;
    constexpr uint8 y = 0b11'111'111;
    constexpr uint8 z = 0b00'000'000;
    constexpr uint8 w = 0b00'000'000;

    constexpr uint32 expected = 0b00'110'011'001'100'110'011'001'100'110'011;
    constexpr uint32 result = Morton::Encode(x, y, z, w);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode_(vec4u8)", "[MortonCodes]")
  {
    constexpr uint8 x = 0b11'111'111;
    constexpr uint8 y = 0b11'111'111;
    constexpr uint8 z = 0b00'000'000;
    constexpr uint8 w = 0b00'000'000;
    constexpr Vector4<uint8> v(x, y, z, w);

    constexpr uint32 expected = 0b00'110'011'001'100'110'011'001'100'110'011;
    constexpr uint32 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Decode(u16)", "[MortonCodes]")
  {
    constexpr uint16 v = 0b0'101'010'101'010'101;
    constexpr Vector2<uint8> expected(0b11'111'111, 0b00'000'000);

    constexpr auto result = Morton::Decode(v);

    REQUIRE(result == expected);
  }

#pragma endregion

#pragma region int8

  TEST_CASE("Morton::Encode(2xi8)", "[MortonCodes]")
  {
    constexpr int8 x = 0b11'111'111;
    constexpr int8 y = 0b00'000'000;

    constexpr int16 expected = 0b0'101'010'101'010'101;
    constexpr int16 result = Morton::Encode(x, y);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode_(vec2i8)", "[MortonCodes]")
  {
    constexpr int8 x = 0b11'111'111;
    constexpr int8 y = 0b00'000'000;
    constexpr Vector2<int8> v(x, y);

    constexpr int16 expected = 0b0'101'010'101'010'101;
    constexpr int16 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(3xi8)", "[MortonCodes]")
  {
    constexpr int8 x = 5;
    constexpr int8 y = 9;
    constexpr int8 z = 1;

    constexpr int32 expected = 0b010'001'000'111;
    constexpr int32 result = Morton::Encode(x, y, z);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec3i8)", "[MortonCodes]")
  {
    constexpr int8 x = 5;
    constexpr int8 y = 9;
    constexpr int8 z = 1;
    constexpr Vector3<int8> v(x, y, z);

    constexpr int32 expected = 0b010'001'000'111;
    constexpr int32 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(4xi8)", "[MortonCodes]")
  {
    constexpr int8 x = 0b11'111'111;
    constexpr int8 y = 0b11'111'111;
    constexpr int8 z = 0b00'000'000;
    constexpr int8 w = 0b00'000'000;

    constexpr int32 expected = 0b00'110'011'001'100'110'011'001'100'110'011;
    constexpr int32 result = Morton::Encode(x, y, z, w);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec4i8)", "[MortonCodes]")
  {
    constexpr int8 x = 0b11'111'111;
    constexpr int8 y = 0b11'111'111;
    constexpr int8 z = 0b00'000'000;
    constexpr int8 w = 0b00'000'000;
    constexpr Vector4<int8> v(x, y, z, w);

    constexpr int32 expected = 0b00'110'011'001'100'110'011'001'100'110'011;
    constexpr int32 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

#pragma endregion

#pragma region uint16

  TEST_CASE("Morton::Encode(2xu16)", "[MortonCodes]")
  {
    constexpr uint16 x = 0b1'111'111'111'111'111;
    constexpr uint16 y = 0b0'000'000'000'000'000;

    constexpr uint32 expected = 0b01'010'101'010'101'010'101'010'101'010'101;
    constexpr uint32 result = Morton::Encode(x, y);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec2u16)", "[MortonCodes]")
  {
    constexpr uint16 x = 0b1'111'111'111'111'111;
    constexpr uint16 y = 0b0'000'000'000'000'000;

    constexpr Vector2<uint16> v(x, y);

    constexpr uint32 expected = 0b01'010'101'010'101'010'101'010'101'010'101;
    constexpr uint32 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(3xu16)", "[MortonCodes]")
  {
    constexpr uint16 x = 5;
    constexpr uint16 y = 9;
    constexpr uint16 z = 1;

    constexpr uint64 expected = 1'095;
    constexpr uint64 result = Morton::Encode(x, y, z);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec3u16)", "[MortonCodes]")
  {
    constexpr uint16 x = 5;
    constexpr uint16 y = 9;
    constexpr uint16 z = 1;
    constexpr Vector3<uint16> v(x, y, z);

    constexpr uint64 expected = 1'095;
    constexpr uint64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(4xu16)", "[MortonCodes]")
  {
    constexpr uint16 x = 0b1'111'111'111'111'111;
    constexpr uint16 y = 0b1'111'111'111'111'111;
    constexpr uint16 z = 0b0'000'000'000'000'000;
    constexpr uint16 w = 0b0'000'000'000'000'000;

    constexpr uint64 expected =
      0b0'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011;
    constexpr uint64 result = Morton::Encode(x, y, z, w);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec4u16)", "[MortonCodes]")
  {
    constexpr uint16 x = 0b1'111'111'111'111'111;
    constexpr uint16 y = 0b1'111'111'111'111'111;
    constexpr uint16 z = 0b0'000'000'000'000'000;
    constexpr uint16 w = 0b0'000'000'000'000'000;
    constexpr Vector4<uint16> v(x, y, z, w);

    constexpr uint64 expected =
      0b0'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011;
    constexpr uint64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Decode(u32)", "[MortonCodes]")
  {
    constexpr uint32 v = 0b01'010'101'010'101'010'101'010'101'010'101;

    constexpr uint16 x = 0b1'111'111'111'111'111;
    constexpr uint16 y = 0b0'000'000'000'000'000;

    constexpr Vector2<uint16> expected(x, y);
    constexpr auto result = Morton::Decode(v);

    REQUIRE(result == expected);
  }

#pragma endregion

#pragma region int16

  TEST_CASE("Morton::Encode(2xi16)", "[MortonCodes]")
  {
    constexpr int16 x = 0b1'111'111'111'111'111;
    constexpr int16 y = 0b0'000'000'000'000'000;

    constexpr int32 expected = 0b01'010'101'010'101'010'101'010'101'010'101;
    constexpr int32 result = Morton::Encode(x, y);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec2i16)", "[MortonCodes]")
  {
    constexpr int16 x = 0b1'111'111'111'111'111;
    constexpr int16 y = 0b0'000'000'000'000'000;

    constexpr Vector2<int16> v(x, y);

    constexpr int32 expected = 0b01'010'101'010'101'010'101'010'101'010'101;
    constexpr int32 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(3xi16)", "[MortonCodes]")
  {
    constexpr int16 x = 5;
    constexpr int16 y = 9;
    constexpr int16 z = 1;

    constexpr int64 expected = 1'095;
    constexpr int64 result = Morton::Encode(x, y, z);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec3i6)", "[MortonCodes]")
  {
    constexpr int16 x = 5;
    constexpr int16 y = 9;
    constexpr int16 z = 1;
    constexpr Vector3<int16> v(x, y, z);

    constexpr int64 expected = 1'095;
    constexpr int64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(4xi16)", "[MortonCodes]")
  {
    constexpr int16 x = 0b1'111'111'111'111'111;
    constexpr int16 y = 0b1'111'111'111'111'111;
    constexpr int16 z = 0b0'000'000'000'000'000;
    constexpr int16 w = 0b0'000'000'000'000'000;

    constexpr int64 expected =
      0b0'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011;
    constexpr int64 result = Morton::Encode(x, y, z, w);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec4i16)", "[MortonCodes]")
  {
    constexpr int16 x = 0b1'111'111'111'111'111;
    constexpr int16 y = 0b1'111'111'111'111'111;
    constexpr int16 z = 0b0'000'000'000'000'000;
    constexpr int16 w = 0b0'000'000'000'000'000;
    constexpr Vector4<int16> v(x, y, z, w);

    constexpr int64 expected =
      0b0'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011'001'100'110'011;
    constexpr int64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

#pragma endregion

#pragma region uint32

  TEST_CASE("Morton::Encode(2xu32)", "[MortonCodes]")
  {
    constexpr uint32 x = 0b11'111'111'111'111'111'111'111'111'111'111;
    constexpr uint32 y = 0b00'000'000'000'000'000'000'000'000'000'000;

    constexpr uint64 expected =
      0b0'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101;
    constexpr uint64 result = Morton::Encode(x, y);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec2u32)", "[MortonCodes]")
  {
    constexpr uint32 x = 0b11'111'111'111'111'111'111'111'111'111'111;
    constexpr uint32 y = 0b00'000'000'000'000'000'000'000'000'000'000;
    constexpr Vector2<uint32> v(x, y);

    constexpr uint64 expected =
      0b0'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101;
    constexpr uint64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(3xu32)", "[MortonCodes]")
  {
    constexpr uint32 x = 5;
    constexpr uint32 y = 9;
    constexpr uint32 z = 1;

    constexpr uint64 expected = 1'095;
    constexpr uint64 result = Morton::Encode(x, y, z);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec3u32)", "[MortonCodes]")
  {
    constexpr uint32 x = 5;
    constexpr uint32 y = 9;
    constexpr uint32 z = 1;
    constexpr Vector3<uint32> v(x, y, z);

    constexpr uint64 expected = 1'095;
    constexpr uint64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Decode(u64)", "[MortonCodes]")
  {
    constexpr uint64 v =
      0b0'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101;

    constexpr uint32 x = 0b11'111'111'111'111'111'111'111'111'111'111;
    constexpr uint32 y = 0b00'000'000'000'000'000'000'000'000'000'000;
    constexpr Vector2<uint32> expected(x, y);

    constexpr auto result = Morton::Decode(v);

    REQUIRE(result == expected);
  }

#pragma endregion

#pragma region int32

  TEST_CASE("Morton::Encode(2xi32)", "[MortonCodes]")
  {
    constexpr int32 x = 0b11'111'111'111'111'111'111'111'111'111'111;
    constexpr int32 y = 0b00'000'000'000'000'000'000'000'000'000'000;

    constexpr int64 expected =
      0b0'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101;
    constexpr int64 result = Morton::Encode(x, y);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec2i32)", "[MortonCodes]")
  {
    constexpr int32 x = 0b11'111'111'111'111'111'111'111'111'111'111;
    constexpr int32 y = 0b00'000'000'000'000'000'000'000'000'000'000;
    constexpr Vector2<int32> v(x, y);

    constexpr int64 expected =
      0b0'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101'010'101;
    constexpr int64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(3xi32)", "[MortonCodes]")
  {
    constexpr int32 x = 5;
    constexpr int32 y = 9;
    constexpr int32 z = 1;

    constexpr int64 expected = 1'095;
    constexpr int64 result = Morton::Encode(x, y, z);

    REQUIRE(result == expected);
  }

  TEST_CASE("Morton::Encode(vec3i32)", "[MortonCodes]")
  {
    constexpr int32 x = 5;
    constexpr int32 y = 9;
    constexpr int32 z = 1;
    constexpr Vector3<int32> v(x, y, z);

    constexpr int64 expected = 1'095;
    constexpr int64 result = Morton::Encode(v);

    REQUIRE(result == expected);
  }

#pragma endregion

#pragma warning(pop)
}