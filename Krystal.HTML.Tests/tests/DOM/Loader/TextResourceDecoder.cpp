#include "Krystal.HTML/Loader/TextResourceDecoder.hpp"
#include "Krystal.Text/Codecs/BasicCodec.hpp"
#include "Krystal.Text/Codecs/CodecRegistry.hpp"
#include "Krystal.Text/Encodings/ASCII.hpp"
#include "Krystal.Text/Encodings/EncodingScheme.hpp"
#include <catch_all.hpp>

namespace
{
  static Krys::List<Krys::byte> ToBytes(const Krys::string &str)
  {
    Krys::List<Krys::byte> bytes;
    for (char ch : str)
      bytes.push_back(static_cast<Krys::byte>(ch));
    return bytes;
  }
}

namespace Krys::Tests
{
  using namespace Krys::HTML;
  using namespace Krys::Text;

  static CodecRegistry MakeRegistry()
  {
    CodecRegistry registry;
    registry.Register<BasicCodec<basic_utf8<byte>>>();
    registry.Register<BasicCodec<utf16_le_t>>();
    registry.Register<BasicCodec<utf16_be_t>>();
    registry.Register<BasicCodec<basic_ascii<byte>>>();
    return registry;
  }

  TEST_CASE("TextResourceDecoder decodes basic UTF-8", "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::PlainText, "");

    auto bytes = ToBytes("hello world");
    utf32_string out = decoder.Decode(bytes, TextResourceDecoder::IsFinalChunk(true));

    REQUIRE(out == U"hello world");
  }

  TEST_CASE("TextResourceDecoder strips UTF-8 BOM", "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::PlainText, "");

    List<byte> bytes = {byte {0xEF}, byte {0xBB}, byte {0xBF}, byte {'h'}, byte {'i'}};

    utf32_string out = decoder.Decode(bytes, TextResourceDecoder::IsFinalChunk(true));
    REQUIRE(out == U"hi");
  }

  TEST_CASE("TextResourceDecoder waits for BOM when chunk too small", "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::PlainText, "");

    // Only first byte of BOM
    List<byte> part1 = {byte {0xEF}};
    utf32_string out1 = decoder.Decode(part1);
    REQUIRE(out1.empty());

    // Remaining BOM + content
    List<byte> part2 = {byte {0xBB}, byte {0xBF}, byte {'x'}};
    utf32_string out2 = decoder.Decode(part2, TextResourceDecoder::IsFinalChunk(true));

    REQUIRE(out2 == U"x");
  }

  TEST_CASE("TextResourceDecoder handles incomplete UTF-8 sequence across chunks",
            "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::PlainText, "");

    // U+20AC (€) = E2 82 AC
    List<byte> part1 = {byte {0xE2}, byte {0x82}};
    List<byte> part2 = {byte {0xAC}};

    utf32_string out1 = decoder.Decode(part1);
    REQUIRE(out1.empty());

    utf32_string out2 = decoder.Decode(part2, TextResourceDecoder::IsFinalChunk(true));
    REQUIRE(out2 == utf32_string {U'\u20AC'});
  }

  TEST_CASE("TextResourceDecoder emits replacement char on final incomplete sequence",
            "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::PlainText, "");

    utf8_string test = u8"€";
    List<byte> bytes = {static_cast<byte>(test[0]), static_cast<byte>(test[1])}; // missing last byte

    utf32_string out = decoder.Decode(bytes, TextResourceDecoder::IsFinalChunk(true));
    REQUIRE(out.size() == 1);
    REQUIRE(out[0] == Unicode::Replacement<char32>);
  }

  TEST_CASE("TextResourceDecoder detects HTML meta charset after buffering", "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::HTML, "");

    string html = "<html><head>"
                  "<meta charset=\"UTF-8\">"
                  "</head><body>hello</body></html>";

    auto bytes = ToBytes(html);

    utf32_string out = decoder.Decode(bytes, TextResourceDecoder::IsFinalChunk(true));
    REQUIRE(out.find(U"hello") != utf32_string::npos);
  }

  TEST_CASE("TextResourceDecoder waits for enough data before HTML auto-detect",
            "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::HTML, "");

    string partial = "<meta charset=\"UTF-8\">";
    auto bytes = ToBytes(partial);

    utf32_string out = decoder.Decode(bytes);
    REQUIRE(out.empty()); // not enough bytes yet

    utf32_string out2 = decoder.Decode({}, TextResourceDecoder::IsFinalChunk(true));
    REQUIRE(out2 == U"<meta charset=\"UTF-8\">");
  }

  TEST_CASE("TextResourceDecoder freezes encoding after decoding starts", "[HTML][TextResourceDecoder]")
  {
    auto registry = MakeRegistry();
    auto decoder = TextResourceDecoder::Create(registry, ContentType::HTML, "");

    auto first = ToBytes("hello ");
    auto second = ToBytes("<meta charset=\"UTF-16\">world");

    utf32_string out1 = decoder.Decode(first);
    REQUIRE(out1.empty());

    utf32_string out2 = decoder.Decode(second, TextResourceDecoder::IsFinalChunk(true));
    REQUIRE(out2.find(U"world") != utf32_string::npos);
  }
}
