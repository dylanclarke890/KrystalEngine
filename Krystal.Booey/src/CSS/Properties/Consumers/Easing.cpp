#include "Krystal.Booey/CSS/Properties/Consumers/Easing.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Easing/EasingFunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/Easing/EasingFunction.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  // MARK: - <steps()>

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedStepsEasingFunction(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <steps-easing-function> = steps( <integer>, <steps-easing-function-position>? )
    // <steps-easing-function-position> = jump-start | jump-end | jump-none | jump-both | start | end
    //
    // with tokens constraints, this is:
    //
    // <steps-easing-function> = steps( <integer [1,∞]>, jump-start )
    //                         | steps( <integer [1,∞]>, jump-end )
    //                         | steps( <integer [1,∞]>, jump-both )
    //                         | steps( <integer [1,∞]>, start )
    //                         | steps( <integer [1,∞]>, end )
    //                         | steps( <integer [2,∞]>, jump-none )
    // https://drafts.csswg.org/css-easing-2/#funcdef-steps

    assert(tokens.Peek().FunctionId() == ValueId::Steps);
    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);

    // Stash args so we can re-parse if we get `jump-none`.
    auto stashedArgs = args;

    auto steps = MetaConsumer<Integer<Range {1, Range::Inf}>>::Consume(args, state);
    if (!steps)
    {
      return {};
    }

    Maybe<StepsEasingParameters> parameters;

    if (ConsumeComma(args))
    {
      switch (args.ConsumeIncludingWhitespace().ValueId())
      {
        case ValueId::JumpStart:
        {
          parameters = {StepsEasingParameters::JumpStart {krys::move(*steps)}};
          break;
        }
        case ValueId::JumpEnd:
        {
          parameters = {StepsEasingParameters::JumpEnd {krys::move(*steps)}};
          break;
        }
        case ValueId::JumpNone:
        {
          // "The first parameter specifies the number of intervals in the function. It must be a
          //  positive integer greater than 0 unless the second parameter is jump-none in which
          //  case it must be a positive integer greater than 1."

          // Re-parse `steps` to account for different type requirement.
          auto stepsJumpNone =
            MetaConsumer<Integer<Range {2, Range::Inf}>>::Consume(stashedArgs, state);
          if (!stepsJumpNone)
          {
            return {};
          }

          parameters = {StepsEasingParameters::JumpNone {krys::move(*stepsJumpNone)}};
          break;
        }

        case ValueId::JumpBoth:
        {
          parameters = {StepsEasingParameters::JumpBoth {krys::move(*steps)}};
          break;
        }
        case ValueId::Start:
        {
          parameters = {StepsEasingParameters::Start {krys::move(*steps)}};
          break;
        }
        case ValueId::End:
        {
          parameters = {StepsEasingParameters::End {krys::move(*steps)}};
          break;
        }
        default:
        {
          return {};
        }
      }
    }
    else
    {
      parameters = {StepsEasingParameters::End {krys::move(*steps)}};
    }

    if (!args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;

    return EasingFunction {StepsEasingFunction {.parameters = krys::move(*parameters)}};
  }

  // MARK: - <linear()>

  KRYS_NODISCARD static Maybe<LinearEasingParameters::Stop::Length>
    ConsumeUnresolvedLinearEasingFunctionStopLength(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <linear-easing-function-stop-length> = <percentage>{0,2}

    auto input = MetaConsumer<Percentage<>>::Consume(args, state);
    if (!input)
      return {};
    auto extra = MetaConsumer<Percentage<>>::Consume(args, state);

    return LinearEasingParameters::Stop::Length {.input = krys::move(*input), .extra = krys::move(extra)};
  }

  KRYS_NODISCARD static Maybe<LinearEasingParameters::Stop>
    ConsumeUnresolvedLinearEasingFunctionStop(TokenRange &args, PropertyParserState &state) noexcept
  {
    // <linear-easing-function-stop> = <number> && <percentage>{0,2}

    auto output = MetaConsumer<Number<>>::Consume(args, state);
    if (!output)
    {
      return {};
    }

    auto input = ConsumeUnresolvedLinearEasingFunctionStopLength(args, state);
    return LinearEasingParameters::Stop {.output = krys::move(*output), .input = krys::move(input)};
  }

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedLinearEasingFunction(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <linear()> = linear( [ <number> && <percentage>{0,2} ]# )
    // https://drafts.csswg.org/css-easing-2/#funcdef-linear

    assert(tokens.Peek().FunctionId() == ValueId::Linear);
    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);

    SmallList<LinearEasingParameters::Stop> stops;

    while (true)
    {
      auto stop = ConsumeUnresolvedLinearEasingFunctionStop(args, state);
      if (!stop)
      {
        break;
      }

      stops.push_back(krys::move(*stop));

      if (!ConsumeComma(args))
      {
        break;
      }
    }

    if (!args.IsAtEnd() || stops.size() < 2)
    {
      return {};
    }

    tokens = rangeCopy;

    return EasingFunction {LinearEasingFunction {.parameters = {.stops = {krys::move(stops)}}}};
  }

  // MARK: - <cubic-bezier()>

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedCubicBezierEasingFunction(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <cubic-bezier()> = cubic-bezier( [ <number [0,1]>, <number> ]#{2} )
    // https://drafts.csswg.org/css-easing-2/#funcdef-cubic-bezier

    assert(tokens.Peek().FunctionId() == ValueId::CubicBezier);
    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);

    auto x1 = MetaConsumer<Number<ClosedUnitRange>>::Consume(args, state);
    if (!x1)
    {
      return {};
    }

    if (!ConsumeComma(args))
    {
      return {};
    }

    auto y1 = MetaConsumer<Number<>>::Consume(args, state);
    if (!y1)
    {
      return {};
    }

    if (!ConsumeComma(args))
    {
      return {};
    }

    auto x2 = MetaConsumer<Number<ClosedUnitRange>>::Consume(args, state);
    if (!x2)
    {
      return {};
    }

    if (!ConsumeComma(args))
    {
      return {};
    }

    auto y2 = MetaConsumer<Number<>>::Consume(args, state);
    if (!y2)
    {
      return {};
    }

    if (!args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;

    return EasingFunction {CubicBezierEasingFunction {
      .parameters = {.value = {
                       CubicBezierEasingParameters::Coordinate {krys::move(*x1), krys::move(*y1)},
                       CubicBezierEasingParameters::Coordinate {krys::move(*x2), krys::move(*y2)},
                     }}}};
  }

  // MARK: - <spring()>

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedSpringEasingFunction(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <spring()> = spring( <number [>0,∞]> <number [>0,∞]> <number [0,∞]> <number> )
    // Non-standard

    assert(tokens.Peek().FunctionId() == ValueId::Spring);

    if (!state.Context.springTimingFunctionEnabled)
    {
      return {};
    }

    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);

    auto mass = MetaConsumer<Number<SpringEasingParameters::Positive>>::Consume(args, state);
    if (!mass)
    {
      return {};
    }

    auto stiffness = MetaConsumer<Number<SpringEasingParameters::Positive>>::Consume(args, state);
    if (!stiffness)
    {
      return {};
    }

    auto damping = MetaConsumer<Number<NonNegative>>::Consume(args, state);
    if (!damping)
    {
      return {};
    }

    auto initialVelocity = MetaConsumer<Number<>>::Consume(args, state);
    if (!initialVelocity)
    {
      return {};
    }

    if (!args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;

    return EasingFunction {SpringEasingFunction {.parameters = {
                                                   .mass = krys::move(*mass),
                                                   .stiffness = krys::move(*stiffness),
                                                   .damping = krys::move(*damping),
                                                   .initialVelocity = krys::move(*initialVelocity),
                                                 }}};
  }

  // MARK: - <easing-function>

  Maybe<EasingFunction> ConsumeUnresolvedEasingFunction(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept
  {
    // <easing-function> = linear | ease | ease-in | ease-out | ease-in-out | step-start | step-end |
    // <linear()> | <cubic-bezier()> | <steps()> NOTE: also includes non-standard <spring()>.
    // https://drafts.csswg.org/css-easing/#typedef-easing-function

    switch (tokens.Peek().ValueId())
    {
      case ValueId::Linear:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {keywords::Linear {}};
      }
      case ValueId::Ease:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {keywords::Ease {}};
      }
      case ValueId::EaseIn:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {keywords::EaseIn {}};
      }
      case ValueId::EaseOut:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {keywords::EaseOut {}};
      }
      case ValueId::EaseInOut:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {keywords::EaseInOut {}};
      }
      case ValueId::StepStart:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {StepsEasingFunction {
          .parameters = {StepsEasingParameters::Start {Integer<Range {1, Range::Inf}> {1}}}}};
      }
      case ValueId::StepEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {StepsEasingFunction {
          .parameters = {StepsEasingParameters::End {Integer<Range {1, Range::Inf}> {1}}}}};
      }
      default:
      {
        break;
      }
    }

    switch (tokens.Peek().FunctionId())
    {
      case ValueId::Linear:
      {
        return ConsumeUnresolvedLinearEasingFunction(tokens, state);
      }
      case ValueId::CubicBezier:
      {
        return ConsumeUnresolvedCubicBezierEasingFunction(tokens, state);
      }
      case ValueId::Steps:
      {
        return ConsumeUnresolvedStepsEasingFunction(tokens, state);
      }
      case ValueId::Spring:
      {
        return ConsumeUnresolvedSpringEasingFunction(tokens, state);
      }
      default:
      {
        break;
      }
    }

    return {};
  }

  RefPtr<Value> ConsumeEasingFunction(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // Avoid allocation of a EasingFunctionValue when the result is a just a value ID.
    switch (tokens.Peek().ValueId())
    {
      case ValueId::Linear:
      case ValueId::Ease:
      case ValueId::EaseIn:
      case ValueId::EaseOut:
      case ValueId::EaseInOut:
      {
        return ConsumeIdent(tokens);
      }
      default:
      {
        break;
      }
    }

    if (auto value = ConsumeUnresolvedEasingFunction(tokens, state))
    {
      return EasingFunctionValue::Create(krys::move(*value));
    }

    return {};
  }
}