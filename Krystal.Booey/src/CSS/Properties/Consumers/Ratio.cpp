/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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