#include "Krystal.HTML/DOM/Loader/EncodingSniffer.hpp"
#include "Krystal.Lib/String/String.hpp"
#include "Krystal.Lib/Types/List.hpp"
#include "Krystal.Text/Codecs/BasicCodec.hpp"
#include "Krystal.Text/Codecs/CodecRegistry.hpp"
#include "Krystal.Text/Codecs/ICodec.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "Krystal.Text/Encodings/EncodingScheme.hpp"
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
  using namespace Krys::Text;

  TEST_CASE("EncodingSniffer(Detect)", "[HTML][DOM][Loader][EncodingSniffer]")
  {
    CodecRegistry registry;
    registry.Register<BasicCodec<basic_utf8<byte>>>();
    registry.Register<BasicCodec<utf16_le_t>>();
    registry.Register<BasicCodec<utf16_be_t>>();
    registry.Register<BasicCodec<basic_ascii<byte>>>();

    // Detects BOM
    {
      {
        Array<byte, 3u> data = {byte {0xEF}, byte {0xBB}, byte {0xBF}};
        const ICodec *codec = EncodingSniffer::Detect(data, registry, "", "UTF-8");
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-8"_s);
      }

      {
        Array<byte, 2u> data = {byte {0xFE}, byte {0xFF}};
        const ICodec *codec = EncodingSniffer::Detect(data, registry, "", "UTF-8");
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-16BE"_s);
      }

      {
        Array<byte, 2u> data = {byte {0xFF}, byte {0xFE}};
        const ICodec *codec = EncodingSniffer::Detect(data, registry, "", "UTF-8");
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-16LE"_s);
      }
    }

    // Invalid BOM and a valid transport codec should return transports codec
    {
      List<byte> data = {byte {0xFF}};
      const ICodec *codec = EncodingSniffer::Detect(data, registry, "ascii", "");
      REQUIRE(codec != nullptr);
      REQUIRE(codec->Name() == "US-ASCII"_s);
    }

    // Invalid BOM, invalid transport, invalid meta charset should return fallback codec
    {
      string content = "<html>";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = EncodingSniffer::Detect(data, registry, "invalid-codec", "US-ASCII");
      REQUIRE(codec != nullptr);
      REQUIRE(codec->Name() == "US-ASCII"_s);
    }

    // invalid BOM, invalid transport, valid meta charset should return meta charset codec
    {
      string content =
        "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta charset='UTF-8'>";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = EncodingSniffer::Detect(data, registry, "invalid-codec", "US-ASCII");
      REQUIRE(codec != nullptr);
      REQUIRE(codec->Name() == "UTF-8"_s);
    }

    // malformed charset should also return meta charset codec
    {
      string content =
        "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta charset='UTF-8";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = EncodingSniffer::Detect(data, registry, "invalid-codec", "US-ASCII");
      REQUIRE(codec != nullptr);
      REQUIRE(codec->Name() == "UTF-8"_s);
    }

    // utf16 charset should return utf8 codec
    {
      {
        string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta "
                         "charset='UTF-16BE";
        List<byte> data;
        StringToBytes(content, data);

        const ICodec *codec = EncodingSniffer::Detect(data, registry, "invalid-codec", "US-ASCII");
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-8"_s);
      }

      {
        string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta "
                         "charset='UTF-16LE";
        List<byte> data;
        StringToBytes(content, data);

        const ICodec *codec = EncodingSniffer::Detect(data, registry, "invalid-codec", "US-ASCII");
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-8"_s);
      }
    }

    // no valid charset/fallback should return nullptr
    {
      string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta ";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = EncodingSniffer::Detect(data, registry, "invalid-codec", "asdas");
      REQUIRE(codec == nullptr);
    }
  }
}