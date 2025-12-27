#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.UI/Styles/Enums/Display.hpp"
#include <cstdint>
#include <forward_list>
#include <utility>

namespace Krys::UI
{
  class Node;

  template <typename T>
  class LayoutableChildren
  {
  private:
    const T *_node {nullptr};

  public:
    struct Iterator
    {
      friend LayoutableChildren;

    private:
      const T *_iterNode {nullptr};
      size_t _childIndex {0};
      std::forward_list<std::pair<const T *, size_t>> _backtrack;

    public:
      using iterator_category = std::input_iterator_tag;
      using difference_type = std::ptrdiff_t;
      using value_type = T *;
      using pointer = T *;
      using reference = T *;

      Iterator() = default;

      Iterator(const T *node, size_t childIndex) : _iterNode(node), _childIndex(childIndex)
      {
      }

      T *operator*() const
      {
        return _iterNode->GetChild(_childIndex);
      }

      Iterator &operator++()
      {
        Next();
        return *this;
      }

      Iterator operator++(int)
      {
        Iterator tmp = *this;
        ++(*this);
        return tmp;
      }

      friend bool operator==(const Iterator &a, const Iterator &b)
      {
        return a._iterNode == b._iterNode && a._childIndex == b._childIndex;
      }

      friend bool operator!=(const Iterator &a, const Iterator &b)
      {
        return a._iterNode != b._iterNode || a._childIndex != b._childIndex;
      }

    private:
      void Next()
      {
        if (_childIndex + 1 >= _iterNode->GetChildCount())
        {
          // if the current node has no more children, try to backtrack and
          // visit its successor
          if (_backtrack.empty()) KRYS_LIKELY
          {
            // if there are no nodes to backtrack to, the last node has been
            // visited
            *this = Iterator {};
          }
          else
          {
            // pop and restore the latest backtrack entry
            const auto &back = _backtrack.front();
            _iterNode = back.first;
            _childIndex = back.second;
            _backtrack.pop_front();

            // go to the next node
            Next();
          }
        }
        else
        {
          // current node has more children to visit, go to next
          ++_childIndex;

          // skip all display: contents nodes, possibly going deeper into the tree
          if (_iterNode->GetChild(_childIndex)->GetStyle().GetDisplay() == Display::Contents) KRYS_UNLIKELY
          {
            SkipContentsNodes();
          }
        }
      }

      void SkipContentsNodes()
      {
        // get the node that would be returned from the iterator
        auto currentNode = _iterNode->GetChild(_childIndex);
        while (currentNode->GetStyle().GetDisplay() == Display::Contents && currentNode->GetChildCount() > 0)
        {
          // if it has display: contents set, it shouldn't be returned but its
          // children should in its place push the current node and child index
          // so that the current state can be restored when backtracking
          _backtrack.push_front({_iterNode, _childIndex});
          // traverse the child
          _iterNode = currentNode;
          _childIndex = 0;

          // repeat until a node without display: contents is found in the
          // subtree or a leaf is reached
          currentNode = currentNode->GetChild(_childIndex);
        }

        // if no node without display: contents was found, try to backtrack
        if (currentNode->GetStyle().GetDisplay() == Display::Contents)
        {
          Next();
        }
      }
    };

    explicit LayoutableChildren(const T *node) : _node(node)
    {
      static_assert(std::input_iterator<LayoutableChildren<T>::Iterator>);
      static_assert(std::is_base_of<Node, T>::value,
                    "Type parameter of LayoutableChildren must derive from yoga::Node");
    }

    Iterator begin() const
    {
      if (_node->GetChildCount() > 0)
      {
        auto result = Iterator(_node, 0);
        if (_node->GetChild(0)->GetStyle().GetDisplay() == Display::Contents) KRYS_UNLIKELY
        {
          result.SkipContentsNodes();
        }
        return result;
      }
      else
      {
        return Iterator {};
      }
    }

    Iterator end() const
    {
      return Iterator {};
    }
  };
}