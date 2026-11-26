#include "Krystal.Web/Css/Parser/ParserContext.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web.Tests/Css/ValueTestUtils.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Css/Values/SerialisationContext.hpp"
#include "Krystal.Web/Css/Values/ValuePool.hpp"
#include <string>

namespace krys::web::css::tests
{
  TEST_CASE("ParserContext(ParserMode, URL)", "[CSS][Parser][ParserContext]")
  {
    SECTION("Defaults")
    {
      ParserContext context {ParserMode::HTMLStandard};
      CHECK(context.Mode == ParserMode::HTMLStandard);
      CHECK(context.BaseUrl.IsEmpty());
      CHECK(context.Charset.empty());
      CHECK_FALSE(context.EnclosingRuleType);
      CHECK_FALSE(context.IsHTMLDocument);
      CHECK_FALSE(context.springTimingFunctionEnabled);
      CHECK_FALSE(context.thumbAndTrackPseudoElementsEnabled);
      CHECK(context.PropertySettings == PropertySettings {});
    }

    SECTION("Base URL")
    {
      ParserContext context {ParserMode::HTMLQuirks, net::Url {u8"https://example.com/a.css"}};
      CHECK(context.Mode == ParserMode::HTMLQuirks);
      CHECK(context.BaseUrl == net::Url {u8"https://example.com/a.css"});
    }

    SECTION("UA sheets enable the features they rely on")
    {
      ParserContext context {ParserMode::UASheet};
      CHECK(context.thumbAndTrackPseudoElementsEnabled);
      CHECK(context.cssInternalAutoBaseParsingEnabled);
      CHECK_FALSE(context.springTimingFunctionEnabled);
    }

    SECTION("Initialises the static value pool")
    {
      ParserContext context {ParserMode::HTMLStandard};
      CHECK(ToNarrow(PrimitiveValue::Create(ValueId::Auto)->CssText(DefaultSerialisationContext()))
            == "auto");
      CHECK(ToNarrow(PrimitiveValue::Create(12.0, UnitType::px)->CssText(DefaultSerialisationContext()))
            == "12px");
      CHECK(ToNarrow(PrimitiveValue::Create(12.5, UnitType::em)->CssText(DefaultSerialisationContext()))
            == "12.5em");
    }
  }

  TEST_CASE("ParserContext::SetUASheetMode", "[CSS][Parser][ParserContext]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    context.SetUASheetMode();
    CHECK(context.Mode == ParserMode::UASheet);
    CHECK(context == ParserContext {ParserMode::UASheet});
  }

  TEST_CASE("ParserContext equality and hashing", "[CSS][Parser][ParserContext]")
  {
    ParserContext a {ParserMode::HTMLStandard};
    ParserContext b {ParserMode::HTMLStandard};
    CHECK(a == b);
    CHECK(a.Hash() == b.Hash());
    CHECK(FastHash<ParserContext> {}(a) == FastHash<ParserContext> {}(b));

    SECTION("Mode")
    {
      b.Mode = ParserMode::HTMLQuirks;
      CHECK(a != b);
      CHECK(a.Hash() != b.Hash());
    }

    SECTION("Base URL")
    {
      b.BaseUrl = net::Url {u8"https://example.com/"};
      CHECK(a != b);
      CHECK(a.Hash() != b.Hash());
    }

    SECTION("Charset")
    {
      b.Charset = u8"utf-8";
      CHECK(a != b);
      CHECK(a.Hash() != b.Hash());
    }

    SECTION("Settings")
    {
      b.springTimingFunctionEnabled = true;
      CHECK(a != b);
      CHECK(a.Hash() != b.Hash());
    }

    SECTION("Enclosing rule type is compared but not hashed")
    {
      b.EnclosingRuleType = RuleType::Style;
      CHECK(a != b);
      CHECK(a.Hash() == b.Hash());
    }
  }

  TEST_CASE("StrictParserContext", "[CSS][Parser][ParserContext]")
  {
    CHECK(StrictParserContext() == ParserContext {ParserMode::HTMLStandard});
    CHECK(&StrictParserContext() == &StrictParserContext());
  }
}
