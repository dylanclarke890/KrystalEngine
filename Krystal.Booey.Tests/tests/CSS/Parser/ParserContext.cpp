#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include <string>

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD std::string ToNarrow(CSSOMStringView str) noexcept
    {
      return std::string(reinterpret_cast<const char *>(str.data()), str.size());
    }
  }

  TEST_CASE("ParserContext(ParserMode, URL)", "[CSS][Parser][ParserContext]")
  {
    SECTION("Defaults")
    {
      ParserContext context {ParserMode::HTMLStandard};
      CHECK(context.Mode == ParserMode::HTMLStandard);
      CHECK(context.BaseURL.IsEmpty());
      CHECK(context.Charset.empty());
      CHECK_FALSE(context.EnclosingRuleType);
      CHECK_FALSE(context.IsHTMLDocument);
      CHECK_FALSE(context.HasDocumentSecurityOrigin);
      CHECK(context.LoadedFromOpaqueSource == LoadedFromOpaqueSource::No);
      CHECK_FALSE(context.springTimingFunctionEnabled);
      CHECK_FALSE(context.thumbAndTrackPseudoElementsEnabled);
      CHECK(context.PropertySettings == PropertySettings {});
    }

    SECTION("Base URL")
    {
      ParserContext context {ParserMode::HTMLQuirks, url::URL {u8"https://example.com/a.css"}};
      CHECK(context.Mode == ParserMode::HTMLQuirks);
      CHECK(context.BaseURL == url::URL {u8"https://example.com/a.css"});
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
    CHECK(DefaultHash<ParserContext>::Equal(a, b));
    CHECK(DefaultHash<ParserContext>::Hash(a) == a.Hash());

    SECTION("Mode")
    {
      b.Mode = ParserMode::HTMLQuirks;
      CHECK(a != b);
      CHECK(a.Hash() != b.Hash());
    }

    SECTION("Base URL")
    {
      b.BaseURL = url::URL {u8"https://example.com/"};
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

    SECTION("Property settings")
    {
      b.PropertySettings.colorFilterEnabled = true;
      CHECK(a != b);
      CHECK(a.Hash() != b.Hash());
      CHECK(a.PropertySettings.Hash() != b.PropertySettings.Hash());
    }

    SECTION("Enclosing rule type is compared but not hashed")
    {
      b.EnclosingRuleType = RuleType::Style;
      CHECK(a != b);
      CHECK(a.Hash() == b.Hash());
    }
  }

  TEST_CASE("HashTraits<ParserContext>", "[CSS][Parser][ParserContext]")
  {
    using Traits = HashTraits<ParserContext>;
    CHECK(Traits::EmptyValue() == ParserContext {ParserMode::HTMLStandard});
    CHECK_FALSE(Traits::IsDeletedValue(Traits::EmptyValue()));

    auto deleted = Traits::EmptyValue();
    deleted.BaseURL.~URL();
    Traits::ConstructDeletedValue(deleted);
    CHECK(Traits::IsDeletedValue(deleted));
  }

  TEST_CASE("StrictParserContext", "[CSS][Parser][ParserContext]")
  {
    CHECK(StrictParserContext() == ParserContext {ParserMode::HTMLStandard});
    CHECK(&StrictParserContext() == &StrictParserContext());
  }
}
