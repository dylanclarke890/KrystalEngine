#include "Krystal.Booey/CSS/CSSOM/DOMCSSNamespace.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSS.escape", "[CSS][CSSOM][cssom:dom-css-escape]")
  {
    auto escape = [](const char8_t *ident)
    {
      return DOMCSSNamespace::Escape(CSSOMString(ident));
    };

    CHECK(escape(u8"") == u8"");
    CHECK(escape(u8"abc-_1") == u8"abc-_1");
    CHECK(escape(u8"a b") == u8"a\\ b");
    CHECK(escape(u8"a.b#c") == u8"a\\.b\\#c");
    // A digit can't start an identifier, or follow a hyphen that starts it.
    CHECK(escape(u8"1a") == u8"\\31 a");
    CHECK(escape(u8"-1a") == u8"-\\31 a");
    // A hyphen on its own can't be one either, but two can start one.
    CHECK(escape(u8"-") == u8"\\-");
    CHECK(escape(u8"--a") == u8"--a");
    // Control characters are escaped as code points, and other non-ASCII characters are kept.
    CHECK(escape(u8"a\x7f") == u8"a\\7f ");
    CHECK(escape(u8"\u00e9") == u8"\u00e9");
  }
}
