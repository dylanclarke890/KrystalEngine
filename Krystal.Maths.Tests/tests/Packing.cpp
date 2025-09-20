#include "pch.hpp"

#include "Krystal.Maths/Packing.hpp"

namespace Krys::Tests
{
  using namespace Krys::Maths;

  TEST_CASE("2x8To16", "[Packing]")
  {
    REQUIRE(Pack2x8To16(1u, 1u) == 257u);
    REQUIRE(Pack2x8To16(Vector2<uint8>(1u, 1u)) == 257u);
    REQUIRE(Unpack16To2x8(257u) == Vector2<uint8>(1u, 1u));

    REQUIRE(Pack2x8To16(1, 1) == 257);
    REQUIRE(Pack2x8To16(Vector2<int8>(1, 1)) == 257);
    REQUIRE(Unpack16To2x8(257) == Vector2<int8>(1, 1));
  }

  TEST_CASE("4x8To32", "[Packing]")
  {
    REQUIRE(Pack4x8To32(1u, 1u, 1u, 1u) == 16'843'009u);
    REQUIRE(Pack4x8To32(Vector4<uint8>(1u, 1u, 1u, 1u)) == 16'843'009u);
    REQUIRE(Unpack32To4x8(16'843'009u) == Vector4<uint8>(1u, 1u, 1u, 1u));

    REQUIRE(Pack4x8To32(1, 1, 1, 1) == 16'843'009);
    REQUIRE(Pack4x8To32(Vector4<int8>(1, 1, 1, 1)) == 16'843'009);
    REQUIRE(Unpack32To4x8(16'843'009) == Vector4<int8>(1, 1, 1, 1));
  }

  TEST_CASE("2x16To32", "[Packing]")
  {
    REQUIRE(Pack2x16To32(1u, 1u) == 65'537u);
    REQUIRE(Pack2x16To32(Vector2<uint16>(1u, 1u)) == 65'537u);
    REQUIRE(Unpack32To2x16(65'537u) == Vector2<uint16>(1u, 1u));

    REQUIRE(Pack2x16To32(1, 1) == 65'537);
    REQUIRE(Pack2x16To32(Vector2<int16>(1, 1)) == 65'537);
    REQUIRE(Unpack32To2x16(65'537) == Vector2<int16>(1, 1));
  }

  TEST_CASE("4x16To64", "[Packing]")
  {
    REQUIRE(Pack4x16To64(1u, 1u, 1u, 1u) == 281'479'271'743'489u);
    REQUIRE(Pack4x16To64(Vector4<uint16>(1u, 1u, 1u, 1u)) == 281'479'271'743'489u);
    REQUIRE(Unpack64To4x16(281'479'271'743'489u) == Vector4<uint16>(1u, 1u, 1u, 1u));

    REQUIRE(Pack4x16To64(1, 1, 1, 1) == 281'479'271'743'489);
    REQUIRE(Pack4x16To64(Vector4<int16>(1, 1, 1, 1)) == 281'479'271'743'489);
    REQUIRE(Unpack64To4x16(281'479'271'743'489) == Vector4<int16>(1, 1, 1, 1));
  }

  TEST_CASE("2x32To64", "[Packing]")
  {
    REQUIRE(Pack2x32To64(1u, 1u) == 4'294'967'297u);
    REQUIRE(Pack2x32To64(Vector2<uint32>(1u, 1u)) == 4'294'967'297u);
    REQUIRE(Unpack64To2x32(4'294'967'297u) == Vector2<uint32>(1u, 1u));

    REQUIRE(Pack2x32To64(1, 1) == 4'294'967'297);
    REQUIRE(Pack2x32To64(Vector2<int32>(1, 1)) == 4'294'967'297);
    REQUIRE(Unpack64To2x32(4'294'967'297) == Vector2<int32>(1, 1));
  }

  TEST_CASE("NormFloatTo8", "[Packing]")
  {
    REQUIRE(PackNormFloatToInt8(0.5f) == 64);
    REQUIRE_THAT(UnpackInt8ToNormFloat(64), Catch::Matchers::WithinRel(0.5f, 0.4f));

    REQUIRE(PackNormFloatToUint8(0.5f) == 128);
    REQUIRE_THAT(UnpackUint8ToNormFloat(128), Catch::Matchers::WithinRel(0.5f, 0.4f));
  }

  TEST_CASE("NormFloatTo16", "[Packing]")
  {
    REQUIRE(PackNormFloatToInt16(0.5f) == 16'384);
    REQUIRE_THAT(UnpackInt16ToNormFloat(16'384), Catch::Matchers::WithinRel(0.5f, 0.0002f));

    REQUIRE(PackNormFloatToUint16(0.5f) == 32'768);
    REQUIRE_THAT(UnpackUint16ToNormFloat(32'768), Catch::Matchers::WithinRel(0.5f, 0.0002f));
  }
}