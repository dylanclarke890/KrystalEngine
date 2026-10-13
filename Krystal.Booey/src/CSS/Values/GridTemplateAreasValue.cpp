#include "Krystal.Booey/CSS/Values/GridTemplateAreasValue.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::css
{
  GridTemplateAreasValue::GridTemplateAreasValue(GridTemplateAreas &&areas) noexcept
      : Value(ValueType::GridTemplateAreas), _areas(krys::move(areas))
  {
    krys_debug_assert(_areas.Map.RowCount);
    krys_debug_assert(_areas.Map.ColumnCount);
  }

  Ref<GridTemplateAreasValue> GridTemplateAreasValue::Create(GridTemplateAreas areas) noexcept
  {
    return AdoptRef(*new GridTemplateAreasValue(krys::move(areas)));
  }

  CSSOMString GridTemplateAreasValue::StringForRow(size_t row) const noexcept
  {
    List<const CSSOMString *> columns(_areas.Map.ColumnCount);
    for (const auto &[name, area] : _areas.Map.Map)
    {
      if (row >= area.Rows.StartLine() && row < area.Rows.EndLine())
      {
        for (unsigned i = area.Columns.StartLine(); i < area.Columns.EndLine(); i++)
        {
          columns[i] = &name;
        }
      }
    }

    CSSOMString builder;
    bool first = true;
    for (const auto *name : columns)
    {
      if (!first)
      {
        builder.push_back(u8' ');
      }
      first = false;

      if (!name)
      {
        builder.push_back(u8'.');
      }
      else
      {
        builder.append(*name);
      }
    }

    return builder;
  }

  CSSOMString GridTemplateAreasValue::CustomCSSText(const SerialisationContext &context) const noexcept
  {
    return Serialise(context, _areas);
  }

  bool GridTemplateAreasValue::Equals(const GridTemplateAreasValue &other) const noexcept
  {
    return _areas == other._areas;
  }
}
