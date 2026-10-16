#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief The style resolution state of a node: whether its descendants need resolving, and the style
  /// relations selector matching recorded on it.
  enum class NodeStyleFlags : uint16
  {
    None = 0,
    DescendantNeedsStyleResolution = 1 << 0,
    DirectChildNeedsStyleResolution = 1 << 1,

    AffectedByHasWithPositionalPseudoClass = 1 << 2,
    ChildrenAffectedByFirstChildRules = 1 << 3,
    ChildrenAffectedByLastChildRules = 1 << 4,
    AffectsNextSiblingElementStyle = 1 << 5,
    StyleIsAffectedByPreviousSibling = 1 << 6,
    DescendantsAffectedByPreviousSibling = 1 << 7,
    StyleAffectedByEmpty = 1 << 8,
    // We optimize for :first-child and :last-child. The other positional child selectors like nth-child or
    // *-child-of-type, we will just give up and re-evaluate whenever children change at all.
    ChildrenAffectedByForwardPositionalRules = 1 << 9,
    DescendantsAffectedByForwardPositionalRules = 1 << 10,
    ChildrenAffectedByBackwardPositionalRules = 1 << 11,
    DescendantsAffectedByBackwardPositionalRules = 1 << 12,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::dom::NodeStyleFlags, 13u);
