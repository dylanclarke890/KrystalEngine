#include "Krystal.HTML/DOM/Parser/EncodingSniffer.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "Krystal.Text/Encodings/UTF/UTF16.hpp"
#include "Krystal.Text/Encodings/UTF/UTF8.hpp"
#include <catch_all.hpp>

namespace
{
  static void StringToBytes(const Krys::string &str, Krys::List<Krys::byte> &bytes)
  {
    for (char ch : str)
    {
      bytes.push_back(static_cast<Krys::byte>(ch));
    }
  }
}

namespace Krys::Tests
{
  using namespace Krys::HTML::DOM;

  TEST_CASE("EncodingSniffer(Detect)", "[HTML][DOM][Parser][EncodingSniffer]")
  {
    EncodingRegistry registry;
    registry.Register<UTF8Encoding>();
    registry.Register<UTF16EncodingBE>();
    registry.Register<UTF16EncodingLE>();
    registry.Register<ASCIIEncoding>();

    // Detects BOM
    {
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

    // Invalid BOM and a valid transport encoding should return transports encoding
    {
      List<byte> data = {byte {0xFF}};
      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"ascii", u8"");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"US-ASCII");
    }

    // Invalid BOM, invalid transport, invalid meta charset should return fallback encoding
    {
      string content = "<html>";
      List<byte> data;
      StringToBytes(content, data);

      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"invalid-encoding", u8"US-ASCII");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"US-ASCII");
    }

    // invalid BOM, invalid transport, valid meta charset should return meta charset encoding
    {
      string content =
        "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta charset='UTF-8'>";
      List<byte> data;
      StringToBytes(content, data);

      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"invalid-encoding", u8"US-ASCII");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"UTF-8");
    }

    // malformed charset should also return meta charset encoding
    {
      string content =
        "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta charset='UTF-8";
      List<byte> data;
      StringToBytes(content, data);

      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"invalid-encoding", u8"US-ASCII");
      REQUIRE(encoding != nullptr);
      REQUIRE(encoding->GetInfo().Name == u8"UTF-8");
    }

    // utf16 charset should return utf8 encoding
    {
      {
        string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta "
                         "charset='UTF-16BE";
        List<byte> data;
        StringToBytes(content, data);

        const Encoding *encoding =
          EncodingSniffer::Detect(data, registry, u8"invalid-encoding", u8"US-ASCII");
        REQUIRE(encoding != nullptr);
        REQUIRE(encoding->GetInfo().Name == u8"UTF-8");
      }

      {
        string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta "
                         "charset='UTF-16LE";
        List<byte> data;
        StringToBytes(content, data);

        const Encoding *encoding =
          EncodingSniffer::Detect(data, registry, u8"invalid-encoding", u8"US-ASCII");
        REQUIRE(encoding != nullptr);
        REQUIRE(encoding->GetInfo().Name == u8"UTF-8");
      }
    }

    // no valid charset/fallback should return nullptr
    {
      string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta ";
      List<byte> data;
      StringToBytes(content, data);

      const Encoding *encoding = EncodingSniffer::Detect(data, registry, u8"invalid-encoding", u8"asdas");
      REQUIRE(encoding == nullptr);
    }
  }
}