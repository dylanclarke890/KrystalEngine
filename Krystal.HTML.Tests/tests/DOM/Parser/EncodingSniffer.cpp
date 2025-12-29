#include "Krystal.HTML/DOM/Parser/EncodingSniffer.hpp"
#include "Krystal.Text/Encodings/UTF/UTF8.hpp"
#include "Krystal.Text/Encodings/UTF/UTF16.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML::DOM;

  TEST_CASE("EncodingSniffer(Detect)", "[HTML][DOM][Parser][EncodingSniffer]")
  {
    EncodingRegistry registry;
    registry.Register<UTF8Encoding>();
    registry.Register<UTF16EncodingBE>();
    registry.Register<UTF16EncodingLE>();

    // Detects BOM
    {
      Array<byte, 3u> data = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"", u8"UTF-8");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"UTF-8");
    }

    {
      Array<byte, 2u> data = {byte {0xFE}, byte {0xFF}};
      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"", u8"UTF-8");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"UTF-16BE");
    }

    {
      Array<byte, 2u> data = {byte {0xFF}, byte {0xFE}};
      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"", u8"UTF-8");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"UTF-16LE");
    }
  }
}