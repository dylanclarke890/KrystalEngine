#pragma once

#include "Krystal.Booey/DOM/Nodes/Node.hpp"

namespace krys::boo::dom
{
  class ExtensibilityHooks;
  class MutationAlgorithms;
}

namespace krys::boo
{
  class Element;

  /// @brief A node that can have children: an `Element`, a `Document` or a `DocumentFragment`.
  /// @note The DOM has no such interface. Keeping the child links here, rather than in every node, saves the
  /// nodes that can't have children the memory.
  class ContainerNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ContainerNode);

    friend class dom::ExtensibilityHooks;
    friend class dom::MutationAlgorithms;

  public:
    /// @brief A child inserted into or removed from a container, or a child's text changing, as style
    /// invalidation sees it: what changed, and the element siblings either side of it.
    struct ChildChange
    {
      /// @brief What changed. A non-contents child is a comment, processing instruction or doctype: it
      /// changes the tree but not what's rendered.
      enum class Type : uint8
      {
        ElementInserted,
        ElementRemoved,
        TextInserted,
        TextRemoved,
        TextChanged,
        NonContentsChildRemoved,
        NonContentsChildInserted,
      };

      /// @brief The parser appends to elements whose children it's still parsing; everything else is the API.
      enum class Source : uint8
      {
        Parser,
        API,
      };

      /// @brief What changed.
      ChildChange::Type ChangeType;
      /// @brief The element siblings before and after the child that changed; null at either end.
      Element *PreviousSiblingElement;
      Element *NextSiblingElement;
      /// @brief Who made the change.
      ChildChange::Source ChangeSource;

      /// @brief Whether a child was inserted.
      KRYS_NODISCARD bool IsInsertion() const noexcept
      {
        switch (ChangeType)
        {
          case ChildChange::Type::ElementInserted:
          case ChildChange::Type::TextInserted:
          case ChildChange::Type::NonContentsChildInserted: return true;
          case ChildChange::Type::ElementRemoved:
          case ChildChange::Type::TextRemoved:
          case ChildChange::Type::TextChanged:
          case ChildChange::Type::NonContentsChildRemoved:  return false;
        }
        return false;
      }
    };

  private:
    CheckedPtr<Node> _firstChild;
    CheckedPtr<Node> _lastChild;

  protected:
    ContainerNode(Document &document, dom::NodeType type, dom::NodeFlags flags) noexcept;

  public:
    ~ContainerNode() noexcept override;

  protected:
    /// @brief Unlinks the children, without the removal steps (the container is going), and drops the tree
    /// references they hold.
    void RemoveDetachedChildren() noexcept;

  public:
#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-firstchild
    KRYS_NODISCARD Node *FirstChild() const noexcept
    {
      return _firstChild.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-lastchild
    KRYS_NODISCARD Node *LastChild() const noexcept
    {
      return _lastChild.get();
    }

#pragma endregion

  protected:
#pragma region Extension Hooks

    /// @brief Called after the container's children change, for containers that act on their contents.
    virtual void OnChildrenChanged() noexcept
    {
    }

#pragma endregion

#pragma region Relationships

    /// @warning Nothing checks that the tree stays valid.
    void SetFirstChild(Node *child) noexcept
    {
      _firstChild = ShareCheckedPtr(child);
    }

    /// @warning Nothing checks that the tree stays valid.
    void SetLastChild(Node *child) noexcept
    {
      _lastChild = ShareCheckedPtr(child);
    }

#pragma endregion
  };

  inline ContainerNode *Node::ParentNode() const noexcept
  {
    return _parentNode.get();
  }

  inline void Node::SetParentNode(ContainerNode *parent) noexcept
  {
    _parentNode = ShareCheckedPtr(parent);
  }
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ContainerNode)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
