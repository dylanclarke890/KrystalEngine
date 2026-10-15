#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridAutoFlow.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateAreas.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTemplateList.hpp"
#include "Krystal.Booey/Style/Values/Grid/GridTrackSizes.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class RenderStyle;
  class StyleGridData : public RefCounted<StyleGridData>
  {
  public:
    KRYS_NODISCARD static Ref<StyleGridData> Create() noexcept
    {
      return AdoptRef(*new StyleGridData);
    }
    KRYS_NODISCARD Ref<StyleGridData> Copy() const noexcept;

    bool operator==(const StyleGridData &) const noexcept;

    style::GridAutoFlow GridAutoFlow;
    style::GridTrackSizes GridAutoColumns;
    style::GridTrackSizes GridAutoRows;
    style::GridTemplateAreas GridTemplateAreas;
    style::GridTemplateList GridTemplateColumns;
    style::GridTemplateList GridTemplateRows;

  private:
    StyleGridData() noexcept;
    StyleGridData(const StyleGridData &) noexcept;
  };
}
