#pragma once

#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleFilterData : public RefCounted<StyleFilterData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleFilterData> Create() noexcept
    {
      return AdoptRef(*new StyleFilterData);
    }
    KRYS_NODISCARD Ref<StyleFilterData> Copy() const noexcept;

    bool operator==(const StyleFilterData &) const noexcept;

    style::Filter Filter;

  private:
    StyleFilterData() noexcept;
    StyleFilterData(const StyleFilterData &) noexcept;
  };
}
