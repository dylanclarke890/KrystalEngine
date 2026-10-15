#pragma once

#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleBackdropFilterData : public RefCounted<StyleBackdropFilterData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleBackdropFilterData> Create() noexcept
    {
      return AdoptRef(*new StyleBackdropFilterData);
    }
    KRYS_NODISCARD Ref<StyleBackdropFilterData> Copy() const noexcept;

    bool operator==(const StyleBackdropFilterData &) const noexcept;

    style::Filter BackdropFilter;

  private:
    StyleBackdropFilterData() noexcept;
    StyleBackdropFilterData(const StyleBackdropFilterData &) noexcept;
  };
}
