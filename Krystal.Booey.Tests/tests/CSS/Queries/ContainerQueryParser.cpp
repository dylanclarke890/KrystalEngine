#include "Krystal.Booey/CSS/Queries/ContainerQueryParser.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ValueTestUtils.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"

namespace krys::boo::css::tests
{
  namespace
  {
    /// @brief Parses `prelude` as the prelude of an `@container` rule, as the sheet parser does: the whole
    /// prelude must be consumed.
    KRYS_NODISCARD Maybe<cq::ContainerQuery> ParseContainerQuery(utf8_stringview prelude)
    {
      InputStream inputStream {CSSOMString(prelude)};
      Tokenizer tokenizer(inputStream);
      std::ignore = tokenizer.PumpTokenizer();
      auto tokens = tokenizer.Tokens();
      tokens.DiscardWhitespace();
      if (tokens.IsAtEnd())
      {
        return {};
      }

      auto query = cq::ContainerQueryParser::ConsumeContainerQuery(tokens, MediaQueryParserContext());
      if (!query)
      {
        return {};
      }

      tokens.DiscardWhitespace();
      if (!tokens.IsAtEnd())
      {
        return {};
      }
      return query;
    }

    enum class ConditionResult
    {
      Invalid,
      Known,
      Unknown
    };

    /// @brief Classifies `condition` as WPT's `test_cq_condition_*` helpers do (with a `name` prefix).
    KRYS_NODISCARD ConditionResult ClassifyCondition(std::u8string_view condition)
    {
      auto query = ParseContainerQuery(std::u8string(u8"name ") + std::u8string(condition));
      if (!query)
      {
        return ConditionResult::Invalid;
      }
      return query->ContainsUnknownFeature == cq::ContainsUnknownFeature::Yes ? ConditionResult::Unknown
                                                                              : ConditionResult::Known;
    }

    /// @brief Serialises the parsed prelude, as `CSSContainerRule.conditionText` does.
    KRYS_NODISCARD std::string ConditionText(utf8_stringview prelude)
    {
      auto query = ParseContainerQuery(prelude);
      REQUIRE(query);
      CSSOMString result;
      cq::Serialise(result, *query);
      return ToNarrow(result);
    }
  }

  // Cases from WPT css/css-conditional/container-queries/at-container-parsing.html.
  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: size feature conditions",
                   "[CSS][Queries][ContainerQueryParser]")
  {
    for (auto condition : {
           u8"(width)",
           u8"(min-width: 0px)",
           u8"(max-width: 0px)",
           u8"(height)",
           u8"(min-height: 0px)",
           u8"(max-height: 0px)",
           u8"(aspect-ratio)",
           u8"(min-aspect-ratio: 1/2)",
           u8"(max-aspect-ratio: 1/2)",
           u8"(orientation: portrait)",
           u8"(inline-size)",
           u8"(min-inline-size: 0px)",
           u8"(max-inline-size: 0px)",
           u8"(block-size)",
           u8"(min-block-size: 0px)",
           u8"(max-block-size: 0px)",
           u8"(width: 100px)",
           u8"((width: 100px))",
           u8"(not (width: 100px))",
           u8"((width: 100px) and (height: 100px))",
           u8"(((width: 40px) or (width: 50px)) and (height: 100px))",
           u8"((width: 100px) and ((height: 40px) or (height: 50px)))",
           u8"(((width: 40px) and (height: 50px)) or (height: 100px))",
           u8"((width: 50px) or ((width: 40px) and (height: 50px)))",
           u8"((width: 100px) and (not (height: 100px)))",
           u8"(width < 100px)",
           u8"(width <= 100px)",
           u8"(width = 100px)",
           u8"(width > 100px)",
           u8"(width >= 100px)",
           u8"(100px < width)",
           u8"(100px <= width)",
           u8"(100px = width)",
           u8"(100px > width)",
           u8"(100px >= width)",
           u8"(100px < width < 200px)",
           u8"(100px < width <= 200px)",
           u8"(100px <= width < 200px)",
           u8"(100px > width > 200px)",
           u8"(100px > width >= 200px)",
           u8"(100px >= width > 200px)",
           u8"(width: calc(10px))",
           u8"(width: calc(10em))",
           u8"(width: calc(10px + 10em))",
           u8"(width < calc(10px + 10em))",
           u8"(width < max(10px, 10em))",
           u8"(calc(10px + 10em) < width)",
           u8"(calc(10px + 10em) < width < max(30px, 30em))",
           u8"(width: 100px) and (height: 100px)",
           u8"(width: 100px) or (height: 100px)",
           u8"not (width: 100px)",
         })
    {
      INFO(ToNarrow(condition));
      CHECK(ClassifyCondition(condition) == ConditionResult::Known);
    }

    for (auto condition : {
           u8"foo(width)",
           u8"size(width)",
           u8"(asdf)",
           u8"(resolution > 100dpi)",
           u8"(resolution: 150dpi)",
           u8"(resolution: calc(2x))",
           u8"(color)",
           u8"(min-color: 1)",
           u8"(color-index >= 1)",
           u8"size(grid)",
           u8"(grid)",
           u8"(width == 100px)",
           u8"(100px == width)",
           u8"(100px = width = 200px)",
           u8"(100px < width > 200px)",
           u8"(100px <= width >= 200px)",
           u8"(100px <= width > 200px)",
           u8"(100px < width >= 200px)",
           u8"(100px : width : 200px)",
         })
    {
      INFO(ToNarrow(condition));
      CHECK(ClassifyCondition(condition) == ConditionResult::Unknown);
    }

    for (auto condition : {
           u8"screen",
           u8"print",
           u8"not print",
           u8"only print",
           u8"screen and (width: 100px)",
           u8"screen or (width: 100px)",
           u8"not screen and (width: 100px)",
           u8"not screen or (width: 100px)",
           u8"foo (width: 100px)",
         })
    {
      INFO(ToNarrow(condition));
      CHECK(ClassifyCondition(condition) == ConditionResult::Invalid);
    }
  }

  // Cases from WPT css/css-conditional/container-queries/at-container-parsing.html.
  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: names", "[CSS][Queries][ContainerQueryParser]")
  {
    CHECK(ParseContainerQuery(u8"name not (width <= 500px)"));
    CHECK(ParseContainerQuery(u8"not (width <= 500px)"));
    CHECK(ParseContainerQuery(u8"--foo"));
    CHECK(ParseContainerQuery(u8"container"));
    CHECK(ParseContainerQuery(u8"auto (width)"));
    CHECK(ParseContainerQuery(u8"normal (width)"));

    CHECK_FALSE(ParseContainerQuery(u8""));
    CHECK_FALSE(ParseContainerQuery(u8"(width),"));
    CHECK_FALSE(ParseContainerQuery(u8",(width)"));
    CHECK_FALSE(ParseContainerQuery(u8"(width),,(height)"));
    CHECK_FALSE(ParseContainerQuery(u8"none (width)"));
    CHECK_FALSE(ParseContainerQuery(u8"and (width)"));
    CHECK_FALSE(ParseContainerQuery(u8"OR (width)"));
    CHECK_FALSE(ParseContainerQuery(u8"inherit (width)"));
    CHECK_FALSE(ParseContainerQuery(u8"\"foo\" (width)"));
    CHECK_FALSE(ParseContainerQuery(u8"foo foo (width)"));

    auto query = ParseContainerQuery(u8"card (inline-size > 30em)");
    REQUIRE(query);
    CHECK(ToNarrow(query->Name.View()) == "card");
    CHECK(query->RequiredAxes == cq::Axis::Inline);

    query = ParseContainerQuery(u8"not (width <= 500px)");
    REQUIRE(query);
    CHECK(query->Name == CSSOMStringAtom::Null());
    CHECK(query->Condition.LogicalOperator == mq::LogicalOperator::Not);

    // A name alone is an empty condition.
    query = ParseContainerQuery(u8"--foo");
    REQUIRE(query);
    CHECK(query->Condition.Queries.empty());
    CHECK(query->RequiredAxes == cq::Axis::None);
    CHECK(query->ContainsUnknownFeature == cq::ContainsUnknownFeature::No);
  }

  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: required axes",
                   "[CSS][Queries][ContainerQueryParser]")
  {
    auto axes = [](utf8_stringview prelude)
    {
      auto query = ParseContainerQuery(prelude);
      REQUIRE(query);
      return query->RequiredAxes;
    };

    CHECK(axes(u8"(width > 1px)") == cq::Axis::Width);
    CHECK(axes(u8"(height > 1px)") == cq::Axis::Height);
    CHECK(axes(u8"(inline-size > 1px)") == cq::Axis::Inline);
    CHECK(axes(u8"(block-size > 1px)") == cq::Axis::Block);
    CHECK(axes(u8"(aspect-ratio > 1/2)") == (cq::Axis::Inline | cq::Axis::Block));
    CHECK(axes(u8"(orientation: portrait)") == (cq::Axis::Inline | cq::Axis::Block));
    CHECK(axes(u8"(width > 1px) or (not (block-size > 1px))") == (cq::Axis::Width | cq::Axis::Block));
    CHECK(axes(u8"style(--foo)") == cq::Axis::None);
    CHECK(axes(u8"(asdf)") == cq::Axis::None);
  }

  // Cases from WPT css/css-conditional/container-queries/at-container-style-parsing.html.
  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: style()", "[CSS][Queries][ContainerQueryParser]")
  {
    for (auto condition : {
           u8"style(--my-prop: foo)",
           u8"style(--my-prop: foo - bar ())",
           u8"style(not ((--foo: calc(10px + 2em)) and ((--foo: url(x)))))",
           u8"style((--foo: bar) or (--bar: 10px))",
           u8"style(--my-prop:)",
           u8"style(--my-prop: )",
           u8"style(--foo: bar !important)",
           u8"style(--foo)",
           u8"style(--my-prop: attr(data-foo))",
         })
    {
      INFO(ToNarrow(condition));
      CHECK(ClassifyCondition(condition) == ConditionResult::Known);
    }

    for (auto condition : {
           u8"style(--foo: bar;)",
           u8"style(style(--foo: bar))",
           u8"style(10px < 10em > 10)",
           u8"style( < 10em)",
           u8"style(10px < 10em !)",
           u8"style(10px ! < 10em)",
           // Only custom properties can be queried.
           u8"style(color: red)",
         })
    {
      INFO(ToNarrow(condition));
      CHECK(ClassifyCondition(condition) == ConditionResult::Unknown);
    }

    auto query = ParseContainerQuery(u8"style(--foo: bar)");
    REQUIRE(query);
    auto *feature = std::get_if<mq::Feature>(&query->Condition.Queries[0]);
    REQUIRE(feature);
    CHECK(feature->Schema == &cq::Features::Style());
    CHECK(feature->FunctionId == ValueId::Style);
    REQUIRE(feature->RightComparison);
    CHECK(feature->RightComparison->Value->IsCustomPropertyValue());
  }

  // FIXME: Range syntax in style queries and lists of container queries (css-conditional-5) aren't supported
  // (WPT at-container-style-parsing.html and at-container-parsing.html).
  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: style() ranges and query lists",
                   "[CSS][Queries][ContainerQueryParser][!shouldfail]")
  {
    CHECK(ClassifyCondition(u8"style(--foo >= --bar)") == ConditionResult::Known);
    CHECK(ClassifyCondition(u8"style(10px < --foo <= 10em)") == ConditionResult::Known);
    CHECK(ParseContainerQuery(u8"(width: 100px), (height: 100px)"));
    CHECK(ParseContainerQuery(u8"container, container2"));
  }

  // Cases from WPT css/css-conditional/container-queries/at-container-serialization.html and
  // at-container-style-serialization.html.
  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: serialisation",
                   "[CSS][Queries][ContainerQueryParser]")
  {
    CHECK(ConditionText(u8"(width = 100px)") == "(width = 100px)");
    CHECK(ConditionText(u8"\\!-name (inline-size > 200px)") == "\\!-name (inline-size > 200px)");
    CHECK(ConditionText(u8"(  wiDTh  )") == "(width)");
    CHECK(ConditionText(u8"(width:100px)") == "(width: 100px)");
    CHECK(ConditionText(u8"(min-width:  100px)") == "(min-width: 100px)");
    CHECK(ConditionText(u8"(   MAX-WIDTH:100px  )") == "(max-width: 100px)");
    CHECK(ConditionText(u8"(width > 100px)") == "(width > 100px)");
    CHECK(ConditionText(u8"(width < 100px)") == "(width < 100px)");
    CHECK(ConditionText(u8"(widTH >= 100px)") == "(width >= 100px)");
    CHECK(ConditionText(u8"(width <= 100px)") == "(width <= 100px)");
    CHECK(ConditionText(u8"(10px < width < 100px)") == "(10px < width < 100px)");
    CHECK(ConditionText(u8"(10px <=  width  <=  100px)") == "(10px <= width <= 100px)");
    CHECK(ConditionText(u8"(100px>WIDTH>10px)") == "(100px > width > 10px)");
    CHECK(ConditionText(u8"(  100px >= width >= 10px  )") == "(100px >= width >= 10px)");
    CHECK(ConditionText(u8"(calc(1em + 1px) >= width >= max(10em, 10px))")
          == "(calc(1em + 1px) >= width >= max(10em, 10px))");

    CHECK(ConditionText(u8"style(        --foo:bar)") == "style(--foo: bar)");
    CHECK(ConditionText(u8"STyle(--foo:    )") == "style(--foo: )");
    CHECK(ConditionText(u8"STyle(--foo:)") == "style(--foo: )");
    CHECK(ConditionText(u8"STyle(--foo)") == "style(--foo)");
    CHECK(ConditionText(u8"style (--foo: bar)") == "style (--foo: bar)");

    CHECK(ConditionText(u8"card not (width > 1px)") == "card not (width > 1px)");
    CHECK(ConditionText(u8"(width > 1px) AND (asdf)") == "(width > 1px) and (asdf)");
    CHECK(ConditionText(u8"foo(bar)") == "foo(bar)");
  }

  // FIXME: The tokenizer doesn't keep the original text of numbers or runs of whitespace, so custom property
  // values don't round-trip (see Token.cpp). The `or` case fails upstream too.
  TEST_CASE_METHOD(ValueFixture, "ContainerQueryParser: style() serialisation of original text",
                   "[CSS][Queries][ContainerQueryParser][!shouldfail]")
  {
    CHECK(ConditionText(u8"style(--foo: bar   baz)") == "style(--foo: bar   baz)");
    CHECK(ConditionText(u8"style(--foo:2.100  )") == "style(--foo: 2.100)");
    CHECK(ConditionText(u8" style(  ( --FOO: BAR) OR ( prop: val  ) )")
          == "style((--FOO: BAR) or ( prop: val  ))");
  }

  TEST_CASE("ContainerQueryFeatures: AllSchemas", "[CSS][Queries][ContainerQueryFeatures]")
  {
    auto schemas = cq::Features::AllSchemas();
    CHECK(schemas
          == List<const mq::FeatureSchema *> {&cq::Features::Width(), &cq::Features::Height(),
                                              &cq::Features::InlineSize(), &cq::Features::BlockSize(),
                                              &cq::Features::AspectRatio(), &cq::Features::Orientation()});
    CHECK(ToNarrow(cq::Features::Style().Name.View()) == "style");
    CHECK(cq::Features::Style().FeatureValueType == mq::FeatureSchema::ValueType::CustomProperty);
  }
}
