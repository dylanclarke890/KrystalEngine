#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexBasis.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexGrow.hpp"
#include "Krystal.Booey/Style/Values/Flexbox/FlexShrink.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class StyleFlexibleBoxData : public RefCounted<StyleFlexibleBoxData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleFlexibleBoxData> Create() noexcept
    {
      return AdoptRef(*new StyleFlexibleBoxData);
    }
    KRYS_NODISCARD Ref<StyleFlexibleBoxData> Copy() const noexcept;

    bool operator==(const StyleFlexibleBoxData &) const noexcept;

    style::FlexGrow FlexGrow;
    style::FlexShrink FlexShrink;
    style::FlexBasis FlexBasis;

    unsigned FlexDirection : 2;
    unsigned FlexWrap : 2;

  private:
    StyleFlexibleBoxData() noexcept;
    StyleFlexibleBoxData(const StyleFlexibleBoxData &) noexcept;
  };
}
