#include "Krystal.Booey/CSS/Calc/TreeParser.hpp"
#include "Krystal.Booey/CSS/Calc/Operator.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Calc/Tree.hpp"
#include "Krystal.Booey/CSS/Calc/TreeSimplification.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserIdioms.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRangeGuard.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/Style/AnchorSizeDimension.hpp"
#include <limits>
#include <numbers>

namespace krys::boo::css::calc
{
  // MARK: - Constants

  constexpr static int MaxExpressionDepth = 100;

  KRYS_NODISCARD static Maybe<Number> LookupConstantNumber(ValueId symbol) noexcept
  {
    switch (symbol)
    {
      case ValueId::E:                return Number {.Value = std::numbers::e};
      case ValueId::Pi:               return Number {.Value = std::numbers::pi};
      case ValueId::Infinity:         return Number {.Value = std::numeric_limits<double>::infinity()};
      case ValueId::NegativeInfinity: return Number {.Value = -1 * std::numeric_limits<double>::infinity()};
      case ValueId::NaN:              return Number {.Value = std::numeric_limits<double>::quiet_NaN()};
      default:                        return null;
    }
  }

  // MARK: - Parser State

  namespace
  {
    enum class ParseStatus
    {
      Ok,
      TooDeep
    };

    struct ParserState
    {
      /// @brief css::PropertyParserState used to initiate the parse.
      css::PropertyParserState &PropertyParserState;

      /// @brief ParserOptions used to initiate the parse.
      const calc::ParserOptions &ParserOptions;

      /// @brief SimplificationOptions used to initiate the parse, if provided.
      const calc::SimplificationOptions *SimplificationOptions {nullptr};

      /// @brief Tracks whether the parse tree contains any non-canonical dimension units that require
      /// conversion data (e.g. em, vh, etc.).
      bool RequiresConversionData = false;
    };

    struct TypedChild
    {
      calc::Child Child;
      calc::Type Type;
    };
  }

  KRYS_NODISCARD static ParseStatus CheckDepth(int depth) noexcept
  {
    if (depth > MaxExpressionDepth)
    {
      return ParseStatus::TooDeep;
    }

    return ParseStatus::Ok;
  }

  // MARK: - Parser

  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcFunction(TokenRange &, ValueId functionId, int depth,
                                                            ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcSum(TokenRange &, int depth, ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcProduct(TokenRange &, int depth, ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcValue(TokenRange &, int depth, ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcKeyword(const Token &, ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcNumber(const Token &, ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcPercentage(const Token &, ParserState &) noexcept;
  KRYS_NODISCARD static Maybe<TypedChild> ParseCalcDimension(const Token &, ParserState &) noexcept;

  Maybe<Tree> ParseAndSimplify(TokenRange &range, css::PropertyParserState &propertyParserState,
                               const ParserOptions &parserOptions,
                               const SimplificationOptions &simplificationOptions) noexcept
  {
    auto function = range.Peek().FunctionId();
    if (!IsCalcFunction(function))
    {
      return null;
    }

    auto tokens = PropertyParserHelpers::ConsumeFunction(range);

    // -- Parsing --

    ParserState state {
      .PropertyParserState = propertyParserState,
      .ParserOptions = parserOptions,
      .SimplificationOptions = &simplificationOptions,
    };

    auto root = ParseCalcFunction(tokens, function, 0, state);

    if (!root || !tokens.IsAtEnd())
    {
      return null;
    }

    // -- Type Checking --

    if (!root->Type.Matches(parserOptions.Category))
    {
      return null;
    }

    return Tree {
      .Root = krys::move(root->Child),
      .Type = root->Type,
      .Stage = Stage::Specified,
      .RequiresConversionData = state.RequiresConversionData,
    };
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> SimplifyOrCreateChild(Op &&op, Type outputType,
                                                                ParserState &state) noexcept
  {
    if (auto *simplificationOptions = state.SimplificationOptions)
    {
      if (auto replacement = Simplify(op, *simplificationOptions))
      {
        return TypedChild {krys::move(*replacement), outputType};
      }
    }

    return TypedChild {CreateChild(krys::move(op), outputType), outputType};
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> ConsumeZeroArguments(TokenRange &tokens, int,
                                                               ParserState &) noexcept
  {
    if (!tokens.IsAtEnd())
    {
      return null;
    }

    auto child = Op {};
    auto type = GetType(child);

    return TypedChild {CreateChild(krys::move(child)), type};
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> ConsumeExactlyOneArgument(TokenRange &tokens, int depth,
                                                                    ParserState &state) noexcept
  {
    auto sum = ParseCalcSum(tokens, depth, state);
    if (!sum)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    if (!ValidateType<Op::Input>(sum->Type))
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(sum->Type);
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(Op {krys::move(sum->Child)}, *outputType, state);
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> ConsumeOneOrMoreArguments(TokenRange &tokens, int depth,
                                                                    ParserState &state) noexcept
  {
    Maybe<Type> mergedType;
    List<Child> children;

    bool requireComma = false;
    uint32 argumentCount = 0;

    while (!tokens.IsAtEnd())
    {
      tokens.DiscardWhitespace();
      if (requireComma && !PropertyParserHelpers::ConsumeComma(tokens))
      {
        return null;
      }

      auto sum = ParseCalcSum(tokens, depth, state);
      if (!sum)
      {
        return null;
      }

      if (!ValidateType<Op::Input>(sum->Type))
      {
        return null;
      }

      if (!mergedType)
      {
        mergedType = sum->Type;
      }
      else
      {
        auto mergeResult = MergeTypes<Op::Merge>(*mergedType, sum->Type);
        if (!mergeResult)
        {
          return null;
        }

        mergedType = *mergeResult;
      }

      ++argumentCount;
      children.push_back(krys::move(sum->Child));
      requireComma = true;
    }

    if (argumentCount < 1)
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(*mergedType);
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(Op {krys::move(children)}, *outputType, state);
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> ConsumeExactlyTwoArguments(TokenRange &tokens, int depth,
                                                                     ParserState &state) noexcept
  {
    auto sumA = ParseCalcSum(tokens, depth, state);
    if (!sumA)
    {
      return null;
    }

    if (!ValidateType<Op::Input>(sumA->Type))
    {
      return null;
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto sumB = ParseCalcSum(tokens, depth, state);
    if (!sumB)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    if (!ValidateType<Op::Input>(sumB->Type))
    {
      return null;
    }

    auto mergedType = MergeTypes<Op::Merge>(sumA->Type, sumB->Type);
    if (!mergedType)
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(*mergedType);
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(Op {krys::move(sumA->Child), krys::move(sumB->Child)}, *outputType, state);
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> ConsumeOneOrTwoArguments(TokenRange &tokens, int depth,
                                                                   ParserState &state) noexcept
  {
    auto sumA = ParseCalcSum(tokens, depth, state);
    if (!sumA)
    {
      return null;
    }

    if (!ValidateType<Op::Input>(sumA->Type))
    {
      return null;
    }

    if (tokens.IsAtEnd())
    {
      auto outputType = TransformType<Op::Output>(sumA->Type);
      if (!outputType)
      {
        return null;
      }

      return SimplifyOrCreateChild(Op {krys::move(sumA->Child), null}, *outputType, state);
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto sumB = ParseCalcSum(tokens, depth, state);
    if (!sumB)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    if (!ValidateType<Op::Input>(sumB->Type))
    {
      return null;
    }

    auto mergedType = MergeTypes<Op::Merge>(sumA->Type, sumB->Type);
    if (!mergedType)
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(*mergedType);
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(Op {krys::move(sumA->Child), krys::move(sumB->Child)}, *outputType, state);
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeClamp(TokenRange &tokens, int depth,
                                                       ParserState &state) noexcept
  {
    // <clamp()> = clamp( [ <calc-sum> | none ], <calc-sum>, [ <calc-sum> | none ] )

    using Op = Clamp;

    struct TypedChildOrNone
    {
      calc::ChildOrNone Child;
      calc::Type Type;
    };

    auto parseCalcSumOrNone = [](TokenRange &tokens, int depth, ParserState &state) -> Maybe<TypedChildOrNone>
    {
      if (tokens.Peek().ValueId() == ValueId::None)
      {
        tokens.Discard();
        return TypedChildOrNone {ChildOrNone {keywords::None {}}, Type {}};
      }

      auto sum = ParseCalcSum(tokens, depth, state);
      if (!sum)
      {
        return null;
      }

      return TypedChildOrNone {ChildOrNone {krys::move(sum->Child)}, sum->Type};
    };

    auto min = parseCalcSumOrNone(tokens, depth, state);
    if (!min)
    {
      return null;
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto val = ParseCalcSum(tokens, depth, state);
    if (!val)
    {
      return null;
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto max = parseCalcSumOrNone(tokens, depth, state);
    if (!max)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    auto computeType = [&] -> Maybe<Type>
    {
      bool minIsNone = HoldsAlternative<keywords::None>(min->Child.Value);
      bool maxIsNone = HoldsAlternative<keywords::None>(max->Child.Value);

      if (minIsNone && maxIsNone)
      {
        return val->Type;
      }

      if (minIsNone)
      {
        return MergeTypes<MergePolicy::Consistent>(val->Type, max->Type);
      }

      if (maxIsNone)
      {
        return MergeTypes<Op::Merge>(min->Type, val->Type);
      }

      auto minAndValType = MergeTypes<Op::Merge>(min->Type, val->Type);
      if (!minAndValType)
      {
        return null;
      }

      return MergeTypes<Op::Merge>(*minAndValType, max->Type);
    };

    auto outputType = computeType();
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(Op {krys::move(min->Child), krys::move(val->Child), krys::move(max->Child)},
                                 *outputType, state);
  }

  template <typename Op>
  KRYS_NODISCARD static Maybe<TypedChild> ConsumeRoundArguments(TokenRange &tokens, int depth,
                                                                ParserState &state) noexcept
  {
    auto sumA = ParseCalcSum(tokens, depth, state);
    if (!sumA)
    {
      return null;
    }

    if (tokens.IsAtEnd())
    {
      if (!ValidateType<AllowedTypes::Number>(sumA->Type))
      {
        return null;
      }

      auto outputType = TransformType<Op::Output>(sumA->Type);
      if (!outputType)
      {
        return null;
      }

      return SimplifyOrCreateChild(Op {krys::move(sumA->Child), null}, *outputType, state);
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto sumB = ParseCalcSum(tokens, depth, state);
    if (!sumB)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    auto mergedType = MergeTypes<Op::Merge>(sumA->Type, sumB->Type);
    if (!mergedType)
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(*mergedType);
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(Op {krys::move(sumA->Child), krys::move(sumB->Child)}, *outputType, state);
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeRound(TokenRange &tokens, int depth,
                                                       ParserState &state) noexcept
  {
    // <round()> = round( <rounding-strategy>?, <calc-sum>, <calc-sum>? )

    auto roundingStrategy =
      PropertyParserHelpers::ConsumeIdentRaw<ValueId::Nearest, ValueId::ToZero, ValueId::Up, ValueId::Down>(
        tokens);
    if (!roundingStrategy)
    {
      return ConsumeRoundArguments<RoundNearest>(tokens, depth, state);
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    switch (*roundingStrategy)
    {
      case ValueId::Nearest: return ConsumeRoundArguments<RoundNearest>(tokens, depth, state);
      case ValueId::ToZero:  return ConsumeRoundArguments<RoundToZero>(tokens, depth, state);
      case ValueId::Up:      return ConsumeRoundArguments<RoundUp>(tokens, depth, state);
      case ValueId::Down:    return ConsumeRoundArguments<RoundDown>(tokens, depth, state);
      default:               break;
    }

    return null;
  }

  KRYS_NODISCARD static Maybe<Random::SharingFixed>
    ConsumeOptionalRandomSharingFixed(TokenRange &tokens, ParserState &state) noexcept
  {
    // <random-value-sharing-fixed> = fixed <number [0,1]>

    krys_debug_assert(tokens.Peek().ValueId() == ValueId::Fixed);

    TokenRangeGuard guard {tokens};

    tokens.Discard();
    tokens.DiscardWhitespace();

    // Use a non-property parsing state for the fixed number value to disconnect it from the current parse.
    // FIXME: Add a mechanism to pass along the depth count when doing this so that we can limit stack usage.
    auto numberParsingState = css::PropertyParserState {
      .Context = state.PropertyParserState.Context,
      .Pool = state.PropertyParserState.Pool,
    };
    auto number = MetaConsumer<css::Number<ClosedUnitRange>>::Consume(tokens, numberParsingState);
    if (!number)
    {
      return null;
    }

    guard.Commit();

    return Random::SharingFixed {.Value = krys::move(*number)};
  }

  KRYS_NODISCARD static Random::SharingOptions::Auto CreateRandomSharingAuto(ParserState &state) noexcept
  {
    return {
      .Property = state.PropertyParserState.CurrentProperty,
      .Index = state.PropertyParserState.CSSRandomFunctionCount,
    };
  }

  KRYS_NODISCARD static Maybe<Random::SharingOptions>
    ConsumeOptionalRandomSharingOptions(TokenRange &tokens, ParserState &state) noexcept
  {
    // <random-value-sharing-options> = [ [ auto | <dashed-ident> ] || element-shared ]

    Maybe<Variant<Random::SharingOptions::Auto, CSSOMStringAtom>> identifier;
    Maybe<keywords::ElementShared> elementShared;

    TokenRangeGuard guard {tokens};

    auto consumeIdentifier = [&] -> bool
    {
      if (identifier)
      {
        return false;
      }

      if (tokens.Peek().ValueId() == ValueId::Auto)
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        identifier = CreateRandomSharingAuto(state);
        return true;
      }

      if (tokens.Peek().Type() == TokenType::Ident && IsValidCustomIdentifier(tokens.Peek().ValueId())
          && tokens.Peek().IdentCodePoints().starts_with(u8"--"))
      {
        identifier = CSSOMStringAtom {tokens.ConsumeIncludingWhitespace().IdentCodePoints()};
        return true;
      }

      return false;
    };

    auto consumeElementShared = [&] -> bool
    {
      if (elementShared)
      {
        return false;
      }

      if (tokens.Peek().ValueId() == ValueId::ElementShared)
      {
        tokens.Discard();
        tokens.DiscardWhitespace();
        elementShared = keywords::ElementShared {};
        return true;
      }

      return false;
    };

    for (uint32 i = 0; i < 2; ++i)
    {
      if (consumeIdentifier() || consumeElementShared())
      {
        continue;
      }

      break;
    }

    if (!identifier && !elementShared)
    {
      return null;
    }

    guard.Commit();

    return Random::SharingOptions {
      .Identifier = identifier.value_or(CreateRandomSharingAuto(state)),
      .ElementShared = elementShared,
    };
  }

  KRYS_NODISCARD static Maybe<Random::SharingType> ConsumeOptionalRandomSharing(TokenRange &tokens,
                                                                                ParserState &state) noexcept
  {
    // <random-value-sharing> = [ [ auto | <dashed-ident> ] || element-shared ] | fixed <number [0,1]>

    if (tokens.Peek().ValueId() == ValueId::Fixed)
    {
      if (auto fixed = ConsumeOptionalRandomSharingFixed(tokens, state))
      {
        return Random::SharingType {krys::move(*fixed)};
      }

      return null;
    }

    if (auto options = ConsumeOptionalRandomSharingOptions(tokens, state))
    {
      return Random::SharingType {krys::move(*options)};
    }

    return null;
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeRandom(TokenRange &tokens, int depth,
                                                        ParserState &state) noexcept
  {
    // <random()> = random( <random-value-sharing>? , <calc-sum>, <calc-sum>, <calc-sum>? )

    if (!state.PropertyParserState.Context.cssRandomFunctionEnabled)
    {
      return null;
    }

    if (state.PropertyParserState.CurrentRule != RuleType::Style
        && state.PropertyParserState.CurrentRule != RuleType::Keyframe)
    {
      return null;
    }

    if (state.PropertyParserState.CurrentProperty == PropertyId::Invalid)
    {
      return null;
    }

    // FIXME: Add support for custom properties by including the custom property name in
    // css::PropertyParserState for registered properties.
    if (state.PropertyParserState.CurrentProperty == PropertyId::Custom)
    {
      return null;
    }

    using Op = Random;

    Maybe<Random::SharingType> sharing;
    if (auto optionalSharing = ConsumeOptionalRandomSharing(tokens, state))
    {
      if (!PropertyParserHelpers::ConsumeComma(tokens))
      {
        return null;
      }

      sharing = krys::move(optionalSharing);
    }
    else
    {
      sharing = Random::SharingOptions {
        .Identifier = CreateRandomSharingAuto(state),
        .ElementShared = {},
      };
    }

    // Increment the random function count early, but after processing the the sharing production to
    // ensure that any nested random() functions in the <calc-sum> productions have an incremented value.
    ++state.PropertyParserState.CSSRandomFunctionCount;

    auto min = ParseCalcSum(tokens, depth, state);
    if (!min)
    {
      return null;
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto max = ParseCalcSum(tokens, depth, state);
    if (!max)
    {
      return null;
    }

    if (tokens.IsAtEnd())
    {
      // - Validate arguments

      if (!ValidateType<Op::Input>(min->Type))
      {
        return null;
      }

      if (!ValidateType<Op::Input>(max->Type))
      {
        return null;
      }

      // - Merge arguments

      auto mergedType = MergeTypes<Op::Merge>(min->Type, max->Type);
      if (!mergedType)
      {
        return null;
      }

      auto outputType = TransformType<Op::Output>(*mergedType);
      if (!outputType)
      {
        return null;
      }

      state.RequiresConversionData = true;

      return SimplifyOrCreateChild(
        Op {krys::move(*sharing), krys::move(min->Child), krys::move(max->Child), null}, *outputType, state);
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto step = ParseCalcSum(tokens, depth, state);
    if (!step)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    // - Validate arguments

    if (!ValidateType<Op::Input>(step->Type))
    {
      return null;
    }

    // - Merge arguments

    auto mergedType = MergeTypes<Op::Merge>(min->Type, max->Type);
    if (!mergedType)
    {
      return null;
    }

    mergedType = MergeTypes<Op::Merge>(*mergedType, step->Type);
    if (!mergedType)
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(*mergedType);
    if (!outputType)
    {
      return null;
    }

    state.RequiresConversionData = true;

    return SimplifyOrCreateChild(
      Op {krys::move(*sharing), krys::move(min->Child), krys::move(max->Child), krys::move(step->Child)},
      *outputType, state);
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeProgress(TokenRange &tokens, int depth,
                                                          ParserState &state) noexcept
  {
    // <progress()> = progress( <calc-sum>, <calc-sum>, <calc-sum> )

    if (!state.PropertyParserState.Context.cssProgressFunctionEnabled)
    {
      return null;
    }

    using Op = Progress;

    auto value = ParseCalcSum(tokens, depth, state);
    if (!value)
    {
      return null;
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto start = ParseCalcSum(tokens, depth, state);
    if (!start)
    {
      return null;
    }

    if (!PropertyParserHelpers::ConsumeComma(tokens))
    {
      return null;
    }

    auto end = ParseCalcSum(tokens, depth, state);
    if (!end)
    {
      return null;
    }

    if (!tokens.IsAtEnd())
    {
      return null;
    }

    // - Validate arguments

    if (!ValidateType<Op::Input>(value->Type))
    {
      return null;
    }

    if (!ValidateType<Op::Input>(start->Type))
    {
      return null;
    }

    if (!ValidateType<Op::Input>(end->Type))
    {
      return null;
    }

    // - Merge arguments

    auto mergedType = MergeTypes<Op::Merge>(value->Type, start->Type);
    if (!mergedType)
    {
      return null;
    }

    mergedType = MergeTypes<Op::Merge>(*mergedType, end->Type);
    if (!mergedType)
    {
      return null;
    }

    auto outputType = TransformType<Op::Output>(*mergedType);
    if (!outputType)
    {
      return null;
    }

    return SimplifyOrCreateChild(
      Op {krys::move(value->Child), krys::move(start->Child), krys::move(end->Child)}, *outputType, state);
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeValueWithoutSimplifyingCalc(TokenRange &tokens, int depth,
                                                                             ParserState &state) noexcept
  {
    // Complex arguments need to be surrounded by a math function.
    if (tokens.Peek().Type() == TokenType::OpenParen)
    {
      return null;
    }

    auto isFunction = tokens.Peek().FunctionId() != ValueId::Invalid;

    auto typedValue = ParseCalcValue(tokens, depth, state);
    if (!typedValue)
    {
      return null;
    }

    auto isLeafValue = krys::SwitchOn(
      typedValue->Child, []<Leaf T>(const T &) { return true; }, [](const auto &) { return false; });

    if (isFunction && isLeafValue)
    {
      // Wrap in Sum to keep top level calc() function in serialization.
      List<Child> children;
      children.push_back(krys::move(typedValue->Child));

      return TypedChild {CreateChild(Sum {krys::move(children)}, typedValue->Type), typedValue->Type};
    }

    return typedValue;
  }

  KRYS_NODISCARD static CSSOMStringAtom ToAtom(Maybe<CSSOMString> &&string) noexcept
  {
    if (!string)
    {
      return CSSOMStringAtom::Null();
    }

    return CSSOMStringAtom {krys::move(*string)};
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeAnchor(TokenRange &tokens, int depth,
                                                        ParserState &state) noexcept
  {
    // <anchor()> = anchor( <anchor-element>? && <anchor-side>, <length-percentage>? )

    if (!state.ParserOptions.PropertyOptions.AnchorPolicy)
    {
      return null;
    }

    if (!state.PropertyParserState.Context.PropertySettings.cssAnchorPositioningEnabled)
    {
      return null;
    }

    auto anchorElement = PropertyParserHelpers::ConsumeDashedIdentRaw(tokens);

    // <anchor-side> = inside | outside | top | left | right | bottom | start | end | self-start | self-end |
    // <percentage> | center
    auto anchorSide = [&]() -> Maybe<AnchorSide>
    {
      auto sideIdent =
        PropertyParserHelpers::ConsumeIdentRaw<ValueId::Inside, ValueId::Outside, ValueId::Top, ValueId::Left,
                                               ValueId::Right, ValueId::Bottom, ValueId::Start, ValueId::End,
                                               ValueId::SelfStart, ValueId::SelfEnd, ValueId::Center>(tokens);
      if (sideIdent)
      {
        return AnchorSide {*sideIdent};
      }

      auto percentageOptions = ParserOptions {
        .Category = Category::Percentage,
        .Range = All,
        .AllowedSymbols = {},
        .PropertyOptions = {},
      };
      auto percentageState = ParserState {
        .PropertyParserState = state.PropertyParserState,
        .ParserOptions = percentageOptions,
        .SimplificationOptions = nullptr,
      };

      auto percentage = ConsumeValueWithoutSimplifyingCalc(tokens, depth, percentageState);
      if (!percentage)
      {
        return null;
      }

      auto category = percentage->Type.CalculationCategory();
      if (!category || category != Category::Percentage)
      {
        return null;
      }

      return AnchorSide {krys::move(percentage->Child)};
    }();

    if (!anchorSide)
    {
      return null;
    }

    if (!anchorElement)
    {
      anchorElement = PropertyParserHelpers::ConsumeDashedIdentRaw(tokens);
    }

    auto type = Type::CreateLength();
    Maybe<Child> fallback;

    if (PropertyParserHelpers::ConsumeComma(tokens))
    {
      auto typedFallback = ConsumeValueWithoutSimplifyingCalc(tokens, depth, state);
      if (!typedFallback)
      {
        return null;
      }

      auto category = typedFallback->Type.CalculationCategory();
      if (!category)
      {
        return null;
      }

      if (*category != Category::Length && *category != Category::LengthPercentage)
      {
        return null;
      }

      fallback = krys::move(typedFallback->Child);
      type.PercentHint = Type::DeterminePercentHint(*category);
    }

    state.RequiresConversionData = true;

    auto anchor = Anchor {
      .ElementName = ToAtom(krys::move(anchorElement)),
      .Side = krys::move(*anchorSide),
      .Fallback = krys::move(fallback),
    };

    return TypedChild {CreateChild(krys::move(anchor), type), type};
  }

  KRYS_NODISCARD static Maybe<style::AnchorSizeDimension> ToAnchorSizeDimension(ValueId value) noexcept
  {
    switch (value)
    {
      case ValueId::Width:      return style::AnchorSizeDimension::Width;
      case ValueId::Height:     return style::AnchorSizeDimension::Height;
      case ValueId::Block:      return style::AnchorSizeDimension::Block;
      case ValueId::Inline:     return style::AnchorSizeDimension::Inline;
      case ValueId::SelfBlock:  return style::AnchorSizeDimension::SelfBlock;
      case ValueId::SelfInline: return style::AnchorSizeDimension::SelfInline;
      default:                  return null;
    }
  }

  KRYS_NODISCARD static Maybe<TypedChild> ConsumeAnchorSize(TokenRange &tokens, int depth,
                                                            ParserState &state) noexcept
  {
    // anchor-size() = anchor-size( [ <anchor-element> || <anchor-size> ]? , <length-percentage>? )
    // <anchor-element> = <dashed-ident>
    // <anchor-size> = width | height | block | inline | self-block | self-inline

    if (!state.ParserOptions.PropertyOptions.AnchorSizePolicy)
    {
      return null;
    }

    if (!state.PropertyParserState.Context.PropertySettings.cssAnchorPositioningEnabled)
    {
      return null;
    }

    // parse <anchor-element>
    auto maybeAnchorElement = PropertyParserHelpers::ConsumeDashedIdentRaw(tokens);

    // then parse <anchor-size>
    auto maybeAnchorSize =
      PropertyParserHelpers::ConsumeIdentRaw<ValueId::Width, ValueId::Height, ValueId::Block, ValueId::Inline,
                                             ValueId::SelfBlock, ValueId::SelfInline>(tokens);

    // if we could parse <anchor-size> but not <anchor-element>, it's possible <anchor-element> is specified
    // after <anchor-size>, so re-parse <anchor-element>
    if (maybeAnchorSize && !maybeAnchorElement)
    {
      maybeAnchorElement = PropertyParserHelpers::ConsumeDashedIdentRaw(tokens);
    }

    Maybe<TypedChild> fallback;

    // if either <anchor-element> or <anchor-size> is present
    if (maybeAnchorSize || maybeAnchorElement)
    {
      // if a comma follows...
      if (PropertyParserHelpers::ConsumeComma(tokens))
      {
        // it must be followed by the fallback value.
        fallback = ConsumeValueWithoutSimplifyingCalc(tokens, depth, state);
        if (!fallback)
        {
          return null;
        }
      }
      // if a comma does not follow, then there's no fallback value.
    }
    else if (!tokens.IsAtEnd())
    {
      // if <anchor-element> and <anchor-size> is not present
      // then an optional fallback value follows
      fallback = ConsumeValueWithoutSimplifyingCalc(tokens, depth, state);
      if (!fallback)
      {
        return null;
      }
    }

    auto type = Type::CreateLength();

    // anchor-size() resolves to a <length> if it can be resolved, otherwise the fallback
    // value is resolved, which is of type <length-percentage>. Therefore the overall type
    // of anchor-size() is <length> or <length-percentage>, depending on the type of the
    // fallback value.
    if (fallback)
    {
      auto category = fallback->Type.CalculationCategory();
      if (!category)
      {
        return null;
      }

      if (*category != Category::Length && *category != Category::LengthPercentage)
      {
        return null;
      }

      type.PercentHint = Type::DeterminePercentHint(*category);
    }

    state.RequiresConversionData = true;

    auto anchorSize = AnchorSize {
      .ElementName = ToAtom(krys::move(maybeAnchorElement)),
      .Dimension = maybeAnchorSize ? ToAnchorSizeDimension(*maybeAnchorSize) : null,
      .Fallback = fallback ? Maybe<Child> {krys::move(fallback->Child)} : null,
    };

    return TypedChild {
      .Child = CreateChild(krys::move(anchorSize), type),
      .Type = type,
    };
  }

  Maybe<TypedChild> ParseCalcFunction(TokenRange &tokens, ValueId functionId, int depth,
                                      ParserState &state) noexcept
  {
    if (CheckDepth(depth) != ParseStatus::Ok)
    {
      return null;
    }

    switch (functionId)
    {
      case ValueId::WebkitCalc:
      case ValueId::Calc:
      {
        // <calc()>  = calc( <calc-sum> )
        return ParseCalcSum(tokens, depth, state);
      }

      case ValueId::Min:
      {
        // <min()>   = min( <calc-sum># )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: consistent type
        return ConsumeOneOrMoreArguments<Min>(tokens, depth, state);
      }

      case ValueId::Max:
      {
        // <max()>   = max( <calc-sum># )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: consistent type
        return ConsumeOneOrMoreArguments<Max>(tokens, depth, state);
      }

      case ValueId::Clamp:
      {
        // <clamp()> = clamp( [ <calc-sum> | none ], <calc-sum>, [ <calc-sum> | none ] )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: consistent type
        return ConsumeClamp(tokens, depth, state);
      }

      case ValueId::Round:
      {
        // <round()> = round( <rounding-strategy>?, <calc-sum>, <calc-sum>? )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: consistent type
        return ConsumeRound(tokens, depth, state);
      }

      case ValueId::Mod:
      {
        // <mod()>   = mod( <calc-sum>, <calc-sum> )
        //     - INPUT: "same" <number>, <dimension>, or <percentage>
        //     - OUTPUT: same type
        return ConsumeExactlyTwoArguments<Mod>(tokens, depth, state);
      }

      case ValueId::Rem:
      {
        // <rem()>   = rem( <calc-sum>, <calc-sum> )
        //     - INPUT: "same" <number>, <dimension>, or <percentage>
        //     - OUTPUT: same type
        return ConsumeExactlyTwoArguments<Rem>(tokens, depth, state);
      }

      case ValueId::Sin:
      {
        // <sin()>   = sin( <calc-sum> )
        //     - INPUT: <number> or <angle>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeExactlyOneArgument<Sin>(tokens, depth, state);
      }

      case ValueId::Cos:
      {
        // <cos()>   = cos( <calc-sum> )
        //     - INPUT: <number> or <angle>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeExactlyOneArgument<Cos>(tokens, depth, state);
      }

      case ValueId::Tan:
      {
        // <tan()>   = tan( <calc-sum> )
        //     - INPUT: <number> or <angle>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeExactlyOneArgument<Tan>(tokens, depth, state);
      }

      case ValueId::Asin:
      {
        // <asin()>  = asin( <calc-sum> )
        //     - INPUT: <number>
        //     - OUTPUT: <angle> "made consistent"
        return ConsumeExactlyOneArgument<Asin>(tokens, depth, state);
      }

      case ValueId::Acos:
      {
        // <acos()>  = acos( <calc-sum> )
        //     - INPUT: <number>
        //     - OUTPUT: <angle> "made consistent"
        return ConsumeExactlyOneArgument<Acos>(tokens, depth, state);
      }

      case ValueId::Atan:
      {
        // <atan()>  = atan( <calc-sum> )
        //     - INPUT: <number>
        //     - OUTPUT: <angle> "made consistent"
        return ConsumeExactlyOneArgument<Atan>(tokens, depth, state);
      }

      case ValueId::Atan2:
      {
        // <atan2()> = atan2( <calc-sum>, <calc-sum> )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: <angle> "made consistent"
        return ConsumeExactlyTwoArguments<Atan2>(tokens, depth, state);
      }

      case ValueId::Pow:
      {
        // <pow()>   = pow( <calc-sum>, <calc-sum> )
        //     - INPUT: "consistent" <number>
        //     - OUTPUT: consistent type
        return ConsumeExactlyTwoArguments<Pow>(tokens, depth, state);
      }

      case ValueId::Sqrt:
      {
        // <sqrt()>  = sqrt( <calc-sum> )
        //     - INPUT: <number>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeExactlyOneArgument<Sqrt>(tokens, depth, state);
      }

      case ValueId::Hypot:
      {
        // <hypot()> = hypot( <calc-sum># )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: consistent type
        return ConsumeOneOrMoreArguments<Hypot>(tokens, depth, state);
      }

      case ValueId::Log:
      {
        // <log()>   = log( <calc-sum>, <calc-sum>? )
        //     - INPUT: <number>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeOneOrTwoArguments<Log>(tokens, depth, state);
      }

      case ValueId::Exp:
      {
        // <exp()>   = exp( <calc-sum> )
        //     - INPUT: <number>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeExactlyOneArgument<Exp>(tokens, depth, state);
      }

      case ValueId::Abs:
      {
        // <abs()>   = abs( <calc-sum> )
        //     - INPUT: any
        //     - OUTPUT: input type
        return ConsumeExactlyOneArgument<Abs>(tokens, depth, state);
      }

      case ValueId::Sign:
      {
        // <sign()>  = sign( <calc-sum> )
        //     - INPUT: any
        //     - OUTPUT: <number> "made consistent"
        return ConsumeExactlyOneArgument<Sign>(tokens, depth, state);
      }

      case ValueId::Random:
      {
        // <random()> = random( <random-value-sharing>? , <calc-sum>, <calc-sum>, <calc-sum>? )
        //     - INPUT: "same" <number>, <dimension>, or <percentage>
        //     - OUTPUT: same type
        return ConsumeRandom(tokens, depth, state);
      }

      case ValueId::Progress:
      {
        // <progress()> = progress( <calc-sum>, <calc-sum>, <calc-sum> )
        //     - INPUT: "consistent" <number>, <dimension>, or <percentage>
        //     - OUTPUT: <number> "made consistent"
        return ConsumeProgress(tokens, depth, state);
      }

      case ValueId::SiblingCount:
      {
        // <sibling-count()> = sibling-count()
        //     - INPUT: none
        //     - OUTPUT: <integer>
        if (!state.PropertyParserState.Context.cssTreeCountingFunctionsEnabled)
        {
          return null;
        }

        if (state.PropertyParserState.CurrentRule != RuleType::Style
            && state.PropertyParserState.CurrentRule != RuleType::Keyframe)
        {
          return null;
        }

        if (state.PropertyParserState.CurrentProperty == PropertyId::Invalid)
        {
          return null;
        }

        return ConsumeZeroArguments<SiblingCount>(tokens, depth, state);
      }

      case ValueId::SiblingIndex:
      {
        // <sibling-index()> = sibling-index()
        //     - INPUT: none
        //     - OUTPUT: <integer>
        if (!state.PropertyParserState.Context.cssTreeCountingFunctionsEnabled)
        {
          return null;
        }

        if (state.PropertyParserState.CurrentRule != RuleType::Style
            && state.PropertyParserState.CurrentRule != RuleType::Keyframe)
        {
          return null;
        }

        if (state.PropertyParserState.CurrentProperty == PropertyId::Invalid)
        {
          return null;
        }

        return ConsumeZeroArguments<SiblingIndex>(tokens, depth, state);
      }

      case ValueId::Anchor:
      {
        return ConsumeAnchor(tokens, depth, state);
      }

      case ValueId::AnchorSize:
      {
        return ConsumeAnchorSize(tokens, depth, state);
      }

      default:
      {
        break;
      }
    }

    return null;
  }

  Maybe<TypedChild> ParseCalcSum(TokenRange &tokens, int depth, ParserState &state) noexcept
  {
    // <calc-sum> = <calc-product> [ [ '+' | '-' ] <calc-product> ]*

    if (CheckDepth(depth) != ParseStatus::Ok)
    {
      return null;
    }

    auto originalTokens = tokens.Tokens();

    auto firstValue = ParseCalcProduct(tokens, depth, state);
    if (!firstValue)
    {
      return null;
    }

    auto sumType = firstValue->Type;
    List<Child> children;

    while (!tokens.IsAtEnd())
    {
      const auto &token = tokens.Peek();
      char32 operatorCharacter = token.Type() == TokenType::Delim ? token.Delimiter() : 0;
      if (operatorCharacter != static_cast<char32>(Operator::Sum)
          && operatorCharacter != static_cast<char32>(Operator::Negate))
      {
        break;
      }

      const auto &previousToken = originalTokens[tokens.Tokens().data() - originalTokens.data() - 1];
      if (previousToken.Type() != TokenType::Whitespace)
      {
        return null; // calc(1px+ 2px) is invalid
      }

      tokens.Discard();
      if (tokens.Peek().Type() != TokenType::Whitespace)
      {
        return null; // calc(1px +2px) is invalid
      }

      tokens.DiscardWhitespace();

      auto nextValue = ParseCalcProduct(tokens, depth, state);
      if (!nextValue)
      {
        return null;
      }

      if (operatorCharacter == static_cast<char32>(Operator::Negate))
      {
        auto negateType = nextValue->Type;
        nextValue = SimplifyOrCreateChild(Negate {krys::move(nextValue->Child)}, negateType, state);
      }

      if (firstValue)
      {
        children.push_back(krys::move(firstValue->Child));
        firstValue = null;
      }

      auto newType = Type::Add(sumType, nextValue->Type);
      if (!newType)
      {
        return null;
      }

      sumType = *newType;
      children.push_back(krys::move(nextValue->Child));
    }

    if (children.empty())
    {
      return firstValue;
    }

    return SimplifyOrCreateChild(Sum {krys::move(children)}, sumType, state);
  }

  Maybe<TypedChild> ParseCalcProduct(TokenRange &tokens, int depth, ParserState &state) noexcept
  {
    // <calc-product> = <calc-value> [ [ '*' | '/' ] <calc-value> ]*

    if (CheckDepth(depth) != ParseStatus::Ok)
    {
      return null;
    }

    auto firstValue = ParseCalcValue(tokens, depth, state);
    if (!firstValue)
    {
      return null;
    }

    auto productType = firstValue->Type;
    List<Child> children;

    while (!tokens.IsAtEnd())
    {
      const auto &token = tokens.Peek();
      char32 operatorCharacter = token.Type() == TokenType::Delim ? token.Delimiter() : 0;
      if (operatorCharacter != static_cast<char32>(Operator::Product)
          && operatorCharacter != static_cast<char32>(Operator::Invert))
      {
        break;
      }

      tokens.Discard();
      tokens.DiscardWhitespace();

      auto nextValue = ParseCalcValue(tokens, depth, state);
      if (!nextValue)
      {
        return null;
      }

      if (operatorCharacter == static_cast<char32>(Operator::Invert))
      {
        auto invertType = Type::Invert(nextValue->Type);
        nextValue = SimplifyOrCreateChild(Invert {krys::move(nextValue->Child)}, invertType, state);
      }

      if (firstValue)
      {
        children.push_back(krys::move(firstValue->Child));
        firstValue = null;
      }

      auto newType = Type::Multiply(productType, nextValue->Type);
      if (!newType)
      {
        return null;
      }

      productType = *newType;
      children.push_back(krys::move(nextValue->Child));
    }

    if (children.empty())
    {
      return firstValue;
    }

    return SimplifyOrCreateChild(Product {krys::move(children)}, productType, state);
  }

  Maybe<TypedChild> ParseCalcValue(TokenRange &tokens, int depth, ParserState &state) noexcept
  {
    // <calc-value> = <number> | <dimension> | <percentage> | <calc-keyword> | ( <calc-sum> )
    // <calc-keyword> = e | pi | infinity | -infinity | NaN
    //
    // NOTE: <calc-keyword> is extended for identifiers specified via AllowedSymbols.

    if (CheckDepth(depth) != ParseStatus::Ok)
    {
      return null;
    }

    auto findBlock = [](const TokenRange &tokens) -> Maybe<ValueId>
    {
      if (tokens.Peek().Type() == TokenType::OpenParen)
      {
        // Simple blocks (e.g. parenthesis around additional expressions) can be treated just like a nested
        // calc().
        return ValueId::Calc;
      }

      if (auto functionId = tokens.Peek().FunctionId(); IsCalcFunction(functionId))
      {
        return functionId;
      }

      return null;
    };

    if (auto functionId = findBlock(tokens))
    {
      TokenRange innerRange = tokens.ConsumeBlock();
      tokens.DiscardWhitespace();
      innerRange.DiscardWhitespace();

      auto function = ParseCalcFunction(innerRange, *functionId, depth + 1, state);
      if (!function)
      {
        return null;
      }

      if (!innerRange.IsAtEnd())
      {
        return null;
      }

      return function;
    }

    const auto &token = tokens.ConsumeIncludingWhitespace();

    switch (token.Type())
    {
      case TokenType::Ident:      return ParseCalcKeyword(token, state);
      case TokenType::Number:     return ParseCalcNumber(token, state);
      case TokenType::Percentage: return ParseCalcPercentage(token, state);
      case TokenType::Dimension:  return ParseCalcDimension(token, state);
      default:                    break;
    }

    return null;
  }

  Maybe<TypedChild> ParseCalcKeyword(const Token &token, ParserState &state) noexcept
  {
    if (auto unit = state.ParserOptions.AllowedSymbols.Get(token.ValueId()))
    {
      auto child = Symbol {token.ValueId(), *unit};
      auto type = Type::DetermineType(*unit);

      if (ConversionToCanonicalUnitRequiresConversionData(*unit))
      {
        state.RequiresConversionData = true;
      }

      if (auto *simplificationOptions = state.SimplificationOptions)
      {
        if (auto replacement = Simplify(child, *simplificationOptions))
        {
          return TypedChild {krys::move(*replacement), type};
        }
      }

      return TypedChild {CreateChild(krys::move(child)), type};
    }

    if (auto constant = LookupConstantNumber(token.ValueId()))
    {
      return TypedChild {CreateChild(krys::move(*constant)), Type {}};
    }

    return null;
  }

  Maybe<TypedChild> ParseCalcNumber(const Token &token, ParserState &) noexcept
  {
    auto child = Number {.Value = token.NumericValue()};
    auto type = Type {};

    return TypedChild {CreateChild(krys::move(child)), type};
  }

  Maybe<TypedChild> ParseCalcPercentage(const Token &token, ParserState &state) noexcept
  {
    auto child = Percentage {
      .Value = token.NumericValue(),
      .Hint = Type::DeterminePercentHint(state.ParserOptions.Category),
    };
    auto type = GetType(child);

    return TypedChild {CreateChild(krys::move(child)), type};
  }

  Maybe<TypedChild> ParseCalcDimension(const Token &token, ParserState &state) noexcept
  {
    auto unitType = ParseUnitType(token.Unit());
    if (unitType == UnitType::Unknown)
    {
      return null;
    }

    auto child = CreateNumeric(token.NumericValue(), unitType);
    auto type = Type::DetermineType(unitType);

    if (ConversionToCanonicalUnitRequiresConversionData(unitType))
    {
      state.RequiresConversionData = true;
    }

    if (auto *simplificationOptions = state.SimplificationOptions)
    {
      return TypedChild {CopyAndSimplify(krys::move(child), *simplificationOptions), type};
    }

    return TypedChild {krys::move(child), type};
  }
}
