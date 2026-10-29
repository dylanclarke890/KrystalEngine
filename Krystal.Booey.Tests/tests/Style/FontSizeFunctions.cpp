#include "Krystal.Booey/Style/FontSizeFunctions.hpp"
#include "catch2/catch_approx.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Graphics/FontSizeAdjust.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"

namespace krys::boo::style::tests
{
  namespace
  {
    KRYS_NODISCARD uint32 Keyword(css::ValueId id) noexcept
    {
      return static_cast<uint32>(id);
    }
  }

  TEST_CASE("FontSizeFunctions: keyword sizes", "[Style][FontSizeFunctions]")
  {
    const SettingsValues settings {};

    SECTION("Standards mode uses the strict table for the default size")
    {
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxSmall), false, settings) == 9);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XSmall), false, settings) == 10);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Small), false, settings) == 13);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Medium), false, settings) == 16);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Large), false, settings) == 18);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XLarge), false, settings) == 24);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxLarge), false, settings) == 32);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxxLarge), false, settings) == 48);
    }

    SECTION("Quirks mode uses the quirks table")
    {
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XSmall), false, settings, true) == 10);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Medium), false, settings, true) == 16);
      // The fixed default size (13) picks a different row.
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxxLarge), true, settings, true) == 40);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxxLarge), true, settings) == 39);
    }

    SECTION("Monospace uses the fixed default size")
    {
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Medium), true, settings) == 13);
    }

    SECTION("Sizes outside the table are scaled, but not below the minimum logical size")
    {
      SettingsValues large {.DefaultFontSize = 20};
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Medium), false, large) == 20);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxxLarge), false, large) == 60);
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxSmall), false, large) == 12);

      large.MinimumLogicalFontSize = 14;
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::XxSmall), false, large) == 14);
    }

    SECTION("From the document")
    {
      auto document = CreateRef<dom::HTMLDocument>();
      CHECK(FontSizeForKeyword(Keyword(css::ValueId::Small), false, *document) == 13);
    }
  }

  TEST_CASE("FontSizeFunctions: computed sizes", "[Style][FontSizeFunctions]")
  {
    SettingsValues settings {.MinimumFontSize = 6, .MinimumLogicalFontSize = 9};

    SECTION("A zero size stays zero")
    {
      CHECK(ComputedFontSizeFromSpecifiedSize(0, false, 2, MinimumFontSizeRule::AbsoluteAndRelative, settings)
            == 0);
    }

    SECTION("Without a rule the size isn't zoomed")
    {
      CHECK(ComputedFontSizeFromSpecifiedSize(4, false, 2, MinimumFontSizeRule::None, settings) == 4);
    }

    SECTION("The hard minimum applies to every size")
    {
      CHECK(ComputedFontSizeFromSpecifiedSize(2, true, 2, MinimumFontSizeRule::Absolute, settings) == 6);
      CHECK(ComputedFontSizeFromSpecifiedSize(10, true, 2, MinimumFontSizeRule::Absolute, settings) == 20);
    }

    SECTION("The smart minimum applies only to relative or acceptable sizes")
    {
      // An explicit small size is allowed.
      CHECK(ComputedFontSizeFromSpecifiedSize(3, true, 2, MinimumFontSizeRule::AbsoluteAndRelative, settings)
            == 6);
      CHECK(ComputedFontSizeFromSpecifiedSize(3, false, 2, MinimumFontSizeRule::AbsoluteAndRelative, settings)
            == 9);
    }

    SECTION("Sizes are clamped")
    {
      CHECK(ComputedFontSizeFromSpecifiedSize(MaximumAllowedFontSize, false, 2, MinimumFontSizeRule::None,
                                              settings)
            == MaximumAllowedFontSize);
      CHECK(ComputedFontSizeFromSpecifiedSize(MaximumAllowedFontSize, false, 2, MinimumFontSizeRule::Absolute,
                                              settings)
            == MaximumAllowedFontSize);
    }

    SECTION("From a style and document")
    {
      auto document = CreateRef<dom::HTMLDocument>();
      auto style = RenderStyle::Create();
      std::ignore = style.SetUsedZoom(2);

      auto computed = ComputedFontSizeFromSpecifiedSize(10, false, false, &style, *document);
      CHECK(computed.Size == 20);
      CHECK(computed.UsedZoomFactor == 2);

      // SVG ignores the zoom.
      computed = ComputedFontSizeFromSpecifiedSize(10, false, true, &style, *document);
      CHECK(computed.Size == 10);
      CHECK(computed.UsedZoomFactor == 1);

      CHECK(ComputedFontSizeFromSpecifiedSizeForSVGInlineText(10, false, 3, *document) == 30);
    }
  }

  TEST_CASE("FontSizeFunctions: legacy sizes", "[Style][FontSizeFunctions]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    CHECK(LegacyFontSizeForPixelSize(10, false, *document) == 1);
    CHECK(LegacyFontSizeForPixelSize(13, false, *document) == 2);
    CHECK(LegacyFontSizeForPixelSize(16, false, *document) == 3);
    CHECK(LegacyFontSizeForPixelSize(18, false, *document) == 4);
    CHECK(LegacyFontSizeForPixelSize(24, false, *document) == 5);
    CHECK(LegacyFontSizeForPixelSize(32, false, *document) == 6);
    CHECK(LegacyFontSizeForPixelSize(100, false, *document) == 7);
  }

  TEST_CASE("FontSizeFunctions: size adjustment", "[Style][FontSizeFunctions]")
  {
    FontMetrics metrics;
    metrics.SetXHeight(8);
    metrics.SetCapHeight(12);

    boo::FontSizeAdjust adjust {.MetricType = boo::FontSizeAdjust::Metric::ExHeight, .Value = 0.5f};
    // The x-height is half the size.
    CHECK(AdjustedFontSize(16, adjust, metrics) == Catch::Approx(16));
    CHECK(AdjustedFontSize(0, adjust, metrics) == 0);

    adjust.MetricType = boo::FontSizeAdjust::Metric::CapHeight;
    CHECK(AdjustedFontSize(16, adjust, metrics) == Catch::Approx(16 * 0.5f / 0.75f));

    // Without the metric the size is kept.
    adjust.MetricType = boo::FontSizeAdjust::Metric::ChWidth;
    CHECK(AdjustedFontSize(16, adjust, metrics) == 16);
  }
}
