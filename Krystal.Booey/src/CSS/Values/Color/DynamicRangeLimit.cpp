#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"

namespace krys::boo::css
{
  // NOTE: All non-template functions must be implemented in the implementation
  // file due to the use of the incomplete type `DynamicRangeLimitMixFunction`.

  DynamicRangeLimit::DynamicRangeLimit(keywords::Standard keyword) noexcept : _value {keyword}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(keywords::Constrained keyword) noexcept : _value {keyword}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(keywords::NoLimit keyword) noexcept : _value {keyword}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(DynamicRangeLimitMixFunction &&mix) noexcept
      : _value {CreateUnique<DynamicRangeLimitMixFunction>(krys::move(mix))}
  {
  }

  DynamicRangeLimit::DynamicRangeLimit(DynamicRangeLimit &&) noexcept = default;
  DynamicRangeLimit &DynamicRangeLimit::operator=(DynamicRangeLimit &&) noexcept = default;
  DynamicRangeLimit::~DynamicRangeLimit() noexcept = default;

  bool DynamicRangeLimit::operator==(const DynamicRangeLimit &) const noexcept = default;
}
