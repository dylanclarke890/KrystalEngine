/*
 * Copyright (C) 2025 Sam Weinig. All rights reserved.
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::css
{
  // dynamic-range-limit-mix() =
  //   dynamic-range-limit-mix( [ <'dynamic-range-limit'> && <percentage [0,100]> ]#{2,} )
  // https://drafts.csswg.org/css-color-hdr/#dynamic-range-limit-mix
  using DynamicRangeLimitMixPercentage = css::Percentage<Range {0, 100}>;
  using DynamicRangeLimitMixComponent =
    SpaceSeparatedTuple<DynamicRangeLimit, DynamicRangeLimitMixPercentage>;
  using DynamicRangeLimitMixParameters = CommaSeparatedList<DynamicRangeLimitMixComponent>;

  // NOTE: Type wrapper is used here to allow forward declaration of the mix function in
  // `DynamicRangeLimit.hpp`.
  using DynamicRangeLimitMixFunctionValue =
    FunctionNotation<ValueId::DynamicRangeLimitMix, DynamicRangeLimitMixParameters>;
  DEFINE_TYPE_WRAPPER(DynamicRangeLimitMixFunction, DynamicRangeLimitMixFunctionValue);

  // Overload of operator== for UniquePtr<DynamicRangeLimitMixFunction> to make DynamicRangeLimit::Kind's
  // operator== work.
  // FIXME: Replace use of direct UniquePtr with something like std::indirect from
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3019r12.pdf to get this for free.
  KRYS_NODISCARD inline bool operator==(const UniquePtr<DynamicRangeLimitMixFunction> &a,
                                        const UniquePtr<DynamicRangeLimitMixFunction> &b) noexcept
  {
    return a.get() == b.get() || (a && b && *a == *b);
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::css::DynamicRangeLimitMixFunction)
