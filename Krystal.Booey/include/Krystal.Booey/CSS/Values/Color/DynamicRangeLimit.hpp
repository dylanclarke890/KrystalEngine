#pragma once

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::css
{
  struct DynamicRangeLimitMixFunction;

  // <'dynamic-range-limit'> = standard | no-limit | constrained | <dynamic-range-limit-mix()>
  // https://drafts.csswg.org/css-color-hdr/#propdef-dynamic-range-limit
  struct DynamicRangeLimit
  {
  private:
    using Kind = Variant<keywords::Standard, keywords::Constrained, keywords::NoLimit,
                         UniquePtr<DynamicRangeLimitMixFunction>>;

    Kind Value;

  public:
    DynamicRangeLimit(keywords::Standard) noexcept;
    DynamicRangeLimit(keywords::Constrained) noexcept;
    DynamicRangeLimit(keywords::NoLimit) noexcept;
    DynamicRangeLimit(DynamicRangeLimitMixFunction &&) noexcept;

    DynamicRangeLimit(DynamicRangeLimit &&) noexcept;
    DynamicRangeLimit &operator=(DynamicRangeLimit &&) noexcept;

    // Delete to avoid unnecessary unnecessary allocations on accidental copy.
    DynamicRangeLimit(const DynamicRangeLimit &) = delete;
    DynamicRangeLimit &operator=(const DynamicRangeLimit &) = delete;

    ~DynamicRangeLimit() noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      using ResultType = decltype(visitor(std::declval<keywords::Standard>()));

      return krys::SwitchOn(
        Value,
        [&]<ValueId Id>(const KeywordValueConstant<Id> &keyword) -> ResultType { return visitor(keyword); },
        [&](const UniquePtr<DynamicRangeLimitMixFunction> &mix) -> ResultType { return visitor(mix.get()); });
    }

    bool operator==(const DynamicRangeLimit &) const noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::DynamicRangeLimit)
