#include "Krystal.Booey/Page/Settings.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Style/ResolverTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/CSS/StyleSheetContents.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include <algorithm>

namespace krys::boo::tests
{
  using style::tests::ResolverFixture;

  TEST_CASE_METHOD(css::tests::ValueFixture, "Settings: defaults and documents", "[Page][Settings]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><p id=p>x</p>)"};
    auto &document = *fixture.Document;

    SECTION("Documents share the default settings")
    {
      CHECK(&document.GetSettings() == &Settings::Default());
      auto &settings = Settings::Default();
      CHECK_FALSE(settings.CSSColorLayersEnabled);
      CHECK_FALSE(settings.SpringTimingFunctionEnabled);
      CHECK(settings.Values.DefaultFontSize == 16);
      CHECK(settings.Values.DefaultFixedFontSize == 13);
    }

    SECTION("Parser contexts take the document's settings")
    {
      auto settings = Settings::Create();
      settings->SpringTimingFunctionEnabled = true;
      settings->CSSColorLayersEnabled = true;
      settings->ThumbAndTrackPseudoElementsEnabled = true;
      settings->ColorFilterEnabled = true;
      document.SetSettings(Ref<Settings>(settings));

      auto &context = document.CssParserContext();
      CHECK(context.springTimingFunctionEnabled);
      CHECK(context.colorLayersEnabled);
      CHECK(context.thumbAndTrackPseudoElementsEnabled);
      CHECK(context.PropertySettings.colorFilterEnabled);
      CHECK(css::SelectorParserContext(document).thumbAndTrackPseudoElementsEnabled);

      // Embedder properties are exposed in computed style only when their setting is on.
      auto &exposed = document.ExposedComputedCSSPropertyIds();
      CHECK(std::ranges::find(exposed, css::PropertyId::AppleColorFilter) != exposed.end());
      document.SetSettings(Settings::Create());
      auto &hidden = document.ExposedComputedCSSPropertyIds();
      CHECK(std::ranges::find(hidden, css::PropertyId::AppleColorFilter) == hidden.end());
      CHECK_FALSE(document.CssParserContext().springTimingFunctionEnabled);
    }

    SECTION("Style resolution reads the settings values")
    {
      document.UpdateStyleIfNeeded();
      auto fontSize = [&]
      {
        auto *style = fixture.ById(u8"p").GetRenderStyle();
        REQUIRE(style);
        return style->ComputedFontSize();
      };
      CHECK(fontSize() == 16.f);

      auto settings = Settings::Create();
      settings->Values.DefaultFontSize = 20;
      document.SetSettings(krys::move(settings));
      CHECK(document.HasPendingFullStyleRebuild());
      document.UpdateStyleIfNeeded();
      CHECK(fontSize() == 20.f);
    }

    SECTION("Documents a document creates share its settings")
    {
      auto settings = Settings::Create();
      document.SetSettings(Ref<Settings>(settings));

      auto created = document.Implementation().CreateHTMLDocument(null);
      REQUIRE_FALSE(created.HasException());
      auto createdDocument = created.ReleaseValue();
      CHECK(&createdDocument->GetSettings() == settings.get());
      CHECK(createdDocument->Body());

      auto clone = document.CloneNode();
      REQUIRE_FALSE(clone.HasException());
      CHECK(&Downcast<dom::Document>(*clone.ReleaseValue()).GetSettings() == settings.get());
    }
  }

  TEST_CASE_METHOD(css::tests::ValueFixture, "Settings: standard features are always on", "[Page][Settings]")
  {
    ResolverFixture fixture {u8R"(<!DOCTYPE html><style>
      @view-transition { navigation: auto }
      @position-try --a { top: 0 }
      #p { display: grid-lanes; width: calc(random(1px, 1px)); color: contrast-color(black);
           text-box: trim-both cap alphabetic; overflow-anchor: none }
      ::target-text { color: red }
      :popover-open { color: red }
    </style><p id=p>x</p>)"};
    auto &document = *fixture.Document;
    auto &contents = document.StyleScope().ActiveStyleSheets().front()->Contents();
    // Every rule parsed.
    CHECK(contents.RuleCount() == 5);

    document.UpdateStyleIfNeeded();
    auto *style = fixture.ById(u8"p").GetRenderStyle();
    REQUIRE(style);
    CHECK(style->Display() == DisplayType::GridLanes);
    CHECK(ResolverFixture::Serialised(*style, style->Width()) == "1px");
    CHECK(style->GetColor() == boo::Color::white);
  }
}
