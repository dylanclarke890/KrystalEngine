#pragma once

#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  /// @brief Iterates over the element and text descendants of a node in tree order, keeping track of their
  /// depth below it.
  class ElementAndTextDescendantIterator
  {
    struct AncestorSibling
    {
      CheckedPtr<dom::Node> Sibling;
      uint32 Depth;
    };

  private:
    CheckedPtr<Node> _current;
    List<AncestorSibling> _ancestorSiblingStack;
    uint32 _depth {0};

  public:
    enum FirstChildTag
    {
      FirstChild
    };

    ElementAndTextDescendantIterator() noexcept = default;

    ElementAndTextDescendantIterator(const ContainerNode &root, FirstChildTag) noexcept
        : _current(ShareCheckedPtr(FirstChildOf(root)))
    {
      if (!_current)
      {
        return;
      }
      _ancestorSiblingStack.push_back({nullptr, 0});
      _depth = 1;
    }

    ElementAndTextDescendantIterator(const ContainerNode &root, Node *current) noexcept
        : _current(ShareCheckedPtr(current))
    {
      if (!_current)
      {
        return;
      }
      krys_debug_assert(IsElementOrText(*_current));
      if (_current.get() == &root)
      {
        return;
      }

      List<Node *> ancestorStack;
      auto *ancestor = static_cast<Node *>(_current->ParentNode());
      while (ancestor != &root)
      {
        ancestorStack.push_back(ancestor);
        ancestor = ancestor->ParentNode();
      }

      _ancestorSiblingStack.push_back({nullptr, 0});
      for (auto i = static_cast<uint32>(ancestorStack.size()); i; --i)
      {
        if (auto *sibling = NextSiblingOf(*ancestorStack[i - 1]))
        {
          _ancestorSiblingStack.push_back({ShareCheckedPtr(sibling), i});
        }
      }

      _depth = static_cast<uint32>(ancestorStack.size()) + 1;
    }

    ElementAndTextDescendantIterator &operator++() noexcept
    {
      return TraverseNext();
    }

    KRYS_NODISCARD Node &operator*() const noexcept
    {
      krys_debug_assert(_current && IsElementOrText(*_current));
      return *_current;
    }

    KRYS_NODISCARD Node *operator->() const noexcept
    {
      krys_debug_assert(_current && IsElementOrText(*_current));
      return _current.get();
    }

    KRYS_NODISCARD bool operator==(const ElementAndTextDescendantIterator &other) const noexcept
    {
      return _current == other._current || (!_depth && !other._depth);
    }

    KRYS_NODISCARD bool operator!() const noexcept
    {
      return !_depth;
    }

    KRYS_NODISCARD explicit operator bool() const noexcept
    {
      return _depth;
    }

    ElementAndTextDescendantIterator &TraverseNext() noexcept
    {
      krys_debug_assert(_current);

      auto *firstChild = FirstChildOf(*_current);
      auto *nextSibling = NextSiblingOf(*_current);
      if (firstChild)
      {
        if (nextSibling)
        {
          _ancestorSiblingStack.push_back({ShareCheckedPtr(nextSibling), _depth});
        }
        ++_depth;
        _current = ShareCheckedPtr(firstChild);
        return *this;
      }
      if (!nextSibling)
      {
        PopAncestorSiblingStack();
        return *this;
      }

      _current = ShareCheckedPtr(nextSibling);
      return *this;
    }

    ElementAndTextDescendantIterator &TraverseNextSkippingChildren() noexcept
    {
      krys_debug_assert(_current);

      auto *nextSibling = NextSiblingOf(*_current);
      if (!nextSibling)
      {
        PopAncestorSiblingStack();
        return *this;
      }

      _current = ShareCheckedPtr(nextSibling);
      return *this;
    }

    ElementAndTextDescendantIterator &TraverseNextSibling() noexcept
    {
      krys_debug_assert(_current);
      _current = ShareCheckedPtr(NextSiblingOf(*_current));
      return *this;
    }

    ElementAndTextDescendantIterator &TraversePreviousSibling() noexcept
    {
      krys_debug_assert(_current);
      _current = ShareCheckedPtr(PreviousSiblingOf(*_current));
      return *this;
    }

    KRYS_NODISCARD uint32 Depth() const noexcept
    {
      return _depth;
    }

  private:
    KRYS_NODISCARD static bool IsElementOrText(const Node &node) noexcept
    {
      return Is<Element>(node) || Is<Text>(node);
    }

    KRYS_NODISCARD static Node *FirstChildOf(const Node &current) noexcept
    {
      auto *node = current.FirstChild();
      while (node && !IsElementOrText(*node))
      {
        node = node->NextSibling();
      }
      return node;
    }

    KRYS_NODISCARD static Node *NextSiblingOf(const Node &current) noexcept
    {
      auto *node = current.NextSibling();
      while (node && !IsElementOrText(*node))
      {
        node = node->NextSibling();
      }
      return node;
    }

    KRYS_NODISCARD static Node *PreviousSiblingOf(const Node &current) noexcept
    {
      auto *node = current.PreviousSibling();
      while (node && !IsElementOrText(*node))
      {
        node = node->PreviousSibling();
      }
      return node;
    }

    void PopAncestorSiblingStack() noexcept
    {
      _current = _ancestorSiblingStack.back().Sibling;
      _depth = _ancestorSiblingStack.back().Depth;
      _ancestorSiblingStack.pop_back();
    }
  };

  /// @brief The element and text descendants of a node, in tree order.
  class ElementAndTextDescendantRange
  {
  private:
    CheckedRef<const ContainerNode> _root;

  public:
    explicit ElementAndTextDescendantRange(const ContainerNode &root) noexcept : _root(ShareCheckedRef(root))
    {
    }

    KRYS_NODISCARD ElementAndTextDescendantIterator begin() const noexcept
    {
      return {*_root, ElementAndTextDescendantIterator::FirstChild};
    }

    KRYS_NODISCARD ElementAndTextDescendantIterator end() const noexcept
    {
      return {};
    }
  };

  KRYS_NODISCARD inline ElementAndTextDescendantRange ElementAndTextDescendants(ContainerNode &root) noexcept
  {
    return ElementAndTextDescendantRange(root);
  }
}
