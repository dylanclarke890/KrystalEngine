#include "Krystal.Booey/CSS/Properties/Consumers/Easing.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/CSS/ConsumerTestUtils.hpp"
#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"

namespace krys::boo::css::PropertyParserHelpers::tests
{
  using css::tests::ConsumeToText;
  using css::tests::Tokens;

  TEST_CASE("PropertyParserHelpers(ConsumeEasingFunction keywords)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"linear", ConsumeEasingFunction) == "linear");
    CHECK(ConsumeToText(u8"ease", ConsumeEasingFunction) == "ease");
    CHECK(ConsumeToText(u8"ease-in", ConsumeEasingFunction) == "ease-in");
    CHECK(ConsumeToText(u8"ease-out", ConsumeEasingFunction) == "ease-out");
    CHECK(ConsumeToText(u8"ease-in-out", ConsumeEasingFunction) == "ease-in-out");

    // step-start/step-end become steps().
    CHECK(ConsumeToText(u8"step-start", ConsumeEasingFunction) == "steps(1, start)");
    CHECK(ConsumeToText(u8"step-end", ConsumeEasingFunction) == "steps(1)");

    CHECK(ConsumeToText(u8"auto", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"5", ConsumeEasingFunction) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeEasingFunction steps)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"steps(3)", ConsumeEasingFunction) == "steps(3)");
    CHECK(ConsumeToText(u8"steps(3, end)", ConsumeEasingFunction) == "steps(3)");
    CHECK(ConsumeToText(u8"steps(3, jump-end)", ConsumeEasingFunction) == "steps(3)");
    CHECK(ConsumeToText(u8"steps(3, start)", ConsumeEasingFunction) == "steps(3, start)");
    CHECK(ConsumeToText(u8"steps(3, jump-start)", ConsumeEasingFunction) == "steps(3, jump-start)");
    CHECK(ConsumeToText(u8"steps(3, jump-both)", ConsumeEasingFunction) == "steps(3, jump-both)");
    CHECK(ConsumeToText(u8"steps(2, jump-none)", ConsumeEasingFunction) == "steps(2, jump-none)");
    CHECK(ConsumeToText(u8"steps(calc(1 + 2))", ConsumeEasingFunction) == "steps(calc(3))");

    // jump-none needs at least two steps; the others at least one.
    CHECK(ConsumeToText(u8"steps(1, jump-none)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"steps(1, jump-start)", ConsumeEasingFunction) == "steps(1, jump-start)");
    CHECK(ConsumeToText(u8"steps(0)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"steps(1.5)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"steps(3, middle)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"steps(3,)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"steps(3, end, end)", ConsumeEasingFunction) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeEasingFunction cubic-bezier)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"cubic-bezier(0.1, 0.7, 1, 0.1)", ConsumeEasingFunction)
          == "cubic-bezier(0.1, 0.7, 1, 0.1)");
    CHECK(ConsumeToText(u8"cubic-bezier(0, -2, 1, 3)", ConsumeEasingFunction) == "cubic-bezier(0, -2, 1, 3)");

    // The x coordinates must be in [0,1].
    CHECK(ConsumeToText(u8"cubic-bezier(1.1, 0, 0, 0)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"cubic-bezier(0, 0, -0.1, 0)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"cubic-bezier(0, 0, 1)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"cubic-bezier(0 0 1 1)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"cubic-bezier(0, 0, 1, 1, 0)", ConsumeEasingFunction) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeEasingFunction linear)", "[CSS][Properties][Consumers]")
  {
    CHECK(ConsumeToText(u8"linear(0, 1)", ConsumeEasingFunction) == "linear(0, 1)");
    CHECK(ConsumeToText(u8"linear(0, 0.5 25% 75%, 1)", ConsumeEasingFunction) == "linear(0, 0.5 25% 75%, 1)");
    CHECK(ConsumeToText(u8"linear(0 0%, 1 100%)", ConsumeEasingFunction) == "linear(0 0%, 1 100%)");

    // At least two stops, each with a number first.
    CHECK(ConsumeToText(u8"linear(0)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"linear()", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"linear(0, 50% 1)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"linear(0, 1 10% 20% 30%)", ConsumeEasingFunction) == "<null>");
    CHECK(ConsumeToText(u8"linear(0, 1,)", ConsumeEasingFunction) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeEasingFunction spring)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    CHECK(ConsumeToText(u8"spring(1 100 10 0)", ConsumeEasingFunction, context) == "<null>");

    context.springTimingFunctionEnabled = true;
    CHECK(ConsumeToText(u8"spring(1 100 10 0)", ConsumeEasingFunction, context) == "spring(1 100 10 0)");
    CHECK(ConsumeToText(u8"spring(1 100 0 -5)", ConsumeEasingFunction, context) == "spring(1 100 0 -5)");

    // Mass and stiffness must be positive; damping non-negative.
    CHECK(ConsumeToText(u8"spring(0 100 10 0)", ConsumeEasingFunction, context) == "<null>");
    CHECK(ConsumeToText(u8"spring(1 0 10 0)", ConsumeEasingFunction, context) == "<null>");
    CHECK(ConsumeToText(u8"spring(1 100 -1 0)", ConsumeEasingFunction, context) == "<null>");
    CHECK(ConsumeToText(u8"spring(1 100 10)", ConsumeEasingFunction, context) == "<null>");
  }

  TEST_CASE("PropertyParserHelpers(ConsumeUnresolvedEasingFunction)", "[CSS][Properties][Consumers]")
  {
    ParserContext context {ParserMode::HTMLStandard};
    PropertyParserState state {.Context = context};

    Tokens tokens {u8"ease  steps(2, start) cubic-bezier(2, 0, 0, 0)"};
    auto range = tokens.Tokenizer.Tokens();

    auto ease = ConsumeUnresolvedEasingFunction(range, state);
    REQUIRE(ease);
    CHECK(*ease == EasingFunction {keywords::Ease {}});

    auto steps = ConsumeUnresolvedEasingFunction(range, state);
    REQUIRE(steps);
    CHECK(*steps
          == EasingFunction {StepsEasingFunction {
            .Parameters = {StepsEasingParameters::Start {Integer<Range {1, Range::Inf}> {2}}}}});

    // A failed function leaves the range untouched.
    CHECK_FALSE(ConsumeUnresolvedEasingFunction(range, state));
    CHECK(range.Peek().FunctionId() == ValueId::CubicBezier);
  }
}
