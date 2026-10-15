#pragma once

#include "Krystal.Booey/Style/Values/Grid/GridPosition.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleGridItemData : public RefCounted<StyleGridItemData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleGridItemData> Create() noexcept
    {
      return AdoptRef(*new StyleGridItemData);
    }
    KRYS_NODISCARD Ref<StyleGridItemData> Copy() const noexcept;

    bool operator==(const StyleGridItemData &o) const noexcept
    {
      return GridItemColumnStart == o.GridItemColumnStart && GridItemColumnEnd == o.GridItemColumnEnd
             && GridItemRowStart == o.GridItemRowStart && GridItemRowEnd == o.GridItemRowEnd;
    }

    style::GridPosition GridItemColumnStart;
    style::GridPosition GridItemColumnEnd;
    style::GridPosition GridItemRowStart;
    style::GridPosition GridItemRowEnd;

  private:
    StyleGridItemData() noexcept;
    StyleGridItemData(const StyleGridItemData &) noexcept;
  };
}
