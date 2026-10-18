#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AnglePercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Filter.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Percentage.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ResolutionDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/String.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CanvasValue.hpp"
#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/CSS/Values/Images/Gradient.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetOptionValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageSetValue.hpp"
#include "Krystal.Booey/CSS/Values/ImageValue.hpp"
#include "Krystal.Booey/CSS/Values/NamedImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"
#include "Krystal.Core/Maths/Base.hpp"

namespace krys::boo::css
{
  namespace PropertyParserHelpers
  {
    // Used by radial gradient consumers internally.
    enum class ShapeKeyword : bool
    {
      Circle,
      Ellipse
    };

    // MARK: Keyword mappings

    KRYS_NODISCARD static Maybe<Vertical> VerticalFromValueId(ValueId id) noexcept
    {
      switch (id)
      {
        case ValueId::Top:
        {
          return Vertical {keywords::Top {}};
        }
        case ValueId::Bottom:
        {
          return Vertical {keywords::Bottom {}};
        }
        default:
        {
          return null;
        }
      }
    }

    KRYS_NODISCARD static Maybe<Horizontal> HorizontalFromValueId(ValueId id) noexcept
    {
      switch (id)
      {
        case ValueId::Left:
        {
          return Horizontal {keywords::Left {}};
        }
        case ValueId::Right:
        {
          return Horizontal {keywords::Right {}};
        }
        default:
        {
          return null;
        }
      }
    }

    KRYS_NODISCARD static Maybe<ShapeKeyword> ShapeKeywordFromValueId(ValueId id) noexcept
    {
      switch (id)
      {
        case ValueId::Circle:
        {
          return ShapeKeyword::Circle;
        }
        case ValueId::Ellipse:
        {
          return ShapeKeyword::Ellipse;
        }
        default:
        {
          return null;
        }
      }
    }

    KRYS_NODISCARD static Maybe<RadialGradient::Extent> RadialGradientExtentFromValueId(ValueId id) noexcept
    {
      switch (id)
      {
        case ValueId::ClosestSide:
        {
          return RadialGradient::Extent {keywords::ClosestSide {}};
        }
        case ValueId::ClosestCorner:
        {
          return RadialGradient::Extent {keywords::ClosestCorner {}};
        }
        case ValueId::FarthestSide:
        {
          return RadialGradient::Extent {keywords::FarthestSide {}};
        }
        case ValueId::FarthestCorner:
        {
          return RadialGradient::Extent {keywords::FarthestCorner {}};
        }
        default:
        {
          return null;
        }
      }
    }

    KRYS_NODISCARD static Maybe<PrefixedRadialGradient::Extent>
      PrefixedRadialGradientExtentFromValueId(ValueId id) noexcept
    {
      switch (id)
      {
        case ValueId::Contain:
        {
          return PrefixedRadialGradient::Extent {keywords::Contain {}};
        }
        case ValueId::Cover:
        {
          return PrefixedRadialGradient::Extent {keywords::Cover {}};
        }
        case ValueId::ClosestSide:
        {
          return PrefixedRadialGradient::Extent {keywords::ClosestSide {}};
        }
        case ValueId::ClosestCorner:
        {
          return PrefixedRadialGradient::Extent {keywords::ClosestCorner {}};
        }
        case ValueId::FarthestSide:
        {
          return PrefixedRadialGradient::Extent {keywords::FarthestSide {}};
        }
        case ValueId::FarthestCorner:
        {
          return PrefixedRadialGradient::Extent {keywords::FarthestCorner {}};
        }
        default:
        {
          return null;
        }
      }
    }

    // MARK: Deprecated <gradient> values

    template <ValueId ZeroValue, ValueId OneHundredValue>
    KRYS_NODISCARD static Maybe<NumberOrPercentage<>>
      ConsumeDeprecatedGradientPositionComponent(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      if (tokens.Peek().Type() == TokenType::Ident)
      {
        if (ConsumeIdentRaw<ZeroValue>(tokens))
        {
          return NumberOrPercentage<> {PercentageRaw<> {0}};
        }

        if (ConsumeIdentRaw<OneHundredValue>(tokens))
        {
          return NumberOrPercentage<> {PercentageRaw<> {100}};
        }

        if (ConsumeIdentRaw<ValueId::Center>(tokens))
        {
          return NumberOrPercentage<> {PercentageRaw<> {50}};
        }

        return null;
      }

      return MetaConsumer<Number<>, Percentage<>>::Consume(tokens, state);
    }

    KRYS_NODISCARD static Maybe<DeprecatedGradientPosition>
      ConsumeDeprecatedGradientPosition(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      auto horizontal =
        ConsumeDeprecatedGradientPositionComponent<ValueId::Left, ValueId::Right>(tokens, state);
      if (!horizontal)
      {
        return null;
      }

      auto vertical =
        ConsumeDeprecatedGradientPositionComponent<ValueId::Top, ValueId::Bottom>(tokens, state);
      if (!vertical)
      {
        return null;
      }

      return DeprecatedGradientPosition {krys::move(*horizontal), krys::move(*vertical)};
    }

    KRYS_NODISCARD static Maybe<Color> ConsumeDeprecatedGradientStopColor(TokenRange &tokens,
                                                                          PropertyParserState &state) noexcept
    {
      if (tokens.Peek().ValueId() == ValueId::Currentcolor)
      {
        return null;
      }

      return ConsumeUnresolvedColor(tokens, state);
    }

    KRYS_NODISCARD static Maybe<GradientDeprecatedColorStop>
      ConsumeDeprecatedGradientColorStop(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      auto id = tokens.Peek().FunctionId();
      switch (id)
      {
        case ValueId::From:
        case ValueId::To:
        case ValueId::ColorStop:
        {
          break;
        }
        default:
        {
          return null;
        }
      }

      auto args = ConsumeFunction(tokens);
      Maybe<GradientDeprecatedColorStopPosition> position;
      switch (id)
      {
        case ValueId::From:
        {
          position = GradientDeprecatedColorStopPosition {NumberRaw<> {0}};
          break;
        }
        case ValueId::To:
        {
          position = GradientDeprecatedColorStopPosition {NumberRaw<> {1}};
          break;
        }
        case ValueId::ColorStop:
        {
          auto numberOrPercentage = MetaConsumer<Number<>, Percentage<>>::Consume(args, state);
          if (!numberOrPercentage)
          {
            return null;
          }

          if (!ConsumeComma(args))
          {
            return null;
          }

          position = GradientDeprecatedColorStopPosition {krys::move(*numberOrPercentage)};
          break;
        }
        default:
        {
          krys_debug_assert(false);
          return null;
        }
      }

      auto color = ConsumeDeprecatedGradientStopColor(args, state);
      if (!color || !args.IsAtEnd())
      {
        return null;
      }

      return GradientDeprecatedColorStop {.Color = krys::move(*color), .Position = krys::move(*position)};
    }

    KRYS_NODISCARD static Maybe<GradientDeprecatedColorStopList>
      ConsumeDeprecatedGradientColorStops(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      GradientDeprecatedColorStopList::Container stops;
      while (ConsumeComma(tokens))
      {
        auto stop = ConsumeDeprecatedGradientColorStop(tokens, state);
        if (!stop)
        {
          return null;
        }

        stops.push_back(krys::move(*stop));
      }

      return GradientDeprecatedColorStopList {krys::move(stops)};
    }

    KRYS_NODISCARD static RefPtr<Value> ConsumeDeprecatedLinearGradient(TokenRange &tokens,
                                                                        PropertyParserState &state) noexcept
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

      return GradientValue::Create(
        FunctionNotation<ValueId::WebkitGradient, DeprecatedLinearGradient> {DeprecatedLinearGradient {
          .ColourInterpolationMethod =
            GradientColorInterpolationMethod::LegacyMethod(AlphaPremultiplication::Premultiplied),
          .GradientLine = {krys::move(*first), krys::move(*second)},
          .Stops = krys::move(*stops),
        }});
    }

    KRYS_NODISCARD static RefPtr<Value> ConsumeDeprecatedRadialGradient(TokenRange &tokens,
                                                                        PropertyParserState &state) noexcept
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

      return GradientValue::Create(
        FunctionNotation<ValueId::WebkitGradient, DeprecatedRadialGradient> {DeprecatedRadialGradient {
          .ColourInterpolationMethod =
            GradientColorInterpolationMethod::LegacyMethod(AlphaPremultiplication::Premultiplied),
          .GradientBox =
            {
              .First = krys::move(*first),
              .FirstRadius = krys::move(*firstRadius),
              .Second = krys::move(*second),
              .SecondRadius = krys::move(*secondRadius),
            },
          .Stops = krys::move(*stops),
        }});
    }

    KRYS_NODISCARD static RefPtr<Value> ConsumeDeprecatedGradient(TokenRange &tokens,
                                                                  PropertyParserState &state) noexcept
    {
      switch (tokens.ConsumeIncludingWhitespace().ValueId())
      {
        case ValueId::Linear:
        {
          return ConsumeDeprecatedLinearGradient(tokens, state);
        }
        case ValueId::Radial:
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

    KRYS_NODISCARD static Maybe<Color> ConsumeStopColor(TokenRange &tokens,
                                                        PropertyParserState &state) noexcept
    {
      return ConsumeUnresolvedColor(tokens, state);
    }

    template <SupportsColorHints TSupportsColorHints, typename TStop, typename TConsumer>
    KRYS_NODISCARD static Maybe<GradientColorStopList<TStop>>
      ConsumeColorStopList(TokenRange &tokens, PropertyParserState &state,
                           TConsumer &&consumeStopPosition) noexcept
    {
      typename GradientColorStopList<TStop>::Container stops;

      // The first color stop cannot be a color hint.
      bool previousStopWasColorHint = true;
      do
      {
        TStop stop {ConsumeStopColor(tokens, state), consumeStopPosition(tokens)};
        if (!stop.Color && !stop.Position)
        {
          return null;
        }

        // Two color hints in a row are not allowed.
        if (!stop.Color && (TSupportsColorHints == SupportsColorHints::No || previousStopWasColorHint))
        {
          return null;
        }
        previousStopWasColorHint = !stop.Color;

        // Stops with both a color and a position can have a second position, which shares the same color.
        if (stop.Color && stop.Position)
        {
          if (auto secondPosition = consumeStopPosition(tokens))
          {
            stops.push_back(stop);
            stop.Position = krys::move(secondPosition);
          }
        }
        stops.push_back(krys::move(stop));
      } while (ConsumeComma(tokens));

      // The last color stop cannot be a color hint.
      if (previousStopWasColorHint)
      {
        return null;
      }

      // Must have at least one stop to be valid.
      if (stops.empty())
      {
        return null;
      }

      return GradientColorStopList<TStop> {krys::move(stops)};
    }

    template <SupportsColorHints TSupportsColorHints>
    KRYS_NODISCARD static Maybe<GradientLinearColorStopList>
      ConsumeLinearColorStopList(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      return ConsumeColorStopList<TSupportsColorHints, GradientLinearColorStop>(
        tokens, state,
        [&](TokenRange &tokens) { return MetaConsumer<LengthPercentage<>>::Consume(tokens, state); });
    }

    template <SupportsColorHints TSupportsColorHints>
    KRYS_NODISCARD static Maybe<GradientAngularColorStopList>
      ConsumeAngularColorStopList(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      // NOTE: Angular color stops accept unitless zero values.
      // https://drafts.csswg.org/css-images-4/#typedef-color-stop-angle

      return ConsumeColorStopList<TSupportsColorHints, GradientAngularColorStop>(
        tokens, state,
        [&](TokenRange &tokens)
        {
          return MetaConsumer<AnglePercentage<>>::Consume(tokens, state,
                                                          {.UnitlessZeroAngle = AllowUnitlessZero(true)});
        });
    }

    KRYS_NODISCARD static bool StopColorIs8Bit(const Color &color) noexcept
    {
      return color.IsKeyword() || color.GetAbsoluteColor().TryGetAsSRGBABytes();
    }

    KRYS_NODISCARD static bool StopColorIs8Bit(const Markable<Color> &color) noexcept
    {
      return !color || StopColorIs8Bit(*color);
    }

    template <typename TStop>
    KRYS_NODISCARD static GradientColorInterpolationMethod
      ComputeGradientColorInterpolationMethod(Maybe<ColourInterpolationMethod> parsedColorInterpolationMethod,
                                              const GradientColorStopList<TStop> &stops) noexcept
    {
      // We detect whether stops use legacy vs. non-legacy CSS color syntax using the following rules:
      //  - A ValueId is always considered legacy since all keyword based colors are considered legacy by the
      //    spec.
      //  - An actual Color value is considered legacy if it is stored as 8-bit sRGB.
      //
      // While this is accurate now, we should consider a more robust mechanism to detect this at parse
      // time, perhaps keeping this information in the PrimitiveValue itself.

      auto defaultColorInterpolationMethod = GradientColorInterpolationMethod::Default::SRGB;
      for (auto &stop : stops)
      {
        if (StopColorIs8Bit(stop.Color))
        {
          continue;
        }

        defaultColorInterpolationMethod = GradientColorInterpolationMethod::Default::OKLab;
        break;
      }

      if (parsedColorInterpolationMethod)
      {
        return {*parsedColorInterpolationMethod, defaultColorInterpolationMethod};
      }

      switch (defaultColorInterpolationMethod)
      {
        case GradientColorInterpolationMethod::Default::SRGB:
        {
          return {{ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied},
                  defaultColorInterpolationMethod};
        }
        case GradientColorInterpolationMethod::Default::OKLab:
        {
          return {{ColourInterpolationMethod::OKLab {}, AlphaPremultiplication::Premultiplied},
                  defaultColorInterpolationMethod};
        }
      }

      krys_debug_assert(false);
      return {{ColourInterpolationMethod::SRGB {}, AlphaPremultiplication::Premultiplied},
              defaultColorInterpolationMethod};
    }

    // MARK: Compat <gradient> values
    // https://compat.spec.whatwg.org/#css-gradient-fns

    // MARK: <-webkit-linear-gradient()> | <-webkit-repeating-linear-gradient()>
    // https://compat.spec.whatwg.org/#css-gradients-webkit-linear-gradient

    template <ValueId Name>
    KRYS_NODISCARD static RefPtr<Value> ConsumePrefixedLinearGradient(TokenRange &tokens,
                                                                      PropertyParserState &state) noexcept
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

      using GradientLine = PrefixedLinearGradient::GradientLineType;

      auto consumeKeywordGradientLineKnownHorizontal = [&](TokenRange &tokens,
                                                           Horizontal knownHorizontal) -> GradientLine
      {
        if (auto vertical = ConsumeIdentUsingMapping(tokens, VerticalFromValueId))
        {
          return SpaceSeparatedTuple<Horizontal, Vertical> {knownHorizontal, *vertical};
        }
        return knownHorizontal;
      };

      auto consumeKeywordGradientLineKnownVertical = [&](TokenRange &tokens,
                                                         Vertical knownVertical) -> GradientLine
      {
        if (auto horizontal = ConsumeIdentUsingMapping(tokens, HorizontalFromValueId))
        {
          return SpaceSeparatedTuple<Horizontal, Vertical> {*horizontal, knownVertical};
        }
        return knownVertical;
      };

      auto consumeKeywordGradientLine = [&](TokenRange &tokens) -> Maybe<GradientLine>
      {
        switch (tokens.Peek().ValueId())
        {
          case ValueId::Left:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownHorizontal(tokens, Horizontal {keywords::Left {}});
          }
          case ValueId::Right:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownHorizontal(tokens, Horizontal {keywords::Right {}});
          }
          case ValueId::Top:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownVertical(tokens, Vertical {keywords::Top {}});
          }
          case ValueId::Bottom:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownVertical(tokens, Vertical {keywords::Bottom {}});
          }
          default:
          {
            return null;
          }
        }
      };

      Maybe<GradientLine> gradientLine;

      // NOTE: Linear gradient <angle> specifiers accept unitless zero values.
      // https://drafts.csswg.org/css-images-4/#typedef-linear-gradient-syntax

      if (auto angle =
            MetaConsumer<Angle<>>::Consume(tokens, state, {.UnitlessZeroAngle = AllowUnitlessZero(true)}))
      {
        gradientLine = GradientLine {krys::move(*angle)};
        if (!ConsumeComma(tokens))
        {
          return nullptr;
        }
      }
      else if (auto keywordGradientLine = consumeKeywordGradientLine(tokens))
      {
        gradientLine = krys::move(*keywordGradientLine);
        if (!ConsumeComma(tokens))
        {
          return nullptr;
        }
      }

      auto stops = ConsumeLinearColorStopList<SupportsColorHints::No>(tokens, state);
      if (!stops)
      {
        return nullptr;
      }

      return GradientValue::Create(FunctionNotation<Name, PrefixedLinearGradient> {PrefixedLinearGradient {
        .ColourInterpolationMethod =
          GradientColorInterpolationMethod::LegacyMethod(AlphaPremultiplication::Premultiplied),
        .GradientLine = gradientLine.value_or(GradientLine {Vertical {keywords::Top {}}}),
        .Stops = krys::move(*stops),
      }});
    }

    // MARK: <-webkit-radial-gradient()> | <-webkit-repeating-radial-gradient()>
    // https://compat.spec.whatwg.org/#css-gradients-webkit-radial-gradient

    template <ValueId Name>
    KRYS_NODISCARD static RefPtr<Value> ConsumePrefixedRadialGradient(TokenRange &tokens,
                                                                      PropertyParserState &state) noexcept
    {
      // https://compat.spec.whatwg.org/#css-gradients-webkit-radial-gradient/ states that
      // -webkit-radial-gradient() and -webkit-repeating-radial-gradient() must be "treated as an alias of
      // radial-gradient as defined in [css3-images-20110217]." In [css3-images-20110217] the grammar was
      // defined as:
      //
      //   <radial-gradient> = radial-gradient([<bg-position>,]? [ [<shape> || <size>] | [<length> |
      //   <percentage>]{2} ,]? <color-stop>[, <color-stop>]+);
      //   <shape> = [ circle | ellipse ]
      //   <size> = [ closest-side | closest-corner | farthest-side | farthest-corner | contain | cover ]
      //
      //      defaults to ‘ellipse cover’.
      //
      // see https://www.w3.org/TR/2011/WD-css3-images-20110217/#radial-gradients.

      auto position = ConsumeOneOrTwoComponentPositionUnresolved(tokens, state);
      if (position)
      {
        if (!ConsumeComma(tokens))
        {
          return nullptr;
        }
      }

      Maybe<ShapeKeyword> shapeKeyword;
      Maybe<PrefixedRadialGradient::Extent> extentKeyword;
      if (tokens.Peek().Type() == TokenType::Ident)
      {
        shapeKeyword = ConsumeIdentUsingMapping(tokens, ShapeKeywordFromValueId);
        extentKeyword = ConsumeIdentUsingMapping(tokens, PrefixedRadialGradientExtentFromValueId);
        if (!shapeKeyword)
        {
          shapeKeyword = ConsumeIdentUsingMapping(tokens, ShapeKeywordFromValueId);
        }

        if (shapeKeyword || extentKeyword)
        {
          if (!ConsumeComma(tokens))
          {
            return nullptr;
          }
        }
      }

      auto consumeGradientBox = [&] -> Maybe<PrefixedRadialGradient::GradientBoxType>
      {
        if (shapeKeyword && extentKeyword)
        {
          switch (*shapeKeyword)
          {
            case ShapeKeyword::Ellipse:
            {
              return PrefixedRadialGradient::Ellipse {
                .Size = *extentKeyword,
                .Position = krys::move(position),
              };
            }
            case ShapeKeyword::Circle:
            {
              return PrefixedRadialGradient::Circle {
                .Size = *extentKeyword,
                .Position = krys::move(position),
              };
            }
          }
        }

        if (shapeKeyword)
        {
          switch (*shapeKeyword)
          {
            case ShapeKeyword::Ellipse:
            {
              return PrefixedRadialGradient::Ellipse {
                .Size = null,
                .Position = krys::move(position),
              };
            }
            case ShapeKeyword::Circle:
            {
              return PrefixedRadialGradient::Circle {
                .Size = null,
                .Position = krys::move(position),
              };
            }
          }
        }

        if (extentKeyword)
        {
          return PrefixedRadialGradient::Ellipse {
            .Size = *extentKeyword,
            .Position = krys::move(position),
          };
        }

        if (auto length1 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state))
        {
          auto length2 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state);
          if (!length2)
          {
            return null;
          }

          if (!ConsumeComma(tokens))
          {
            return null;
          }

          return PrefixedRadialGradient::Ellipse {
            .Size = PrefixedRadialGradient::Ellipse::SizeType {krys::move(*length1), krys::move(*length2)},
            .Position = krys::move(position),
          };
        }

        // If no size is provided, default to an ellipse.
        return PrefixedRadialGradient::Ellipse {
          .Size = null,
          .Position = krys::move(position),
        };
      };

      auto gradientBox = consumeGradientBox();
      if (!gradientBox)
      {
        return nullptr;
      }

      auto stops = ConsumeLinearColorStopList<SupportsColorHints::No>(tokens, state);
      if (!stops)
      {
        return nullptr;
      }

      return GradientValue::Create(FunctionNotation<Name, PrefixedRadialGradient> {PrefixedRadialGradient {
        .ColourInterpolationMethod =
          GradientColorInterpolationMethod::LegacyMethod(AlphaPremultiplication::Premultiplied),
        .GradientBox = krys::move(*gradientBox),
        .Stops = krys::move(*stops),
      }});
    }

    // MARK: Standard <gradient> values

    // MARK: <linear-gradient()> | <repeating-linear-gradient()>
    // https://drafts.csswg.org/css-images-4/#linear-gradients

    template <ValueId Name>
    KRYS_NODISCARD static RefPtr<Value> ConsumeLinearGradient(TokenRange &tokens,
                                                              PropertyParserState &state) noexcept
    {
      // <side-or-corner> = [left | right] || [top | bottom]
      // linear-gradient() = linear-gradient(
      //   [ <angle> | to <side-or-corner> ]? || <color-interpolation-method>,
      //   <color-stop-list>
      // )

      using GradientLine = LinearGradient::GradientLineType;

      auto consumeKeywordGradientLineKnownHorizontal = [&](TokenRange &tokens,
                                                           Horizontal knownHorizontal) -> GradientLine
      {
        if (auto vertical = ConsumeIdentUsingMapping(tokens, VerticalFromValueId))
        {
          return SpaceSeparatedTuple<Horizontal, Vertical> {knownHorizontal, *vertical};
        }
        return knownHorizontal;
      };

      auto consumeKeywordGradientLineKnownVertical = [&](TokenRange &tokens,
                                                         Vertical knownVertical) -> GradientLine
      {
        if (auto horizontal = ConsumeIdentUsingMapping(tokens, HorizontalFromValueId))
        {
          return SpaceSeparatedTuple<Horizontal, Vertical> {*horizontal, knownVertical};
        }
        return knownVertical;
      };

      auto consumeKeywordGradientLine = [&](TokenRange &tokens) -> Maybe<GradientLine>
      {
        krys_debug_assert(tokens.Peek().ValueId() == ValueId::To);
        tokens.Discard();
        tokens.DiscardWhitespace();

        switch (tokens.Peek().ValueId())
        {
          case ValueId::Left:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownHorizontal(tokens, Horizontal {keywords::Left {}});
          }
          case ValueId::Right:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownHorizontal(tokens, Horizontal {keywords::Right {}});
          }
          case ValueId::Top:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownVertical(tokens, Vertical {keywords::Top {}});
          }
          case ValueId::Bottom:
          {
            tokens.Discard();
            tokens.DiscardWhitespace();
            return consumeKeywordGradientLineKnownVertical(tokens, Vertical {keywords::Bottom {}});
          }
          default:
          {
            return null;
          }
        }
      };

      Maybe<ColourInterpolationMethod> colorInterpolationMethod;

      if (tokens.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(tokens, state);
        if (!colorInterpolationMethod)
        {
          return nullptr;
        }
      }

      Maybe<GradientLine> gradientLine;

      // NOTE: Linear gradient <angle> specifiers accept unitless zero values.
      // https://drafts.csswg.org/css-images-4/#typedef-linear-gradient-syntax

      if (auto angle =
            MetaConsumer<Angle<>>::Consume(tokens, state, {.UnitlessZeroAngle = AllowUnitlessZero(true)}))
      {
        gradientLine = GradientLine {krys::move(*angle)};
      }
      else if (tokens.Peek().ValueId() == ValueId::To)
      {
        auto keywordGradientLine = consumeKeywordGradientLine(tokens);
        if (!keywordGradientLine)
        {
          return nullptr;
        }
        gradientLine = krys::move(*keywordGradientLine);
      }

      if (gradientLine && !colorInterpolationMethod && tokens.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(tokens, state);
        if (!colorInterpolationMethod)
        {
          return nullptr;
        }
      }

      if (gradientLine || colorInterpolationMethod)
      {
        if (!ConsumeComma(tokens))
        {
          return nullptr;
        }
      }

      auto stops = ConsumeLinearColorStopList<SupportsColorHints::Yes>(tokens, state);
      if (!stops)
      {
        return nullptr;
      }

      auto computedColorInterpolationMethod =
        ComputeGradientColorInterpolationMethod(colorInterpolationMethod, *stops);

      return GradientValue::Create(FunctionNotation<Name, LinearGradient> {LinearGradient {
        .ColourInterpolationMethod = computedColorInterpolationMethod,
        .GradientLine = gradientLine.value_or(GradientLine {Vertical {keywords::Bottom {}}}),
        .Stops = krys::move(*stops),
      }});
    }

    // MARK: <radial-gradient()> | <repeating-radial-gradient()>
    // https://drafts.csswg.org/css-images-4/#radial-gradients

    template <ValueId Name>
    KRYS_NODISCARD static RefPtr<Value> ConsumeRadialGradient(TokenRange &tokens,
                                                              PropertyParserState &state) noexcept
    {
      // radial-gradient() = radial-gradient(
      //   [[ <ending-shape> || <size> ]? [ at <position> ]? ] || <color-interpolation-method>,
      //   <color-stop-list>
      // )

      const auto defaultExtent = RadialGradient::Extent {keywords::FarthestCorner {}};

      Maybe<ColourInterpolationMethod> colorInterpolationMethod;

      if (tokens.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(tokens, state);
        if (!colorInterpolationMethod)
        {
          return nullptr;
        }
      }

      Maybe<ShapeKeyword> shape;

      using EllipseSize = RadialGradient::Ellipse::SizeType;
      using Size = Variant<RadialGradient::Extent, Length<NonNegative>, EllipseSize>;
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
        if (tokens.Peek().Type() == TokenType::Ident)
        {
          if (auto peekedShape = ShapeKeywordFromValueId(tokens.Peek().ValueId()))
          {
            if (shape)
            {
              return nullptr;
            }
            shape = *peekedShape;
            tokens.Discard();
            tokens.DiscardWhitespace();
          }
          else if (auto peekedExtent = RadialGradientExtentFromValueId(tokens.Peek().ValueId()))
          {
            if (size)
            {
              return nullptr;
            }
            size = Size {*peekedExtent};
            tokens.Discard();
            tokens.DiscardWhitespace();
          }

          if (!shape && !size)
          {
            break;
          }
        }
        else
        {
          auto rangeCopy = tokens;
          auto length1 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(rangeCopy, state);
          if (!length1)
          {
            break;
          }

          if (size)
          {
            return nullptr;
          }

          if (auto length2 = MetaConsumer<LengthPercentage<NonNegative>>::Consume(rangeCopy, state))
          {
            size = Size {EllipseSize {krys::move(*length1), krys::move(*length2)}};
            tokens = rangeCopy;

            // Additional increment is necessary since we consumed a second token.
            ++i;
          }
          else
          {
            // Reset to before the first length-percentage, and re-parse to make sure it is a valid
            // <length [0,∞]> production.
            rangeCopy = tokens;
            auto length = MetaConsumer<Length<NonNegative>>::Consume(rangeCopy, state);
            if (!length)
            {
              return nullptr;
            }
            size = Size {krys::move(*length)};
            tokens = rangeCopy;
          }
        }
      }

      Maybe<Position> position;
      if (ConsumeIdentRaw<ValueId::At>(tokens))
      {
        position = ConsumePositionUnresolved(tokens, state);
        if (!position)
        {
          return nullptr;
        }
      }

      if ((shape || size || position) && !colorInterpolationMethod && tokens.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(tokens, state);
        if (!colorInterpolationMethod)
        {
          return nullptr;
        }
      }

      if ((shape || size || position || colorInterpolationMethod) && !ConsumeComma(tokens))
      {
        return nullptr;
      }

      auto stops = ConsumeLinearColorStopList<SupportsColorHints::Yes>(tokens, state);
      if (!stops)
      {
        return nullptr;
      }

      using GradientBox = RadialGradient::GradientBoxType;

      auto consumeGradientBox = [&] -> Maybe<GradientBox>
      {
        if (shape && size)
        {
          switch (*shape)
          {
            case ShapeKeyword::Ellipse:
            {
              return SwitchOn(
                krys::move(*size),
                [&](RadialGradient::Extent &&extent) -> Maybe<GradientBox>
                {
                  return RadialGradient::Ellipse {
                    .Size = krys::move(extent),
                    .Position = krys::move(position),
                  };
                },
                [&](Length<NonNegative> &&) -> Maybe<GradientBox>
                {
                  // Ellipses must have two length-percentages specified.
                  return null;
                },
                [&](EllipseSize &&size) -> Maybe<GradientBox>
                {
                  return RadialGradient::Ellipse {
                    .Size = krys::move(size),
                    .Position = krys::move(position),
                  };
                });
            }
            case ShapeKeyword::Circle:
            {
              return SwitchOn(
                krys::move(*size),
                [&](RadialGradient::Extent &&extent) -> Maybe<GradientBox>
                {
                  return RadialGradient::Circle {
                    .Size = krys::move(extent),
                    .Position = krys::move(position),
                  };
                },
                [&](Length<NonNegative> &&length) -> Maybe<GradientBox>
                {
                  return RadialGradient::Circle {
                    .Size = krys::move(length),
                    .Position = krys::move(position),
                  };
                },
                [&](EllipseSize &&) -> Maybe<GradientBox>
                {
                  // Circles must have a maximum of only one length specified.
                  return null;
                });
            }
          }
        }

        if (shape)
        {
          switch (*shape)
          {
            case ShapeKeyword::Ellipse:
            {
              return RadialGradient::Ellipse {
                .Size = defaultExtent,
                .Position = krys::move(position),
              };
            }
            case ShapeKeyword::Circle:
            {
              return RadialGradient::Circle {
                .Size = defaultExtent,
                .Position = krys::move(position),
              };
            }
          }
        }

        if (size)
        {
          return SwitchOn(
            krys::move(*size),
            [&](RadialGradient::Extent &&extent) -> Maybe<GradientBox>
            {
              return RadialGradient::Ellipse {
                .Size = krys::move(extent),
                .Position = krys::move(position),
              };
            },
            [&](Length<NonNegative> &&length) -> Maybe<GradientBox>
            {
              return RadialGradient::Circle {
                .Size = krys::move(length),
                .Position = krys::move(position),
              };
            },
            [&](EllipseSize &&size) -> Maybe<GradientBox>
            {
              return RadialGradient::Ellipse {
                .Size = krys::move(size),
                .Position = krys::move(position),
              };
            });
        }

        // If no size is provided, default to an ellipse.
        return RadialGradient::Ellipse {
          .Size = defaultExtent,
          .Position = krys::move(position),
        };
      };

      auto gradientBox = consumeGradientBox();
      if (!gradientBox)
      {
        return nullptr;
      }

      auto computedColorInterpolationMethod =
        ComputeGradientColorInterpolationMethod(colorInterpolationMethod, *stops);

      return GradientValue::Create(FunctionNotation<Name, RadialGradient> {RadialGradient {
        .ColourInterpolationMethod = computedColorInterpolationMethod,
        .GradientBox = krys::move(*gradientBox),
        .Stops = krys::move(*stops),
      }});
    }

    // MARK: <conic-gradient()> | <repeating-conic-gradient()>
    // https://drafts.csswg.org/css-images-4/#conic-gradient-syntax

    template <ValueId Name>
    KRYS_NODISCARD static RefPtr<Value> ConsumeConicGradient(TokenRange &tokens,
                                                             PropertyParserState &state) noexcept
    {
      // conic-gradient() = conic-gradient(
      //   [ [ from <angle> ]? [ at <position> ]? ] || <color-interpolation-method>,
      //   <angular-color-stop-list>
      // )

      Maybe<ColourInterpolationMethod> colorInterpolationMethod;

      if (tokens.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(tokens, state);
        if (!colorInterpolationMethod)
        {
          return nullptr;
        }
      }

      // NOTE: Conic gradient <angle> specifiers accept unitless zero values.
      // https://drafts.csswg.org/css-images-4/#typedef-conic-gradient-syntax

      Maybe<Angle<>> angle;
      if (ConsumeIdentRaw<ValueId::From>(tokens))
      {
        angle = MetaConsumer<Angle<>>::Consume(tokens, state, {.UnitlessZeroAngle = AllowUnitlessZero(true)});
        if (!angle)
        {
          return nullptr;
        }
      }

      Maybe<Position> position;
      if (ConsumeIdentRaw<ValueId::At>(tokens))
      {
        position = ConsumePositionUnresolved(tokens, state);
        if (!position)
        {
          return nullptr;
        }
      }

      if ((angle || position) && !colorInterpolationMethod && tokens.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(tokens, state);
        if (!colorInterpolationMethod)
        {
          return nullptr;
        }
      }

      if (angle || position || colorInterpolationMethod)
      {
        if (!ConsumeComma(tokens))
        {
          return nullptr;
        }
      }

      auto stops = ConsumeAngularColorStopList<SupportsColorHints::Yes>(tokens, state);
      if (!stops)
      {
        return nullptr;
      }

      auto computedColorInterpolationMethod =
        ComputeGradientColorInterpolationMethod(colorInterpolationMethod, *stops);

      return GradientValue::Create(FunctionNotation<Name, ConicGradient> {ConicGradient {
        .ColourInterpolationMethod = computedColorInterpolationMethod,
        .GradientBox =
          {
            .Angle = krys::move(angle),
            .Position = krys::move(position),
          },
        .Stops = krys::move(*stops),
      }});
    }

    // MARK: <cross-fade()>

    KRYS_NODISCARD static RefPtr<Value> ConsumeCrossFade(TokenRange &args, PropertyParserState &state,
                                                         ValueId functionId) noexcept
    {
      // FIXME: The current CSS Images spec has a pretty different construction than is being parsed here:
      //
      //    cross-fade() = cross-fade( <cf-image># )
      //    <cf-image> = <percentage [0,100]>? && [ <image> | <color> ]
      //
      //  https://drafts.csswg.org/css-images-4/#funcdef-cross-fade

      auto fromImageValueOrNone = ConsumeImageOrNone(args, state);
      if (!fromImageValueOrNone || !ConsumeComma(args))
      {
        return nullptr;
      }

      auto toImageValueOrNone = ConsumeImageOrNone(args, state);
      if (!toImageValueOrNone || !ConsumeComma(args))
      {
        return nullptr;
      }

      auto value = ConsumePercentageDividedBy100OrNumber(args, state);
      if (!value)
      {
        return nullptr;
      }

      if (value->IsNumber())
      {
        if (auto numberValue = value->ResolveAsNumberIfNotCalculated();
            numberValue && (*numberValue < 0 || *numberValue > 1))
        {
          value = PrimitiveValue::Create(ClampTo<double>(*numberValue, 0.0, 1.0));
        }
      }

      return CrossFadeValue::Create(krys::move(fromImageValueOrNone), krys::move(toImageValueOrNone),
                                    krys::move(value), functionId == ValueId::WebkitCrossFade);
    }

    // MARK: <-webkit-canvas()>

    KRYS_NODISCARD static RefPtr<Value> ConsumeWebkitCanvas(TokenRange &args) noexcept
    {
      if (args.Peek().Type() != TokenType::Ident)
      {
        return nullptr;
      }

      return CanvasValue::Create(CSSOMString(args.ConsumeIncludingWhitespace().IdentCodePoints()));
    }

    // MARK: <-webkit-named-image()>

    KRYS_NODISCARD static RefPtr<Value> ConsumeWebkitNamedImage(TokenRange &args) noexcept
    {
      if (args.Peek().Type() != TokenType::Ident)
      {
        return nullptr;
      }

      return NamedImageValue::Create(CSSOMString(args.ConsumeIncludingWhitespace().IdentCodePoints()));
    }

    // MARK: <filter()>

    KRYS_NODISCARD static RefPtr<Value> ConsumeFilterImage(TokenRange &args,
                                                           PropertyParserState &state) noexcept
    {
      // FIXME: The current Filter Effects spec has a different construction than is being parsed here:
      //
      //    filter() = filter( [ <image> | <string> ], <filter-value-list> )
      //
      //  https://drafts.fxtf.org/filter-effects/#funcdef-filter
      //
      // Importantly, `none` is not a valid value for either parameter.

      auto imageValueOrNone = ConsumeImageOrNone(args, state);
      if (!imageValueOrNone || !ConsumeComma(args))
      {
        return nullptr;
      }

      auto filter = ConsumeUnresolvedFilter(args, state);
      if (!filter)
      {
        return nullptr;
      }

      return FilterImageValue::Create(krys::move(imageValueOrNone), krys::move(*filter));
    }

    // MARK: <paint()>
    // https://drafts.css-houdini.org/css-paint-api/#funcdef-paint

    KRYS_NODISCARD static RefPtr<Value> ConsumeCustomPaint(TokenRange &args, PropertyParserState &) noexcept
    {
      if (args.Peek().Type() != TokenType::Ident)
      {
        return nullptr;
      }

      auto name = CSSOMString(args.ConsumeIncludingWhitespace().IdentCodePoints());

      if (!args.IsAtEnd() && args.Peek().Type() != TokenType::Comma)
      {
        return nullptr;
      }

      if (!args.IsAtEnd())
      {
        args.Discard();
      }

      auto argumentList = VariableData::Create(args.ConsumeAll());
      return PaintImageValue::Create(krys::move(name), krys::move(argumentList));
    }

    // MARK: <image-set()>

    struct ImageSetTypeFunctionRaw
    {
      CSSOMString Value;

      KRYS_NODISCARD bool operator==(const ImageSetTypeFunctionRaw &) const noexcept = default;
    };
    using ImageSetTypeFunction = ImageSetTypeFunctionRaw;

    struct ImageSetTypeFunctionRawKnownTokenTypeFunctionConsumer
    {
      constexpr static TokenType TokenType = TokenType::Function;

      KRYS_NODISCARD static Maybe<ImageSetTypeFunctionRaw>
        Consume(TokenRange &tokens, PropertyParserState &, AllowedSymbols, PropertyParserOptions) noexcept
      {
        krys_debug_assert(tokens.Peek().Type() == TokenType::Function);
        if (tokens.Peek().FunctionId() != ValueId::Type)
        {
          return null;
        }

        auto rangeCopy = tokens;
        auto typeArg = ConsumeFunction(rangeCopy);
        if (typeArg.Peek().Type() != TokenType::String)
        {
          return null;
        }

        auto result = ConsumeStringRaw(typeArg);
        if (!typeArg.IsAtEnd())
        {
          return null;
        }

        tokens = rangeCopy;
        return ImageSetTypeFunctionRaw {CSSOMString(result)};
      }
    };
  }

  template <>
  struct ConsumerDefinition<PropertyParserHelpers::ImageSetTypeFunction>
  {
    using FunctionToken = PropertyParserHelpers::ImageSetTypeFunctionRawKnownTokenTypeFunctionConsumer;
  };

  namespace PropertyParserHelpers
  {
    // MARK: Image Set Resolution + Type Function

    KRYS_NODISCARD static RefPtr<PrimitiveValue>
      ConsumeImageSetResolutionOrTypeFunction(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      // [ <resolution> || type(<string>) ]
      //
      //   as part of
      //
      // <image-set()> = image-set( <image-set-option># )
      // <image-set-option> = [ <image> | <string> ] [ <resolution> || type(<string>) ]?

      return MetaConsumer<Resolution<>, ImageSetTypeFunction>::Consume(
               tokens, state, [&](const ImageSetTypeFunction &typeFunction) -> RefPtr<PrimitiveValue>
               { return PrimitiveValue::Create(typeFunction.Value); },
               [&](const Resolution<> &resolution) -> RefPtr<PrimitiveValue>
               { return PrimitiveValueResolverBase::Resolve(resolution); })
        .value_or(nullptr);
    }

    // https://w3c.github.io/csswg-drafts/css-images-4/#image-set-notation
    KRYS_NODISCARD static RefPtr<ImageSetOptionValue>
      ConsumeImageSetOption(TokenRange &tokens, PropertyParserState &state,
                            AllowedImageType allowedImageTypes) noexcept
    {
      auto image = ConsumeImage(tokens, state, allowedImageTypes);
      if (!image)
      {
        return nullptr;
      }

      auto result = ImageSetOptionValue::Create(krys::move(image));

      RefPtr<PrimitiveValue> resolution;
      RefPtr<PrimitiveValue> type;

      // Optional resolution and type in any order.
      for (size_t i = 0; i < 2 && !tokens.IsAtEnd(); ++i)
      {
        if (auto optionalArgument = ConsumeImageSetResolutionOrTypeFunction(tokens, state))
        {
          if ((resolution && optionalArgument->IsResolution()) || (type && optionalArgument->IsString()))
          {
            return nullptr;
          }

          if (optionalArgument->IsResolution())
          {
            resolution = optionalArgument;
            result->SetResolution(krys::move(optionalArgument));
            continue;
          }

          if (optionalArgument->IsString())
          {
            type = optionalArgument;
            result->SetType(type->StringValue());
            continue;
          }
        }
        break;
      }

      if (!tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Comma)
      {
        return nullptr;
      }

      return result;
    }

    KRYS_NODISCARD static RefPtr<Value> ConsumeImageSet(TokenRange &args, PropertyParserState &state,
                                                        AllowedImageType allowedImageTypes) noexcept
    {
      ValueListBuilder imageSet;
      do
      {
        auto option = ConsumeImageSetOption(args, state, allowedImageTypes);
        if (!option)
        {
          return nullptr;
        }

        imageSet.push_back(krys::move(option));
      } while (ConsumeComma(args));

      return ImageSetValue::Create(krys::move(imageSet));
    }

    // MARK: <image>
    // https://drafts.csswg.org/css-images-4/#image-values

    RefPtr<Value> ConsumeImage(TokenRange &tokens, PropertyParserState &state,
                               AllowedImageType allowedImageTypes) noexcept
    {
      if (tokens.Peek().Type() == TokenType::String
          && HasFlag(allowedImageTypes, AllowedImageType::RawStringAsURL))
      {
        auto imageURL = CompleteURL(CSSOMString(tokens.Peek().IdentCodePoints()), state.Context);
        if (!imageURL)
        {
          return nullptr;
        }

        tokens.Discard();
        tokens.DiscardWhitespace();
        return ImageValue::Create(krys::move(*imageURL));
      }

      if (tokens.Peek().Type() == TokenType::Function)
      {
        auto consumeGeneratedImage = [&](auto consumer) -> RefPtr<Value>
        {
          if (!HasFlag(allowedImageTypes, AllowedImageType::GeneratedImage))
          {
            return nullptr;
          }

          auto rangeCopy = tokens;
          auto args = ConsumeFunction(rangeCopy);
          RefPtr<Value> result = consumer(args);
          if (!result || !args.IsAtEnd())
          {
            return nullptr;
          }

          tokens = rangeCopy;
          return result;
        };

        auto consumeImageSetImage = [&](auto consumer) -> RefPtr<Value>
        {
          if (!HasFlag(allowedImageTypes, AllowedImageType::ImageSet))
          {
            return nullptr;
          }

          auto rangeCopy = tokens;
          auto args = ConsumeFunction(rangeCopy);
          RefPtr<Value> result = consumer(args);
          if (!result || !args.IsAtEnd())
          {
            return nullptr;
          }

          tokens = rangeCopy;
          return result;
        };

        auto functionId = tokens.Peek().FunctionId();
        switch (functionId)
        {
          case ValueId::RadialGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args) { return ConsumeRadialGradient<ValueId::RadialGradient>(args, state); });
          }
          case ValueId::RepeatingRadialGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumeRadialGradient<ValueId::RepeatingRadialGradient>(args, state); });
          }
          case ValueId::WebkitLinearGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumePrefixedLinearGradient<ValueId::WebkitLinearGradient>(args, state); });
          }
          case ValueId::WebkitRepeatingLinearGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumePrefixedLinearGradient<ValueId::WebkitRepeatingLinearGradient>(args, state); });
          }
          case ValueId::RepeatingLinearGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumeLinearGradient<ValueId::RepeatingLinearGradient>(args, state); });
          }
          case ValueId::LinearGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args) { return ConsumeLinearGradient<ValueId::LinearGradient>(args, state); });
          }
          case ValueId::WebkitGradient:
          {
            return consumeGeneratedImage([&](TokenRange &args)
                                         { return ConsumeDeprecatedGradient(args, state); });
          }
          case ValueId::WebkitRadialGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumePrefixedRadialGradient<ValueId::WebkitRadialGradient>(args, state); });
          }
          case ValueId::WebkitRepeatingRadialGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumePrefixedRadialGradient<ValueId::WebkitRepeatingRadialGradient>(args, state); });
          }
          case ValueId::ConicGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args) { return ConsumeConicGradient<ValueId::ConicGradient>(args, state); });
          }
          case ValueId::RepeatingConicGradient:
          {
            return consumeGeneratedImage(
              [&](TokenRange &args)
              { return ConsumeConicGradient<ValueId::RepeatingConicGradient>(args, state); });
          }
          case ValueId::WebkitCrossFade:
          case ValueId::CrossFade:
          {
            return consumeGeneratedImage([&](TokenRange &args)
                                         { return ConsumeCrossFade(args, state, functionId); });
          }
          case ValueId::WebkitCanvas:
          {
            return consumeGeneratedImage([&](TokenRange &args) { return ConsumeWebkitCanvas(args); });
          }
          case ValueId::WebkitNamedImage:
          {
            return consumeGeneratedImage([&](TokenRange &args) { return ConsumeWebkitNamedImage(args); });
          }
          case ValueId::WebkitFilter:
          case ValueId::Filter:
          {
            return consumeGeneratedImage([&](TokenRange &args) { return ConsumeFilterImage(args, state); });
          }
          case ValueId::Paint:
          {
            return consumeGeneratedImage([&](TokenRange &args) { return ConsumeCustomPaint(args, state); });
          }
          case ValueId::ImageSet:
          case ValueId::WebkitImageSet:
          {
            return consumeImageSetImage(
              [&](TokenRange &args)
              {
                return ConsumeImageSet(args, state,
                                       (allowedImageTypes | AllowedImageType::RawStringAsURL)
                                         & ~AllowedImageType::ImageSet);
              });
          }
          default:
          {
            break;
          }
        }
      }

      if (HasFlag(allowedImageTypes, AllowedImageType::URLFunction))
      {
        if (auto imageURL = ConsumeURLRaw(
              tokens, state, AllowedURLModifiers::CrossOrigin | AllowedURLModifiers::ReferrerPolicy))
        {
          return ImageValue::Create(krys::move(*imageURL));
        }
      }

      return nullptr;
    }

    // MARK: <image> | none

    RefPtr<Value> ConsumeImageOrNone(TokenRange &tokens, PropertyParserState &state,
                                     AllowedImageType allowedImageTypes) noexcept
    {
      if (tokens.Peek().ValueId() == ValueId::None)
      {
        return ConsumeIdent(tokens);
      }

      return ConsumeImage(tokens, state, allowedImageTypes);
    }
  }
}
