#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Stack.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Elements/Element.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <cassert>

namespace Krys::UI
{
  class Document
  {
    NO_COPY_MOVE(Document)

    using ElementManager = Gfx::ResourceManager<Element, ElementHandle>;

  private:
    ConfigRef _layoutConfig;
    ElementManager _elements;
    ElementHandle _body;

  public:
    Document() : _layoutConfig(ConfigCreate())
    {
      _body = Create<Element>();
    }

    ~Document()
    {
      ConfigDestroy(_layoutConfig);
    }

    ElementHandle Body() const noexcept
    {
      return _body;
    }

    void Reflow(float width, float height)
    {
      NodeCalculateLayout(_elements.Get(_body).LayoutNode, width, height, Direction::LTR);
    }

    /// @brief Create a new element.
    template <DerivedFrom<Element> TElement, typename... Args>
    NO_DISCARD ElementHandle Create(Args &&...args)
    {
      ElementHandle handle = _elements.NextHandle();
      TElement element = TElement(handle, _layoutConfig, std::forward<Args>(args)...);

      _elements.Set(handle, std::move(element));
      return handle;
    }

    template <typename Fn>
    void SetStyle(ElementHandle handle, Fn &&setter)
    {
      assert(handle.IsValid() && "Invalid handle");
      auto &element = _elements.Get(handle);
      setter(element.LayoutNode);
    }

    /// @brief Detaches the element from it's parent but does not destroy it. Child elements remain intact.
    void Remove(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &element = _elements.Get(handle);
      if (!element.Parent.IsValid())
      {
        return;
      }

      auto &parentElement = _elements.Get(element.Parent);
      auto it = std::find(parentElement.Children.begin(), parentElement.Children.end(), handle);
      if (it != parentElement.Children.end())
      {
        parentElement.Children.erase(it);
        NodeRemoveChild(parentElement.LayoutNode, element.LayoutNode);
      }
      element.Parent = ElementHandle {};
    }

    /// @brief Appends a child element under the given parent element.
    void AppendChild(ElementHandle parent, ElementHandle child)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement.Children.push_back(child);

      auto &childElement = _elements.Get(child);
      childElement.Parent = parent;

      NodeInsertChild(parentElement.LayoutNode, childElement.LayoutNode, parentElement.Children.size() - 1);
    }

    /// @brief Inserts a child element at the specified index under the given parent element.
    void InsertChild(ElementHandle parent, ElementHandle child, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement.Children.insert(parentElement.Children.begin() + index, child);

      auto &childElement = _elements.Get(child);
      childElement.Parent = parent;

      NodeInsertChild(parentElement.LayoutNode, childElement.LayoutNode, index);
    }

    /// @brief Removes the child element at the specified index from the given parent element.
    void RemoveChildAt(ElementHandle parent, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");

      auto &parentElement = _elements.Get(parent);
      assert(index < parentElement.Children.size() && "Index out of bounds");

      ElementHandle childHandle = parentElement.Children[index];
      parentElement.Children.erase(parentElement.Children.begin() + index);

      auto &childElement = _elements.Get(childHandle);
      childElement.Parent = ElementHandle {};

      NodeRemoveChild(parentElement.LayoutNode, childElement.LayoutNode);
    }

    /// @brief Destroys the element and removes it from the document. Child elements are also destroyed
    /// recursively.
    void Destroy(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &element = _elements.Get(handle);
      for (const auto &childHandle : element.Children)
      {
        Destroy(childHandle);
      }

      if (element.Parent.IsValid())
      {
        auto &parentElement = _elements.Get(element.Parent);
        auto it = std::find(parentElement.Children.begin(), parentElement.Children.end(), handle);
        if (it != parentElement.Children.end())
        {
          NodeRemoveChild(parentElement.LayoutNode, element.LayoutNode);
          parentElement.Children.erase(it);
        }
      }

      _elements.Remove(handle);
    }

    /// @brief Depth-first iterator for element traversal
    class DepthFirstIterator
    {
    private:
      Document *_doc;
      Stack<ElementHandle> _stack;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = ElementHandle;
      using reference = ElementHandle &;
      using pointer = ElementHandle *;
      using difference_type = std::ptrdiff_t;

      DepthFirstIterator(Document *doc, ElementHandle start) : _doc(doc)
      {
        if (start.IsValid())
        {
          _stack.push(start);
        }
      }

      ElementHandle operator*() const
      {
        return _stack.top();
      }

      DepthFirstIterator &operator++()
      {
        if (_stack.empty())
        {
          return *this;
        }

        ElementHandle current = _stack.top();
        _stack.pop();

        auto &element = _doc->_elements.Get(current);
        // Push children in reverse order so leftmost child is processed first
        for (auto it = element.Children.rbegin(); it != element.Children.rend(); ++it)
        {
          _stack.push(*it);
        }

        return *this;
      }

      bool operator==(const DepthFirstIterator &other) const
      {
        if (_stack.empty() && other._stack.empty())
        {
          return true;
        }
        if (_stack.empty() || other._stack.empty())
        {
          return false;
        }
        return _stack.top() == other._stack.top();
      }

      bool operator!=(const DepthFirstIterator &other) const
      {
        return !(*this == other);
      }
    };

    DepthFirstIterator BeginDepthFirst()
    {
      return DepthFirstIterator(this, _body);
    }

    DepthFirstIterator EndDepthFirst()
    {
      return DepthFirstIterator(this, ElementHandle {});
    }

    /// @brief Breadth-first iterator for element traversal
    class BreadthFirstIterator
    {
    private:
      Document *_doc;
      Queue<ElementHandle> _queue;

    public:
      using iterator_category = std::forward_iterator_tag;
      using value_type = ElementHandle;
      using reference = ElementHandle &;
      using pointer = ElementHandle *;
      using difference_type = std::ptrdiff_t;

      BreadthFirstIterator(Document *doc, ElementHandle start) : _doc(doc)
      {
        if (start.IsValid())
        {
          _queue.push(start);
        }
      }

      ElementHandle operator*() const
      {
        return _queue.front();
      }

      BreadthFirstIterator &operator++()
      {
        if (_queue.empty())
        {
          return *this;
        }

        ElementHandle current = _queue.front();
        _queue.pop();

        auto &element = _doc->_elements.Get(current);
        for (auto &child : element.Children)
        {
          _queue.push(child);
        }

        return *this;
      }

      bool operator==(const BreadthFirstIterator &other) const
      {
        if (_queue.empty() && other._queue.empty())
        {
          return true;
        }
        if (_queue.empty() || other._queue.empty())
        {
          return false;
        }
        return _queue.front() == other._queue.front();
      }

      bool operator!=(const BreadthFirstIterator &other) const
      {
        return !(*this == other);
      }
    };

    BreadthFirstIterator BeginBreadthFirst()
    {
      return BreadthFirstIterator(this, _body);
    }

    BreadthFirstIterator EndBreadthFirst()
    {
      return BreadthFirstIterator(this, ElementHandle {});
    }
  };
}