#include "Krystal.Booey/CSS/Queries/MediaQueryParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryFeatures.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    KRYS_NODISCARD mq::MediaQueryList Parse(utf8_stringview text)
    {
      return mq::MediaQueryParser::Parse(text, StrictParserContext());
    }

    /// @brief Parses `text` as a media query list and serialises it, as `matchMedia(text).media` does.
    KRYS_NODISCARD std::string Roundtrip(utf8_stringview text)
    {
      CSSOMString result;
      mq::Serialise(result, Parse(text));
      return ToNarrow(result);
    }

    /// @brief The single feature of a single-query list, or null if the query isn't exactly one feature.
    KRYS_NODISCARD Maybe<mq::Feature> SingleFeature(utf8_stringview text)
    {
      auto list = Parse(text);
      if (list.size() != 1 || !list[0].Condition || list[0].Condition->Queries.size() != 1)
      {
        return {};
      }
      auto *feature = std::get_if<mq::Feature>(&list[0].Condition->Queries[0]);
      return feature ? Maybe<mq::Feature> {*feature} : Maybe<mq::Feature> {};
    }

    /// @brief Whether `text` parses as a single feature that matches a schema.
    KRYS_NODISCARD bool IsKnownFeature(utf8_stringview text)
    {
      auto feature = SingleFeature(text);
      return feature && feature->Schema;
    }

    /// @brief Whether `text` parses as a single `<general-enclosed>`: valid, but evaluates to unknown.
    KRYS_NODISCARD bool IsGeneralEnclosed(utf8_stringview text)
    {
      auto list = Parse(text);
      return list.size() == 1 && list[0].Condition && list[0].Condition->Queries.size() == 1
             && std::holds_alternative<mq::GeneralEnclosed>(list[0].Condition->Queries[0]);
    }
  }

  // Cases from WPT css/mediaqueries/match-media-parsing.html.
  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: list parsing", "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"") == "");
    CHECK(Roundtrip(u8" ") == "");
    CHECK(Roundtrip(u8"all") == "all");
    CHECK(Roundtrip(u8" all") == "all");
    CHECK(Roundtrip(u8"   all   ") == "all");
    CHECK(Roundtrip(u8"all,all") == "all, all");
    CHECK(Roundtrip(u8" all , all ") == "all, all");
    CHECK(Roundtrip(u8"(color)") == "(color)");
    CHECK(Roundtrip(u8"(color") == "(color)");
    CHECK(Roundtrip(u8" (color)") == "(color)");
    CHECK(Roundtrip(u8" ( color  )  ") == "(color)");
    CHECK(Roundtrip(u8" ( color   ") == "(color)");
    CHECK(Roundtrip(u8"color)") == "not all");
    CHECK(Roundtrip(u8"  color)") == "not all");
    CHECK(Roundtrip(u8"  color ), ( color") == "not all, (color)");
    CHECK(Roundtrip(u8" foo ") == "foo");
    CHECK(Roundtrip(u8",") == "not all, not all");
    CHECK(Roundtrip(u8" , ") == "not all, not all");
    CHECK(Roundtrip(u8",,") == "not all, not all, not all");
    CHECK(Roundtrip(u8"  ,  ,  ") == "not all, not all, not all");
    CHECK(Roundtrip(u8" foo,") == "foo, not all");

    // A comma inside a block doesn't split the list.
    CHECK(Roundtrip(u8"(a, b), screen") == "(a, b), screen");
  }

  // Cases from WPT css/mediaqueries/match-media-parsing.html.
  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: resolution", "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"(min-resolution: 1x)") == "(min-resolution: 1x)");
    CHECK(Roundtrip(u8"(min-resolution: calc(1x))") == "(min-resolution: calc(1dppx))");
    CHECK(Roundtrip(u8"(resolution: 2x)") == "(resolution: 2x)");
    CHECK(Roundtrip(u8"(resolution: calc(2x))") == "(resolution: calc(2dppx))");
    CHECK(Roundtrip(u8"(max-resolution: 7x)") == "(max-resolution: 7x)");
    CHECK(Roundtrip(u8"(max-resolution: calc(7x))") == "(max-resolution: calc(7dppx))");
    CHECK(Roundtrip(u8"(resolution: 2dppx)") == "(resolution: 2dppx)");
    CHECK(Roundtrip(u8"(resolution: 600dpi)") == "(resolution: 600dpi)");
    CHECK(Roundtrip(u8"(resolution: 77dpcm)") == "(resolution: 77dpcm)");
    CHECK(Roundtrip(u8"(resolution: calc(1x + 2x))") == "(resolution: calc(3dppx))");
    CHECK(Roundtrip(u8"(resolution: calc(5x - 2x))") == "(resolution: calc(3dppx))");
    CHECK(Roundtrip(u8"(resolution: calc(1x * 3))") == "(resolution: calc(3dppx))");
    CHECK(Roundtrip(u8"(resolution: calc(6x / 2))") == "(resolution: calc(3dppx))");
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: media types and prefixes",
                   "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"screen") == "screen");
    CHECK(Roundtrip(u8"SCREEN") == "screen");
    CHECK(Roundtrip(u8"not screen") == "not screen");
    CHECK(Roundtrip(u8"NOT print") == "not print");
    CHECK(Roundtrip(u8"only screen") == "only screen");
    CHECK(Roundtrip(u8"screen and (color)") == "screen and (color)");
    CHECK(Roundtrip(u8"screen AND (color)") == "screen and (color)");
    CHECK(Roundtrip(u8"not screen and (color)") == "not screen and (color)");

    // `all` is dropped before a condition unless there's a prefix.
    CHECK(Roundtrip(u8"all and (color)") == "(color)");
    CHECK(Roundtrip(u8"not all and (color)") == "not all and (color)");
    CHECK(Roundtrip(u8"only all and (color)") == "only all and (color)");

    // WPT css/mediaqueries/mq-invalid-media-type-*.html: these can't be media types.
    CHECK(Roundtrip(u8"not") == "not all");
    CHECK(Roundtrip(u8"only") == "not all");
    CHECK(Roundtrip(u8"and") == "not all");
    CHECK(Roundtrip(u8"or") == "not all");
    CHECK(Roundtrip(u8"not and") == "not all");
    CHECK(Roundtrip(u8"layer") == "not all");
    CHECK(Roundtrip(u8"LAYER") == "not all");
    CHECK(Roundtrip(u8"only not screen") == "not all");

    // Only `and` can follow a media type, and the condition can't use `or`.
    CHECK(Roundtrip(u8"screen (color)") == "not all");
    CHECK(Roundtrip(u8"screen or (color)") == "not all");
    CHECK(Roundtrip(u8"screen and (color) or (hover)") == "not all");
    CHECK(Roundtrip(u8"screen and (color) and (hover)") == "screen and (color) and (hover)");
    CHECK(Roundtrip(u8"screen and not (color)") == "screen and not (color)");
    CHECK(Roundtrip(u8"screen and") == "not all");
    CHECK(Roundtrip(u8"screen print") == "not all");
    CHECK(Roundtrip(u8"1px") == "not all");
    CHECK(Roundtrip(u8"screen, 1px, print") == "screen, not all, print");
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: conditions", "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"(color) and (hover)") == "(color) and (hover)");
    CHECK(Roundtrip(u8"(color) or (hover)") == "(color) or (hover)");
    CHECK(Roundtrip(u8"(color) OR (hover) or (grid)") == "(color) or (hover) or (grid)");
    CHECK(Roundtrip(u8"not (color)") == "not (color)");
    CHECK(Roundtrip(u8"not (not (color))") == "not (not (color))");
    CHECK(Roundtrip(u8"((color) and (hover)) or (grid)") == "((color) and (hover)) or (grid)");
    CHECK(Roundtrip(u8"((color))") == "((color))");

    // Mixed operators, a missing operator, and `not` with more than one query are invalid.
    CHECK(Roundtrip(u8"(color) and (hover) or (grid)") == "not all");
    CHECK(Roundtrip(u8"(color) (hover)") == "not all");
    CHECK(Roundtrip(u8"not (color) and (hover)") == "not all");
    CHECK(Roundtrip(u8"(color) and") == "not all");
    CHECK(Roundtrip(u8"(color) xor (hover)") == "not all");
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: plain and boolean features",
                   "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"(min-width: 100px)") == "(min-width: 100px)");
    CHECK(Roundtrip(u8"(MIN-WIDTH:100PX)") == "(min-width: 100px)");
    CHECK(Roundtrip(u8"(max-width:  100px  )") == "(max-width: 100px)");
    CHECK(Roundtrip(u8"(width: 0)") == "(width: 0)");
    CHECK(Roundtrip(u8"(orientation: portrait)") == "(orientation: portrait)");
    CHECK(Roundtrip(u8"(orientation:LANDSCAPE)") == "(orientation: landscape)");
    CHECK(Roundtrip(u8"(color: 8)") == "(color: 8)");
    CHECK(Roundtrip(u8"(grid)") == "(grid)");
    CHECK(Roundtrip(u8"(prefers-color-scheme: dark)") == "(prefers-color-scheme: dark)");
    CHECK(Roundtrip(u8"(display-mode: minimal-ui)") == "(display-mode: minimal-ui)");

    auto feature = SingleFeature(u8"(min-width: 100px)");
    REQUIRE(feature);
    CHECK(ToNarrow(feature->Name.View()) == "width");
    CHECK(feature->Syntax == mq::Syntax::Plain);
    CHECK(feature->Schema == &mq::Features::Width());
    CHECK_FALSE(feature->LeftComparison);
    REQUIRE(feature->RightComparison);
    CHECK(feature->RightComparison->Op == mq::ComparisonOperator::GreaterThanOrEqual);

    feature = SingleFeature(u8"(max-height: 1px)");
    REQUIRE(feature);
    CHECK(feature->RightComparison->Op == mq::ComparisonOperator::LessThanOrEqual);

    feature = SingleFeature(u8"(hover)");
    REQUIRE(feature);
    CHECK(feature->Syntax == mq::Syntax::Boolean);
    CHECK(feature->Schema == &mq::Features::Hover());
    CHECK_FALSE(feature->RightComparison);
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: range features", "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"(width >= 100px)") == "(width >= 100px)");
    CHECK(Roundtrip(u8"(width>100px)") == "(width > 100px)");
    CHECK(Roundtrip(u8"(width < 100px)") == "(width < 100px)");
    CHECK(Roundtrip(u8"(width <= 100px)") == "(width <= 100px)");
    CHECK(Roundtrip(u8"(width = 100px)") == "(width = 100px)");
    CHECK(Roundtrip(u8"(100px < width)") == "(100px < width)");
    CHECK(Roundtrip(u8"(100px<=WIDTH<=200px)") == "(100px <= width <= 200px)");
    CHECK(Roundtrip(u8"(200px > width >= 100px)") == "(200px > width >= 100px)");

    auto feature = SingleFeature(u8"(100px < width <= 200px)");
    REQUIRE(feature);
    CHECK(feature->Syntax == mq::Syntax::Range);
    CHECK(feature->Schema == &mq::Features::Width());
    REQUIRE(feature->LeftComparison);
    REQUIRE(feature->RightComparison);
    CHECK(feature->LeftComparison->Op == mq::ComparisonOperator::LessThan);
    CHECK(feature->RightComparison->Op == mq::ComparisonOperator::LessThanOrEqual);

    // Invalid comparisons fall back to <general-enclosed>.
    CHECK(IsGeneralEnclosed(u8"(100px < width > 200px)"));
    CHECK(IsGeneralEnclosed(u8"(100px = width = 200px)"));
    CHECK(IsGeneralEnclosed(u8"(100px = width < 200px)"));
    CHECK(IsGeneralEnclosed(u8"(width == 100px)"));
    CHECK(IsGeneralEnclosed(u8"(100px : width : 200px)"));
    CHECK(IsGeneralEnclosed(u8"((width)(height))"));
    CHECK(IsGeneralEnclosed(u8"(100px < 200px)"));
    CHECK(IsGeneralEnclosed(u8"(width <)"));
    CHECK(Roundtrip(u8"(100px < width > 200px)") == "(100px < width > 200px)");
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: schema validation", "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(IsKnownFeature(u8"(width: 100px)"));
    CHECK(IsKnownFeature(u8"(width: 0)"));
    CHECK(IsKnownFeature(u8"(width: calc(10px + 1em))"));
    CHECK(IsKnownFeature(u8"(color: 8)"));
    CHECK(IsKnownFeature(u8"(monochrome)"));
    CHECK(IsKnownFeature(u8"(resolution: 2dppx)"));
    CHECK(IsKnownFeature(u8"(aspect-ratio: 16/9)"));
    CHECK(IsKnownFeature(u8"(pointer: coarse)"));
    CHECK(IsKnownFeature(u8"(scripting: initial-only)"));

    // Wrong value types.
    CHECK(IsGeneralEnclosed(u8"(width: 10)"));
    CHECK(IsGeneralEnclosed(u8"(width: portrait)"));
    CHECK(IsGeneralEnclosed(u8"(color: 1.5)"));
    CHECK(IsGeneralEnclosed(u8"(color: 1px)"));
    CHECK(IsGeneralEnclosed(u8"(resolution: 2)"));
    CHECK(IsGeneralEnclosed(u8"(orientation: round)"));
    CHECK(IsGeneralEnclosed(u8"(orientation: 1)"));
    CHECK(IsGeneralEnclosed(u8"(aspect-ratio: -1)"));

    // Discrete features don't take ranges or min-/max- prefixes.
    CHECK(IsGeneralEnclosed(u8"(orientation > portrait)"));
    CHECK(IsGeneralEnclosed(u8"(min-orientation: portrait)"));
    CHECK(IsGeneralEnclosed(u8"(grid > 0)"));

    // Prefixes need a value.
    CHECK(IsGeneralEnclosed(u8"(min-width)"));

    // Unknown features, and the non-standard ones that aren't supported.
    CHECK(IsGeneralEnclosed(u8"(asdf)"));
    CHECK(IsGeneralEnclosed(u8"(--foo: bar)"));
    CHECK(IsGeneralEnclosed(u8"(-webkit-min-device-pixel-ratio: 2)"));
    CHECK(IsGeneralEnclosed(u8"(-webkit-transform-3d)"));
    CHECK(IsGeneralEnclosed(u8"(prefers-dark-interface)"));

    // A number for a ratio feature becomes `<number> / 1`.
    auto feature = SingleFeature(u8"(aspect-ratio: 2)");
    REQUIRE(feature);
    CHECK(feature->Schema == &mq::Features::AspectRatio());
    CHECK(Is<RatioValue>(*feature->RightComparison->Value));
    CHECK(Roundtrip(u8"(aspect-ratio: 2)") == "(aspect-ratio: 2 / 1)");

    // WPT css/mediaqueries/aspect-ratio-serialization.html.
    CHECK(Roundtrip(u8"(aspect-ratio: 1/3)") == "(aspect-ratio: 1 / 3)");
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: general enclosed", "[CSS][Queries][MediaQueryParser]")
  {
    CHECK(Roundtrip(u8"(asdf)") == "(asdf)");
    CHECK(Roundtrip(u8"(asdf: 1px)") == "(asdf: 1px)");
    CHECK(Roundtrip(u8"foo(bar)") == "foo(bar)");
    CHECK(Roundtrip(u8"screen and foo(bar)") == "screen and foo(bar)");
    CHECK(Roundtrip(u8"(color) and (asdf)") == "(color) and (asdf)");
    CHECK(Roundtrip(u8"not (asdf)") == "not (asdf)");

    auto list = Parse(u8"foo(bar baz)");
    REQUIRE(list.size() == 1);
    REQUIRE(list[0].Condition);
    auto *enclosed = std::get_if<mq::GeneralEnclosed>(&list[0].Condition->Queries[0]);
    REQUIRE(enclosed);
    CHECK(ToNarrow(enclosed->Name) == "foo");
    CHECK(ToNarrow(enclosed->Text) == "bar baz");
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQueryParser: ParseCondition", "[CSS][Queries][MediaQueryParser]")
  {
    auto parseCondition = [](utf8_stringview text) -> Maybe<std::string>
    {
      InputStream inputStream {CSSOMString(text)};
      Tokenizer tokenizer(inputStream);
      std::ignore = tokenizer.PumpTokenizer();
      auto query = mq::MediaQueryParser::ParseCondition(tokenizer.Tokens(), StrictParserContext());
      if (!query)
      {
        return {};
      }
      CSSOMString result;
      mq::Serialise(result, *query);
      return ToNarrow(result);
    };

    CHECK(parseCondition(u8"") == "all");
    CHECK(parseCondition(u8"  ") == "all");
    CHECK(parseCondition(u8"(color)") == "(color)");
    CHECK(parseCondition(u8" (color) and (hover)") == "(color) and (hover)");
    CHECK_FALSE(parseCondition(u8"screen"));
    CHECK_FALSE(parseCondition(u8"(color) and"));
  }

  TEST_CASE("MediaQueryFeatures: AllSchemas", "[CSS][Queries][MediaQueryFeatures]")
  {
    auto schemas = mq::Features::AllSchemas();
    CHECK(schemas.size() == 29);

    auto findSchema = [&](utf8_stringview name) -> const mq::FeatureSchema *
    {
      for (auto *schema : schemas)
      {
        if (schema->Name == CSSOMStringAtom(name))
        {
          return schema;
        }
      }
      return nullptr;
    };

    for (auto *schema : schemas)
    {
      CHECK(findSchema(schema->Name.View()) == schema);
    }

    CHECK(findSchema(u8"width") == &mq::Features::Width());
    CHECK(findSchema(u8"display-mode") == &mq::Features::DisplayMode());
    CHECK_FALSE(findSchema(u8"-webkit-device-pixel-ratio"));
    CHECK_FALSE(findSchema(u8"prefers-dark-interface"));

    auto &width = mq::Features::Width();
    CHECK(width.FeatureType == mq::FeatureSchema::Type::Range);
    CHECK(width.FeatureValueType == mq::FeatureSchema::ValueType::Length);
    CHECK(width.Dependencies == mq::MediaQueryDynamicDependency::Viewport);

    auto &colorScheme = mq::Features::PrefersColorScheme();
    CHECK(colorScheme.FeatureType == mq::FeatureSchema::Type::Discrete);
    CHECK(colorScheme.Dependencies == mq::MediaQueryDynamicDependency::Appearance);
    CHECK(colorScheme.ValueIdentifiers.contains(ValueId::Dark));
    CHECK_FALSE(colorScheme.ValueIdentifiers.contains(ValueId::None));
  }

  TEST_CASE_METHOD(ValueFixture, "MediaQuery: TraverseFeatures", "[CSS][Queries][MediaQuery]")
  {
    auto list = Parse(u8"screen and ((width > 1px) or (asdf)), (hover)");
    List<std::string> names;
    mq::TraverseFeatures(list,
                         [&](const mq::Feature &feature)
                         {
                           names.push_back(feature.Name == CSSOMStringAtom::Null()
                                             ? "<general-enclosed>"
                                             : ToNarrow(feature.Name.View()));
                         });
    CHECK(names == List<std::string> {"width", "<general-enclosed>", "hover"});
  }
}
