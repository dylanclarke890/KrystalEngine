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
    auto rule = CSSParser::ParseRule(Krys::Move(input), CSSAllowedRules::Charset);

    REQUIRE(rule != nullptr);
    REQUIRE(Is<CSSCharsetRule>(rule));
  }
}