#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Rendering/Style/GridArea.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  // Parsed representation of the `<string>+` of <'grid-template-areas'>.
  struct GridNamedAreaMap
  {
    using MapType = HashMap<CSSOMString, GridArea>;

    MapType Map;
    size_t RowCount {0};
    size_t ColumnCount {0};

    KRYS_NODISCARD bool operator==(const GridNamedAreaMap &) const noexcept = default;
  };

  // A single `<string>` of <'grid-template-areas'>.
  using GridNamedAreaMapRow = SmallList<CSSOMString, 8>;

  // Adds a row to a `GridNamedAreaMap`. Returns `true` on success, `false` on failure.
  KRYS_NODISCARD bool AddRow(GridNamedAreaMap &gridAreaMap, const GridNamedAreaMapRow &row) noexcept;

#pragma region Serialisation

  template <>
  struct Serialiser<GridNamedAreaMap>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const GridNamedAreaMap &value) noexcept;
  };

#pragma endregion
}
