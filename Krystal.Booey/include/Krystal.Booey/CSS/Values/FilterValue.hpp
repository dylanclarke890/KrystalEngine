#pragma once

#include "Krystal.Booey/CSS/Values/FilterEffects/Filter.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `<'filter'>` value, e.g. `blur(2px) url(#shadow)`.
  class FilterValue final : public Value
  {
  private:
    boo::css::Filter _filter;

    explicit FilterValue(boo::css::Filter &&filter) noexcept;

  public:
    KRYS_NODISCARD static Ref<FilterValue> Create(boo::css::Filter filter) noexcept;

    KRYS_NODISCARD const boo::css::Filter &GetFilter() const noexcept
    {
      return _filter;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;
    KRYS_NODISCARD bool Equals(const FilterValue &other) const noexcept;
    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::FilterValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsFilterValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
