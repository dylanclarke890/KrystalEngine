#pragma once

#include "Krystal.Booey/Graphics/Filters/FilterImage.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo
{
  class Filter;

  /// @brief Applies one filter effect to its inputs, writing its result.
  class FilterEffectApplier
  {
  public:
    FilterEffectApplier() noexcept = default;
    virtual ~FilterEffectApplier() noexcept = default;

    /// @return Whether the effect was applied.
    KRYS_NODISCARD virtual bool Apply(const Filter &filter, Span<const Ref<FilterImage>> inputs,
                                      FilterImage &result) const noexcept = 0;
  };
}
