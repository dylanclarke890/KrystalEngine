#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::style
{
  struct GridOrderedNamedLinesMap
  {
    HashMap<uint32, List<css::CSSOMString>> Map;

    KRYS_NODISCARD bool operator==(const GridOrderedNamedLinesMap &) const noexcept = default;
  };
}
