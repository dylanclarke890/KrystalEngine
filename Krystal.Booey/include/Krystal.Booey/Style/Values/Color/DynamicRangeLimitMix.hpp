#pragma once

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo::css
{
  struct DynamicRangeLimitMixFunction;
}

namespace krys::boo::style
{
  struct DynamicRangeLimit;

  using DynamicRangeLimitMixPercentage = Percentage<css::Range {0, 100}>;

  /// @brief The computed `dynamic-range-limit-mix()` percentages of each keyword.
  struct DynamicRangeLimitMixParameters
  {
    DynamicRangeLimitMixPercentage Standard;
    DynamicRangeLimitMixPercentage Constrained;
    DynamicRangeLimitMixPercentage NoLimit;

    KRYS_NODISCARD bool operator==(const DynamicRangeLimitMixParameters &) const noexcept = default;
  };

  using DynamicRangeLimitMixFunction =
    css::FunctionNotation<css::ValueId::DynamicRangeLimitMix, DynamicRangeLimitMixParameters>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const DynamicRangeLimitMixParameters &value) noexcept
  {
    if constexpr (I == 0)
    {
      return value.Standard;
    }
    else if constexpr (I == 1)
    {
      return value.Constrained;
    }
    else if constexpr (I == 2)
    {
      return value.NoLimit;
    }
  }

  // Overload of operator== for UniquePtr<DynamicRangeLimitMixFunction> to make DynamicRangeLimit::Kind's
  // operator== work.
  // FIXME: Replace use of direct UniquePtr with something like std::indirect from
  // https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2025/p3019r12.pdf to get this for free.
  KRYS_NODISCARD inline bool operator==(const UniquePtr<DynamicRangeLimitMixFunction> &a,
                                        const UniquePtr<DynamicRangeLimitMixFunction> &b) noexcept
  {
    return a.get() == b.get() || (a && b && *a == *b);
  }

  /// @brief Adds the `DynamicRangeLimit` to the `DynamicRangeLimitMixFunction` weighted by the `Percentage`.
  void AddWeightedLimitTo(DynamicRangeLimitMixFunction &addingTo, const DynamicRangeLimit &limit,
                          const DynamicRangeLimitMixPercentage &mixPercentage) noexcept;

  // MARK: Conversion

  template <>
  struct ToCSS<DynamicRangeLimitMixFunction>
  {
    KRYS_NODISCARD css::DynamicRangeLimitMixFunction operator()(const DynamicRangeLimitMixFunction &mix,
                                                                const RenderStyle &style) noexcept;
  };

  template <>
  struct ToStyle<css::DynamicRangeLimitMixFunction>
  {
    KRYS_NODISCARD DynamicRangeLimitMixFunction operator()(const css::DynamicRangeLimitMixFunction &mix,
                                                           const BuilderState &state) noexcept;
    KRYS_NODISCARD DynamicRangeLimitMixFunction
      operator()(const css::DynamicRangeLimitMixFunction &mix,
                 const css::ToLengthConversionData &conversionData) noexcept;
  };
}

DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::DynamicRangeLimitMixParameters, 3)
