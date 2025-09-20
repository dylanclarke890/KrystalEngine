#pragma once

#include "Krystal.Lib/BitUtils.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys;

  TEST_CASE("BitUtils::Mask", "[BitUtils]")
  {
    REQUIRE(BitUtils::Mask(0) == 0);
    REQUIRE(BitUtils::Mask(1) == 1);
    REQUIRE(BitUtils::Mask(2) == 3);
    REQUIRE(BitUtils::Mask(3) == 7);
    REQUIRE(BitUtils::Mask(4) == 15);
    REQUIRE(BitUtils::Mask(5) == 31);
    REQUIRE(BitUtils::Mask(6) == 63);
    REQUIRE(BitUtils::Mask(7) == 127);
    REQUIRE(BitUtils::Mask(8) == 255);
  }

  TEST_CASE("BitUtils::Set", "[BitUtils]")
  {
    REQUIRE(BitUtils::Set(0, 0) == 1);
    REQUIRE(BitUtils::Set(0, 1) == 2);
    REQUIRE(BitUtils::Set(0, 2) == 4);
    REQUIRE(BitUtils::Set(0, 3) == 8);
    REQUIRE(BitUtils::Set(0, 4) == 16);
    REQUIRE(BitUtils::Set(0, 5) == 32);
    REQUIRE(BitUtils::Set(0, 6) == 64);
    REQUIRE(BitUtils::Set(0, 7) == 128);
    REQUIRE(BitUtils::Set(0, 8) == 256);
  }

  TEST_CASE("BitUtils::Unset", "[BitUtils]")
  {
    REQUIRE(BitUtils::Unset(1, 0) == 0);
    REQUIRE(BitUtils::Unset(2, 1) == 0);
    REQUIRE(BitUtils::Unset(4, 2) == 0);
    REQUIRE(BitUtils::Unset(8, 3) == 0);
    REQUIRE(BitUtils::Unset(16, 4) == 0);
    REQUIRE(BitUtils::Unset(32, 5) == 0);
    REQUIRE(BitUtils::Unset(64, 6) == 0);
    REQUIRE(BitUtils::Unset(128, 7) == 0);
    REQUIRE(BitUtils::Unset(256, 8) == 0);
    REQUIRE(BitUtils::Unset(255, 8) == 255);
  }

  TEST_CASE("BitUtils::Count", "[BitUtils]")
  {
    REQUIRE(BitUtils::Count(0) == 0);
    REQUIRE(BitUtils::Count(1) == 1);
    REQUIRE(BitUtils::Count(2) == 1);
    REQUIRE(BitUtils::Count(3) == 2);
    REQUIRE(BitUtils::Count(4) == 1);
    REQUIRE(BitUtils::Count(15) == 4);
    REQUIRE(BitUtils::Count(16) == 1);
    REQUIRE(BitUtils::Count(255) == 8);
    REQUIRE(BitUtils::Count(256) == 1);
  }

  TEST_CASE("BitUtils::RotateRight", "[BitUtils]")
  {
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 1) == 0b01'000'000U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 2) == 0b00'100'000U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 3) == 0b00'010'000U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 4) == 0b00'001'000U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 5) == 0b00'000'100U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 6) == 0b00'000'010U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 7) == 0b00'000'001U);
    REQUIRE(BitUtils::RotateRight<uint8>(0b10'000'000U, 8) == 0b10'000'000U);
  }

  TEST_CASE("BitUtils::RotateLeft", "[BitUtils]")
  {
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 1) == 0b00'000'001U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 2) == 0b00'000'010U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 3) == 0b00'000'100U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 4) == 0b00'001'000U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 5) == 0b00'010'000U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 6) == 0b00'100'000U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 7) == 0b01'000'000U);
    REQUIRE(BitUtils::RotateLeft<uint8>(0b10'000'000U, 8) == 0b10'000'000U);
  }
}