#pragma once

#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Iterators/ElementAndTextDescendantIterator.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <limits>

namespace krys::boo::dom
{
  /// @brief Iterates over the element and text descendants of a node in the composed (flat) tree: shadow
  /// trees replace their hosts' children, and slots their fallback content with their assigned nodes.
  class ComposedTreeIterator
  {
    static constexpr size_t NotFound = std::numeric_limits<size_t>::max();

    struct Context
    {
      enum SlottedTag
      {
        Slotted
      };

      ElementAndTextDescendantIterator Iterator;
      ElementAndTextDescendantIterator End;
      size_t SlotNodeIndex {NotFound};

      Context() noexcept = default;

      Context(ContainerNode &root, ElementAndTextDescendantIterator::FirstChildTag) noexcept
          : Iterator(root, ElementAndTextDescendantIterator::FirstChild)
      {
      }

      Context(ContainerNode &root, Node &node) noexcept : Iterator(root, &node)
      {
      }

      Context(ContainerNode &root, Node &node, SlottedTag) noexcept : Iterator(root, &node), End(Iterator)
      {
        End.TraverseNextSkippingChildren();
      }
    };

  private:
    bool _rootIsInShadowTree {false};
    List<Context> _contextStack;

  public:
    enum FirstChildTag
    {
      FirstChild
    };

    ComposedTreeIterator() noexcept : _contextStack({Context {}})
    {
    }

    ComposedTreeIterator(ContainerNode &root, FirstChildTag) noexcept
        : _rootIsInShadowTree(TreeQueries::IsInShadowTree(root))
    {
      krys_debug_assert(!Is<ShadowRoot>(root));

      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(root))
      {
        if (auto *assignedNodes = slot->AssignedNodes())
        {
          InitializeContextStack(root, *assignedNodes->front());
          return;
        }
      }
      if (auto *shadowRoot = ShadowRootOf(root))
      {
        ElementAndTextDescendantIterator firstChild(*shadowRoot,
                                                    ElementAndTextDescendantIterator::FirstChild);
        InitializeContextStack(root, firstChild ? *firstChild : root);
        return;
      }

      _contextStack.push_back(Context(root, ElementAndTextDescendantIterator::FirstChild));
    }

    ComposedTreeIterator(ContainerNode &root, Node &current) noexcept
        : _rootIsInShadowTree(TreeQueries::IsInShadowTree(root))
    {
      krys_debug_assert(!Is<ShadowRoot>(root));
      krys_debug_assert(!Is<ShadowRoot>(current));

      bool mayNeedShadowStack = ShadowRootOf(root) || (&current != &root && current.ParentNode() != &root);
      if (mayNeedShadowStack)
      {
        InitializeContextStack(root, current);
      }
      else
      {
        _contextStack.push_back(Context(root, current));
      }
    }

    KRYS_NODISCARD Node &operator*() const noexcept
    {
      return Current();
    }

    KRYS_NODISCARD Node *operator->() const noexcept
    {
      return &Current();
    }

    KRYS_NODISCARD bool operator==(const ComposedTreeIterator &other) const noexcept
    {
      return GetContext().Iterator == other.GetContext().Iterator;
    }

    ComposedTreeIterator &operator++() noexcept
    {
      return TraverseNext();
    }

    ComposedTreeIterator &TraverseNext() noexcept
    {
      if (auto *shadowRoot = ShadowRootOf(*GetContext().Iterator))
      {
        TraverseShadowRoot(*shadowRoot);
        return *this;
      }

      if (_contextStack.size() > 1 || _rootIsInShadowTree)
      {
        TraverseNextInShadowTree();
        return *this;
      }

      GetContext().Iterator.TraverseNext();
      return *this;
    }

    ComposedTreeIterator &TraverseNextSkippingChildren() noexcept
    {
      GetContext().Iterator.TraverseNextSkippingChildren();

      if (GetContext().Iterator == GetContext().End)
      {
        TraverseNextLeavingContext();
      }

      return *this;
    }

    ComposedTreeIterator &TraverseNextSibling() noexcept
    {
      if (ShadowRootOf(*Current().ParentNode()))
      {
        TraverseSiblingInSlot(1);
        return *this;
      }
      GetContext().Iterator.TraverseNextSibling();
      return *this;
    }

    ComposedTreeIterator &TraversePreviousSibling() noexcept
    {
      if (ShadowRootOf(*Current().ParentNode()))
      {
        TraverseSiblingInSlot(-1);
        return *this;
      }
      GetContext().Iterator.TraversePreviousSibling();
      return *this;
    }

    KRYS_NODISCARD uint32 Depth() const noexcept
    {
      uint32 depth = 0;
      for (auto &context : _contextStack)
      {
        depth += context.Iterator.Depth();
      }
      return depth;
    }

  private:
    KRYS_NODISCARD static ShadowRoot *ShadowRootOf(const Node &node) noexcept
    {
      auto *element = DynamicDowncast<Element>(node);
      return element ? TreeQueries::GetShadowRoot(*element) : nullptr;
    }

    KRYS_NODISCARD Context &GetContext() noexcept
    {
      return _contextStack.back();
    }

    KRYS_NODISCARD const Context &GetContext() const noexcept
    {
      return _contextStack.back();
    }

    KRYS_NODISCARD Node &Current() const noexcept
    {
      return *GetContext().Iterator;
    }

    void InitializeContextStack(ContainerNode &root, Node &current) noexcept
    {
      // This code sets up the iterator for arbitrary node/root pair. It is not needed in common cases
      // or completes fast because node and root are close (like in composedTreeChildren(*parent).at(node)
      // case).
      auto *node = &current;
      auto *contextCurrent = node;
      size_t currentSlotNodeIndex = NotFound;
      while (node != &root)
      {
        auto *parent = node->ParentNode();
        if (!parent)
        {
          *this = {};
          return;
        }
        if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*parent))
        {
          _contextStack.push_back(Context(*shadowRoot, *contextCurrent));
          _contextStack.back().SlotNodeIndex = currentSlotNodeIndex;

          node = shadowRoot->Host();
          contextCurrent = node;
          currentSlotNodeIndex = NotFound;
          continue;
        }
        if (ShadowRootOf(*parent))
        {
          _contextStack.push_back(Context(*parent, *contextCurrent, Context::Slotted));
          _contextStack.back().SlotNodeIndex = currentSlotNodeIndex;

          auto *assignedSlot = SlotAlgorithms::GetAssignedSlot(*node);
          if (assignedSlot)
          {
            currentSlotNodeIndex = IndexOfAssignedNode(*assignedSlot, *node);
            krys_debug_assert(currentSlotNodeIndex != NotFound);
            node = assignedSlot;
            contextCurrent = assignedSlot;
            continue;
          }
          // The node is not part of the composed tree.
          *this = {};
          return;
        }
        node = parent;
      }
      _contextStack.push_back(Context(root, *contextCurrent));
      _contextStack.back().SlotNodeIndex = currentSlotNodeIndex;

      std::reverse(_contextStack.begin(), _contextStack.end());
    }

    void TraverseShadowRoot(ShadowRoot &shadowRoot) noexcept
    {
      Context shadowContext(shadowRoot, ElementAndTextDescendantIterator::FirstChild);
      if (!shadowContext.Iterator)
      {
        // Empty shadow root.
        TraverseNextSkippingChildren();
        return;
      }

      _contextStack.push_back(krys::move(shadowContext));
    }

    void TraverseNextInShadowTree() noexcept
    {
      krys_debug_assert(_contextStack.size() > 1 || _rootIsInShadowTree);

      if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(Current()))
      {
        if (auto *assignedNodes = slot->AssignedNodes())
        {
          GetContext().SlotNodeIndex = 0;
          auto &assignedNode = *assignedNodes->front();
          krys_debug_assert(Is<Element>(assignedNode.ParentNode()));
          _contextStack.push_back(Context(*assignedNode.ParentNode(), assignedNode, Context::Slotted));
          return;
        }
      }

      GetContext().Iterator.TraverseNext();

      if (GetContext().Iterator == GetContext().End)
      {
        TraverseNextLeavingContext();
      }
    }

    void TraverseNextLeavingContext() noexcept
    {
      while (GetContext().Iterator == GetContext().End && _contextStack.size() > 1)
      {
        _contextStack.pop_back();
        if (auto *slot = DynamicDowncast<html::HTMLSlotElement>(Current()); slot && AdvanceInSlot(1, *slot))
        {
          return;
        }
        if (GetContext().Iterator == GetContext().End)
        {
          return;
        }
        GetContext().Iterator.TraverseNextSkippingChildren();
      }
    }

    bool AdvanceInSlot(int32 direction, const html::HTMLSlotElement &slot) noexcept
    {
      krys_debug_assert(GetContext().SlotNodeIndex != NotFound);

      auto &assignedNodes = *slot.AssignedNodes();
      // It is fine to underflow this.
      GetContext().SlotNodeIndex += static_cast<size_t>(direction);
      if (GetContext().SlotNodeIndex >= assignedNodes.size())
      {
        return false;
      }

      auto &slotNode = *assignedNodes[GetContext().SlotNodeIndex];
      krys_debug_assert(Is<Element>(slotNode.ParentNode()));
      _contextStack.push_back(Context(*slotNode.ParentNode(), slotNode, Context::Slotted));
      return true;
    }

    void TraverseSiblingInSlot(int32 direction) noexcept
    {
      krys_debug_assert(_contextStack.size() > 1);
      krys_debug_assert(ShadowRootOf(*Current().ParentNode()));

      _contextStack.pop_back();

      if (!AdvanceInSlot(direction, Downcast<html::HTMLSlotElement>(Current())))
      {
        *this = {};
      }
    }

    KRYS_NODISCARD static size_t IndexOfAssignedNode(const html::HTMLSlotElement &slot,
                                                     const Node &node) noexcept
    {
      auto &assignedNodes = *slot.AssignedNodes();
      for (size_t i = 0; i < assignedNodes.size(); ++i)
      {
        if (assignedNodes[i].get() == &node)
        {
          return i;
        }
      }
      return NotFound;
    }
  };

  /// @brief The descendants of a node in the composed tree.
  class ComposedTreeDescendantAdapter
  {
  private:
    CheckedRef<ContainerNode> _parent;

  public:
    explicit ComposedTreeDescendantAdapter(ContainerNode &parent) noexcept : _parent(ShareCheckedRef(parent))
    {
    }

    KRYS_NODISCARD ComposedTreeIterator begin() const noexcept
    {
      return {*_parent, ComposedTreeIterator::FirstChild};
    }

    KRYS_NODISCARD ComposedTreeIterator end() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD ComposedTreeIterator at(const Node &child) const noexcept
    {
      return {*_parent, const_cast<Node &>(child)};
    }
  };

  /// @brief The children of a node in the composed tree.
  class ComposedTreeChildAdapter
  {
  public:
    class Iterator : public ComposedTreeIterator
    {
    public:
      Iterator() noexcept = default;

      explicit Iterator(ContainerNode &root) noexcept
          : ComposedTreeIterator(root, ComposedTreeIterator::FirstChild)
      {
      }

      Iterator(ContainerNode &root, Node &current) noexcept : ComposedTreeIterator(root, current)
      {
      }

      Iterator &operator++() noexcept
      {
        return static_cast<Iterator &>(ComposedTreeIterator::TraverseNextSibling());
      }

      Iterator &operator--() noexcept
      {
        return static_cast<Iterator &>(ComposedTreeIterator::TraversePreviousSibling());
      }
    };

  private:
    CheckedRef<ContainerNode> _parent;

  public:
    explicit ComposedTreeChildAdapter(ContainerNode &parent) noexcept : _parent(ShareCheckedRef(parent))
    {
    }

    KRYS_NODISCARD Iterator begin() const noexcept
    {
      return Iterator(*_parent);
    }

    KRYS_NODISCARD Iterator end() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD Iterator at(const Node &child) const noexcept
    {
      return Iterator(*_parent, const_cast<Node &>(child));
    }
  };

  KRYS_NODISCARD inline ComposedTreeDescendantAdapter ComposedTreeDescendants(ContainerNode &parent) noexcept
  {
    return ComposedTreeDescendantAdapter(parent);
  }

  KRYS_NODISCARD inline ComposedTreeChildAdapter ComposedTreeChildren(ContainerNode &parent) noexcept
  {
    return ComposedTreeChildAdapter(parent);
  }

  enum class ComposedTreeAsTextMode : uint8
  {
    Normal,
    WithPointers
  };

  /// @brief A description of the composed tree under @p root, one node per line, indented by depth.
  KRYS_NODISCARD DOMString ComposedTreeAsText(
    ContainerNode &root, ComposedTreeAsTextMode mode = ComposedTreeAsTextMode::Normal) noexcept;
}
