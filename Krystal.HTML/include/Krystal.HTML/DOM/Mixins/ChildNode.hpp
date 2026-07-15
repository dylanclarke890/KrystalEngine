#pragma once

#include "Krystal.HTML/DOM/Types/ExceptionOr.hpp"
#include "Krystal.HTML/DOM/Types/NodeOrString.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class Element;
  class Node;
}

namespace Krys::HTML::Mixins
{
  /// @see https://dom.spec.whatwg.org/#childnode
  class ChildNode
  {
  public:
    /// @brief Inserts `nodes` just before `node`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-before
    KRYS_NODISCARD static ExceptionOr<void> Before(Node &node, const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `node`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    KRYS_NODISCARD static ExceptionOr<void> After(Node &node, const List<NodeOrString> &nodes) noexcept;

    /// @brief Replaces `node` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    KRYS_NODISCARD static ExceptionOr<void> ReplaceWith(Node &node, const List<NodeOrString> &nodes) noexcept;

    /// @brief Removes `node`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    KRYS_NODISCARD static ExceptionOr<void> Remove(Node &node) noexcept;
  };
}