#include "Krystal.Booey/Rendering/Style/StyleGridData.hpp"

#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"

namespace krys::boo
{

  StyleGridData::StyleGridData() noexcept
      : GridAutoFlow(style::ComputedStyle::InitialGridAutoFlow()),
        GridAutoColumns(style::ComputedStyle::InitialGridAutoColumns()),
        GridAutoRows(style::ComputedStyle::InitialGridAutoRows()),
        GridTemplateAreas(style::ComputedStyle::InitialGridTemplateAreas()),
        GridTemplateColumns(style::ComputedStyle::InitialGridTemplateColumns()),
        GridTemplateRows(style::ComputedStyle::InitialGridTemplateRows())
  {
  }

  inline StyleGridData::StyleGridData(const StyleGridData &o) noexcept
      : RefCounted<StyleGridData>(), GridAutoFlow(o.GridAutoFlow), GridAutoColumns(o.GridAutoColumns),
        GridAutoRows(o.GridAutoRows), GridTemplateAreas(o.GridTemplateAreas),
        GridTemplateColumns(o.GridTemplateColumns), GridTemplateRows(o.GridTemplateRows)
  {
  }

  bool StyleGridData::operator==(const StyleGridData &o) const noexcept
  {
    return GridAutoFlow == o.GridAutoFlow && GridAutoColumns == o.GridAutoColumns
           && GridAutoRows == o.GridAutoRows && GridTemplateAreas == o.GridTemplateAreas
           && GridTemplateColumns == o.GridTemplateColumns && GridTemplateRows == o.GridTemplateRows;
  }

  Ref<StyleGridData> StyleGridData::Copy() const noexcept
  {
    return AdoptRef(*new StyleGridData(*this));
  }
}
