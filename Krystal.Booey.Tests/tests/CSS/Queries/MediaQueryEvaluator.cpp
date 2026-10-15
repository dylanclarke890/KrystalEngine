#include "Krystal.Booey/CSS/Queries/MediaQueryEvaluator.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/CSS/Queries/MediaEnvironment.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    using mq::EvaluationResult;
    using mq::MediaQueryDynamicDependency;

    KRYS_NODISCARD mq::MediaQueryList Parse(utf8_stringview text)
    {
      return mq::MediaQueryParser::Parse(text, StrictParserContext());
    }

    KRYS_NODISCARD const CSSOMStringAtom &Screen()
    {
      static const CSSOMStringAtom screen {u8"screen"};
      return screen;
    }

    KRYS_NODISCARD const CSSOMStringAtom &Print()
    {
      static const CSSOMStringAtom print {u8"print"};
      return print;
    }

    /// @brief Evaluates queries against a document with a root element and an 800x600 screen view.
    struct EvaluationFixture : ValueFixture
    {
      Ref<dom::Document> Document {CreateRef<dom::Document>()};
      RenderStyle RootStyle {RenderStyle::Create()};
      mq::MediaEnvironment Environment {};

      EvaluationFixture()
      {
        auto root = CreateRef<dom::tests::TestElement>(*Document);
        REQUIRE_FALSE(Document->AppendChild(*root).HasException());
        Environment.LayoutSize = IntSize {800, 600};
        Environment.ScreenSize = FloatSize {1'920, 1'080};
      }

      ~EvaluationFixture() noexcept
      {
        mq::SetMediaEnvironment(null);
      }

      /// @brief Whether @p text matches with the current `Environment`, for @p mediaType.
      KRYS_NODISCARD bool Matches(utf8_stringview text, const CSSOMStringAtom &mediaType = Screen())
      {
        mq::SetMediaEnvironment(Environment);
        return mq::MediaQueryEvaluator {mediaType, *Document, &RootStyle}.Evaluate(Parse(text));
      }
    };
  }

  TEST_CASE("MediaQueryEvaluator: media types without a document", "[CSS][Queries][MediaQueryEvaluator]")
  {
    ValueFixture fixture;
    mq::MediaQueryEvaluator screen {Screen()};

    CHECK(screen.Evaluate(Parse(u8"")));
    CHECK(screen.Evaluate(Parse(u8"screen")));
    CHECK(screen.Evaluate(Parse(u8"all")));
    CHECK(screen.Evaluate(Parse(u8"only screen")));
    CHECK(screen.Evaluate(Parse(u8"SCREEN")));
    CHECK_FALSE(screen.Evaluate(Parse(u8"print")));
    CHECK_FALSE(screen.Evaluate(Parse(u8"not screen")));
    CHECK(screen.Evaluate(Parse(u8"not print")));
    CHECK(screen.Evaluate(Parse(u8"print, screen")));
    CHECK_FALSE(screen.Evaluate(Parse(u8"print, tv")));

    // An invalid query is `not all`.
    CHECK_FALSE(screen.Evaluate(Parse(u8"screen and")));

    CHECK_FALSE(screen.IsPrintMedia());
    CHECK(mq::MediaQueryEvaluator {Print()}.IsPrintMedia());

    // The null media type only matches queries without one (or `all`).
    mq::MediaQueryEvaluator none {};
    CHECK(none.Evaluate(Parse(u8"all")));
    CHECK_FALSE(none.Evaluate(Parse(u8"screen")));
  }

  TEST_CASE("MediaQueryEvaluator: conditions without a document use the static result",
            "[CSS][Queries][MediaQueryEvaluator]")
  {
    ValueFixture fixture;

    SECTION("False (the default)")
    {
      mq::MediaQueryEvaluator evaluator {Screen()};
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"(color)")));
      CHECK(evaluator.Evaluate(Parse(u8"not all and (color)")));
      CHECK(evaluator.Evaluate(Parse(u8"(color), screen")));
    }

    SECTION("True")
    {
      mq::MediaQueryEvaluator evaluator {Screen(), EvaluationResult::True};
      CHECK(evaluator.Evaluate(Parse(u8"screen and (color)")));
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"not screen and (color)")));
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"print and (color)")));
    }

    SECTION("Unknown never matches, negated or not")
    {
      mq::MediaQueryEvaluator evaluator {Screen(), EvaluationResult::Unknown};
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"(color)")));
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"not all and (color)")));
    }
  }

  TEST_CASE("MediaQueryEvaluator: dynamic dependencies", "[CSS][Queries][MediaQueryEvaluator]")
  {
    ValueFixture fixture;
    mq::MediaQueryEvaluator evaluator {Screen()};

    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"screen")) == MediaQueryDynamicDependency::None);
    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"(color)")) == MediaQueryDynamicDependency::None);
    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"(min-width: 10px)"))
          == MediaQueryDynamicDependency::Viewport);
    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"(orientation: portrait) or (prefers-reduced-motion)"))
          == (MediaQueryDynamicDependency::Viewport | MediaQueryDynamicDependency::Accessibility));
    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"(width), (prefers-color-scheme: dark)"))
          == (MediaQueryDynamicDependency::Viewport | MediaQueryDynamicDependency::Appearance));

    // Queries for another media type contribute nothing.
    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"print and (width)"))
          == MediaQueryDynamicDependency::None);

    // Unknown features have no schema.
    CHECK(evaluator.CollectDynamicDependencies(Parse(u8"(foo: bar)")) == MediaQueryDynamicDependency::None);
  }

  TEST_CASE("MediaQueryEvaluator: a document needs a view and a root element",
            "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;
    REQUIRE(fixture.Matches(u8"(width)"));

    SECTION("No view: the condition is unknown")
    {
      mq::SetMediaEnvironment(null);
      mq::MediaQueryEvaluator evaluator {Screen(), *fixture.Document, &fixture.RootStyle};
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"(width)")));
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"not all and (width)")));
      CHECK(evaluator.Evaluate(Parse(u8"screen")));
    }

    SECTION("No root element: the condition is unknown")
    {
      auto document = CreateRef<dom::Document>();
      mq::SetMediaEnvironment(fixture.Environment);
      mq::MediaQueryEvaluator evaluator {Screen(), *document, &fixture.RootStyle};
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"(width)")));
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"not all and (width)")));
    }

    SECTION("No root style: unknown (the static result defaults to unknown)")
    {
      mq::MediaQueryEvaluator evaluator {Screen(), *fixture.Document, nullptr};
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"(width)")));
      CHECK_FALSE(evaluator.Evaluate(Parse(u8"not all and (width)")));
    }
  }

  TEST_CASE("MediaQueryEvaluator: three-valued logic", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;

    CHECK(fixture.Matches(u8"(min-width: 100px) and (max-width: 800px)"));
    CHECK_FALSE(fixture.Matches(u8"(min-width: 100px) and (max-width: 799px)"));
    CHECK(fixture.Matches(u8"(max-width: 100px) or (min-width: 800px)"));
    CHECK(fixture.Matches(u8"not (max-width: 100px)"));
    CHECK(fixture.Matches(u8"not all and (max-width: 100px)"));

    // `<general-enclosed>` is unknown: it can't make `or` true or `and` false.
    CHECK(fixture.Matches(u8"(width) or (foo: bar)"));
    CHECK_FALSE(fixture.Matches(u8"(max-width: 1px) or (foo: bar)"));
    CHECK_FALSE(fixture.Matches(u8"not ((max-width: 1px) or (foo: bar))"));
    CHECK_FALSE(fixture.Matches(u8"(width) and (foo: bar)"));
    CHECK_FALSE(fixture.Matches(u8"not ((width) and (foo: bar))"));
    CHECK(fixture.Matches(u8"not ((max-width: 1px) and (foo: bar))"));
  }

  TEST_CASE("MediaQueryEvaluator: viewport size features", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;

    SECTION("width and height")
    {
      CHECK(fixture.Matches(u8"(width)"));
      CHECK(fixture.Matches(u8"(width: 800px)"));
      CHECK(fixture.Matches(u8"(min-width: 800px)"));
      CHECK_FALSE(fixture.Matches(u8"(min-width: 801px)"));
      CHECK(fixture.Matches(u8"(width > 799px)"));
      CHECK_FALSE(fixture.Matches(u8"(width > 800px)"));
      CHECK(fixture.Matches(u8"(400px < width <= 800px)"));
      CHECK_FALSE(fixture.Matches(u8"(400px < width < 800px)"));
      CHECK(fixture.Matches(u8"(800px >= width)"));
      CHECK(fixture.Matches(u8"(height: 600px)"));
      CHECK(fixture.Matches(u8"(max-height: 6.25in)"));    // 600px
      CHECK_FALSE(fixture.Matches(u8"(max-height: 6in)")); // 576px
      CHECK(fixture.Matches(u8"(min-width: calc(400px * 2))"));

      // Only zero can be unitless.
      CHECK(fixture.Matches(u8"(min-width: 0)"));
      CHECK_FALSE(fixture.Matches(u8"(min-width: 1)"));
      CHECK_FALSE(fixture.Matches(u8"not all and (min-width: 1)"));

      fixture.Environment.LayoutSize = IntSize {0, 0};
      CHECK_FALSE(fixture.Matches(u8"(width)"));
      CHECK(fixture.Matches(u8"(width: 0)"));
    }

    SECTION("aspect-ratio")
    {
      CHECK(fixture.Matches(u8"(aspect-ratio)"));
      CHECK(fixture.Matches(u8"(aspect-ratio: 4/3)"));
      CHECK(fixture.Matches(u8"(aspect-ratio: 8 / 6)"));
      CHECK(fixture.Matches(u8"(min-aspect-ratio: 1)"));
      CHECK_FALSE(fixture.Matches(u8"(min-aspect-ratio: 16/9)"));
      CHECK(fixture.Matches(u8"(max-aspect-ratio: 16/9)"));
      CHECK(fixture.Matches(u8"(1 < aspect-ratio < 2)"));

      // A zero denominator is infinite.
      CHECK(fixture.Matches(u8"(aspect-ratio < 1/0)"));
      CHECK_FALSE(fixture.Matches(u8"(aspect-ratio >= 1/0)"));
    }

    SECTION("orientation")
    {
      CHECK(fixture.Matches(u8"(orientation: landscape)"));
      CHECK_FALSE(fixture.Matches(u8"(orientation: portrait)"));
      CHECK(fixture.Matches(u8"(orientation)"));

      fixture.Environment.LayoutSize = IntSize {600, 800};
      CHECK(fixture.Matches(u8"(orientation: portrait)"));

      // A square viewport is portrait.
      fixture.Environment.LayoutSize = IntSize {500, 500};
      CHECK(fixture.Matches(u8"(orientation: portrait)"));
    }
  }

  TEST_CASE("MediaQueryEvaluator: device features", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;

    SECTION("device-width, device-height and device-aspect-ratio")
    {
      CHECK(fixture.Matches(u8"(device-width: 1920px)"));
      CHECK(fixture.Matches(u8"(device-height: 1080px)"));
      CHECK(fixture.Matches(u8"(device-aspect-ratio: 16/9)"));
      CHECK_FALSE(fixture.Matches(u8"(device-width: 800px)"));
    }

    SECTION("resolution")
    {
      CHECK(fixture.Matches(u8"(resolution: 1dppx)"));
      CHECK(fixture.Matches(u8"(resolution: 96dpi)"));
      CHECK(fixture.Matches(u8"(resolution)"));

      fixture.Environment.DeviceScaleFactor = 2.f;
      CHECK(fixture.Matches(u8"(resolution: 2x)"));
      CHECK(fixture.Matches(u8"(min-resolution: 192dpi)"));
      CHECK_FALSE(fixture.Matches(u8"(min-resolution: 3dppx)"));

      // Printing uses 300dpi; other media types have no resolution.
      fixture.Environment.MediaType = Print();
      CHECK(fixture.Matches(u8"(resolution: 300dpi)", Print()));
      fixture.Environment.MediaType = CSSOMStringAtom {u8"tv"};
      CHECK_FALSE(fixture.Matches(u8"(resolution)"));
    }

    SECTION("color, color-index and monochrome")
    {
      CHECK(fixture.Matches(u8"(color)"));
      CHECK(fixture.Matches(u8"(color: 8)"));
      CHECK(fixture.Matches(u8"(min-color: 4)"));
      CHECK_FALSE(fixture.Matches(u8"(min-color: 10)"));
      CHECK_FALSE(fixture.Matches(u8"(color-index)"));
      CHECK(fixture.Matches(u8"(color-index: 0)"));
      CHECK_FALSE(fixture.Matches(u8"(monochrome)"));
      CHECK(fixture.Matches(u8"(monochrome: 0)"));

      fixture.Environment.ScreenIsMonochrome = true;
      CHECK(fixture.Matches(u8"(monochrome)"));
      CHECK(fixture.Matches(u8"(monochrome: 8)"));
    }

    SECTION("color-gamut and dynamic-range")
    {
      CHECK(fixture.Matches(u8"(color-gamut: srgb)"));
      CHECK_FALSE(fixture.Matches(u8"(color-gamut: p3)"));
      CHECK_FALSE(fixture.Matches(u8"(color-gamut: rec2020)"));
      CHECK(fixture.Matches(u8"(dynamic-range: standard)"));
      CHECK_FALSE(fixture.Matches(u8"(dynamic-range: high)"));

      fixture.Environment.ScreenSupportsExtendedColor = true;
      fixture.Environment.ScreenSupportsHighDynamicRange = true;
      CHECK(fixture.Matches(u8"(color-gamut: srgb)"));
      CHECK(fixture.Matches(u8"(color-gamut: p3)"));
      CHECK_FALSE(fixture.Matches(u8"(color-gamut: rec2020)"));
      CHECK(fixture.Matches(u8"(dynamic-range: high)"));
    }

    SECTION("grid and scan")
    {
      CHECK_FALSE(fixture.Matches(u8"(grid)"));
      CHECK(fixture.Matches(u8"(grid: 0)"));
      CHECK_FALSE(fixture.Matches(u8"(grid: 1)"));
      // Any other value is unknown.
      CHECK_FALSE(fixture.Matches(u8"not all and (grid: 2)"));

      CHECK_FALSE(fixture.Matches(u8"(scan)"));
      CHECK_FALSE(fixture.Matches(u8"(scan: progressive)"));
      CHECK_FALSE(fixture.Matches(u8"(scan: interlace)"));
    }

    SECTION("update, overflow-block and overflow-inline")
    {
      CHECK(fixture.Matches(u8"(update: fast)"));
      CHECK(fixture.Matches(u8"(overflow-block: scroll)"));
      CHECK(fixture.Matches(u8"(overflow-inline: scroll)"));
      CHECK_FALSE(fixture.Matches(u8"(overflow-inline: none)"));

      fixture.Environment.IsPaginated = true;
      CHECK(fixture.Matches(u8"(overflow-block: paged)"));

      fixture.Environment.IsPaginated = false;
      fixture.Environment.MediaType = Print();
      CHECK(fixture.Matches(u8"(update: none)", Print()));
      CHECK_FALSE(fixture.Matches(u8"(update)", Print()));
      CHECK(fixture.Matches(u8"(overflow-block: paged)", Print()));
    }
  }

  TEST_CASE("MediaQueryEvaluator: interaction features", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;

    SECTION("A mouse (the default)")
    {
      CHECK(fixture.Matches(u8"(hover)"));
      CHECK(fixture.Matches(u8"(hover: hover)"));
      CHECK(fixture.Matches(u8"(any-hover: hover)"));
      CHECK(fixture.Matches(u8"(pointer: fine)"));
      CHECK(fixture.Matches(u8"(pointer)"));
      CHECK_FALSE(fixture.Matches(u8"(pointer: coarse)"));
      CHECK(fixture.Matches(u8"(any-pointer: fine)"));
    }

    SECTION("A touch screen with a stylus")
    {
      fixture.Environment.HoverSupportedByPrimaryPointingDevice = false;
      fixture.Environment.PrimaryPointingDevice = mq::PointerCharacteristics::Coarse;
      fixture.Environment.AllAvailablePointingDevices =
        mq::PointerCharacteristics::Coarse | mq::PointerCharacteristics::Fine;

      CHECK_FALSE(fixture.Matches(u8"(hover)"));
      CHECK(fixture.Matches(u8"(hover: none)"));
      CHECK(fixture.Matches(u8"(any-hover: hover)"));
      CHECK(fixture.Matches(u8"(pointer: coarse)"));
      CHECK_FALSE(fixture.Matches(u8"(pointer: fine)"));
      CHECK(fixture.Matches(u8"(any-pointer: coarse)"));
      CHECK(fixture.Matches(u8"(any-pointer: fine)"));
    }

    SECTION("No pointing device")
    {
      fixture.Environment.HoverSupportedByPrimaryPointingDevice = false;
      fixture.Environment.HoverSupportedByAnyAvailablePointingDevice = false;
      fixture.Environment.PrimaryPointingDevice = mq::PointerCharacteristics::None;
      fixture.Environment.AllAvailablePointingDevices = mq::PointerCharacteristics::None;

      CHECK_FALSE(fixture.Matches(u8"(any-hover)"));
      CHECK(fixture.Matches(u8"(pointer: none)"));
      CHECK_FALSE(fixture.Matches(u8"(pointer)"));
      CHECK(fixture.Matches(u8"(any-pointer: none)"));
    }
  }

  TEST_CASE("MediaQueryEvaluator: user preference features", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;

    SECTION("No preferences (the default)")
    {
      CHECK(fixture.Matches(u8"(prefers-color-scheme: light)"));
      CHECK_FALSE(fixture.Matches(u8"(prefers-color-scheme: dark)"));
      CHECK(fixture.Matches(u8"(prefers-contrast: no-preference)"));
      CHECK_FALSE(fixture.Matches(u8"(prefers-contrast)"));
      CHECK(fixture.Matches(u8"(prefers-reduced-motion: no-preference)"));
      CHECK_FALSE(fixture.Matches(u8"(prefers-reduced-motion)"));
      CHECK(fixture.Matches(u8"(inverted-colors: none)"));
      CHECK_FALSE(fixture.Matches(u8"(inverted-colors)"));
      CHECK(fixture.Matches(u8"(forced-colors: none)"));
      CHECK_FALSE(fixture.Matches(u8"(forced-colors)"));
    }

    SECTION("With preferences")
    {
      fixture.Environment.UseDarkAppearance = true;
      fixture.Environment.UserPrefersContrast = true;
      fixture.Environment.UserPrefersReducedMotion = true;
      fixture.Environment.ColorsAreInverted = true;

      CHECK(fixture.Matches(u8"(prefers-color-scheme: dark)"));
      CHECK(fixture.Matches(u8"(prefers-contrast: more)"));
      CHECK_FALSE(fixture.Matches(u8"(prefers-contrast: less)"));
      CHECK(fixture.Matches(u8"(prefers-contrast)"));
      CHECK(fixture.Matches(u8"(prefers-reduced-motion: reduce)"));
      CHECK(fixture.Matches(u8"(prefers-reduced-motion)"));
      CHECK(fixture.Matches(u8"(inverted-colors: inverted)"));
    }
  }

  TEST_CASE("MediaQueryEvaluator: scripting and display-mode", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;

    CHECK(fixture.Matches(u8"(scripting: enabled)"));
    CHECK(fixture.Matches(u8"(scripting)"));
    CHECK_FALSE(fixture.Matches(u8"(scripting: initial-only)"));
    CHECK(fixture.Matches(u8"(display-mode: browser)"));

    fixture.Environment.CanExecuteScripts = false;
    fixture.Environment.DisplayMode = mq::DisplayMode::Standalone;
    CHECK(fixture.Matches(u8"(scripting: none)"));
    CHECK_FALSE(fixture.Matches(u8"(scripting)"));
    CHECK(fixture.Matches(u8"(display-mode: standalone)"));
    CHECK_FALSE(fixture.Matches(u8"(display-mode: browser)"));
  }

  TEST_CASE("MediaQueryEvaluator: every schema evaluates", "[CSS][Queries][MediaQueryEvaluator]")
  {
    EvaluationFixture fixture;
    mq::SetMediaEnvironment(fixture.Environment);

    mq::FeatureEvaluationContext context {CreateWeakRef<const dom::Document>(*fixture.Document),
                                          {fixture.RootStyle, &fixture.RootStyle, nullptr, nullptr},
                                          nullptr};
    for (auto *schema : mq::Features::AllSchemas())
    {
      mq::Feature feature {schema->Name};
      feature.Schema = schema;
      INFO(ToNarrow(schema->Name.View()));
      // In a boolean context, every feature is known (true or false).
      CHECK(schema->Evaluate(feature, context) != EvaluationResult::Unknown);
    }
  }
}
