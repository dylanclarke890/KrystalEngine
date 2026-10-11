#pragma once

#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::css
{
  // dynamic-range-limit-mix() = dynamic-range-limit-mix( [ <'dynamic-range-limit'> && <percentage [0,100]>
  // ]#{2,} ) https://drafts.csswg.org/css-color-hdr/#dynamic-range-limit-mix
  using DynamicRangeLimitMixPercentage = css::Percentage<Range {0, 100}>;

  using DynamicRangeLimitMixComponent =
    SpaceSeparatedTuple<DynamicRangeLimit, DynamicRangeLimitMixPercentage>;

  using DynamicRangeLimitMixParameters = CommaSeparatedList<DynamicRangeLimitMixComponent>;

  // NOTE: Type wrapper is used here to allow forward declaration of the mix function in
  // `DynamicRangeLimit.hpp`.
  using DynamicRangeLimitMixFunctionValue =
    FunctionNotation<ValueId::DynamicRangeLimitMix, DynamicRangeLimitMixParameters>;

  DEFINE_TYPE_WRAPPER(DynamicRangeLimitMixFunction, DynamicRangeLimitMixFunctionValue);

  // Overload of operator== for UniqueRef<DynamicRangeLimitMixFunction> to make DynamicRangeLimit::Kind's
  // operator== work.
  // FIXME: Replace use of direct UniqueRef with something like std::indirect from
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3019r12.pdf to get this for free.
  inline bool operator==(const UniquePtr<DynamicRangeLimitMixFunction> &a,
                         const UniquePtr<DynamicRangeLimitMixFunction> &b) noexcept
  {
    return (a.get() == b.get()) || (*a == *b);
  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::css::DynamicRangeLimitMixFunction)
