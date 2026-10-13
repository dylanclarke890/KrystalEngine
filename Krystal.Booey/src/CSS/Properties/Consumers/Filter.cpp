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
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/AppleColorFilterValue.hpp"
#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/AppleColorFilter.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/FilterFunctionDescriptor.hpp"
#include "Krystal.Booey/CSS/Values/FilterValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  template <ValueId FilterFunction>
  KRYS_NODISCARD static decltype(auto)
    ConsumeNumberOrPercentFilterParameter(TokenRange &args, PropertyParserState &state) noexcept
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
    ConsumeFilterAppleInvertLightness(TokenRange &range, PropertyParserState &) noexcept
  {
    // <-apple-invert-lightness()> = -apple-invert-lightness()
    // Non-standard

    auto args = ConsumeFunction(range);
    if (!args.IsAtEnd())
    {
      return {};
    }

    return AppleInvertLightnessFunction {.Parameters = {}};
  }

  KRYS_NODISCARD static Maybe<BlurFunction> ConsumeFilterBlur(TokenRange &range,
                                                              PropertyParserState &state) noexcept
  {
    // blur() = blur( <length [0,∞]>? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-blur

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return {BlurFunction {.Parameters = {}}};
    }

    auto parsedValue = MetaConsumer<Length<NonNegative>>::Consume(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return BlurFunction {.Parameters = {Blur::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<BrightnessFunction> ConsumeFilterBrightness(TokenRange &range,
                                                                          PropertyParserState &state) noexcept
  {
    // brightness() = brightness( [ <number [0,∞]> | <percentage [0,∞]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return BrightnessFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<BrightnessFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return BrightnessFunction {.Parameters = {Brightness::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<ContrastFunction> ConsumeFilterContrast(TokenRange &range,
                                                                      PropertyParserState &state) noexcept
  {
    // contrast() = contrast( [ <number [0,∞]> | <percentage [0,∞]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-contrast

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return ContrastFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<ContrastFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return ContrastFunction {.Parameters = {Contrast::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<DropShadowFunction> ConsumeFilterDropShadow(TokenRange &range,
                                                                          PropertyParserState &state) noexcept
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

    return DropShadowFunction {.Parameters = {.Color = krys::move(color),
                                              .Location = {krys::move(*x), krys::move(*y)},
                                              .StdDeviation = krys::move(stdDeviation)}};
  }

  KRYS_NODISCARD static Maybe<GrayscaleFunction> ConsumeFilterGrayscale(TokenRange &range,
                                                                        PropertyParserState &state) noexcept
  {
    // grayscale() = grayscale( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-grayscale

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return GrayscaleFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<GrayscaleFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return GrayscaleFunction {.Parameters = {Grayscale::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<HueRotateFunction> ConsumeFilterHueRotate(TokenRange &range,
                                                                        PropertyParserState &state) noexcept
  {
    // hue-rotate() = hue-rotate( [ <angle> | <zero> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-hue-rotate

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return HueRotateFunction {.Parameters = {}};
    }

    auto parsedValue =
      MetaConsumer<Angle<>>::Consume(args, state, {.UnitlessZeroAngle = AllowUnitlessZero(true)});
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return HueRotateFunction {.Parameters = {HueRotate::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<InvertFunction> ConsumeFilterInvert(TokenRange &range,
                                                                  PropertyParserState &state) noexcept
  {
    // invert() = invert( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-invert

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return InvertFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<InvertFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return InvertFunction {.Parameters = {Invert::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<OpacityFunction> ConsumeFilterOpacity(TokenRange &range,
                                                                    PropertyParserState &state) noexcept
  {
    // opacity() = opacity( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-opacity

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return OpacityFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<OpacityFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return OpacityFunction {.Parameters = {Opacity::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<SaturateFunction> ConsumeFilterSaturate(TokenRange &range,
                                                                      PropertyParserState &state) noexcept
  {
    // saturate() = saturate( [ <number [0,∞]> | <percentage [0,∞]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-saturate

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return SaturateFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<SaturateFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return SaturateFunction {.Parameters = {Saturate::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<SepiaFunction> ConsumeFilterSepia(TokenRange &range,
                                                                PropertyParserState &state) noexcept
  {
    // sepia() = sepia( [ <number [0,1(clamp upper)] > | <percentage [0,100(clamp upper)]> ]? )
    // https://drafts.fxtf.org/filter-effects/#funcdef-filter-sepia

    auto args = ConsumeFunction(range);
    if (args.IsAtEnd())
    {
      return SepiaFunction {.Parameters = {}};
    }

    auto parsedValue = ConsumeNumberOrPercentFilterParameter<SepiaFunction::Name>(args, state);
    if (!parsedValue || !args.IsAtEnd())
    {
      return {};
    }

    return SepiaFunction {.Parameters = {Sepia::Parameter {krys::move(*parsedValue)}}};
  }

  KRYS_NODISCARD static Maybe<FilterValueList>
    ConsumeUnresolvedFilterValueList(TokenRange &range, PropertyParserState &state) noexcept
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

      list.Value.push_back(krys::move(*value));
      return true;
    };

    do
    {
      if (auto url = ConsumeURLRaw(rangeCopy, state, {}))
      {
        list.Value.push_back(FilterReference {krys::move(*url)});
        continue;
      }

      switch (rangeCopy.Peek().FunctionId())
      {
        case ValueId::Blur:
        {
          if (!AppendOnSuccess(ConsumeFilterBlur(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Brightness:
        {
          if (!AppendOnSuccess(ConsumeFilterBrightness(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Contrast:
        {
          if (!AppendOnSuccess(ConsumeFilterContrast(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::DropShadow:
        {
          if (!AppendOnSuccess(ConsumeFilterDropShadow(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Grayscale:
        {
          if (!AppendOnSuccess(ConsumeFilterGrayscale(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::HueRotate:
        {
          if (!AppendOnSuccess(ConsumeFilterHueRotate(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Invert:
        {
          if (!AppendOnSuccess(ConsumeFilterInvert(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Opacity:
        {
          if (!AppendOnSuccess(ConsumeFilterOpacity(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Saturate:
        {
          if (!AppendOnSuccess(ConsumeFilterSaturate(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Sepia:
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

  Maybe<Filter> ConsumeUnresolvedFilter(TokenRange &range, PropertyParserState &state) noexcept
  {
    if (range.Peek().ValueId() == ValueId::None)
    {
      range.Discard();
      range.DiscardWhitespace();
      return Filter {keywords::None {}};
    }

    if (auto filterValueList = ConsumeUnresolvedFilterValueList(range, state))
    {
      return Filter {krys::move(*filterValueList)};
    }

    return {};
  }

  RefPtr<Value> ConsumeFilter(TokenRange &range, PropertyParserState &state) noexcept
  {
    if (auto property = ConsumeUnresolvedFilter(range, state))
    {
      return FilterValue::Create({krys::move(*property)});
    }

    return nullptr;
  }

  // MARK: - <-apple-color-filter>

  KRYS_NODISCARD static Maybe<AppleColorFilter::List>
    ConsumeUnresolvedAppleColorFilterValueList(TokenRange &range, PropertyParserState &state) noexcept
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

      list.Value.push_back(krys::move(*value));
      return true;
    };

    do
    {
      switch (rangeCopy.Peek().FunctionId())
      {
        case ValueId::AppleInvertLightness:
        {
          if (!AppendOnSuccess(ConsumeFilterAppleInvertLightness(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Brightness:
        {
          if (!AppendOnSuccess(ConsumeFilterBrightness(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Contrast:
        {
          if (!AppendOnSuccess(ConsumeFilterContrast(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Grayscale:
        {
          if (!AppendOnSuccess(ConsumeFilterGrayscale(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::HueRotate:
        {
          if (!AppendOnSuccess(ConsumeFilterHueRotate(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Invert:
        {
          if (!AppendOnSuccess(ConsumeFilterInvert(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Opacity:
        {
          if (!AppendOnSuccess(ConsumeFilterOpacity(rangeCopy, state)))
          {
            return {};
          }

          break;
        }
        case ValueId::Saturate:
        {
          if (!AppendOnSuccess(ConsumeFilterSaturate(rangeCopy, state)))
          {
            return {};
          }
          break;
        }
        case ValueId::Sepia:
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
                                                            PropertyParserState &state) noexcept
  {
    if (range.Peek().ValueId() == ValueId::None)
    {
      range.Discard();
      range.DiscardWhitespace();
      return AppleColorFilter {keywords::None {}};
    }

    if (auto filterValueList = ConsumeUnresolvedAppleColorFilterValueList(range, state))
    {
      return AppleColorFilter {krys::move(*filterValueList)};
    }

    return {};
  }

  RefPtr<Value> ConsumeAppleColorFilter(TokenRange &range, PropertyParserState &state) noexcept
  {
    if (auto property = ConsumeUnresolvedAppleColorFilter(range, state))
    {
      return AppleColorFilterValue::Create({krys::move(*property)});
    }

    return nullptr;
  }
}