#include "Krystal.HTML/CSS/Properties/Consumers/Ratio.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSRatioValue.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  Maybe<Ratio> ConsumeUnresolvedRatio(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <ratio> = <number [0,∞]> [ / <number [0,∞]> ]?
    // https://drafts.csswg.org/css-values-4/#ratio-value

    auto rangeCopy = tokens;

    auto numerator = MetaConsumer<Number<NonNegative>>::Consume(rangeCopy, state);
    if (!numerator)
    {
      return {};
    }

    if (!ConsumeSlash(rangeCopy))
    {
      tokens = rangeCopy;
      return Ratio {Krys::Move(*numerator)};
    }

    auto denominator = MetaConsumer<Number<NonNegative>>::Consume(rangeCopy, state);
    if (!denominator)
    {
      return {};
    }

    tokens = rangeCopy;
    return Ratio {Krys::Move(*numerator), Krys::Move(*denominator)};
  }

  Maybe<Ratio> ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(CSSTokenRange &tokens,
                                                                     CSSPropertyParserState &state) noexcept
  {
    // <ratio> = <number [0,∞]> [ / <number [0,∞]> ]?
    // https://drafts.csswg.org/css-values-4/#ratio-value

    auto rangeCopy = tokens;

    auto numerator = MetaConsumer<Number<NonNegative>>::Consume(rangeCopy, state);
    if (!numerator)
    {
      return {};
    }

    if (!ConsumeSlash(rangeCopy))
    {
      return {};
    }

    auto denominator = MetaConsumer<Number<NonNegative>>::Consume(rangeCopy, state);
    if (!denominator)
    {
      return {};
    }

    tokens = rangeCopy;
    return Ratio {Krys::Move(*numerator), Krys::Move(*denominator)};
  }

  RefPtr<CSSValue> ConsumeRatio(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (auto ratio = ConsumeUnresolvedRatio(tokens, state))
    {
      return CSSRatioValue::Create(Krys::Move(*ratio));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumeRatioWithBothNumeratorAndDenominator(CSSTokenRange &tokens,
                                                               CSSPropertyParserState &state) noexcept
  {
    if (auto ratio = ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(tokens, state))
    {
      return CSSRatioValue::Create(Krys::Move(*ratio));
    }

    return nullptr;
  }
}