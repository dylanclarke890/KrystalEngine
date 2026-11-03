#pragma once

#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::dom
{
  class Node;

  /// @brief Opens the closed `details` elements and reveals the `hidden=until-found` elements that hide
  /// @p node (firing `beforematch` at each of the latter), so a fragment or text fragment can scroll to it.
  /// @return Whether anything was revealed.
  /// @see https://html.spec.whatwg.org/#ancestor-revealing-algorithm
  bool RevealClosedDetailsAndHiddenUntilFoundAncestors(Node &node) noexcept;
}
