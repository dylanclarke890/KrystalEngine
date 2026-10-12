#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/ScopeOrdinal.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::style
{
  struct ScopedName
  {
    css::CSSOMStringAtom Name;
    style::ScopeOrdinal ScopeOrdinal {style::ScopeOrdinal::Element};
    bool IsIdentifier {true};

    KRYS_NODISCARD bool operator==(const ScopedName &) const noexcept = default;
  };
}
