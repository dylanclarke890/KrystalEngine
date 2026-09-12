#include "Krystal.Booey/CSS/CSSCharsetRule.hpp"
#include "Krystal.Booey/CSS/CSSImportRule.hpp"
#include "Krystal.Booey/CSS/CSSNamespaceRule.hpp"
#include "Krystal.Booey/CSS/CSSRuleList.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/MediaList.hpp"
#include "Krystal.Booey/CSS/Parser/Parser.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include <catch_all.hpp>

namespace krys::boo::css::tests
{
  TEST_CASE("Parser::ParseRule - @charset", "[Parser]")
  {
    utf8_string input = u8"@charset \"utf-8\";";
    auto rule = Parser::ParseRule(krys::move(input), {}, AllowedRules::Charset);

    REQUIRE(rule != nullptr);
    REQUIRE(Is<CSSCharsetRule>(rule));
  }

  TEST_CASE("Parser::ParseRule - @namespace", "[Parser]")
  {
    SECTION("Default namespace")
    {
      utf8_string input = u8"@namespace \"http://www.w3.org/1999/xhtml\";";
      auto rule = css::Parser::ParseRule(krys::move(input), {}, AllowedRules::Charset);
      REQUIRE(rule != nullptr);
      REQUIRE(Is<CSSNamespaceRule>(rule));
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->Prefix() == CSSOMStringAtom::Null());
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->NamespaceURI() == u8"http://www.w3.org/1999/xhtml");
    }

    SECTION("Prefix and namespace")
    {
      utf8_string input = u8"@namespace svg \"http://www.w3.org/2000/svg\";";
      auto rule = Parser::ParseRule(krys::move(input), {}, AllowedRules::Charset);

      REQUIRE(rule != nullptr);
      REQUIRE(Is<CSSNamespaceRule>(rule));
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->Prefix() == u8"svg");
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->NamespaceURI() == u8"http://www.w3.org/2000/svg");
    }
  }
}