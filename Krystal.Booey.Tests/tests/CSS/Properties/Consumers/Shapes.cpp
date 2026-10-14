#include "Krystal.Booey/CSS/Properties/Consumers/Shapes.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;

  namespace
  {
    KRYS_NODISCARD RefPtr<Value> BasicShape(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      return ConsumeBasicShape(tokens, state, PathParsingOption::None);
    }
  }

  TEST_CASE("PropertyParserHelpers(ConsumeBasicShape)", "[CSS][Properties][Consumers]")
  {
    SECTION("circle() and ellipse()")
    {
      CHECK(ConsumeToText(u8"circle()", BasicShape) == "circle()");
      CHECK(ConsumeToText(u8"circle(closest-side)", BasicShape) == "circle()");
      CHECK(ConsumeToText(u8"circle(10px at 0 0)", BasicShape) == "circle(10px at 0px 0px)");
      CHECK(ConsumeToText(u8"ellipse(1px 2px)", BasicShape) == "ellipse(1px 2px)");
      CHECK(ConsumeToText(u8"ellipse(farthest-side closest-side at left top)", BasicShape)
            == "ellipse(farthest-side closest-side at left top)");
      CHECK(ConsumeToText(u8"circle(-1px)", BasicShape) == "<null>");
    }

    SECTION("inset(), rect() and xywh()")
    {
      CHECK(ConsumeToText(u8"inset(1px)", BasicShape) == "inset(1px)");
      CHECK(ConsumeToText(u8"inset(1px 2px round 3px)", BasicShape) == "inset(1px 2px round 3px)");
      CHECK(ConsumeToText(u8"inset(1px round 2px / 3px)", BasicShape) == "inset(1px round 2px / 3px)");
      CHECK(ConsumeToText(u8"rect(1px 2px 3px 4px)", BasicShape) == "rect(1px 2px 3px 4px)");
      CHECK(ConsumeToText(u8"rect(auto auto auto auto round 1px)", BasicShape)
            == "rect(auto auto auto auto round 1px)");
      CHECK(ConsumeToText(u8"xywh(1px 2px 3px 4px)", BasicShape) == "xywh(1px 2px 3px 4px)");
      CHECK(ConsumeToText(u8"xywh(0 0 100% 100% round 5px)", BasicShape)
            == "xywh(0px 0px 100% 100% round 5px)");
      CHECK(ConsumeToText(u8"xywh(1px 2px -3px 4px)", BasicShape) == "<null>");
    }

    SECTION("polygon() and path()")
    {
      CHECK(ConsumeToText(u8"polygon(0 0, 1px 1px, 2px 0)", BasicShape)
            == "polygon(0px 0px, 1px 1px, 2px 0px)");
      CHECK(ConsumeToText(u8"polygon(evenodd, 0 0, 1px 1px, 2px 0)", BasicShape)
            == "polygon(evenodd, 0px 0px, 1px 1px, 2px 0px)");
      CHECK(ConsumeToText(u8"path(\"M 0 0 L 1 1\")", BasicShape) == "path(\"M 0 0 L 1 1\")");
      CHECK(ConsumeToText(u8"path(evenodd, \"M 0 0 L 1 1\")", BasicShape)
            == "path(evenodd, \"M 0 0 L 1 1\")");

      auto rejectFillRule = [](TokenRange &tokens, PropertyParserState &state)
      {
        return ConsumeBasicShape(tokens, state, PathParsingOption::RejectPathFillRule);
      };
      auto rejectPath = [](TokenRange &tokens, PropertyParserState &state)
      {
        return ConsumeBasicShape(tokens, state, PathParsingOption::RejectPath);
      };

      CHECK(ConsumeToText(u8"path(evenodd, \"M 0 0 L 1 1\")", rejectFillRule) == "<null>");
      CHECK(ConsumeToText(u8"path(\"M 0 0 L 1 1\")", rejectFillRule) == "path(\"M 0 0 L 1 1\")");
      CHECK(ConsumeToText(u8"path(\"M 0 0 L 1 1\")", rejectPath) == "<null>");
    }

    SECTION("shape()")
    {
      ParserContext context {ParserMode::HTMLStandard};
      CHECK(ConsumeToText(u8"shape(from 0 0, line to 10px 10px)", BasicShape, context) == "<null>");

      context.cssShapeFunctionEnabled = true;
      CHECK(ConsumeToText(u8"shape(from 0 0, line to 10px 10px)", BasicShape, context)
            == "shape(from 0px 0px, line to 10px 10px)");
      CHECK(ConsumeToText(u8"shape(nonzero from 0 0, hline by 5px, close)", BasicShape, context)
            == "shape(from 0px 0px, hline by 5px, close)");
      CHECK(ConsumeToText(u8"shape(from 0 0, curve to 1px 1px with 2px 2px)", BasicShape, context)
            == "shape(from 0px 0px, curve to 1px 1px with 2px 2px)");
    }
  }

  TEST_CASE("PropertyParserHelpers(ConsumeShapeOutside)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard, url::URL(u8"https://example.com/dir/")};

    CHECK(ConsumeToText(u8"none", ConsumeShapeOutside) == "none");
    CHECK(ConsumeToText(u8"circle()", ConsumeShapeOutside) == "circle()");
    CHECK(ConsumeToText(u8"margin-box", ConsumeShapeOutside) == "margin-box");
    CHECK(ConsumeToText(u8"content-box circle()", ConsumeShapeOutside) == "circle() content-box");
    CHECK(ConsumeToText(u8"url(a.png)", ConsumeShapeOutside, context) == "url(\"a.png\")");
    CHECK(ConsumeToText(u8"linear-gradient(red, blue)", ConsumeShapeOutside) == "linear-gradient(red, blue)");

    // `margin-box` is the default reference box, so it's omitted.
    CHECK(ConsumeToText(u8"circle() margin-box", ConsumeShapeOutside) == "circle()");

    CHECK(ConsumeToText(u8"fill-box", ConsumeShapeOutside) == "<null>");
  }
}
