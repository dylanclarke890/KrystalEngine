#include "Krystal.Booey/HTML/Parser/HTMLEncodingSniffer.hpp"
#include "Krystal.Core/Types/String.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Text/Codecs/CodecRegistry.hpp"
#include "Krystal.Text/Codecs/ICodec.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "catch2/catch_test_macros.hpp"

namespace
{
  static void StringToBytes(const krys::string &str, krys::List<krys::byte> &bytes)
  {
    for (char ch : str)
    {
      bytes.push_back(static_cast<krys::byte>(ch));
    }
  }
}

namespace krys::tests
{
  using namespace krys::boo;
  using namespace krys::Text;

  TEST_CASE("HTMLEncodingSniffer(Detect)", "[HTML][DOM][Loader][HTMLEncodingSniffer]")
  {
    CodecRegistry registry;
    registry.Register<BasicCodec<basic_utf8<byte>>>();
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
