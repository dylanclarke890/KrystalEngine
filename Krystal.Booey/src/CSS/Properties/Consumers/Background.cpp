#include "Krystal.Booey/CSS/Properties/Consumers/Background.hpp"
#include "Krystal.Booey/CSS/Calc/CalcTreeParser.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/Background/BoxShadow.hpp"
#include "Krystal.Booey/CSS/Values/Background/BoxShadowProperty.hpp"
#include "Krystal.Booey/CSS/Values/Background/BackgroundRepeatValue.hpp"
#include "Krystal.Booey/CSS/Values/Background/BoxShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/Background/ReflectValue.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImage.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImageSliceValue.hpp"
#include "Krystal.Booey/CSS/Values/Borders/BorderImageWidthValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
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
  KRYS_NODISCARD static Maybe<BorderRadius> ConsumeBorderRadius(TokenRange &tokens,
                                                                PropertyParserState &state) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius

    using OptionalRadiiForAxis = Array<Maybe<LengthPercentage<NonNegative>>, 4uz>;

    OptionalRadiiForAxis horizontalRadii;
    size_t i = 0uz;
    for (; i < 4uz && !tokens.IsAtEnd() && tokens.Peek().Type() != TokenType::Delim; ++i)
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
          horizontalRadii[1] = null;

          return BorderRadius {
            .Horizontal = CompleteQuadFromArray<BorderRadius::Axis>(krys::move(horizontalRadii)),
            .Vertical = CompleteQuadFromArray<BorderRadius::Axis>(krys::move(verticalRadii))};
        }
      }

      auto horizontal = CompleteQuadFromArray<BorderRadius::Axis>(krys::move(horizontalRadii));
      auto vertical = horizontal; // Copy `horizontal` radii to `vertical`.

      return BorderRadius {.Horizontal = krys::move(horizontal), .Vertical = krys::move(vertical)};
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

    return BorderRadius {.Horizontal = CompleteQuadFromArray<BorderRadius::Axis>(krys::move(horizontalRadii)),
                         .Vertical = CompleteQuadFromArray<BorderRadius::Axis>(krys::move(verticalRadii))};
  }

  Maybe<BorderRadius> ConsumeUnresolvedBorderRadius(TokenRange &tokens,
                                                    PropertyParserState &state) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius

    return ConsumeBorderRadius<SupportWebKitBorderRadiusQuirk::No>(tokens, state);
  }

  Maybe<BorderRadius> ConsumeUnresolvedWebKitBorderRadius(TokenRange &tokens,
                                                          PropertyParserState &state) noexcept
  {
    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius

    // Includes the legacy syntax quirk where `-webkit-border-radius: l1 l2` is equivalent to border-radius:
    // `l1 / l2`.
    return ConsumeBorderRadius<SupportWebKitBorderRadiusQuirk::Yes>(tokens, state);
  }

  // MARK: - Border Image

  RefPtr<Value> ConsumeBorderImageSlice(TokenRange &tokens, PropertyParserState &state,
                                           BorderImageSliceFillDefault defaultFill) noexcept
  {
    // <'border-image-slice'> = [<number [0,∞]> | <percentage [0,∞]>]{1,4} && fill?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-slice

    bool fill = ConsumeIdentRaw<ValueId::Fill>(tokens).has_value();
    Array<RefPtr<PrimitiveValue>, 4uz> slices;

    for (auto &value : slices)
    {
      value = PrimitiveValueResolver<Percentage<NonNegative>>::ConsumeAndResolve(tokens, state);
      if (!value)
      {
        value = PrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state);
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

    if (ConsumeIdent<ValueId::Fill>(tokens))
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

    return BorderImageSliceValue::Create(
      {krys::move(slices[0]), krys::move(slices[1]), krys::move(slices[2]), krys::move(slices[3])}, fill);
  }

  RefPtr<Value> ConsumeBorderImageWidth(TokenRange &tokens, PropertyParserState &state,
                                           BorderImageWidthOverridesWidthForLength overridesWidth) noexcept
  {
    // <'border-image-width'> = [ <length-percentage [0,∞]> | <number [0,∞]> | auto ]{1,4}
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-width

    Array<RefPtr<PrimitiveValue>, 4> widths;

    bool hasLength = false;
    for (auto &value : widths)
    {
      value = PrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(tokens, state);
      if (value)
      {
        continue;
      }

      // FIXME: Figure out and document why overrideParserMode is explicitly set to HTMLStandardMode here or
      // remove the special case.
      // FIXME: As this falls into the "<length> ambiguous with <number>" case, this should probably be
      // `.unitlessZeroLength = UnitlessZeroQuirk::Forbid` in case the order of checks ever changes.
      if (auto numericValue = PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(
            tokens, state, {.OverrideParserMode = ParserMode::HTMLStandard}))
      {
        if (numericValue->IsLength())
        {
          hasLength = true;
        }

        value = numericValue;
        continue;
      }

      value = ConsumeIdent<ValueId::Auto>(tokens);
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

    return BorderImageWidthValue::Create(
      {krys::move(widths[0]), krys::move(widths[1]), krys::move(widths[2]), krys::move(widths[3])},
      overridesWidth == BorderImageWidthOverridesWidthForLength::Yes && hasLength);
  }

  Maybe<BorderImageComponents>
    ConsumeBorderImageComponents(TokenRange &tokens, PropertyParserState &state,
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
        components.Repeat = PropertyParsing::ConsumeBorderImageRepeat(tokens);
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
              components.Outset = PropertyParsing::ConsumeBorderImageOutset(tokens, state);
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

  template <PropertyId property>
  static RefPtr<Value> ConsumeBackgroundSize(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <bg-size> = [ <length-percentage [0,∞]> | auto ]{1,2} | cover | contain
    // https://drafts.csswg.org/css-backgrounds/#propdef-background-size

    if (IdentMatches<ValueId::Contain, ValueId::Cover>(tokens.Peek().ValueId()))
    {
      return ConsumeIdent(tokens);
    }

    bool shouldCoalesce = true;
    RefPtr<PrimitiveValue> horizontal = ConsumeIdent<ValueId::Auto>(tokens);
    if (!horizontal)
    {
      horizontal = PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);

      if (!horizontal)
      {
        return nullptr;
      }
      shouldCoalesce = false;
    }

    RefPtr<PrimitiveValue> vertical;
    if (!tokens.IsAtEnd())
    {
      vertical = ConsumeIdent<ValueId::Auto>(tokens);
      if (!vertical)
      {
        vertical = PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(tokens, state);
      }
    }
    if (!vertical)
    {
      if constexpr (property == PropertyId::WebkitBackgroundSize)
      {
        // Legacy syntax: "-webkit-background-size: 10px" is equivalent to "background-size: 10px 10px".
        vertical = horizontal;
      }
      else if constexpr (property == PropertyId::BackgroundSize)
      {
        vertical = PrimitiveValue::Create(ValueId::Auto);
      }
      else if constexpr (property == PropertyId::MaskSize)
      {
        return horizontal;
      }
    }

    if (shouldCoalesce)
    {
      return ValuePair::Create(krys::move(horizontal), krys::move(vertical));
    }

    return ValuePair::CreateNonCoalescing(krys::move(horizontal), krys::move(vertical));
  }

  RefPtr<Value> ConsumeSingleBackgroundSize(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <single-background-size> = <bg-size>
    // https://drafts.csswg.org/css-backgrounds/#background-size

    return ConsumeBackgroundSize<PropertyId::BackgroundSize>(tokens, state);
  }

  RefPtr<Value> ConsumeSingleWebkitBackgroundSize(TokenRange &tokens,
                                                     PropertyParserState &state) noexcept
  {
    // Non-standard.
    return ConsumeBackgroundSize<PropertyId::WebkitBackgroundSize>(tokens, state);
  }

  RefPtr<Value> ConsumeSingleMaskSize(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <single-mask-size> = <bg-size>
    // https://drafts.fxtf.org/css-masking/#the-mask-size

    return ConsumeBackgroundSize<PropertyId::MaskSize>(tokens, state);
  }

  // MARK: - Background Repeat

  RefPtr<Value> ConsumeRepeatStyle(TokenRange &tokens, PropertyParserState &) noexcept
  {
    // <repeat-style> = repeat-x | repeat-y | [repeat | space | round | no-repeat]{1,2}
    // https://drafts.csswg.org/css-backgrounds/#typedef-repeat-style

    if (ConsumeIdentRaw<ValueId::RepeatX>(tokens))
    {
      return BackgroundRepeatValue::Create(ValueId::Repeat, ValueId::NoRepeat);
    }

    if (ConsumeIdentRaw<ValueId::RepeatY>(tokens))
    {
      return BackgroundRepeatValue::Create(ValueId::NoRepeat, ValueId::Repeat);
    }

    auto value1 = ConsumeIdentRaw<ValueId::Repeat, ValueId::NoRepeat, ValueId::Round, ValueId::Space>(tokens);
    if (!value1)
    {
      return nullptr;
    }

    auto value2 = ConsumeIdentRaw<ValueId::Repeat, ValueId::NoRepeat, ValueId::Round, ValueId::Space>(tokens);
    if (!value2)
    {
      value2 = value1;
    }

    return BackgroundRepeatValue::Create(*value1, *value2);
  }

  // MARK: - Box Shadows

  KRYS_NODISCARD static Maybe<BoxShadow> ConsumeSingleUnresolvedBoxShadow(TokenRange &tokens,
                                                                          PropertyParserState &state,
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
    Maybe<keywords::Inset> inset;

    for (size_t i = 0uz; i < 3uz; i++)
    {
      if (rangeCopy.IsAtEnd())
      {
        break;
      }

      const Token &nextToken = rangeCopy.Peek();
      // If we have come to a comma (e.g. if this tokens represents a comma-separated list of <shadow>s), we
      // are done parsing this <shadow>.
      if (nextToken.Type() == TokenType::Comma)
      {
        break;
      }

      if (nextToken.ValueId() == ValueId::Inset)
      {
        if (inset)
        {
          return {};
        }

        rangeCopy.Discard();
        rangeCopy.DiscardWhitespace();
        inset = keywords::Inset {};
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

        color = krys::move(*maybeColor);
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
      if (token.Type() == TokenType::Dimension || token.Type() == TokenType::Number
          || (token.Type() == TokenType::Function && IsCalcFunction(token.FunctionId())))
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

    return BoxShadow {.color = krys::move(color),
                      .location = {krys::move(*x), krys::move(*y)},
                      .blur = krys::move(blur),
                      .spread = krys::move(spread),
                      .inset = krys::move(inset),
                      .isWebkitBoxShadow = isWebkitBoxShadow};
  }

  KRYS_NODISCARD static Maybe<BoxShadowProperty::List>
    ConsumeUnresolvedBoxShadowList(TokenRange &tokens, PropertyParserState &state,
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

      list.value.push_back(krys::move(*shadow));
    } while (ConsumeComma(rangeCopy));

    tokens = rangeCopy;

    return list;
  }

  KRYS_NODISCARD static Maybe<BoxShadowProperty> ConsumeUnresolvedBoxShadow(TokenRange &tokens,
                                                                            PropertyParserState &state,
                                                                            bool isWebkitBoxShadow) noexcept
  {
    if (tokens.Peek().ValueId() == ValueId::None)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();
      return BoxShadowProperty {keywords::None {}};
    }

    if (auto boxShadowList = ConsumeUnresolvedBoxShadowList(tokens, state, isWebkitBoxShadow))
    {
      return BoxShadowProperty {krys::move(*boxShadowList)};
    }

    return {};
  }

  RefPtr<Value> ConsumeBoxShadow(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'box-shadow'> = none | <shadow>#
    // https://drafts.csswg.org/css-backgrounds/#propdef-box-shadow

    if (auto property = ConsumeUnresolvedBoxShadow(tokens, state, false))
      return BoxShadowPropertyValue::Create({krys::move(*property)});
    return nullptr;
  }

  RefPtr<Value> ConsumeWebkitBoxShadow(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (auto property = ConsumeUnresolvedBoxShadow(tokens, state, true))
    {
      return BoxShadowPropertyValue::Create({krys::move(*property)});
    }

    return nullptr;
  }

  // MARK: - Reflect (non-standard)

  RefPtr<Value> ConsumeWebkitBoxReflect(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    auto direction = ConsumeIdentRaw<ValueId::Above, ValueId::Below, ValueId::Left, ValueId::Right>(tokens);
    if (!direction)
    {
      return nullptr;
    }

    // FIXME: Does not seem right to create "0px" here. We'd like to omit "0px" when serializing if there is
    // also no image.
    RefPtr<PrimitiveValue> offset;
    if (tokens.IsAtEnd())
    {
      offset = PrimitiveValue::Create(0, UnitType::px);
    }
    else
    {
      offset = PrimitiveValueResolver<LengthPercentage<>>::ConsumeAndResolve(tokens, state);
      if (!offset)
      {
        return nullptr;
      }
    }

    RefPtr<Value> mask;
    if (!tokens.IsAtEnd())
    {
      auto components = ConsumeBorderImageComponents(tokens, state, BorderImageSliceFillDefault::Yes);
      if (!components)
      {
        return nullptr;
      }
      mask = CreateBorderImageValue(krys::move(*components));
    }

    return ReflectValue::Create(*direction, krys::move(offset), krys::move(mask));
  }
}