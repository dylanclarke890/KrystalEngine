#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/CSSCharsetRule.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  TEST_CASE("CSSParser::ParseRule - @charset", "[CSSParser]")
  {
    utf8_string input = u8"@charset \"utf-8\";";
    auto rule = CSSParser::ParseRule(Krys::Move(input), {}, CSSAllowedRules::Charset);

    REQUIRE(rule != nullptr);
    REQUIRE(Is<CSSCharsetRule>(rule));
  }

  TEST_CASE("CSSParser::ParseRule - @namespace", "[CSSParser]")
  {
    SECTION("Default namespace")
    {
      utf8_string input = u8"@namespace \"http://www.w3.org/1999/xhtml\";";
      auto rule = CSSParser::ParseRule(Krys::Move(input), {}, CSSAllowedRules::Charset);
      REQUIRE(rule != nullptr);
      REQUIRE(Is<CSSNamespaceRule>(rule));
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->Prefix() == CSSOMStringAtom::Null());
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->NamespaceURI() == u8"http://www.w3.org/1999/xhtml");
    }

    SECTION("Prefix and namespace")
    {
      utf8_string input = u8"@namespace svg \"http://www.w3.org/2000/svg\";";
      auto rule = CSSParser::ParseRule(Krys::Move(input), {}, CSSAllowedRules::Charset);

      REQUIRE(rule != nullptr);
      REQUIRE(Is<CSSNamespaceRule>(rule));
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->Prefix() == u8"svg");
      REQUIRE(Downcast<CSSNamespaceRule>(rule.get())->NamespaceURI() == u8"http://www.w3.org/2000/svg");
    }
  }
}