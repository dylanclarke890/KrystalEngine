#include "Krystal.Booey/CSS/CSSOM/CSSNamespaceRule.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/CSSOMTestUtils.hpp"

namespace krys::boo::css::tests
{
  TEST_CASE("CSSNamespaceRule", "[CSS][CSSOM][CSSNamespaceRule]")
  {
    auto sheet = SheetFromText(u8"@namespace url(http://www.w3.org/1999/xhtml); "
                               u8"@namespace svg url(\"http://www.w3.org/2000/svg\"); "
                               u8"@namespace \\31 x \"a\\\"b\";");

    SECTION("default")
    {
      auto &rule = RuleAt<CSSNamespaceRule>(*sheet, 0);
      CHECK(TypeOf(rule) == RuleType::Namespace);
      CHECK(rule.Prefix() == CSSOMStringAtom::Null());
      CHECK(ToNarrow(CSSOMString(rule.NamespaceURI().View())) == "http://www.w3.org/1999/xhtml");
      CHECK(RuleText(rule) == "@namespace url(\"http://www.w3.org/1999/xhtml\");");
    }

    SECTION("prefixed")
    {
      auto &rule = RuleAt<CSSNamespaceRule>(*sheet, 1);
      CHECK(ToNarrow(CSSOMString(rule.Prefix().View())) == "svg");
      CHECK(RuleText(rule) == "@namespace svg url(\"http://www.w3.org/2000/svg\");");
    }

    SECTION("escaped")
    {
      CHECK(RuleText(*sheet->Item(2)) == "@namespace \\31 x url(\"a\\\"b\");");
    }
  }
}
