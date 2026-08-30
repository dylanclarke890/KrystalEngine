#include "Krystal.HTML/CSS/Properties/Consumers/Background.hpp"
#include "Krystal.HTML/CSS/Calc/CSSCalcTreeParser.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParsing.hpp"
#include "Krystal.HTML/CSS/Values/Background/BoxShadow.hpp"
#include "Krystal.HTML/CSS/Values/Background/BoxShadowProperty.hpp"
#include "Krystal.HTML/CSS/Values/Background/CSSBackgroundRepeatValue.hpp"
#include "Krystal.HTML/CSS/Values/Background/CSSBoxShadowPropertyValue.hpp"
#include "Krystal.HTML/CSS/Values/Background/CSSReflectValue.hpp"
#include "Krystal.HTML/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.HTML/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.HTML/CSS/Values/Borders/CSSBorderImageSliceValue.hpp"
#include "Krystal.HTML/CSS/Values/Borders/CSSBorderImageWidthValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  template <typename ElementType>
  static void Complete4Sides(Array<ElementType, 4> &sides) noexcept
  {
    if (!sides[1])
    {
      sides[1] = sides[0];
    }

    if (!sides[2])
    {
      sides[2] = sides[0];
    }

    if (!sides[3])
    {
      sides[3] = sides[1];
    }
  }

  // MARK: - Border Radius

  enum class SupportWebKitBorderRadiusQuirk : bool
  {
    No,
    Yes
  };

  template <SupportWebKitBorderRadiusQuirk supportQuirk>
  KRYS_NODISCARD static Maybe<BorderRadius> ConsumeBorderRadius(CSSTokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius

    using OptionalRadiiForAxis = Array<Maybe<LengthPercentage<NonNegative>>, 4uz>;

    OptionalRadiiForAxis horizontalRadii;
    size_t i = 0uz;
    for (; i < 4uz && !tokens.IsAtEnd() && tokens.Peek().Type() != CSSTokenType::Delim; ++i)
    {
      horizontalRadii[i] = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state);
      if (!horizontalRadii[i])
      {
        return {};
      }
    }

    if (!horizontalRadii[0])
    {
      return {};
    }

    if (tokens.IsAtEnd())
    {
      if constexpr (supportQuirk == SupportWebKitBorderRadiusQuirk::Yes)
      {
        // Legacy syntax: `-webkit-border-radius: l1 l2` is equivalent to border-radius: `l1 / l2`.
        if (i == 2uz)
        {
          OptionalRadiiForAxis verticalRadii;
          verticalRadii[0] = horizontalRadii[1];
          horizontalRadii[1] = Null;

          return BorderRadius {
            .Horizontal = CompleteQuadFromArray<BorderRadius::Axis>(Krys::Move(horizontalRadii)),
            .Vertical = CompleteQuadFromArray<BorderRadius::Axis>(Krys::Move(verticalRadii))};
        }
      }

      auto horizontal = CompleteQuadFromArray<BorderRadius::Axis>(Krys::Move(horizontalRadii));
      auto vertical = horizontal; // Copy `horizontal` radii to `vertical`.

      return BorderRadius {.Horizontal = Krys::Move(horizontal), .Vertical = Krys::Move(vertical)};
    }

    if (!ConsumeSlash(tokens))
    {
      return {};
    }

    OptionalRadiiForAxis verticalRadii;
    for (size_t i = 0uz; i < 4uz && !tokens.IsAtEnd(); ++i)
    {
      verticalRadii[i] = MetaConsumer<LengthPercentage<NonNegative>>::Consume(tokens, state);
      if (!verticalRadii[i])
      {
        return {};
      }
    }
    if (!verticalRadii[0] || !tokens.IsAtEnd())
    {
      return {};
    }

    return BorderRadius {.Horizontal = CompleteQuadFromArray<BorderRadius::Axis>(Krys::Move(horizontalRadii)),
                         .Vertical = CompleteQuadFromArray<BorderRadius::Axis>(Krys::Move(verticalRadii))};
  }

  Maybe<BorderRadius> ConsumeUnresolvedBorderRadius(CSSTokenRange &tokens,
                                                    CSSPropertyParserState &state) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius

    return ConsumeBorderRadius<SupportWebKitBorderRadiusQuirk::No>(tokens, state);
  }

  Maybe<BorderRadius> ConsumeUnresolvedWebKitBorderRadius(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius

    // Includes the legacy syntax quirk where `-webkit-border-radius: l1 l2` is equivalent to border-radius:
    // `l1 / l2`.
    return ConsumeBorderRadius<SupportWebKitBorderRadiusQuirk::Yes>(tokens, state);
  }

  // MARK: - Border Image

  RefPtr<CSSValue> ConsumeBorderImageSlice(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                           BorderImageSliceFillDefault defaultFill) noexcept
  {
    // <'border-image-slice'> = [<number [0,∞]> | <percentage [0,∞]>]{1,4} && fill?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-slice

    bool fill = ConsumeIdentRaw<CSSValueId::Fill>(tokens).has_value();
    Array<RefPtr<CSSPrimitiveValue>, 4uz> slices;

    for (auto &value : slices)
    {
      value = CSSPrimitiveValueResolver<Percentage<NonNegative>>::ConsumeAndResolve(tokens, state);
      if (!value)
      {
        value = CSSPrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state);
      }

      if (!value)
      {
        break;
      }
    }
    if (!slices[0])
    {
      return nullptr;
    }

    if (ConsumeIdent<CSSValueId::Fill>(tokens))
    {
      if (fill)
      {
        return nullptr;
      }
      fill = true;
    }

    Complete4Sides(slices);

    // NOTE: For backwards compatibility, -webkit-border-image, -webkit-mask-box-image and -webkit-box-reflect
    // set fill unconditionally.
    if (defaultFill == BorderImageSliceFillDefault::Yes)
    {
      fill = true;
    }

    return CSSBorderImageSliceValue::Create(
      {Krys::Move(slices[0]), Krys::Move(slices[1]), Krys::Move(slices[2]), Krys::Move(slices[3])}, fill);
  }

  RefPtr<CSSValue> ConsumeBorderImageWidth(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                           BorderImageWidthOverridesWidthForLength overridesWidth) noexcept
  {
    // <'border-image-width'> = [ <length-percentage [0,∞]> | <number [0,∞]> | auto ]{1,4}
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-width

    Array<RefPtr<CSSPrimitiveValue>, 4> widths;

    bool hasLength = false;
    for (auto &value : widths)
    {
      value = CSSPrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state);
      if (value)
      {
        continue;
      }

      // FIXME: Figure out and document why overrideParserMode is explicitly set to HTMLStandardMode here or
      // remove the special case.
      // FIXME: As this falls into the "<length> ambiguous with <number>" case, this should probably be
      // `.unitlessZeroLength = UnitlessZeroQuirk::Forbid` in case the order of checks ever changes.
      if (auto numericValue = CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(
            tokens, state, {.OverrideParserMode = CSSParserMode::HTMLStandard}))
      {
        if (numericValue->IsLength())
        {
          hasLength = true;
        }

        value = numericValue;
        continue;
      }

      value = ConsumeIdent<CSSValueId::Auto>(tokens);
      if (!value)
      {
        break;
      }
    }

    if (!widths[0])
    {
      return nullptr;
    }

    Complete4Sides(widths);

    return CSSBorderImageWidthValue::Create(
      {Krys::Move(widths[0]), Krys::Move(widths[1]), Krys::Move(widths[2]), Krys::Move(widths[3])},
      overridesWidth == BorderImageWidthOverridesWidthForLength::Yes && hasLength);
  }

  Maybe<BorderImageComponents>
    ConsumeBorderImageComponents(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                 BorderImageSliceFillDefault defaultFill,
                                 BorderImageWidthOverridesWidthForLength overridesWidth) noexcept
  {
    // <'border-image'> = <'border-image-source'>
    //                 || <'border-image-slice'> [ / <'border-image-width'> | / <'border-image-width'>? /
    //                 <'border-image-outset'> ]?
    //                 || <'border-image-repeat'>
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image

    BorderImageComponents components;

    do
    {
      if (!components.Source)
      {
        components.Source = ConsumeImageOrNone(tokens, state);
        if (components.Source)
        {
          continue;
        }
      }
      if (!components.Repeat)
      {
        components.Repeat = CSSPropertyParsing::ConsumeBorderImageRepeat(tokens);
        if (components.Repeat)
        {
          continue;
        }
      }
      if (!components.Slice)
      {
        components.Slice = ConsumeBorderImageSlice(tokens, state, defaultFill);
        if (components.Slice)
        {
          assert(!components.Width && !components.Outset);
          if (ConsumeSlash(tokens))
          {
            components.Width = ConsumeBorderImageWidth(tokens, state, overridesWidth);
            if (ConsumeSlash(tokens))
            {
              components.Outset = CSSPropertyParsing::ConsumeBorderImageOutset(tokens, state);
              if (!components.Outset)
              {
                return {};
              }
            }
            else if (!components.Width)
            {
              return {};
            }
          }
        }
        else
        {
          return {};
        }
      }
      else
      {
        return {};
      }
    } while (!tokens.IsAtEnd());

    return components;
  }

  // MARK: - Background Size

  template <CSSPropertyId property>
  static RefPtr<CSSValue> ConsumeBackgroundSize(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <bg-size> = [ <length-percentage [0,∞]> | auto ]{1,2} | cover | contain
    // https://drafts.csswg.org/css-backgrounds/#propdef-background-size

    if (IdentMatches<CSSValueId::Contain, CSSValueId::Cover>(tokens.Peek().ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    bool shouldCoalesce = true;
    RefPtr<CSSPrimitiveValue> horizontal = ConsumeIdent<CSSValueId::Auto>(tokens);
    if (!horizontal)
    {
      horizontal = CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);

      if (!horizontal)
      {
        return nullptr;
      }
      shouldCoalesce = false;
    }

    RefPtr<CSSPrimitiveValue> vertical;
    if (!tokens.IsAtEnd())
    {
      vertical = ConsumeIdent<CSSValueId::Auto>(tokens);
      if (!vertical)
      {
        vertical = CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);
      }
    }
    if (!vertical)
    {
      if constexpr (property == CSSPropertyId::WebkitBackgroundSize)
      {
        // Legacy syntax: "-webkit-background-size: 10px" is equivalent to "background-size: 10px 10px".
        vertical = horizontal;
      }
      else if constexpr (property == CSSPropertyId::BackgroundSize)
      {
        vertical = CSSPrimitiveValue::Create(CSSValueId::Auto);
      }
      else if constexpr (property == CSSPropertyId::MaskSize)
      {
        return horizontal;
      }
    }

    if (shouldCoalesce)
    {
      return CSSValuePair::Create(Krys::Move(horizontal), Krys::Move(vertical));
    }

    return CSSValuePair::CreateNonCoalescing(Krys::Move(horizontal), Krys::Move(vertical));
  }

  RefPtr<CSSValue> ConsumeSingleBackgroundSize(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <single-background-size> = <bg-size>
    // https://drafts.csswg.org/css-backgrounds/#background-size

    return ConsumeBackgroundSize<CSSPropertyId::BackgroundSize>(tokens, state);
  }

  RefPtr<CSSValue> ConsumeSingleWebkitBackgroundSize(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept
  {
    // Non-standard.
    return ConsumeBackgroundSize<CSSPropertyId::WebkitBackgroundSize>(tokens, state);
  }

  RefPtr<CSSValue> ConsumeSingleMaskSize(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <single-mask-size> = <bg-size>
    // https://drafts.fxtf.org/css-masking/#the-mask-size

    return ConsumeBackgroundSize<CSSPropertyId::MaskSize>(tokens, state);
  }

  // MARK: - Background Repeat

  RefPtr<CSSValue> ConsumeRepeatStyle(CSSTokenRange &tokens, CSSPropertyParserState &) noexcept
  {
    // <repeat-style> = repeat-x | repeat-y | [repeat | space | round | no-repeat]{1,2}
    // https://drafts.csswg.org/css-backgrounds/#typedef-repeat-style

    if (ConsumeIdentRaw<CSSValueId::RepeatX>(tokens))
    {
      return CSSBackgroundRepeatValue::Create(CSSValueId::Repeat, CSSValueId::NoRepeat);
    }

    if (ConsumeIdentRaw<CSSValueId::RepeatY>(tokens))
    {
      return CSSBackgroundRepeatValue::Create(CSSValueId::NoRepeat, CSSValueId::Repeat);
    }

    auto value1 =
      ConsumeIdentRaw<CSSValueId::Repeat, CSSValueId::NoRepeat, CSSValueId::Round, CSSValueId::Space>(tokens);
    if (!value1)
    {
      return nullptr;
    }

    auto value2 =
      ConsumeIdentRaw<CSSValueId::Repeat, CSSValueId::NoRepeat, CSSValueId::Round, CSSValueId::Space>(tokens);
    if (!value2)
    {
      value2 = value1;
    }

    return CSSBackgroundRepeatValue::Create(*value1, *value2);
  }

  // MARK: - Box Shadows

  KRYS_NODISCARD static Maybe<BoxShadow> ConsumeSingleUnresolvedBoxShadow(CSSTokenRange &tokens,
                                                                          CSSPropertyParserState &state,
                                                                          bool isWebkitBoxShadow) noexcept
  {
    // <box-shadow> = <color>? && [<length>{2} <length [0,∞]>? <length>?] && inset?
    // https://drafts.csswg.org/css-backgrounds/#propdef-box-shadow

    auto rangeCopy = tokens;

    Maybe<Color> color;
    Maybe<Length<AllUnzoomed>> x;
    Maybe<Length<AllUnzoomed>> y;
    Maybe<Length<NonNegativeUnzoomed>> blur;
    Maybe<Length<AllUnzoomed>> spread;
    Maybe<Keywords::Inset> inset;

    for (size_t i = 0uz; i < 3uz; i++)
    {
      if (rangeCopy.IsAtEnd())
      {
        break;
      }

      const CSSToken &nextToken = rangeCopy.Peek();
      // If we have come to a comma (e.g. if this tokens represents a comma-separated list of <shadow>s), we
      // are done parsing this <shadow>.
      if (nextToken.Type() == CSSTokenType::Comma)
      {
        break;
      }

      if (nextToken.ValueId() == CSSValueId::Inset)
      {
        if (inset)
        {
          return {};
        }

        rangeCopy.Discard();
        rangeCopy.DiscardWhitespace();
        inset = Keywords::Inset {};
        continue;
      }

      auto maybeColor = ConsumeUnresolvedColor(rangeCopy, state);
      if (maybeColor)
      {
        // If we just parsed a color but already had one, the given token tokens is
        // not a valid <shadow>.
        if (color)
        {
          return {};
        }

        color = Krys::Move(*maybeColor);
        continue;
      }

      // If the current token is neither a color nor the `inset` keyword, it must be
      // the lengths component of this value.
      if (x || y || blur || spread)
      {
        // If we've already parsed these lengths, the given value is invalid as there
        // cannot be two lengths components in a single <shadow> value.
        return {};
      }

      x = MetaConsumer<Length<AllUnzoomed>>::Consume(rangeCopy, state);
      if (!x)
      {
        return {};
      }

      y = MetaConsumer<Length<AllUnzoomed>>::Consume(rangeCopy, state);
      if (!y)
      {
        return {};
      }

      const auto &token = rangeCopy.Peek();

      // The explicit check for calc() is unfortunate. This is ensuring that we only fail
      // parsing if there is a length, but it fails the tokens check.
      if (token.Type() == CSSTokenType::Dimension || token.Type() == CSSTokenType::Number
          || (token.Type() == CSSTokenType::Function && IsCalcFunction(token.FunctionId())))
      {
        blur = MetaConsumer<Length<NonNegativeUnzoomed>>::Consume(rangeCopy, state);
        if (!blur)
        {
          return {};
        }
      }

      if (blur)
      {
        spread = MetaConsumer<Length<AllUnzoomed>>::Consume(rangeCopy, state);
      }
    }

    if (!y)
    {
      return {};
    }

    tokens = rangeCopy;

    return BoxShadow {.color = Krys::Move(color),
                      .location = {Krys::Move(*x), Krys::Move(*y)},
                      .blur = Krys::Move(blur),
                      .spread = Krys::Move(spread),
                      .inset = Krys::Move(inset),
                      .isWebkitBoxShadow = isWebkitBoxShadow};
  }

  KRYS_NODISCARD static Maybe<BoxShadowProperty::List>
    ConsumeUnresolvedBoxShadowList(CSSTokenRange &tokens, CSSPropertyParserState &state,
                                   bool isWebkitBoxShadow) noexcept
  {
    auto rangeCopy = tokens;

    BoxShadowProperty::List list;

    do
    {
      auto shadow = ConsumeSingleUnresolvedBoxShadow(rangeCopy, state, isWebkitBoxShadow);
      if (!shadow)
      {
        return {};
      }

      list.value.push_back(Krys::Move(*shadow));
    } while (ConsumeComma(rangeCopy));

    tokens = rangeCopy;

    return list;
  }

  KRYS_NODISCARD static Maybe<BoxShadowProperty> ConsumeUnresolvedBoxShadow(CSSTokenRange &tokens,
                                                                            CSSPropertyParserState &state,
                                                                            bool isWebkitBoxShadow) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
      return BoxShadowProperty {Keywords::None {}};
    }

    if (auto boxShadowList = ConsumeUnresolvedBoxShadowList(tokens, state, isWebkitBoxShadow))
    {
      return BoxShadowProperty {Krys::Move(*boxShadowList)};
    }

    return {};
  }

  RefPtr<CSSValue> ConsumeBoxShadow(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <'box-shadow'> = none | <shadow>#
    // https://drafts.csswg.org/css-backgrounds/#propdef-box-shadow

    if (auto property = ConsumeUnresolvedBoxShadow(tokens, state, false))
      return CSSBoxShadowPropertyValue::Create({Krys::Move(*property)});
    return nullptr;
  }

  RefPtr<CSSValue> ConsumeWebkitBoxShadow(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (auto property = ConsumeUnresolvedBoxShadow(tokens, state, true))
    {
      return CSSBoxShadowPropertyValue::Create({Krys::Move(*property)});
    }

    return nullptr;
  }

  // MARK: - Reflect (non-standard)

  RefPtr<CSSValue> ConsumeWebkitBoxReflect(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    auto direction =
      ConsumeIdentRaw<CSSValueId::Above, CSSValueId::Below, CSSValueId::Left, CSSValueId::Right>(tokens);
    if (!direction)
    {
      return nullptr;
    }

    // FIXME: Does not seem right to create "0px" here. We'd like to omit "0px" when serializing if there is
    // also no image.
    RefPtr<CSSPrimitiveValue> offset;
    if (tokens.IsAtEnd())
    {
      offset = CSSPrimitiveValue::Create(0, CSSUnitType::px);
    }
    else
    {
      offset = CSSPrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
      if (!offset)
      {
        return nullptr;
      }
    }

    RefPtr<CSSValue> mask;
    if (!tokens.IsAtEnd())
    {
      auto components = ConsumeBorderImageComponents(tokens, state, BorderImageSliceFillDefault::Yes);
      if (!components)
      {
        return nullptr;
      }
      mask = CreateBorderImageValue(Krys::Move(*components));
    }

    return CSSReflectValue::Create(*direction, Krys::Move(offset), Krys::Move(mask));
  }
}