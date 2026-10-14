#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"

namespace krys::boo::css
{
  class Selector;
}

namespace krys::boo::style
{
  /// @brief The element whose change can affect whether a selector matches its subject, relative to the
  /// subject.
  enum class MatchElement : uint8
  {
    Subject,              // .changed
    Parent,               // .changed > .subject
    Ancestor,             // .changed .subject
    DirectSibling,        // .changed + .subject
    IndirectSibling,      // .changed ~ .subject
    AnySibling,           // :nth-last-child(even of .changed)
    ParentSibling,        // .changed ~ .a > .subject
    AncestorSibling,      // .changed ~ .a .subject
    ParentAnySibling,     // :nth-last-child(even of .changed) > .subject
    AncestorAnySibling,   // :nth-last-child(even of .changed) .subject
    HasChild,             // :has(> .changed)
    HasDescendant,        // :has(.changed)
    HasSibling,           // :has(~ .changed)
    HasSiblingDescendant, // :has(~ .a .changed)
    HasAnySibling,        // :has(~ :is(.changed ~ .x))
    HasChildParent,       // :has(> .changed) > .subject
    HasChildAncestor,     // :has(> .changed) .subject
    HasDescendantParent,  // :has(.changed) > .subject
    // FIXME: This is a catch-all for the rest of cases where :has() is in a non-subject position.
    HasNonSubject, // :has(.changed) .subject
    // FIXME: This is a catch-all for cases where :has() contains a scope breaking sub-selector.
    HasScopeBreaking, // :has(:is(.changed .a))
    Host,             // :host(.changed) .subject
    HostChild         // ::slotted(.changed)
  };

  KRYS_NODISCARD bool IsHasPseudoClassMatchElement(MatchElement matchElement) noexcept;

  /// @brief Which elements, relative to the `:has()` subject, the relative selector @p hasSelector can match.
  KRYS_NODISCARD MatchElement ComputeHasPseudoClassMatchElement(const css::Selector &hasSelector) noexcept;
}
