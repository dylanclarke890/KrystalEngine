#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::boo::dom
{
  /// @brief Various flags for optimizing certain node checks.
  enum class NodeFlags : uint32
  {
    None = 0,
    IsConnected = 1 << 0,
    IsInShadowTree = 1 << 1,
    IsCharacterData = 1 << 2,
    IsContainerNode = 1 << 3,
    IsShadowRoot = 1 << 4,
    IsUnknownElement = 1 << 5,
    IsHTMLElement = 1 << 6,
    IsSVGElement = 1 << 7,
    IsMathMLElement = 1 << 8,
    IsHTMLSlotElement = 1 << 9,
    /// @brief Set while the parser is inserting the element's children; see `Element::FinishParsingChildren`.
    IsParsingChildren = 1 << 10,
    /// @brief Set when the inline style has changed since the `style` attribute was last written; see
    /// `Element::SynchronizeAllAttributes`.
    StyleAttributeIsDirty = 1 << 11,
    /// @brief Set when the element's computed style (`Element::ComputedStyle`) may be out of date.
    IsComputedStyleInvalid = 1 << 12,
    HasInvalidRenderer = 1 << 13,
    StyleResolutionShouldRecompositeLayer = 1 << 14,
    NeedsUpdateQueryContainerDependentStyle = 1 << 15,
    /// @brief Set while the element has user action state (hover, active or focus) in its document's
    /// `UserActionElementSet`.
    IsUserActionElement = 1 << 16,
    /// @brief Set on the `PseudoElement`s generated for `::before` and `::after`.
    IsPseudoElement = 1 << 17,
    /// @brief Set on a node an end point of the frame's selection is anchored in, and on its ancestors.
    ContainsSelectionEndPoint = 1 << 18,
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::dom::NodeFlags, 19u);
