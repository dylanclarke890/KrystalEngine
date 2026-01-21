#include "Krystal.HTML/Loader/HTMLEncodingSniffer.hpp"
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
  using namespace Krys::HTML;
  using namespace Krys::Text;

  TEST_CASE("HTMLEncodingSniffer(Detect)", "[HTML][DOM][Loader][HTMLEncodingSniffer]")
  {
    CodecRegistry registry;
    registry.Register<BasicCodec<basic_utf8<byte>>>();
    registry.Register<BasicCodec<utf16_le_t>>();
    registry.Register<BasicCodec<utf16_be_t>>();
    registry.Register<BasicCodec<basic_ascii<byte>>>();

    // valid meta charset should return meta charset codec
    {
      string content =
        "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta charset='UTF-8'>";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = HTMLEncodingSniffer::Detect(data, registry);
      REQUIRE(codec != nullptr);
      REQUIRE(codec->Name() == "UTF-8"_s);
    }

    // malformed charset should also return meta charset codec
    {
      string content =
        "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta charset='UTF-8";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = HTMLEncodingSniffer::Detect(data, registry);
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

        const ICodec *codec = HTMLEncodingSniffer::Detect(data, registry);
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-8"_s);
      }

      {
        string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta "
                         "charset='UTF-16LE";
        List<byte> data;
        StringToBytes(content, data);

        const ICodec *codec = HTMLEncodingSniffer::Detect(data, registry);
        REQUIRE(codec != nullptr);
        REQUIRE(codec->Name() == "UTF-8"_s);
      }
    }

    // no valid charset/fallback should return nullptr
    {
      string content = "<!-- this is some comment --> <someotherelement>asdasds</someotherelement> <meta ";
      List<byte> data;
      StringToBytes(content, data);

      const ICodec *codec = HTMLEncodingSniffer::Detect(data, registry);
      REQUIRE(codec == nullptr);
    }
  }
}

