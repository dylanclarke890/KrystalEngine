#include "Krystal.Web/Css/CSSOM/Markup.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/CSSOMTestUtils.hpp"

namespace krys::web::css::tests
{
  namespace
  {
    KRYS_NODISCARD CSSOMString MarkupIdentifier(const CSSOMString &identifier)
    {
      StringBuilder builder;
      SerialiseIdentifier(identifier, builder);
      return krys::move(builder).ToString();
    }
  }

  TEST_CASE("Markup: serialising an identifier", "[CSS][CSSOM][Markup][cssom-1:serialize-an-identifier]")
  {
    CHECK(ToNarrow(MarkupIdentifier(u8"abc-_09XYZ")) == "abc-_09XYZ");
    CHECK(MarkupIdentifier(u8"é中") == u8"é中");

    // A digit first, or second after a hyphen, is escaped as a code point.
    CHECK(ToNarrow(MarkupIdentifier(u8"1a")) == "\\31 a");
    CHECK(ToNarrow(MarkupIdentifier(u8"-1a")) == "-\\31 a");
    CHECK(ToNarrow(MarkupIdentifier(u8"a1")) == "a1");
    CHECK(ToNarrow(MarkupIdentifier(u8"--1")) == "--1");

    // A lone hyphen is escaped; one followed by anything isn't.
    CHECK(ToNarrow(MarkupIdentifier(u8"-")) == "\\-");
    CHECK(ToNarrow(MarkupIdentifier(u8"--")) == "--");
    CHECK(ToNarrow(MarkupIdentifier(u8"-a")) == "-a");

    // Control characters are escaped as code points; other ASCII is escaped as itself.
    CHECK(ToNarrow(MarkupIdentifier(u8"a\u0001b")) == "a\\1 b");
    CHECK(ToNarrow(MarkupIdentifier(u8"\u007F")) == "\\7f ");
    CHECK(ToNarrow(MarkupIdentifier(u8"a b.c#")) == "a\\ b\\.c\\#");

    // NULL becomes U+FFFD.
    CHECK(MarkupIdentifier(CSSOMString(u8"a\0b", 3)) == u8"a�b");
  }

  TEST_CASE("Markup: serialising a string", "[CSS][CSSOM][Markup][cssom-1:serialize-a-string]")
  {
    CHECK(ToNarrow(SerialiseString(u8"")) == "\"\"");
    CHECK(ToNarrow(SerialiseString(u8"a 'b' #1")) == "\"a 'b' #1\"");
    CHECK(ToNarrow(SerialiseString(u8"a\"b\\c")) == "\"a\\\"b\\\\c\"");
    CHECK(ToNarrow(SerialiseString(u8"a\nb\u007F")) == "\"a\\a b\\7f \"");
    CHECK(SerialiseString(u8"é") == u8"\"é\"");
    CHECK(SerialiseString(CSSOMString(u8"\0", 1)) == u8"\"�\"");

    StringBuilder builder;
    builder.Append(u8"x=");
    SerialiseString(u8"y", builder);
    CHECK(ToNarrow(krys::move(builder).ToString()) == "x=\"y\"");
  }

  TEST_CASE("Markup: serialising a URL", "[CSS][CSSOM][Markup][cssom-1:serialize-a-url]")
  {
    CHECK(ToNarrow(SerialiseUrl(u8"a b.png")) == "url(\"a b.png\")");
    CHECK(ToNarrow(SerialiseUrl(u8"\"")) == "url(\"\\\"\")");
  }

  TEST_CASE("Markup: serialising a font family", "[CSS][CSSOM][Markup]")
  {
    // A name that is an identifier is kept as one; anything else is a string.
    CHECK(ToNarrow(SerialiseFontFamily(u8"Arial")) == "Arial");
    CHECK(ToNarrow(SerialiseFontFamily(u8"-a")) == "-a");
    CHECK(ToNarrow(SerialiseFontFamily(u8"Times New Roman")) == "\"Times New Roman\"");
    CHECK(ToNarrow(SerialiseFontFamily(u8"1a")) == "\"1a\"");
    CHECK(ToNarrow(SerialiseFontFamily(u8"")) == "\"\"");
  }
}
