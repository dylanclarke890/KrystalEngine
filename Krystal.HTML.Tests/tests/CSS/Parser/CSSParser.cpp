#include "Krystal.HTML/CSS/Parser/CSSParser.hpp"
#include "Krystal.HTML/CSS/CSSCharsetRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  TEST_CASE("CSSParser::ParseRule - @charset", "[CSSParser]")
  {
    utf32_string input = U"@charset \"utf-8\";";
    auto rule = CSSParser::ParseRule(Krys::Move(input), CSSAllowedRules::Charset);

    REQUIRE(rule != nullptr);
    REQUIRE(Is<CSSCharsetRule>(rule));
  }
}