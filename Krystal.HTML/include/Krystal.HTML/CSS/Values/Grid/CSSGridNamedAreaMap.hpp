#pragma once

#include "Krystal.HTML/CSS/Types/CSSOMString.hpp"
#include "Krystal.HTML/CSS/Values/Grid/GridArea.hpp"
#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Types/Map.hpp"
#include "Krystal.Lib/Types/SmallList.hpp"

namespace Krys::HTML
{
  // Parsed representation of the `<string>+` of <'grid-template-areas'>.
  struct GridNamedAreaMap
  {
    using Map = Map<CSSOMString, GridArea>;

    Map map;
    size_t rowCount {0};
    size_t columnCount {0};

    bool operator==(const GridNamedAreaMap &) const = default;
  };

  // A single `<string>` of <'grid-template-areas'>.
  using GridNamedAreaMapRow = SmallList<CSSOMString, 8uz>;

  // Adds a row to a `GridNamedAreaMap`. Returns `true` on success, `false` on failure.
  KRYS_NODISCARD bool AddRow(GridNamedAreaMap &map, const GridNamedAreaMapRow &row) noexcept;
}