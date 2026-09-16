#include "Krystal.Booey/CSS/Properties/Consumers/Ratio.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RatioValue.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  Maybe<Ratio> ConsumeUnresolvedRatio(TokenRange &tokens, PropertyParserState &state) noexcept
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
                                                                     PropertyParserState &state) noexcept
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

  RefPtr<Value> ConsumeRatio(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    if (auto ratio = ConsumeUnresolvedRatio(tokens, state))
    {
      return RatioValue::Create(krys::move(*ratio));
    }

    return nullptr;
  }

  RefPtr<Value> ConsumeRatioWithBothNumeratorAndDenominator(TokenRange &tokens,
                                                               PropertyParserState &state) noexcept
  {
    if (auto ratio = ConsumeUnresolvedRatioWithBothNumeratorAndDenominator(tokens, state))
    {
      return RatioValue::Create(krys::move(*ratio));
    }

    return nullptr;
  }
}