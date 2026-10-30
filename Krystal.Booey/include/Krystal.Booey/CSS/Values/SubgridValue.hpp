#pragma once

#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A `subgrid` track list, with optional line names, e.g. `subgrid [a] [b]`.
  class SubgridValue final : public ValueContainingList
  {
  private:
    explicit SubgridValue(ValueListBuilder builder) noexcept;

  public:
    KRYS_NODISCARD static Ref<SubgridValue> Create(ValueListBuilder builder) noexcept;

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const SubgridValue &other) const noexcept
    {
      return ItemsEqual(other);
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::SubgridValue)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Value &value) noexcept
  {
    return value.IsSubgridValue();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
