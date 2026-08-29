#include "Krystal.HTML/CSS/Properties/Consumers/TextDecoration.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/List.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/TextDecoration/CSSTextShadowPropertyValue.hpp"
#include "Krystal.HTML/CSS/Values/TextDecoration/TextShadow.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  KRYS_NODISCARD static Maybe<TextShadow>
    ConsumeSingleUnresolvedTextShadow(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
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

      color = Color(Krys::Move(*maybeColor));
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
      .color = Krys::Move(color), .location = {Krys::Move(*x), Krys::Move(*y)}, .blur = Krys::Move(blur)};
  }

  KRYS_NODISCARD static Maybe<TextShadowProperty::List>
    ConsumeUnresolvedTextShadowList(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
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

      list.value.push_back(Krys::Move(*shadow));
    } while (ConsumeComma(rangeCopy));

    tokens = rangeCopy;

    return list;
  }

  KRYS_NODISCARD static Maybe<TextShadowProperty>
    ConsumeUnresolvedTextShadow(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      tokens.Discard();
      tokens.DiscardWhitespace();

      return TextShadowProperty {Keywords::None {}};
    }

    if (auto textShadowList = ConsumeUnresolvedTextShadowList(tokens, state))
    {
      return TextShadowProperty {Krys::Move(*textShadowList)};
    }

    return {};
  }

  // NOTE: we're explicitly qualifying the function name here just to satisfy intellisense, which is having
  // trouble resolving the function name in this context for some reason.
  KRYS_NODISCARD RefPtr<CSSValue>
    Krys::HTML::CSSPropertyParserHelpers::ConsumeTextShadow(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept
  {
    // <'text-shadow'> = none | [ <color>? && <length>{2,3} ]#
    // https://drafts.csswg.org/css-text-decor-3/#propdef-text-shadow

    if (auto property = ConsumeUnresolvedTextShadow(tokens, state))
    {
      return CSSTextShadowPropertyValue::Create({Krys::Move(*property)});
    }

    return nullptr;
  }
}
