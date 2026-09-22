#include "Krystal.Core/Text/EncodingRegistry.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::text::tests
{
  TEST_CASE("EncodingRegistry::Find", "[EncodingRegistry]")
  {
    EncodingRegistry registry;
    registry.Register<krys::text::UTF8>();
    registry.Register<krys::text::UTF16BE>();
    registry.Register<krys::text::UTF16LE>();

    ICodec *utf8Codec = registry.Find(u8"utf-8");
    REQUIRE(utf8Codec != nullptr);
    REQUIRE(utf8Codec->Id() == krys::text::EncodingId::UTF8);

    ICodec *utf16beCodec = registry.Find(u8"utf-16be");
    REQUIRE(utf16beCodec != nullptr);
    REQUIRE(utf16beCodec->Id() == krys::text::EncodingId::UTF16BE);

    ICodec *utf16leCodec = registry.Find(u8"utf-16le");
    REQUIRE(utf16leCodec != nullptr);
    REQUIRE(utf16leCodec->Id() == krys::text::EncodingId::UTF16LE);

    ICodec *nonExistentCodec = registry.Find(u8"non-existent");
    REQUIRE(nonExistentCodec == nullptr);
  }
}