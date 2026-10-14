#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::style
{
  struct GridNamedLinesMap
  {
    HashMap<css::CSSOMString, List<uint32>> Map;

    KRYS_NODISCARD bool operator==(const GridNamedLinesMap &) const noexcept = default;
  };
}
