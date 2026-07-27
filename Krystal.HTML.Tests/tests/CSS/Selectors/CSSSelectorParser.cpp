#include "Krystal.HTML/CSS/Selectors/CSSSelectorParser.hpp"
#include "Krystal.HTML.Tests/TestParsers/TestParserUtils.hpp"
#include "Krystal.HTML/CSS/CSSImportRule.hpp"
#include "Krystal.HTML/CSS/CSSNamespaceRule.hpp"
#include "Krystal.HTML/CSS/CSSRuleList.hpp"
#include "Krystal.HTML/CSS/CSSStyleRule.hpp"
#include "Krystal.HTML/CSS/CSSStyleSheet.hpp"
#include "Krystal.HTML/CSS/MediaList.hpp"
#include "Krystal.HTML/CSS/StyleSheetContents.hpp"
#include <catch_all.hpp>

namespace Krys::HTML::Tests
{
  TEST_CASE("CSSSelectorParser", "[CSS][Selectors]")
  {
    auto result = CSSSelectorParser::ParseSelector(u8"div", {}, nullptr, Null);
    REQUIRE(result);
    REQUIRE(result->SelectorCount() == 1uz);
    REQUIRE(result->ComponentCount() == 1uz);
  }
}