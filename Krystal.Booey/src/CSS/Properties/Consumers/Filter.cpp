#include "Krystal.Booey/CSS/Properties/Consumers/Filter.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/Filter/AppleColorFilter.hpp"
#include "Krystal.Booey/CSS/Values/Filter/CSSAppleColorFilterValue.hpp"
#include "Krystal.Booey/CSS/Values/Filter/CSSFilterValue.hpp"
#include "Krystal.Booey/CSS/Values/Filter/Filter.hpp"
#include "Krystal.Booey/CSS/Values/Filter/FilterFunctionDescriptor.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  template <CSSValueId FilterFunction>
  KRYS_NODISCARD static decltype(auto)
    ConsumeNumberOrPercentFilterParameter(TokenRange &args, CSSPropertyParserState &state) noexcept
  {
    if constexpr (FilterFunctionAllowsValuesGreaterThanOne<FilterFunction>())
    {
      return MetaConsumer<Number<NonNegative>, Percentage<NonNegative>>::Consume(args, state, {});
    }
    else
    {
      return MetaConsumer<Number<ClosedUnitRangeClampUpper>,
                          Percentage<ClosedPercentageRangeClampUpper>>::Consume(args, state, {});
    }
  }

  KRYS_NODISCARD static Maybe<AppleInvertLightnessFunction>
    ConsumeFilterAppleInvertLightness(TokenRange &range, CSSPropertyParserState &) noexcept
  {
    // <-apple-invert-lightness()> = -apple-invert-lightness()
    // Non-standard

    auto args = ConsumeFunction(range);
    if (!args.IsAtEnd())
    {
      return {};
    }

    return AppleInvertLightnessFunction {.parameters = {}};
  }

  KRYS_NODISCARD static Maybe<BlurFunction> ConsumeFilterBlur(TokenRange &range,
                                                              CSSPropertyParserState &state) noexcept
  {
    // blur() = blur( <length [0,∞]>? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-blur

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return {BlurFunction {.parameters = {}}};
    }

    auto parsedValue = MetaConsumer<Length<NonNegative>>::Consume(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return BlurFunction {.parameters = {Blur::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<BrightnessFunction>
    ConsumeFilterBrightness(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // brightness() = brightness( [ <number [0,∞]> | <percentage [0,∞]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return BrightnessFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<BrightnessFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return BrightnessFunction {.parameters = {Brightness::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<ContrastFunction> ConsumeFilterContrast(TokenRange &range,
                                                                      CSSPropertyParserState &state) noexcept
  {
    // contrast() = contrast( [ <number [0,∞]> | <percentage [0,∞]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-contrast

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return ContrastFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<ContrastFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return ContrastFunction {.parameters = {Contrast::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<DropShadowFunction>
    ConsumeFilterDropShadow(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // drop-shadow() = drop-shadow( [ <color>? && [<length>{2} <length [0,∞]>?] ] )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-drop-shadow

    auto args = ConsumeFunction(range);

    Maybe<Color> color;
    Maybe<Length<AllUnzoomed>> x;
    Maybe<Length<AllUnzoomed>> y;
    Maybe<Length<NonNegativeUnzoomed>> stdDeviation;

    auto ConsumeOptionalColor = [&] -> bool
    {
      if (color)
      {
        return false;
      }

      auto maybeColor = ConsumeUnresolvedColor(args, state);
      if (!maybeColor)
      {
        return false;
      }

      color = krys::move(*maybeColor);
      return true;
    };

    auto ConsumeLengths = [&] -> bool
    {
      if (x)
      {
        return false;
      }

      x = MetaConsumer<Length<AllUnzoomed>>::Consume(args, state);
      if (!x)
      {
        return false;
      }

      y = MetaConsumer<Length<AllUnzoomed>>::Consume(args, state);
      if (!y)
      {
        return false;
      }

      stdDeviation = MetaConsumer<Length<NonNegativeUnzoomed>>::Consume(args, state);
      return true;
    };

    while (!args.IsAtEnd())
    {
      if (ConsumeOptionalColor() || ConsumeLengths())
      {
        continue;
      }

      break;
    }

    if (!y || !args.IsAtEnd())
    {
      return {};
    }

    return DropShadowFunction {.parameters = {.color = krys::move(color),
                                              .location = {krys::move(*x), krys::move(*y)},
                                              .stdDeviation = krys::move(stdDeviation)}};
  }

  KRYS_NODISCARD static Maybe<GrayscaleFunction>
    ConsumeFilterGrayscale(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // grayscale() = grayscale( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-grayscale

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return GrayscaleFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<GrayscaleFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return GrayscaleFunction {.parameters = {Grayscale::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<HueRotateFunction>
    ConsumeFilterHueRotate(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // hue-rotate() = hue-rotate( [ <angle> | <zero> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-hue-rotate

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return HueRotateFunction {.parameters = {}};
    }

    auto parsedValue =
      MetaConsumer<Angle<>>::Consume(args, state, {.UnitlessZeroAngle = AllowUnitlessZero(true)});
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return HueRotateFunction {.parameters = {HueRotate::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<InvertFunction> ConsumeFilterInvert(TokenRange &range,
                                                                  CSSPropertyParserState &state) noexcept
  {
    // invert() = invert( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-invert

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return InvertFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<InvertFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return InvertFunction {.parameters = {Invert::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<OpacityFunction> ConsumeFilterOpacity(TokenRange &range,
                                                                    CSSPropertyParserState &state) noexcept
  {
    // opacity() = opacity( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-opacity

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return OpacityFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<OpacityFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return OpacityFunction {.parameters = {Opacity::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<SaturateFunction> ConsumeFilterSaturate(TokenRange &range,
                                                                      CSSPropertyParserState &state) noexcept
  {
    // saturate() = saturate( [ <number [0,∞]> | <percentage [0,∞]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-saturate

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return SaturateFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<SaturateFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return SaturateFunction {.parameters = {Saturate::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<SepiaFunction> ConsumeFilterSepia(TokenRange &range,
                                                                CSSPropertyParserState &state) noexcept
  {
    // sepia() = sepia( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-sepia

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return SepiaFunction {.parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<SepiaFunction::name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return SepiaFunction {.parameters = {Sepia::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<FilterValueList> ConsumeUnresolvedFilterValueList(TokenRange &range,
                                                                                CSSPropertyParserState &state)
  {
    // <filter-value-list> = [ <filter-function> | <url> ]+
    // <filter-function> = <blur()> | <brightness()> | <contrast()> | <drop-shadow()> | <grayscale()> |
    // <hue-rotate()> | <invert()> | <opacity()> | <sepia()> | <saturate()>
    // https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list

    auto rangeCopy = range;

    FilterValueList list;

    auto AppendOnSuccess = [&](auto &&value) -> bool
    {
      if (!value)
      {
        return false;
      }

      list.value.push_back(krys::move(*value));
      return true;
    };

    do
    {
      if (auto url = ConsumeURLRaw(rangeCopy, state, {}))
      {
        list.value.push_back(FilterReference {krys::move(*url)});
        continue;
      }

      switch (rangeCopy.Peek().FunctionId())
      {
        case CSSValueId::Blur:
        {
          if (!AppendOnSuccess(ConsumeFilterBlur(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Brightness:
        {
          if (!AppendOnSuccess(ConsumeFilterBrightness(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Contrast:
        {
          if (!AppendOnSuccess(ConsumeFilterContrast(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::DropShadow:
        {
          if (!AppendOnSuccess(ConsumeFilterDropShadow(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Grayscale:
        {
          if (!AppendOnSuccess(ConsumeFilterGrayscale(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::HueRotate:
        {
          if (!AppendOnSuccess(ConsumeFilterHueRotate(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Invert:
        {
          if (!AppendOnSuccess(ConsumeFilterInvert(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Opacity:
        {
          if (!AppendOnSuccess(ConsumeFilterOpacity(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Saturate:
        {
          if (!AppendOnSuccess(ConsumeFilterSaturate(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Sepia:
        {
          if (!AppendOnSuccess(ConsumeFilterSepia(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        default:
        {
          return {};
        }
      }
    } while (!rangeCopy.IsAtEnd());

    range = rangeCopy;

    return {krys::move(list)};
  }

  Maybe<Filter> ConsumeUnresolvedFilter(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    if (range.Peek().ValueId() == CSSValueId::None)
    {
      range.Discard();
      range.DiscardWhitespace();
      return Filter {Keywords::None {}};
    }

    if (auto filterValueList = ConsumeUnresolvedFilterValueList(range, state))
    {
      return Filter {krys::move(*filterValueList)};
    }

    return {};
  }

  RefPtr<CSSValue> ConsumeFilter(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    if (auto property = ConsumeUnresolvedFilter(range, state))
    {
      return CSSFilterValue::Create({krys::move(*property)});
    }

    return nullptr;
  }

  // MARK: - <-apple-color-filter>

  KRYS_NODISCARD static Maybe<AppleColorFilter::List>
    ConsumeUnresolvedAppleColorFilterValueList(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    // <-apple-color-filter-value-list = <-apple-color-filter-function>+
    // <-apple-color-filter-function> = <-apple-invert-lightness() | <brightness()> | <contrast()> |
    // <grayscale()> | <hue-rotate()> | <invert()> | <opacity()> | <sepia()> | <saturate()>

    auto rangeCopy = range;

    AppleColorFilter::List list;

    auto AppendOnSuccess = [&](auto &&value) -> bool
    {
      if (!value)
      {
        return false;
      }

      list.value.push_back(krys::move(*value));
      return true;
    };

    do
    {
      switch (rangeCopy.Peek().FunctionId())
      {
        case CSSValueId::AppleInvertLightness:
        {
          if (!AppendOnSuccess(ConsumeFilterAppleInvertLightness(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Brightness:
        {
          if (!AppendOnSuccess(ConsumeFilterBrightness(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Contrast:
        {
          if (!AppendOnSuccess(ConsumeFilterContrast(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Grayscale:
        {
          if (!AppendOnSuccess(ConsumeFilterGrayscale(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::HueRotate:
        {
          if (!AppendOnSuccess(ConsumeFilterHueRotate(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Invert:
        {
          if (!AppendOnSuccess(ConsumeFilterInvert(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Opacity:
        {
          if (!AppendOnSuccess(ConsumeFilterOpacity(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case CSSValueId::Saturate:
        {
          if (!AppendOnSuccess(ConsumeFilterSaturate(rangeCopy, state)))
          {
            return {};
          }
          break;
        }
        case CSSValueId::Sepia:
        {
          if (!AppendOnSuccess(ConsumeFilterSepia(rangeCopy, state)))
          {
            return {};
          }
          break;
        }
        default: return {};
      }
    } while (!rangeCopy.IsAtEnd());

    range = rangeCopy;

    return {krys::move(list)};
  }

  Maybe<AppleColorFilter> ConsumeUnresolvedAppleColorFilter(TokenRange &range,
                                                            CSSPropertyParserState &state) noexcept
  {
    if (range.Peek().ValueId() == CSSValueId::None)
    {
      range.Discard();
      range.DiscardWhitespace();
      return AppleColorFilter {Keywords::None {}};
    }

    if (auto filterValueList = ConsumeUnresolvedAppleColorFilterValueList(range, state))
    {
      return AppleColorFilter {krys::move(*filterValueList)};
    }

    return {};
  }

  RefPtr<CSSValue> ConsumeAppleColorFilter(TokenRange &range, CSSPropertyParserState &state) noexcept
  {
    if (auto property = ConsumeUnresolvedAppleColorFilter(range, state))
    {
      return CSSAppleColorFilterValue::Create({krys::move(*property)});
    }

    return nullptr;
  }

  // Maybe<Style::Filter> parseFilterValueListOrNoneRaw(const String &string,
  //                                                            const ParserContext &context,
  //                                                            const Document &document, RenderStyle &style)
  //{
  //   auto tokenizer = Tokenizer(string);
  //   auto range = tokenizer.tokenRange();

  // // Handle leading whitespace.
  // range.consumeWhitespace();

  // auto state = CSSPropertyParserState {.context = context};
  // auto filter = consumeUnresolvedFilter(range, state);
  // if (!filter)
  //   return {};

  // // Handle trailing whitespace.
  // range.consumeWhitespace();

  // if (!range.IsAtEnd())
  //   return {};

  // auto builderState = Style::BuilderState::create(style, Style::BuilderContext {document});
  // return Style::toStyle(*filter, *CheckedPtr {builderState.ptr()});
  // }
}