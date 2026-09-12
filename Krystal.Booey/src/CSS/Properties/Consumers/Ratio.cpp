#include "Krystal.Booey/CSS/Properties/Consumers/Ratio.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSRatioValue.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  Maybe<Ratio> ConsumeUnresolvedRatio(TokenRange &tokens, CSSPropertyParserState &state) noexcept
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
      return Ratio {krys::move(*numerator)};
    }

    auto denominator = MetaConsumer<Number<NonNegative>>::Consume(rangeCopy, state);
    if (!denominator)
    {
      return {};
    }

    tokens = rangeCopy;
    return Ratio {krys::move(*numerator), krys::move(*denominator)};
  }

  Maybe<Ratio> ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(TokenRange &tokens,
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
    return Ratio {krys::move(*numerator), krys::move(*denominator)};
  }

  RefPtr<CSSValue> ConsumeRatio(TokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    if (auto ratio = ConsumeUnresolvedRatio(tokens, state))
    {
      return CSSRatioValue::Create(krys::move(*ratio));
    }

    return nullptr;
  }

  RefPtr<CSSValue> ConsumeRatioWithBothNumeratorAndDenominator(TokenRange &tokens,
                                                               CSSPropertyParserState &state) noexcept
  {
    if (auto ratio = ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(tokens, state))
    {
      return CSSRatioValue::Create(krys::move(*ratio));
    }

    return nullptr;
  }
}