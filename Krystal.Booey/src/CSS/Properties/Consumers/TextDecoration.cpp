#include "Krystal.Booey/CSS/Properties/Consumers/TextDecoration.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/List.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/TextDecoration/CSSTextShadowPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/TextDecoration/TextShadow.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  KRYS_NODISCARD static Maybe<TextShadow>
    ConsumeSingleUnresolvedTextShadow(TokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <single-text-shadow> = [ <color>? && <length>{2,3} ]
    // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow

    // FIXME: CSS Text Decoration 4 has updated text-shadow to use the complete box-shadow grammar:
    // <shadow> = <color>? && [<length>{2} <length [0,∞]>? <length>?] && inset?
    // https://drafts.csswg.org/css-text-decor-4/#propdef-text-shadow

    auto rangeCopy = tokens;

    Maybe<Color> color;
    Maybe<Length<AllUnzoomed>> x;
    Maybe<Length<AllUnzoomed>> y;
    Maybe<Length<NonNegativeUnzoomed>> blur;

    auto consumeOptionalColor = [&] -> bool
    {
      if (color)
      {
        return false;
      }

      auto maybeColor = ConsumeUnresolvedColor(rangeCopy, state);
      if (!maybeColor)
      {
        return false;
      }

      color = Color(krys::move(*maybeColor));
      return !!color;
    };

    auto consumeLengths = [&] -> bool
    {
      if (x)
      {
        return false;
      }

      x = MetaConsumer<Length<AllUnzoomed>>::Consume(rangeCopy, state);
      if (!x)
      {
        return false;
      }

      y = MetaConsumer<Length<AllUnzoomed>>::Consume(rangeCopy, state);
      if (!y)
      {
        return false;
      }

      blur = MetaConsumer<Length<NonNegativeUnzoomed>>::Consume(rangeCopy, state);

      return true;
    };

    while (!rangeCopy.IsAtEnd())
    {
      if (consumeOptionalColor() || consumeLengths())
      {
        continue;
      }

      break;
    }

    if (!y)
    {
      return {};
    }

    tokens = rangeCopy;

    return TextShadow {
      .color = krys::move(color), .location = {krys::move(*x), krys::move(*y)}, .blur = krys::move(blur)};
  }

  KRYS_NODISCARD static Maybe<TextShadowProperty::List>
    ConsumeUnresolvedTextShadowList(TokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    auto rangeCopy = tokens;

    TextShadowProperty::List list;

    do
    {
      auto shadow = ConsumeSingleUnresolvedTextShadow(rangeCopy, state);
      if (!shadow)
      {
        return {};
      }

      list.value.push_back(krys::move(*shadow));
    } while (ConsumeComma(rangeCopy));

    tokens = rangeCopy;

    return list;
  }

  KRYS_NODISCARD static Maybe<TextShadowProperty>
    ConsumeUnresolvedTextShadow(TokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      return TextShadowProperty {Keywords::None {}};
    }

    if (auto textShadowList = ConsumeUnresolvedTextShadowList(tokens, state))
    {
      return TextShadowProperty {krys::move(*textShadowList)};
    }

    return {};
  }

  // NOTE: we're explicitly qualifying the function name here just to satisfy intellisense, which is having
  // trouble resolving the function name in this context for some reason.
  KRYS_NODISCARD RefPtr<CSSValue>
    krys::boo::css::CSSPropertyParserHelpers::ConsumeTextShadow(TokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept
  {
    // <'text-shadow'> = none | [ <color>? && <length>{2,3} ]#
    // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow

    if (auto property = ConsumeUnresolvedTextShadow(tokens, state))
    {
      return CSSTextShadowPropertyValue::Create({krys::move(*property)});
    }

    return nullptr;
  }
}
