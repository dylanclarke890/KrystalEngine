#pragma once

#include "Krystal.HTML/DOM/Node.hpp"

namespace Krys::HTML
{
  /// @brief A ContainerNode is a Node that can have child nodes. It provides the common implementation for
  /// Element, Document, and DocumentFragment.
  /// @note The DOM specification does not define a ContainerNode interface, but we use it internally to save
  /// memory.
  class ContainerNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ContainerNode);

    friend class ExtensibilityHooks;
    friend class MutationAlgorithms;

  private:
    CheckedPtr<Node> _firstChild;
    CheckedPtr<Node> _lastChild;

  protected:
    ContainerNode(Document &document, HTML::NodeType type, NodeFlags flags) noexcept;

  public:
#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-firstchild
    KRYS_NODISCARD RawPtr<Node> FirstChild() const noexcept
    {
      return _firstChild.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-lastchild
    KRYS_NODISCARD RawPtr<Node> LastChild() const noexcept
    {
      return _lastChild.get();
    }

#pragma endregion

  protected:
#pragma region Extension Hooks

    virtual void OnChildrenChanged() noexcept
    {
    }

#pragma endregion

#pragma region Relationships

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetFirstChild(RawPtr<Node> child) noexcept
    {
      _firstChild = ShareCheckedPtr(child);
    }

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetLastChild(RawPtr<Node> child) noexcept
    {
      _lastChild = ShareCheckedPtr(child);
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::ContainerNode)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();