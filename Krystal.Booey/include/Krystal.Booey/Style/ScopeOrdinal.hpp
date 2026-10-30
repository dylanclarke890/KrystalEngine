#pragma once

#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::style
{
  // This is used to identify style scopes that can affect an element.
  // Scopes are in tree-of-trees order. Styles from earlier scopes win over later ones (modulo !important).
  enum class ScopeOrdinal : int8
  {
    ContainingHostLimit = Limits<int8>::min(),
    // ::part rules and author-exposed UA pseudo classes from the host tree scope. Values less than
    // ContainingHost indicate enclosing scopes.
    ContainingHost = -1,
    // Normal rules in the same tree where the element is.
    Element = 0,
    // ::slotted rules in the parent's shadow tree. Values greater than FirstSlot indicate subsequent slots in
    // the chain.
    FirstSlot = 1,
    SlotLimit = Limits<int8>::max() - 1,
    // :host rules in element's own shadow tree.
    Shadow = Limits<int8>::max(),
  };

  constexpr ScopeOrdinal &operator++(ScopeOrdinal &ordinal) noexcept
  {
    krys_debug_assert(ordinal < ScopeOrdinal::SlotLimit);
    return ordinal = static_cast<ScopeOrdinal>(ToUnderlying(ordinal) + 1);
  }

  constexpr ScopeOrdinal &operator--(ScopeOrdinal &ordinal) noexcept
  {
    krys_debug_assert(ordinal > ScopeOrdinal::ContainingHostLimit);
    return ordinal = static_cast<ScopeOrdinal>(ToUnderlying(ordinal) - 1);
  }
}
