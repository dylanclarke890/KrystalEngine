#include "Krystal.Booey/Core/DateTimeFormat.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::tests
{
  namespace
  {
    /// @brief Records the tokens as text: fields as their pattern character and count, literals quoted.
    class TokenRecorder final : public DateTimeFormat::TokenHandler
    {
    public:
      utf8_string Tokens;

      void VisitField(DateTimeFormat::FieldType fieldType, int count) noexcept final
      {
        Tokens.push_back(static_cast<char8>(fieldType));
        Tokens.append(reinterpret_cast<const char8 *>(std::to_string(count).c_str()));
        Tokens.push_back(' ');
      }

      void VisitLiteral(utf8_stringview text) noexcept final
      {
        Tokens.push_back('[');
        Tokens.append(text);
        Tokens.append(u8"] ");
      }
    };

    KRYS_NODISCARD utf8_string Parse(utf8_stringview format, bool expectSuccess = true)
    {
      TokenRecorder recorder;
      CHECK(DateTimeFormat::Parse(format, recorder) == expectSuccess);
      return recorder.Tokens;
    }

    KRYS_NODISCARD utf8_string Quote(utf8_stringview literal)
    {
      utf8_string buffer;
      DateTimeFormat::QuoteAndAppendLiteral(literal, buffer);
      return buffer;
    }
  }

  TEST_CASE("DateTimeFormat: parsing", "[Core][DateTimeFormat]")
  {
    CHECK(Parse(u8"yyyy-MM-dd") == u8"y4 [-] M2 [-] d2 ");
    CHECK(Parse(u8"h:mm:ss a") == u8"h1 [:] m2 [:] s2 [ ] a1 ");
    CHECK(Parse(u8"dd.MM.yy") == u8"d2 [.] M2 [.] y2 ");
    CHECK(Parse(u8"MMMM y") == u8"M4 [ ] y1 ");
    // Quoted letters are literals, and two quotes are one.
    CHECK(Parse(u8"yyyy-MM-dd'T'HH:mm") == u8"y4 [-] M2 [-] d2 [T] H2 [:] m2 ");
    CHECK(Parse(u8"HH 'o''clock'") == u8"H2 [ o'clock] ");
    CHECK(Parse(u8"''h") == u8"['] h1 ");
    // Non-ASCII characters are literals, kept whole.
    CHECK(Parse(u8"y\u5e74M\u6708") == u8"y1 [\u5e74] M1 [\u6708] ");
    CHECK(Parse(u8"h:mm\u202fa") == u8"h1 [:] m2 [\u202f] a1 ");
    // Letters that aren't fields fail, as does an unclosed quote.
    CHECK(Parse(u8"yyyy-bb", false) == u8"y4 ");
    CHECK(Parse(u8"HH 'open", false) == u8"H2 [ open] ");
    CHECK(Parse(u8"").empty());
  }

  TEST_CASE("DateTimeFormat: quoting literals", "[Core][DateTimeFormat]")
  {
    CHECK(Quote(u8"") == u8"");
    CHECK(Quote(u8"-/: ") == u8"-/: ");
    CHECK(Quote(u8"T") == u8"'T'");
    CHECK(Quote(u8"o'clock") == u8"'o''clock'");
    CHECK(Quote(u8"''x") == u8"'''''x'");
    CHECK(Parse(Quote(u8"o'clock")) == u8"[o'clock] ");
  }
}
