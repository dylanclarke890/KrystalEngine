#include "Krystal.HTML/CSS/Properties/Consumers/Easing.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/IntegerDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/Easing/CSSEasingFunctionValue.hpp"
#include "Krystal.HTML/CSS/Values/Easing/EasingFunction.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  // MARK: - <steps()>

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedStepsEasingFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
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

    assert(tokens.Peek().FunctionId() == CSSValueId::Steps);
    auto rangeCopy = tokens;
    auto args = ConsumeFunction(rangeCopy);

    // Stash args so we can re-parse if we get `jump-none`.
    auto stashedArgs = args;

    auto steps = MetaConsumer<Integer<CSSRange {1, CSSRange::Inf}>>::Consume(args, state);
    if (!steps)
    {
      return {};
    }

    Maybe<StepsEasingParameters> parameters;

    if (ConsumeComma(args))
    {
      switch (args.ConsumeIncludingWhitespace().ValueId())
      {
        case CSSValueId::JumpStart:
        {
          parameters = {StepsEasingParameters::JumpStart {Krys::Move(*steps)}};
          break;
        }
        case CSSValueId::JumpEnd:
        {
          parameters = {StepsEasingParameters::JumpEnd {Krys::Move(*steps)}};
          break;
        }
        case CSSValueId::JumpNone:
        {
          // "The first parameter specifies the number of intervals in the function. It must be a
          //  positive integer greater than 0 unless the second parameter is jump-none in which
          //  case it must be a positive integer greater than 1."

          // Re-parse `steps` to account for different type requirement.
          auto stepsJumpNone =
            MetaConsumer<Integer<CSSRange {2, CSSRange::Inf}>>::Consume(stashedArgs, state);
          if (!stepsJumpNone)
          {
            return {};
          }

          parameters = {StepsEasingParameters::JumpNone {Krys::Move(*stepsJumpNone)}};
          break;
        }

        case CSSValueId::JumpBoth:
        {
          parameters = {StepsEasingParameters::JumpBoth {Krys::Move(*steps)}};
          break;
        }
        case CSSValueId::Start:
        {
          parameters = {StepsEasingParameters::Start {Krys::Move(*steps)}};
          break;
        }
        case CSSValueId::End:
        {
          parameters = {StepsEasingParameters::End {Krys::Move(*steps)}};
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
      parameters = {StepsEasingParameters::End {Krys::Move(*steps)}};
    }

    if (!args.IsAtEnd())
    {
      return {};
    }

    tokens = rangeCopy;

    return EasingFunction {StepsEasingFunction {.parameters = Krys::Move(*parameters)}};
  }

  // MARK: - <linear()>

  KRYS_NODISCARD static Maybe<LinearEasingParameters::Stop::Length>
    ConsumeUnresolvedLinearEasingFunctionStopLength(CSSTokenRange &args,
                                                    CSSPropertyParserState &state) noexcept
  {
    // <linear-easing-function-stop-length> = <percentage>{0,2}

    auto input = MetaConsumer<Percentage<>>::Consume(args, state);
    if (!input)
      return {};
    auto extra = MetaConsumer<Percentage<>>::Consume(args, state);

    return LinearEasingParameters::Stop::Length {.input = Krys::Move(*input), .extra = Krys::Move(extra)};
  }

  KRYS_NODISCARD static Maybe<LinearEasingParameters::Stop>
    ConsumeUnresolvedLinearEasingFunctionStop(CSSTokenRange &args, CSSPropertyParserState &state) noexcept
  {
    // <linear-easing-function-stop> = <number> && <percentage>{0,2}

    auto output = MetaConsumer<Number<>>::Consume(args, state);
    if (!output)
    {
      return {};
    }

    auto input = ConsumeUnresolvedLinearEasingFunctionStopLength(args, state);
    return LinearEasingParameters::Stop {.output = Krys::Move(*output), .input = Krys::Move(input)};
  }

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedLinearEasingFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <linear()> = linear( [ <number> && <percentage>{0,2} ]# )
    // https://drafts.csswg.org/css-easing-2/#funcdef-linear

    assert(tokens.Peek().FunctionId() == CSSValueId::Linear);
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

      stops.push_back(Krys::Move(*stop));

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

    return EasingFunction {LinearEasingFunction {.parameters = {.stops = {Krys::Move(stops)}}}};
  }

  // MARK: - <cubic-bezier()>

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedCubicBezierEasingFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <cubic-bezier()> = cubic-bezier( [ <number [0,1]>, <number> ]#{2} )
    // https://drafts.csswg.org/css-easing-2/#funcdef-cubic-bezier

    assert(tokens.Peek().FunctionId() == CSSValueId::CubicBezier);
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
                       CubicBezierEasingParameters::Coordinate {Krys::Move(*x1), Krys::Move(*y1)},
                       CubicBezierEasingParameters::Coordinate {Krys::Move(*x2), Krys::Move(*y2)},
                     }}}};
  }

  // MARK: - <spring()>

  KRYS_NODISCARD static Maybe<EasingFunction>
    ConsumeUnresolvedSpringEasingFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <spring()> = spring( <number [>0,∞]> <number [>0,∞]> <number [0,∞]> <number> )
    // Non-standard

    assert(tokens.Peek().FunctionId() == CSSValueId::Spring);

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
                                                   .mass = Krys::Move(*mass),
                                                   .stiffness = Krys::Move(*stiffness),
                                                   .damping = Krys::Move(*damping),
                                                   .initialVelocity = Krys::Move(*initialVelocity),
                                                 }}};
  }

  // MARK: - <easing-function>

  Maybe<EasingFunction> ConsumeUnresolvedEasingFunction(CSSTokenRange &tokens,
                                                        CSSPropertyParserState &state) noexcept
  {
    // <easing-function> = linear | ease | ease-in | ease-out | ease-in-out | step-start | step-end |
    // <linear()> | <cubic-bezier()> | <steps()> NOTE: also includes non-standard <spring()>.
    // https://drafts.csswg.org/css-easing/#typedef-easing-function

    switch (tokens.Peek().ValueId())
    {
      case CSSValueId::Linear:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {Keywords::Linear {}};
      }
      case CSSValueId::Ease:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {Keywords::Ease {}};
      }
      case CSSValueId::EaseIn:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {Keywords::EaseIn {}};
      }
      case CSSValueId::EaseOut:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {Keywords::EaseOut {}};
      }
      case CSSValueId::EaseInOut:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {Keywords::EaseInOut {}};
      }
      case CSSValueId::StepStart:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {StepsEasingFunction {
          .parameters = {StepsEasingParameters::Start {Integer<CSSRange {1, CSSRange::Inf}> {1}}}}};
      }
      case CSSValueId::StepEnd:
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        return EasingFunction {StepsEasingFunction {
          .parameters = {StepsEasingParameters::End {Integer<CSSRange {1, CSSRange::Inf}> {1}}}}};
      }
      default:
      {
        break;
      }
    }

    switch (tokens.Peek().FunctionId())
    {
      case CSSValueId::Linear:
      {
        return ConsumeUnresolvedLinearEasingFunction(tokens, state);
      }
      case CSSValueId::CubicBezier:
      {
        return ConsumeUnresolvedCubicBezierEasingFunction(tokens, state);
      }
      case CSSValueId::Steps:
      {
        return ConsumeUnresolvedStepsEasingFunction(tokens, state);
      }
      case CSSValueId::Spring:
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

  RefPtr<CSSValue> ConsumeEasingFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // Avoid allocation of a CSSEasingFunctionValue when the result is a just a value ID.
    switch (tokens.Peek().ValueId())
    {
      case CSSValueId::Linear:
      case CSSValueId::Ease:
      case CSSValueId::EaseIn:
      case CSSValueId::EaseOut:
      case CSSValueId::EaseInOut:
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
      return CSSEasingFunctionValue::Create(Krys::Move(*value));
    }

    return {};
  }
}