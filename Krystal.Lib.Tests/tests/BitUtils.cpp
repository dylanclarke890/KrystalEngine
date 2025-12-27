#pragma once

#include "Krystal.Lib/Bit.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys;

  TEST_CASE("Bit::Mask", "[Bit]")
  {
    REQUIRE(Bit::Mask(0) == 0);
    REQUIRE(Bit::Mask(1) == 1);
    REQUIRE(Bit::Mask(2) == 3);
    REQUIRE(Bit::Mask(3) == 7);
    REQUIRE(Bit::Mask(4) == 15);
    REQUIRE(Bit::Mask(5) == 31);
    REQUIRE(Bit::Mask(6) == 63);
    REQUIRE(Bit::Mask(7) == 127);
    REQUIRE(Bit::Mask(8) == 255);
  }

  TEST_CASE("Bit::Set", "[Bit]")
  {
    REQUIRE(Bit::Set(0, 0) == 1);
    REQUIRE(Bit::Set(0, 1) == 2);
    REQUIRE(Bit::Set(0, 2) == 4);
    REQUIRE(Bit::Set(0, 3) == 8);
    REQUIRE(Bit::Set(0, 4) == 16);
    REQUIRE(Bit::Set(0, 5) == 32);
    REQUIRE(Bit::Set(0, 6) == 64);
    REQUIRE(Bit::Set(0, 7) == 128);
    REQUIRE(Bit::Set(0, 8) == 256);
  }

  TEST_CASE("Bit::Unset", "[Bit]")
  {
    REQUIRE(Bit::Unset(1, 0) == 0);
    REQUIRE(Bit::Unset(2, 1) == 0);
    REQUIRE(Bit::Unset(4, 2) == 0);
    REQUIRE(Bit::Unset(8, 3) == 0);
    REQUIRE(Bit::Unset(16, 4) == 0);
    REQUIRE(Bit::Unset(32, 5) == 0);
    REQUIRE(Bit::Unset(64, 6) == 0);
    REQUIRE(Bit::Unset(128, 7) == 0);
    REQUIRE(Bit::Unset(256, 8) == 0);
    REQUIRE(Bit::Unset(255, 8) == 255);
  }

  TEST_CASE("Bit::Count", "[Bit]")
  {
    REQUIRE(Bit::Count(0) == 0);
    REQUIRE(Bit::Count(1) == 1);
    REQUIRE(Bit::Count(2) == 1);
    REQUIRE(Bit::Count(3) == 2);
    REQUIRE(Bit::Count(4) == 1);
    REQUIRE(Bit::Count(15) == 4);
    REQUIRE(Bit::Count(16) == 1);
    REQUIRE(Bit::Count(255) == 8);
    REQUIRE(Bit::Count(256) == 1);
  }

  TEST_CASE("Bit::RotateRight", "[Bit]")
  {
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 1) == 0b01'000'000U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 2) == 0b00'100'000U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 3) == 0b00'010'000U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 4) == 0b00'001'000U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 5) == 0b00'000'100U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 6) == 0b00'000'010U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 7) == 0b00'000'001U);
    REQUIRE(Bit::RotateRight<uint8>(0b10'000'000U, 8) == 0b10'000'000U);
  }

  TEST_CASE("Bit::RotateLeft", "[Bit]")
  {
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 1) == 0b00'000'001U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 2) == 0b00'000'010U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 3) == 0b00'000'100U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 4) == 0b00'001'000U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 5) == 0b00'010'000U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 6) == 0b00'100'000U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 7) == 0b01'000'000U);
    REQUIRE(Bit::RotateLeft<uint8>(0b10'000'000U, 8) == 0b10'000'000U);
  }
}