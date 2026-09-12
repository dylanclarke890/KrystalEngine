#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/CSS/Values/Grid/GridArea.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  // Parsed representation of the `<string>+` of <'grid-template-areas'>.
  struct GridNamedAreaMap
  {
    using Map = HashMap<CSSOMString, GridArea>;

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