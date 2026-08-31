#include "Krystal.HTML/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  // Used by radial gradient consumers internally.
  enum class ShapeKeyword : bool
  {
    Circle,
    Ellipse
  };

  // MARK: Deprecated <gradient> values

  template <CSSValueId ZeroValue, CSSValueId OneHundredValue>
  KRYS_NODISCARD static Maybe<NumberOrPercentage<>>
    ConsumeDeprecatedGradientPositionComponent(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      if (ConsumeIdent<ZeroValue>(tokens))
      {
        return NumberOrPercentage<> {PercentageRaw<> {0}};
      }

      if (ConsumeIdent<OneHundredValue>(tokens))
      {
        return NumberOrPercentage<> {PercentageRaw<> {100}};
      }

      if (ConsumeIdent<CSSValueId::Center>(tokens))
      {
        return NumberOrPercentage<> {PercentageRaw<> {50}};
      }

      return Null;
    }
    return MetaConsumer<Number<>, Percentage<>>::Consume(tokens, state);
  }

  KRYS_NODISCARD static Maybe<DeprecatedGradientPosition>
    ConsumeDeprecatedGradientPosition(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    auto horizontal =
      ConsumeDeprecatedGradientPositionComponent<CSSValueId::Left, CSSValueId::Right>(tokens, state);
    if (!horizontal)
    {
      return Null;
    }

    auto vertical =
      ConsumeDeprecatedGradientPositionComponent<CSSValueId::Top, CSSValueId::Bottom>(tokens, state);
    if (!vertical)
    {
      return Null;
    }

    return {{Krys::Move(*horizontal), Krys::Move(*vertical)}};
  }

  KRYS_NODISCARD static Maybe<Color>
    ConsumeDeprecatedGradientStopColor(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::Currentcolor)
    {
      return Null;
    }

    return ConsumeUnresolvedColor(tokens, state);
  }

  KRYS_NODISCARD static Maybe<GradientDeprecatedColorStop>
    ConsumeDeprecatedGradientColorStop(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    auto id = tokens.Peek().FunctionId();
    switch (id)
    {
      case CSSValueId::From:
      case CSSValueId::To:
      case CSSValueId::ColorStop:
      {
        break;
      }
      default:
      {
        return Null;
      }
    }

    auto args = ConsumeFunction(tokens);
    Maybe<GradientDeprecatedColorStopPosition> position;
    switch (id)
    {
      case CSSValueId::From:
      {
        position = NumberRaw<> {0};
        break;
      }
      case CSSValueId::To:
      {
        position = NumberRaw<> {1};
        break;
      }
      case CSSValueId::ColorStop:
      {
        auto numberOrPercentage = MetaConsumer<Number<>, Percentage<>>::Consume(args, state);
        if (!numberOrPercentage)
        {
          return Null;
        }

        if (!ConsumeComma(args))
        {
          return Null;
        }

        position = Krys::Move(*numberOrPercentage);
        break;
      }
      default:
      {
        assert(false);
        return Null;
      }
    }

    auto color = ConsumeDeprecatedGradientStopColor(args, state);
    if (!color || !args.IsAtEnd())
    {
      return Null;
    }

    return GradientDeprecatedColorStop {.color = Krys::Move(*color), .position = Krys::Move(*position)};
  }

  KRYS_NODISCARD static Maybe<GradientDeprecatedColorStopList>
    ConsumeDeprecatedGradientColorStops(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    GradientDeprecatedColorStopList::Container stops;
    while (ConsumeComma(tokens))
    {
      auto stop = ConsumeDeprecatedGradientColorStop(tokens, state);
      if (!stop)
      {
        return Null;
      }

      stops.push_back(Krys::Move(*stop));
    }

    return {{Krys::Move(stops)}};
  }

  KRYS_NODISCARD static RefPtr<CSSValue>
    ConsumeDeprecatedLinearGradient(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (!ConsumeComma(tokens))
    {
      return nullptr;
    }

    auto first = ConsumeDeprecatedGradientPosition(tokens, state);
    if (!first)
    {
      return nullptr;
    }

    if (!ConsumeComma(tokens))
    {
      return nullptr;
    }

    auto second = ConsumeDeprecatedGradientPosition(tokens, state);
    if (!second)
    {
      return nullptr;
    }

    auto stops = ConsumeDeprecatedGradientColorStops(tokens, state);
    if (!stops)
    {
      return nullptr;
    }

    return CSSGradientValue::Create(FunctionNotation<CSSValueId::WebkitGradient, DeprecatedLinearGradient> {
      .parameters = {.colorInterpolationMethod =
                       GradientColorInterpolationMethod::legacyMethod(AlphaPremultiplication::Premultiplied),
                     .gradientLine = {Krys::Move(*first), Krys::Move(*second)},
                     .stops = Krys::Move(*stops)}});
  }

  KRYS_NODISCARD static RefPtr<CSSValue>
    ConsumeDeprecatedRadialGradient(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (!ConsumeComma(tokens))
    {
      return nullptr;
    }

    auto first = ConsumeDeprecatedGradientPosition(tokens, state);
    if (!first)
    {
      return nullptr;
    }

    if (!ConsumeComma(tokens))
    {
      return nullptr;
    }

    auto firstRadius = MetaConsumer<Number<NonNegative>>::Consume(tokens, state);
    if (!firstRadius)
    {
      return nullptr;
    }

    if (!ConsumeComma(tokens))
    {
      return nullptr;
    }

    auto second = ConsumeDeprecatedGradientPosition(tokens, state);
    if (!second)
    {
      return nullptr;
    }

    if (!ConsumeComma(tokens))
    {
      return nullptr;
    }

    auto secondRadius = MetaConsumer<Number<NonNegative>>::Consume(tokens, state);
    if (!secondRadius)
    {
      return nullptr;
    }

    auto stops = ConsumeDeprecatedGradientColorStops(tokens, state);
    if (!stops)
    {
      return nullptr;
    }

    return CSSGradientValue::Create(FunctionNotation<CSSValueId::WebkitGradient, DeprecatedRadialGradient> {
      .parameters = {.colorInterpolationMethod =
                       GradientColorInterpolationMethod::legacyMethod(AlphaPremultiplication::Premultiplied),
                     .gradientBox =
                       {
                         .first = Krys::Move(*first),
                         .firstRadius = Krys::Move(*firstRadius),
                         .second = Krys::Move(*second),
                         .secondRadius = Krys::Move(*secondRadius),
                       },
                     .stops = Krys::Move(*stops)}});
  }

  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeDeprecatedGradient(CSSTokenRange &tokens,
                                                                   CSSPropertyParserState &state) noexcept
  {
    switch (tokens.ConsumeIncludingWhitespace().ValueId())
    {
      case CSSValueId::Linear:
      {
        return ConsumeDeprecatedLinearGradient(tokens, state);
      }
      case CSSValueId::Radial:
      {
        return ConsumeDeprecatedRadialGradient(tokens, state);
      }
      default:
      {
        return nullptr;
      }
    }
  }

  // MARK: <color-stop-list> | <angular-color-stop-list>
  // https://drafts.csswg.org/css-images-4/#typedef-color-stop-list

  enum class SupportsColorHints : bool
  {
    No,
    Yes
  };

  KRYS_NODISCARD static Maybe<Color> ConsumeStopColor(CSSTokenRange &tokens,
                                                      CSSPropertyParserState &state) noexcept
  {
    return ConsumeUnresolvedColor(tokens, state);
  }

  template <SupportsColorHints supportsColorHints, typename Stop, typename Consumer>
  KRYS_NODISCARD static Maybe<GradientColorStopList<Stop>>
    ConsumeColorStopList(CSSTokenRange &tokens, CSSPropertyParserState &state,
                         Consumer &&consumeStopPosition) noexcept
  {
    typename GradientColorStopList<Stop>::Container stops;

    // The first color stop cannot be a color hint.
    bool previousStopWasColorHint = true;
    do
    {
      Stop stop {ConsumeStopColor(tokens, state), consumeStopPosition(tokens)};
      if (!stop.color && !stop.position)
      {
        return Null;
      }

      // Two color hints in a row are not allowed.
      if (!stop.color && (supportsColorHints == SupportsColorHints::No || previousStopWasColorHint))
      {
        return Null;
      }

      previousStopWasColorHint = !stop.color;

      // Stops with both a color and a position can have a second position, which shares the same color.
      if (stop.color && stop.position)
      {
        if (auto secondPosition = consumeStopPosition(tokens))
        {
          stops.push_back(stop);
          stop.position = Krys::Move(secondPosition);
        }
      }
      stops.push_back(Krys::Move(stop));
    } while (ConsumeComma(tokens));

    // The last color stop cannot be a color hint.
    if (previousStopWasColorHint)
    {
      return Null;
    }

    // Must have at least one stop to be valid.
    if (stops.empty())
    {
      return Null;
    }

    return {Krys::Move(stops)};
  }

  template <SupportsColorHints supportsColorHints>
  KRYS_NODISCARD static Maybe<GradientLinearColorStopList>
    ConsumeLinearColorStopList(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    return ConsumeColorStopList<supportsColorHints, GradientLinearColorStop>(
      tokens, state, [&](auto &tokens) { return MetaConsumer<LengthPercentage<>>::Consume(tokens, state); });
  }

  template <SupportsColorHints supportsColorHints>
  KRYS_NODISCARD static Maybe<GradientAngularColorStopList>
    ConsumeAngularColorStopList(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // NOTE: Angular color stops accept unitless zero values.
    // https://drafts.csswg.org/css-images-4/#typedef-color-stop-angle

    return ConsumeColorStopList<supportsColorHints, GradientAngularColorStop>(
      tokens, state,
      [&](auto &tokens)
      {
        return MetaConsumer<AnglePercentage<>>::Consume(tokens, state,
                                                        {.UnitlessZeroAngle = AllowUnitlessZero(true)});
      });
  }

  KRYS_NODISCARD static bool StopColorIs8Bit(const Color &color) noexcept
  {
    return color.isKeyword() || color.absoluteColor().tryGetAsSRGBABytes();
  }

  KRYS_NODISCARD static bool StopColorIs8Bit(const Markable<Color> &color) noexcept
  {
    return !color || StopColorIs8Bit(*color);
  }

  template <typename Stop>
  KRYS_NODISCARD static GradientColorInterpolationMethod
    ComputeGradientColorInterpolationMethod(Maybe<ColorInterpolationMethod> parsedColorInterpolationMethod,
                                            const GradientColorStopList<Stop> &stops) noexcept
  {
    // We detect whether stops use legacy vs. non-legacy CSS color syntax using the following rules:
    //  - A CSSValueId is always considered legacy since all keyword based colors are considered legacy by the
    //  spec.
    //  - An actual Color value is considered legacy if it is stored as 8-bit sRGB.
    //
    // While this is accurate now, we should consider a more robust mechanism to detect this at parse
    // time, perhaps keeping this information in the CSSPrimitiveValue itself.

    auto defaultColorInterpolationMethod = CSS::GradientColorInterpolationMethod::Default::SRGB;
    for (auto &stop : stops)
    {
      if (stopColorIs8Bit(stop.color))
        continue;

      defaultColorInterpolationMethod = CSS::GradientColorInterpolationMethod::Default::OKLab;
      break;
    }

    if (parsedColorInterpolationMethod)
      return {*parsedColorInterpolationMethod, defaultColorInterpolationMethod};

    switch (defaultColorInterpolationMethod)
    {
      case CSS::GradientColorInterpolationMethod::Default::SRGB:
        return {{ColorInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied},
                defaultColorInterpolationMethod};

      case CSS::GradientColorInterpolationMethod::Default::OKLab:
        return {{ColorInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied},
                defaultColorInterpolationMethod};
    }

    ASSERT_NOT_REACHED();
    return {{ColorInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied},
            defaultColorInterpolationMethod};
  }

  // MARK: Compat <gradient> values
  // https://compat.spec.whatwg.org/#css-gradient-fns

  // MARK: <-webkit-linear-gradient()> | <-webkit-repeating-linear-gradient()>
  // https://compat.spec.whatwg.org/#css-gradients-webkit-linear-gradient

  template <CSSValueId Name>
  KRYS_NODISCARD static RefPtr<CSSValue> consumePrefixedLinearGradient(CSSTokenRange &tokens,
                                                                       CSSPropertyParserState &state)
  {
    // https://compat.spec.whatwg.org/#css-gradients-webkit-linear-gradient/ states that
    // -webkit-linear-gradient() and -webkit-repeating-linear-gradient() must be "treated as an alias of
    // linear-gradient as defined in [css3-images-20110217]." In [css3-images-20110217] the grammar was
    // defined as:
    //
    //   <linear-gradient> = linear-gradient([ [ [top | bottom] || [left | right] ] | <angle> ,]?
    //   <color-stop>[, <color-stop>]+);
    //
    // see https://www.w3.org/TR/2011/WD-css3-images-20110217/#linear-gradients.

    KRYS_NODISCARD constexpr static auto verticalMappings =
      std::to_array<std::pair<CSSValueId, CSS::Vertical>>({
        {CSSValueTop, CSS::Vertical {CSS::Keyword::Top {}}},
        {CSSValueBottom, CSS::Vertical {CSS::Keyword::Bottom {}}},
      });
    KRYS_NODISCARD constexpr static SortedArrayMap verticalMap {verticalMappings};

    KRYS_NODISCARD constexpr static auto horizontalMappings =
      std::to_array<std::pair<CSSValueId, CSS::Horizontal>>({
        {CSSValueLeft, CSS::Horizontal {CSS::Keyword::Left {}}},
        {CSSValueRight, CSS::Horizontal {CSS::Keyword::Right {}}},
      });
    KRYS_NODISCARD constexpr static SortedArrayMap horizontalMap {horizontalMappings};

    auto consumeKeywordGradientLineKnownHorizontal =
      [&](CSSTokenRange &tokens, CSS::Horizontal knownHorizontal) -> CSS::PrefixedLinearGradient::GradientLine
    {
      if (auto vertical = consumeIdentUsingMapping(tokens, verticalMap))
        return SpaceSeparatedTuple {knownHorizontal, *vertical};
      return knownHorizontal;
    };

    auto consumeKeywordGradientLineKnownVertical =
      [&](CSSTokenRange &tokens, CSS::Vertical knownVertical) -> CSS::PrefixedLinearGradient::GradientLine
    {
      if (auto horizontal = consumeIdentUsingMapping(tokens, horizontalMap))
        return SpaceSeparatedTuple {*horizontal, knownVertical};
      return knownVertical;
    };

    auto consumeKeywordGradientLine =
      [&](CSSTokenRange &tokens) -> Maybe<CSS::PrefixedLinearGradient::GradientLine>
    {
      switch (tokens.Peek().ValueId())
      {
        case CSSValueLeft:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownHorizontal(tokens, CSS::Horizontal {CSS::Keyword::Left {}});
        case CSSValueRight:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownHorizontal(tokens, CSS::Horizontal {CSS::Keyword::Right {}});
        case CSSValueTop:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownVertical(tokens, CSS::Vertical {CSS::Keyword::Top {}});
        case CSSValueBottom:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownVertical(tokens, CSS::Vertical {CSS::Keyword::Bottom {}});
        default: return {};
      }
    };

    Maybe<CSS::PrefixedLinearGradient::GradientLine> gradientLine;

    // NOTE: Linear gradient <angle> specifiers accept unitless zero values.
    // https://drafts.csswg.org/css-images-4/#typedef-linear-gradient-syntax

    if (auto angle =
          MetaConsumer<CSS::Angle<>>::Consume(tokens, state, {.unitlessZeroAngle = UnitlessZeroQuirk::Allow}))
    {
      gradientLine = WTF::switchOn(
        Krys::Move(*angle), [](auto &&value) -> CSS::PrefixedLinearGradient::GradientLine { return value; });
      if (!ConsumeComma(tokens))
        return nullptr;
    }
    else if (auto keywordGradientLine = consumeKeywordGradientLine(tokens))
    {
      gradientLine = Krys::Move(*keywordGradientLine);
      if (!ConsumeComma(tokens))
        return nullptr;
    }

    auto stops = consumeLinearColorStopList<SupportsColorHints::No>(tokens, state);
    if (!stops)
      return nullptr;

    return CSSGradientValue::create(FunctionNotation<Name, CSS::PrefixedLinearGradient> {
      .parameters = {.colorInterpolationMethod = CSS::GradientColorInterpolationMethod::legacyMethod(
                       AlphaPremultiplication::Premultiplied),
                     .gradientLine = gradientLine.value_or(CSS::Vertical {CSS::Keyword::Top {}}),
                     .stops = Krys::Move(*stops)}});
  }

  // MARK: <-webkit-radial-gradient()> | <-webkit-repeating-radial-gradient()>
  // https://compat.spec.whatwg.org/#css-gradients-webkit-radial-gradient

  template <CSSValueId Name>
  KRYS_NODISCARD static RefPtr<CSSValue> consumePrefixedRadialGradient(CSSTokenRange &tokens,
                                                                       CSSPropertyParserState &state)
  {
    // https://compat.spec.whatwg.org/#css-gradients-webkit-radial-gradient/ states that
    // -webkit-radial-gradient() and -webkit-repeating-radial-gradient() must be "treated as an alias of
    // radial-gradient as defined in [css3-images-20110217]." In [css3-images-20110217] the grammar was
    // defined as:
    //
    //   <radial-gradient> = radial-gradient([<bg-position>,]? [ [<shape> || <size>] | [<length> |
    //   <percentage>]{2} ,]? <color-stop>[, <color-stop>]+); <shape> = [ circle | ellipse ] <size> = [
    //   closest-side | closest-corner | farthest-side | farthest-corner | contain | cover ]
    //
    //      defaults to ‘ellipse cover’.
    //
    // see https://www.w3.org/TR/2011/WD-css3-images-20110217/#radial-gradients.

    KRYS_NODISCARD constexpr static auto shapeMappings = std::to_array<std::pair<CSSValueId, ShapeKeyword>>({
      {CSSValueCircle, ShapeKeyword::Circle},
      {CSSValueEllipse, ShapeKeyword::Ellipse},
    });
    KRYS_NODISCARD constexpr static SortedArrayMap shapeMap {shapeMappings};

    KRYS_NODISCARD constexpr static auto extentMappings =
      std::to_array<std::pair<CSSValueId, CSS::PrefixedRadialGradient::Extent>>({
        {CSSValueContain, CSS::PrefixedRadialGradient::Extent {CSS::Keyword::Contain {}}},
        {CSSValueCover, CSS::PrefixedRadialGradient::Extent {CSS::Keyword::Cover {}}},
        {CSSValueClosestSide, CSS::PrefixedRadialGradient::Extent {CSS::Keyword::ClosestSide {}}},
        {CSSValueClosestCorner, CSS::PrefixedRadialGradient::Extent {CSS::Keyword::ClosestCorner {}}},
        {CSSValueFarthestSide, CSS::PrefixedRadialGradient::Extent {CSS::Keyword::FarthestSide {}}},
        {CSSValueFarthestCorner, CSS::PrefixedRadialGradient::Extent {CSS::Keyword::FarthestCorner {}}},
      });
    KRYS_NODISCARD constexpr static SortedArrayMap extentMap {extentMappings};

    auto position = consumeOneOrTwoComponentPositionUnresolved(tokens, state);
    if (position)
    {
      if (!ConsumeComma(tokens))
        return nullptr;
    }

    Maybe<ShapeKeyword> shapeKeyword;
    Maybe<CSS::PrefixedRadialGradient::Extent> extentKeyword;
    if (tokens.Peek().Type() == CSSTokenType::Ident)
    {
      shapeKeyword = consumeIdentUsingMapping(tokens, shapeMap);
      extentKeyword = consumeIdentUsingMapping(tokens, extentMap);
      if (!shapeKeyword)
        shapeKeyword = consumeIdentUsingMapping(tokens, shapeMap);
      if (shapeKeyword || extentKeyword)
      {
        if (!ConsumeComma(tokens))
          return nullptr;
      }
    }

    auto consumeGradientBox = [&] -> Maybe<CSS::PrefixedRadialGradient::GradientBox>
    {
      if (shapeKeyword && extentKeyword)
      {
        switch (*shapeKeyword)
        {
          case ShapeKeyword::Ellipse:
            return CSS::PrefixedRadialGradient::Ellipse {
              .size = *extentKeyword,
              .position = Krys::Move(position),
            };

          case ShapeKeyword::Circle:
            return CSS::PrefixedRadialGradient::Circle {
              .size = *extentKeyword,
              .position = Krys::Move(position),
            };
        }
      }

      if (shapeKeyword)
      {
        switch (*shapeKeyword)
        {
          case ShapeKeyword::Ellipse:
            return CSS::PrefixedRadialGradient::Ellipse {
              .size = Null,
              .position = Krys::Move(position),
            };

          case ShapeKeyword::Circle:
            return CSS::PrefixedRadialGradient::Circle {
              .size = Null,
              .position = Krys::Move(position),
            };
        }
      }

      if (extentKeyword)
      {
        return CSS::PrefixedRadialGradient::Ellipse {
          .size = *extentKeyword,
          .position = Krys::Move(position),
        };
      }

      if (auto length1 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state))
      {
        auto length2 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state);
        if (!length2)
          return Null;
        if (!ConsumeComma(tokens))
          return Null;
        return CSS::PrefixedRadialGradient::Ellipse {
          .size = SpaceSeparatedArray {Krys::Move(*length1), Krys::Move(*length2)},
          .position = Krys::Move(position),
        };
      }

      // If no size is provided, default to an ellipse.
      return CSS::PrefixedRadialGradient::Ellipse {.size = Null, .position = Krys::Move(position)};
    };

    auto gradientBox = consumeGradientBox();
    if (!gradientBox)
      return nullptr;

    auto stops = consumeLinearColorStopList<SupportsColorHints::No>(tokens, state);
    if (!stops)
      return nullptr;

    return CSSGradientValue::create(FunctionNotation<Name, CSS::PrefixedRadialGradient> {
      .parameters = {.colorInterpolationMethod = CSS::GradientColorInterpolationMethod::legacyMethod(
                       AlphaPremultiplication::Premultiplied),
                     .gradientBox = Krys::Move(*gradientBox),
                     .stops = Krys::Move(*stops)}});
  }

  // MARK: Standard <gradient> values

  // MARK: <linear-gradient()> | <repeating-linear-gradient()>
  // https://drafts.csswg.org/css-images-4/#linear-gradients

  template <CSSValueId Name>
  KRYS_NODISCARD static RefPtr<CSSValue> consumeLinearGradient(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state)
  {
    // <side-or-corner> = [left | right] || [top | bottom]
    // linear-gradient() = linear-gradient(
    //   [ <angle> | to <side-or-corner> ]? || <color-interpolation-method>,
    //   <color-stop-list>
    // )

    KRYS_NODISCARD constexpr static auto verticalMappings =
      std::to_array<std::pair<CSSValueId, CSS::Vertical>>({
        {CSSValueTop, CSS::Vertical {CSS::Keyword::Top {}}},
        {CSSValueBottom, CSS::Vertical {CSS::Keyword::Bottom {}}},
      });
    KRYS_NODISCARD constexpr static SortedArrayMap verticalMap {verticalMappings};

    KRYS_NODISCARD constexpr static auto horizontalMappings =
      std::to_array<std::pair<CSSValueId, CSS::Horizontal>>({
        {CSSValueLeft, CSS::Horizontal {CSS::Keyword::Left {}}},
        {CSSValueRight, CSS::Horizontal {CSS::Keyword::Right {}}},
      });
    KRYS_NODISCARD constexpr static SortedArrayMap horizontalMap {horizontalMappings};

    auto consumeKeywordGradientLineKnownHorizontal =
      [&](CSSTokenRange &tokens, CSS::Horizontal knownHorizontal) -> CSS::LinearGradient::GradientLine
    {
      if (auto vertical = consumeIdentUsingMapping(tokens, verticalMap))
        return SpaceSeparatedTuple {knownHorizontal, *vertical};
      return knownHorizontal;
    };

    auto consumeKeywordGradientLineKnownVertical =
      [&](CSSTokenRange &tokens, CSS::Vertical knownVertical) -> CSS::LinearGradient::GradientLine
    {
      if (auto horizontal = consumeIdentUsingMapping(tokens, horizontalMap))
        return SpaceSeparatedTuple {*horizontal, knownVertical};
      return knownVertical;
    };

    auto consumeKeywordGradientLine = [&](CSSTokenRange &tokens) -> Maybe<CSS::LinearGradient::GradientLine>
    {
      ASSERT(tokens.Peek().ValueId() == CSSValueTo);
      tokens.consumeIncludingWhitespace();

      switch (tokens.Peek().ValueId())
      {
        case CSSValueLeft:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownHorizontal(tokens, CSS::Horizontal {CSS::Keyword::Left {}});
        case CSSValueRight:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownHorizontal(tokens, CSS::Horizontal {CSS::Keyword::Right {}});
        case CSSValueTop:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownVertical(tokens, CSS::Vertical {CSS::Keyword::Top {}});
        case CSSValueBottom:
          tokens.consumeIncludingWhitespace();
          return consumeKeywordGradientLineKnownVertical(tokens, CSS::Vertical {CSS::Keyword::Bottom {}});
        default: return {};
      }
    };

    Maybe<ColorInterpolationMethod> colorInterpolationMethod;

    if (tokens.Peek().ValueId() == CSSValueIn)
    {
      colorInterpolationMethod = consumeColorInterpolationMethod(tokens, state);
      if (!colorInterpolationMethod)
        return nullptr;
    }

    Maybe<CSS::LinearGradient::GradientLine> gradientLine;

    // NOTE: Linear gradient <angle> specifiers accept unitless zero values.
    // https://drafts.csswg.org/css-images-4/#typedef-linear-gradient-syntax

    if (auto angle =
          MetaConsumer<CSS::Angle<>>::Consume(tokens, state, {.unitlessZeroAngle = UnitlessZeroQuirk::Allow}))
      gradientLine = Krys::Move(angle);
    else if (tokens.Peek().ValueId() == CSSValueTo)
    {
      auto keywordGradientLine = consumeKeywordGradientLine(tokens);
      if (!keywordGradientLine)
        return nullptr;
      gradientLine = Krys::Move(*keywordGradientLine);
    }

    if (gradientLine && !colorInterpolationMethod && tokens.Peek().ValueId() == CSSValueIn)
    {
      colorInterpolationMethod = consumeColorInterpolationMethod(tokens, state);
      if (!colorInterpolationMethod)
        return nullptr;
    }

    if (gradientLine || colorInterpolationMethod)
    {
      if (!ConsumeComma(tokens))
        return nullptr;
    }

    auto stops = consumeLinearColorStopList<SupportsColorHints::Yes>(tokens, state);
    if (!stops)
      return nullptr;

    auto computedColorInterpolationMethod =
      computeGradientColorInterpolationMethod(colorInterpolationMethod, *stops);

    return CSSGradientValue::create(FunctionNotation<Name, CSS::LinearGradient> {
      .parameters = {.colorInterpolationMethod = computedColorInterpolationMethod,
                     .gradientLine = gradientLine.value_or(CSS::Vertical {CSS::Keyword::Bottom {}}),
                     .stops = Krys::Move(*stops)}});
  }

  // MARK: <radial-gradient()> | <repeating-radial-gradient()>
  // https://drafts.csswg.org/css-images-4/#radial-gradients

  template <CSSValueId Name>
  KRYS_NODISCARD static RefPtr<CSSValue> consumeRadialGradient(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state)
  {
    // radial-gradient() = radial-gradient(
    //   [[ <ending-shape> || <size> ]? [ at <position> ]? ] || <color-interpolation-method>,
    //   <color-stop-list>
    // )

    KRYS_NODISCARD constexpr static auto shapeMappings = std::to_array<std::pair<CSSValueId, ShapeKeyword>>({
      {CSSValueCircle, ShapeKeyword::Circle},
      {CSSValueEllipse, ShapeKeyword::Ellipse},
    });
    KRYS_NODISCARD constexpr static SortedArrayMap shapeMap {shapeMappings};

    KRYS_NODISCARD constexpr static auto extentMappings =
      std::to_array<std::pair<CSSValueId, CSS::RadialGradient::Extent>>({
        {CSSValueClosestSide, CSS::RadialGradient::Extent {CSS::Keyword::ClosestSide {}}},
        {CSSValueClosestCorner, CSS::RadialGradient::Extent {CSS::Keyword::ClosestCorner {}}},
        {CSSValueFarthestSide, CSS::RadialGradient::Extent {CSS::Keyword::FarthestSide {}}},
        {CSSValueFarthestCorner, CSS::RadialGradient::Extent {CSS::Keyword::FarthestCorner {}}},
      });
    KRYS_NODISCARD constexpr static SortedArrayMap extentMap {extentMappings};

    KRYS_NODISCARD constexpr static auto defaultExtent =
      CSS::RadialGradient::Extent {CSS::Keyword::FarthestCorner {}};

    Maybe<ColorInterpolationMethod> colorInterpolationMethod;

    if (tokens.Peek().ValueId() == CSSValueIn)
    {
      colorInterpolationMethod = consumeColorInterpolationMethod(tokens, state);
      if (!colorInterpolationMethod)
        return nullptr;
    }

    Maybe<ShapeKeyword> shape;

    using Size = Variant<CSS::RadialGradient::Extent, CSS::Length<NonNegative>,
                         SpaceSeparatedArray<LengthPercentage<NonNegative>, 2>>;
    Maybe<Size> size;

    // First part of grammar, the size/shape clause:
    //
    //   [ [ circle               || <length [0,∞]> ]                          [ at <position> ]? , |
    //     [ ellipse              || <length-percentage [0,∞]>{2} ]            [ at <position> ]? , |
    //     [ [ circle | ellipse ] || <extent-keyword> ]                        [ at <position> ]? , |
    //     at <position> ,
    //   ]?
    for (int i = 0; i < 3; ++i)
    {
      if (tokens.Peek().Type() == CSSTokenType::Ident)
      {
        if (auto peekedShape = peekIdentUsingMapping(tokens, shapeMap))
        {
          if (shape)
            return nullptr;
          shape = *peekedShape;
          tokens.consumeIncludingWhitespace();
        }
        else if (auto peekedExtent = peekIdentUsingMapping(tokens, extentMap))
        {
          if (size)
            return nullptr;
          size = *peekedExtent;
          tokens.consumeIncludingWhitespace();
        }

        if (!shape && !size)
          break;
      }
      else
      {
        auto rangeCopy = tokens;
        auto length1 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(rangeCopy, state);
        if (!length1)
          break;
        if (size)
          return nullptr;
        if (auto length2 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(rangeCopy, state))
        {
          size = SpaceSeparatedArray {Krys::Move(*length1), Krys::Move(*length2)};
          tokens = rangeCopy;

          // Additional increment is necessary since we consumed a second token.
          ++i;
        }
        else
        {
          // Reset to before the first length-percentage, and re-parse to make sure it is a valid <length
          // [0,∞]> production.
          rangeCopy = tokens;
          auto length = MetaConsumer<CSS::Length<NonNegative>>::Consume(rangeCopy, state);
          if (!length)
            return nullptr;
          size = Krys::Move(*length);
          tokens = rangeCopy;
        }
      }
    }

    Maybe<CSS::Position> position;
    if (ConsumeIdent<CSSValueAt>(tokens))
    {
      position = consumePositionUnresolved(tokens, state);
      if (!position)
        return nullptr;
    }

    if ((shape || size || position) && !colorInterpolationMethod && tokens.Peek().ValueId() == CSSValueIn)
    {
      colorInterpolationMethod = consumeColorInterpolationMethod(tokens, state);
      if (!colorInterpolationMethod)
        return nullptr;
    }

    if ((shape || size || position || colorInterpolationMethod) && !ConsumeComma(tokens))
      return nullptr;

    auto stops = consumeLinearColorStopList<SupportsColorHints::Yes>(tokens, state);
    if (!stops)
      return nullptr;

    auto consumeGradientBox = [&] -> Maybe<CSS::RadialGradient::GradientBox>
    {
      if (shape && size)
      {
        switch (*shape)
        {
          case ShapeKeyword::Ellipse:
            return WTF::switchOn(
              Krys::Move(*size),
              [&](CSS::RadialGradient::Extent &&extent) -> Maybe<CSS::RadialGradient::GradientBox>
              {
                return CSS::RadialGradient::Ellipse {
                  .size = Krys::Move(extent),
                  .position = Krys::Move(position),
                };
              },
              [&](CSS::Length<NonNegative> &&) -> Maybe<CSS::RadialGradient::GradientBox>
              {
                // Ellipses must have two length-percentages specified.
                return Null;
              },
              [&](SpaceSeparatedArray<LengthPercentage<NonNegative>, 2> &&size)
                -> Maybe<CSS::RadialGradient::GradientBox>
              {
                return CSS::RadialGradient::Ellipse {
                  .size = Krys::Move(size),
                  .position = Krys::Move(position),
                };
              });

          case ShapeKeyword::Circle:
            return WTF::switchOn(
              Krys::Move(*size),
              [&](CSS::RadialGradient::Extent &&extent) -> Maybe<CSS::RadialGradient::GradientBox>
              {
                return CSS::RadialGradient::Circle {
                  .size = Krys::Move(extent),
                  .position = Krys::Move(position),
                };
              },
              [&](CSS::Length<NonNegative> &&length) -> Maybe<CSS::RadialGradient::GradientBox>
              {
                return CSS::RadialGradient::Circle {
                  .size = Krys::Move(length),
                  .position = Krys::Move(position),
                };
              },
              [&](SpaceSeparatedArray<LengthPercentage<NonNegative>, 2> &&)
                -> Maybe<CSS::RadialGradient::GradientBox>
              {
                // Circles must have a maximum of only one length specified.
                return Null;
              });
        }
      }

      if (shape)
      {
        switch (*shape)
        {
          case ShapeKeyword::Ellipse:
            return CSS::RadialGradient::Ellipse {
              .size = defaultExtent,
              .position = Krys::Move(position),
            };

          case ShapeKeyword::Circle:
            return CSS::RadialGradient::Circle {
              .size = defaultExtent,
              .position = Krys::Move(position),
            };
        }
      }

      if (size)
      {
        return WTF::switchOn(
          Krys::Move(*size),
          [&](CSS::RadialGradient::Extent &&extent) -> Maybe<CSS::RadialGradient::GradientBox>
          {
            return CSS::RadialGradient::Ellipse {
              .size = Krys::Move(extent),
              .position = Krys::Move(position),
            };
          },
          [&](CSS::Length<NonNegative> &&length) -> Maybe<CSS::RadialGradient::GradientBox>
          {
            return CSS::RadialGradient::Circle {
              .size = Krys::Move(length),
              .position = Krys::Move(position),
            };
          },
          [&](SpaceSeparatedArray<LengthPercentage<NonNegative>, 2> &&size)
            -> Maybe<CSS::RadialGradient::GradientBox>
          {
            return CSS::RadialGradient::Ellipse {
              .size = Krys::Move(size),
              .position = Krys::Move(position),
            };
          });
      }

      // If no size is provided, default to an ellipse.
      return CSS::RadialGradient::Ellipse {
        .size = defaultExtent,
        .position = Krys::Move(position),
      };
    };

    auto gradientBox = consumeGradientBox();
    if (!gradientBox)
      return nullptr;

    auto computedColorInterpolationMethod =
      computeGradientColorInterpolationMethod(colorInterpolationMethod, *stops);

    return CSSGradientValue::create(FunctionNotation<Name, CSS::RadialGradient> {
      .parameters = {.colorInterpolationMethod = computedColorInterpolationMethod,
                     .gradientBox = Krys::Move(*gradientBox),
                     .stops = Krys::Move(*stops)}});
  }

  // MARK: <conic-gradient()> | <repeating-conic-gradient()>
  // https://drafts.csswg.org/css-images-4/#conic-gradient-syntax

  template <CSSValueId Name>
  KRYS_NODISCARD static RefPtr<CSSValue> consumeConicGradient(CSSTokenRange &tokens,
                                                              CSSPropertyParserState &state)
  {
    // conic-gradient() = conic-gradient(
    //   [ [ from <angle> ]? [ at <position> ]? ] || <color-interpolation-method>,
    //   <angular-color-stop-list>
    // )

    Maybe<ColorInterpolationMethod> colorInterpolationMethod;

    if (tokens.Peek().ValueId() == CSSValueIn)
    {
      colorInterpolationMethod = consumeColorInterpolationMethod(tokens, state);
      if (!colorInterpolationMethod)
        return nullptr;
    }

    // NOTE: Conic gradient <angle> specifiers accept unitless zero values.
    // https://drafts.csswg.org/css-images-4/#typedef-conic-gradient-syntax

    Maybe<CSS::Angle<>> angle;
    if (ConsumeIdent<CSSValueFrom>(tokens))
    {
      angle =
        MetaConsumer<CSS::Angle<>>::Consume(tokens, state, {.unitlessZeroAngle = UnitlessZeroQuirk::Allow});
      if (!angle)
        return nullptr;
    }

    Maybe<CSS::Position> position;
    if (ConsumeIdent<CSSValueAt>(tokens))
    {
      position = consumePositionUnresolved(tokens, state);
      if (!position)
        return nullptr;
    }

    if ((angle || position) && !colorInterpolationMethod && tokens.Peek().ValueId() == CSSValueIn)
    {
      colorInterpolationMethod = consumeColorInterpolationMethod(tokens, state);
      if (!colorInterpolationMethod)
        return nullptr;
    }

    if (angle || position || colorInterpolationMethod)
    {
      if (!ConsumeComma(tokens))
        return nullptr;
    }

    auto stops = consumeAngularColorStopList<SupportsColorHints::Yes>(tokens, state);
    if (!stops)
      return nullptr;

    auto computedColorInterpolationMethod =
      computeGradientColorInterpolationMethod(colorInterpolationMethod, *stops);

    return CSSGradientValue::create(FunctionNotation<Name, CSS::ConicGradient> {
      .parameters = {
        .colorInterpolationMethod = computedColorInterpolationMethod,
        .gradientBox =
          {
            .angle = Krys::Move(angle),
            .position = Krys::Move(position),
          },
        .stops = Krys::Move(*stops),
      }});
  }

  // MARK: <cross-fade()>

  KRYS_NODISCARD static RefPtr<CSSValue> consumeCrossFade(CSSTokenRange &args, CSSPropertyParserState &state,
                                                          CSSValueId FunctionId)
  {
    // FIXME: The current CSS Images spec has a pretty different construction than is being parsed here:
    //
    //    cross-fade() = cross-fade( <cf-image># )
    //    <cf-image> = <percentage [0,100]>? && [ <image> | <color> ]
    //
    //  https://drafts.csswg.org/css-images-4/#funcdef-cross-fade

    auto fromImageValueOrNone = consumeImageOrNone(args, state);
    if (!fromImageValueOrNone || !ConsumeComma(args))
      return nullptr;
    auto toImageValueOrNone = consumeImageOrNone(args, state);
    if (!toImageValueOrNone || !ConsumeComma(args))
      return nullptr;

    auto value = consumePercentageDividedBy100OrNumber(args, state);
    if (!value)
      return nullptr;

    if (value->isNumber())
    {
      if (auto numberValue = value->resolveAsNumberIfNotCalculated();
          numberValue && (*numberValue < 0 || *numberValue > 1))
        value = CSSPrimitiveValue::create(clampTo<double>(*numberValue, 0, 1));
    }
    return CSSCrossfadeValue::create(fromImageValueOrNone.releaseNonNull(),
                                     toImageValueOrNone.releaseNonNull(), value.releaseNonNull(),
                                     FunctionId == CSSValueWebkitCrossFade);
  }

  // MARK: <-webkit-canvas()>

  KRYS_NODISCARD static RefPtr<CSSValue> consumeWebkitCanvas(CSSTokenRange &args)
  {
    if (args.Peek().Type() != CSSTokenType::Ident)
      return nullptr;
    return CSSCanvasValue::create(args.consumeIncludingWhitespace().value().toString());
  }

  // MARK: <-webkit-named-image()>

  KRYS_NODISCARD static RefPtr<CSSValue> consumeWebkitNamedImage(CSSTokenRange &args)
  {
    if (args.Peek().Type() != CSSTokenType::Ident)
      return nullptr;
    return CSSNamedImageValue::create(args.consumeIncludingWhitespace().value().toString());
  }

  // MARK: <filter()>

  KRYS_NODISCARD static RefPtr<CSSValue> consumeFilterImage(CSSTokenRange &args,
                                                            CSSPropertyParserState &state)
  {
    // FIXME: The current Filter Effects spec has a different construction than is being parsed here:
    //
    //    filter() = filter( [ <image> | <string> ], <filter-value-list> )
    //
    //  https://drafts.fxtf.org/filter-effects/#funcdef-filter
    //
    // Importantly, `none` is not a valid value for either parameter.

    auto imageValueOrNone = consumeImageOrNone(args, state);
    if (!imageValueOrNone || !ConsumeComma(args))
      return nullptr;
    auto filter = consumeUnresolvedFilter(args, state);
    if (!filter)
      return nullptr;
    return CSSFilterImageValue::create(imageValueOrNone.releaseNonNull(), Krys::Move(*filter));
  }

  // MARK: <paint()>
  // https://drafts.css-houdini.org/css-paint-api/#funcdef-paint

  KRYS_NODISCARD static RefPtr<CSSValue> consumeCustomPaint(CSSTokenRange &args,
                                                            CSSPropertyParserState &state)
  {
    if (!state.context.cssPaintingAPIEnabled)
      return nullptr;
    if (args.Peek().Type() != CSSTokenType::Ident)
      return nullptr;
    auto name = args.consumeIncludingWhitespace().value().toString();

    if (!args.IsAtEnd() && args.Peek() != CommaToken)
      return nullptr;
    if (!args.IsAtEnd())
      args.Consume();

    auto argumentList = CSSVariableData::create(args.consumeAll());
    return CSSPaintImageValue::create(name, Krys::Move(argumentList));
  }

  // MARK: <image-set()>

  struct ImageSetTypeFunctionRaw
  {
    String value;

    bool operator==(const ImageSetTypeFunctionRaw &) const = default;
  };
  using ImageSetTypeFunction = ImageSetTypeFunctionRaw;

  struct ImageSetTypeFunctionRawKnownTokenTypeFunctionConsumer
  {
    KRYS_NODISCARD constexpr static CSSParserTokenType tokenType = FunctionToken;

    KRYS_NODISCARD static Maybe<ImageSetTypeFunctionRaw> Consume(CSSTokenRange &tokens,
                                                                 CSSPropertyParserState &,
                                                                 CSSCalcSymbolsAllowed,
                                                                 CSSPropertyParserOptions)
    {
      ASSERT(tokens.Peek().Type() == FunctionToken);
      if (tokens.Peek().FunctionId() != CSSValueType)
        return {};

      auto rangeCopy = tokens;
      auto typeArg = ConsumeFunction(rangeCopy);
      auto result = consumeStringRaw(typeArg);

      if (result.isNull() || !typeArg.IsAtEnd())
        return {};

      tokens = rangeCopy;
      return {{result.toString()}};
    }
  };

  template <>
  struct ConsumerDefinition<ImageSetTypeFunction>
  {
    using FunctionToken = ImageSetTypeFunctionRawKnownTokenTypeFunctionConsumer;
  };

  // MARK: Image Set Resolution + Type Function

  KRYS_NODISCARD static RefPtr<CSSPrimitiveValue>
    consumeImageSetResolutionOrTypeFunction(CSSTokenRange &tokens, CSSPropertyParserState &state)
  {
    // [ <resolution> || type(<string>) ]
    //
    //   as part of
    //
    // <image-set()> = image-set( <image-set-option># )
    // <image-set-option> = [ <image> | <string> ] [ <resolution> || type(<string>) ]?

    return MetaConsumer<CSS::Resolution<>, ImageSetTypeFunction>::Consume(
             tokens, state, [&](const ImageSetTypeFunction &typeFunction) -> RefPtr<CSSPrimitiveValue>
             { return CSSPrimitiveValue::create(typeFunction.value); },
             [&](const CSS::Resolution<> &resolution) -> RefPtr<CSSPrimitiveValue>
             { return CSSPrimitiveValueResolverBase::resolve(resolution); })
      .value_or(nullptr);
  }

  // https://w3c.github.io/csswg-drafts/css-images-4/#image-set-notation
  KRYS_NODISCARD static RefPtr<CSSImageSetOptionValue>
    consumeImageSetOption(CSSTokenRange &tokens, CSSPropertyParserState &state,
                          OptionSet<AllowedImageType> allowedImageTypes)
  {
    auto image = consumeImage(tokens, state, allowedImageTypes);
    if (!image)
      return nullptr;

    auto result = CSSImageSetOptionValue::create(image.releaseNonNull());

    RefPtr<CSSPrimitiveValue> resolution;
    RefPtr<CSSPrimitiveValue> type;

    // Optional resolution and type in any order.
    for (size_t i = 0; i < 2 && !tokens.IsAtEnd(); ++i)
    {
      if (auto optionalArgument = consumeImageSetResolutionOrTypeFunction(tokens, state))
      {
        if ((resolution && optionalArgument->isResolution()) || (type && optionalArgument->isString()))
          return nullptr;

        if (optionalArgument->isResolution())
        {
          resolution = optionalArgument;
          result->setResolution(optionalArgument.releaseNonNull());
          continue;
        }

        if (optionalArgument->isString())
        {
          type = optionalArgument;
          result->setType(type->stringValue());
          continue;
        }
      }
      break;
    }

    if (!tokens.IsAtEnd() && tokens.Peek().Type() != CommaToken)
      return nullptr;
    return result;
  }

  KRYS_NODISCARD static RefPtr<CSSValue> consumeImageSet(CSSTokenRange &args, CSSPropertyParserState &state,
                                                         OptionSet<AllowedImageType> allowedImageTypes)
  {
    CSSValueListBuilder imageSet;
    do
    {
      if (auto option = consumeImageSetOption(args, state, allowedImageTypes))
        imageSet.push_back(option.releaseNonNull());
      else
        return nullptr;
    } while (ConsumeComma(args));

    return CSSImageSetValue::create(Krys::Move(imageSet));
  }

  // MARK: <image>
  // https://drafts.csswg.org/css-images-4/#image-values

  RefPtr<CSSValue> consumeImage(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                OptionSet<AllowedImageType> allowedImageTypes)
  {
    if (tokens.Peek().Type() == StringToken && allowedImageTypes.contains(AllowedImageType::RawStringAsURL))
    {
      auto imageURL = CSS::completeURL(tokens.Peek().value().toAtomString().string(), state.context);
      if (!imageURL)
        return nullptr;
      tokens.consumeIncludingWhitespace();
      return CSSImageValue::create(Krys::Move(*imageURL));
    }

    if (tokens.Peek().Type() == FunctionToken)
    {
      auto consumeGeneratedImage = [&](auto consumer) -> RefPtr<CSSValue>
      {
        if (!allowedImageTypes.contains(AllowedImageType::GeneratedImage))
          return nullptr;
        CSSTokenRange rangeCopy = tokens;
        CSSTokenRange args = ConsumeFunction(rangeCopy);
        RefPtr result = consumer(args);
        if (!result || !args.IsAtEnd())
          return nullptr;
        tokens = rangeCopy;
        return result;
      };

      auto consumeImageSetImage = [&](auto consumer) -> RefPtr<CSSValue>
      {
        if (!allowedImageTypes.contains(AllowedImageType::ImageSet))
          return nullptr;
        CSSTokenRange rangeCopy = tokens;
        CSSTokenRange args = ConsumeFunction(rangeCopy);
        RefPtr result = consumer(args);
        if (!result || !args.IsAtEnd())
          return nullptr;
        tokens = rangeCopy;
        return result;
      };

      auto FunctionId = tokens.Peek().FunctionId();
      switch (FunctionId)
      {
        case CSSValueRadialGradient:
          return consumeGeneratedImage(
            [&](auto &args) { return consumeRadialGradient<CSSValueRadialGradient>(args, state); });
        case CSSValueRepeatingRadialGradient:
          return consumeGeneratedImage(
            [&](auto &args) { return consumeRadialGradient<CSSValueRepeatingRadialGradient>(args, state); });
        case CSSValueWebkitLinearGradient:
          return consumeGeneratedImage(
            [&](auto &args)
            { return consumePrefixedLinearGradient<CSSValueWebkitLinearGradient>(args, state); });
        case CSSValueWebkitRepeatingLinearGradient:
          return consumeGeneratedImage(
            [&](auto &args)
            { return consumePrefixedLinearGradient<CSSValueWebkitRepeatingLinearGradient>(args, state); });
        case CSSValueRepeatingLinearGradient:
          return consumeGeneratedImage(
            [&](auto &args) { return consumeLinearGradient<CSSValueRepeatingLinearGradient>(args, state); });
        case CSSValueLinearGradient:
          return consumeGeneratedImage(
            [&](auto &args) { return consumeLinearGradient<CSSValueLinearGradient>(args, state); });
        case CSSValueWebkitGradient:
          return consumeGeneratedImage([&](auto &args) { return consumeDeprecatedGradient(args, state); });
        case CSSValueWebkitRadialGradient:
          return consumeGeneratedImage(
            [&](auto &args)
            { return consumePrefixedRadialGradient<CSSValueWebkitRadialGradient>(args, state); });
        case CSSValueWebkitRepeatingRadialGradient:
          return consumeGeneratedImage(
            [&](auto &args)
            { return consumePrefixedRadialGradient<CSSValueWebkitRepeatingRadialGradient>(args, state); });
        case CSSValueConicGradient:
          return consumeGeneratedImage([&](auto &args)
                                       { return consumeConicGradient<CSSValueConicGradient>(args, state); });
        case CSSValueRepeatingConicGradient:
          return consumeGeneratedImage(
            [&](auto &args) { return consumeConicGradient<CSSValueRepeatingConicGradient>(args, state); });
        case CSSValueWebkitCrossFade:
          return consumeGeneratedImage([&](auto &args) { return consumeCrossFade(args, state, FunctionId); });
        case CSSValueCrossFade:
          return consumeGeneratedImage([&](auto &args) { return consumeCrossFade(args, state, FunctionId); });
        case CSSValueWebkitCanvas:
          return consumeGeneratedImage([&](auto &args) { return consumeWebkitCanvas(args); });
        case CSSValueWebkitNamedImage:
          return consumeGeneratedImage([&](auto &args) { return consumeWebkitNamedImage(args); });
        case CSSValueWebkitFilter:
        case CSSValueFilter:
          return consumeGeneratedImage([&](auto &args) { return consumeFilterImage(args, state); });
        case CSSValuePaint:
          return consumeGeneratedImage([&](auto &args) { return consumeCustomPaint(args, state); });
        case CSSValueImageSet:
        case CSSValueWebkitImageSet:
          return consumeImageSetImage(
            [&](auto &args)
            {
              return consumeImageSet(args, state,
                                     (allowedImageTypes | AllowedImageType::RawStringAsURL)
                                       - AllowedImageType::ImageSet);
            });
        default: break;
      }
    }

    if (HasFlag(allowedImageTypes, AllowedImageType::URLFunction))
    {
      if (auto imageURL = ConsumeURLRaw(
            tokens, state, AllowedURLModifiers::CrossOrigin | AllowedURLModifiers::ReferrerPolicy))
      {
        return CSSImageValue::create(Krys::Move(*imageURL));
      }
    }

    return nullptr;
  }

  // MARK: <image> | none

  RefPtr<CSSValue> ConsumeImageOrNone(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                      AllowedImageType allowedImageTypes) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    return ConsumeImage(tokens, state, allowedImageTypes);
  }
}