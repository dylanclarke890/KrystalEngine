#include "Krystal.Text/Encodings/EncodingRegistry.hpp"
#include "Krystal.Text/Encodings/UTF/UTF8.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  TEST_CASE("EncodingRegistry(Register)", "[Encodings][Text][EncodingRegistry]")
  {
    EncodingRegistry registry;
    registry.Register<UTF8Encoding>();

    // Retrieve the encoding by its name
    Encoding *encoding = registry.GetByLabel(u8"utf-8");
    REQUIRE(encoding != nullptr);
    REQUIRE(encoding->GetInfo().Name == u8"UTF-8");
  }
}