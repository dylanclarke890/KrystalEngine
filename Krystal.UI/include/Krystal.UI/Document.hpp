#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include "Krystal.UI/Element.hpp"
#include <cassert>

namespace Krys::UI
{
  class Document
  {
    NO_COPY_MOVE(Document)

    using ElementManager = Gfx::ResourceManager<Unique<Element>, ElementHandle>;

  private:
    Layout::ConfigRef _layoutConfig;
    ElementManager _elements;
    ElementHandle _body;

  public:
    Document();
    ~Document();

    /// @brief Create a new element.
    template <DerivedFrom<Element> TElement, typename... Args>
    NO_DISCARD ElementHandle CreateElement(Args &&...args)
    {
      ElementHandle handle = _elements.NextHandle();
      Unique<TElement> element = CreateUnique<TElement>(handle, _layoutConfig, std::forward<Args>(args)...);

      _elements.Set(handle, std::move(element));
      return handle;
    }

    Element &GetBody()
    {
      return *_elements.Get(_body).get();
    }

    const Element &GetBody() const
    {
      return *_elements.Get(_body).get();
    }

    template <DerivedFrom<Element> TElement = Element>
    NO_DISCARD TElement &GetByHandle(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");
      return static_cast<TElement &>(*_elements.Get(handle).get());
    }

    template <DerivedFrom<Element> TElement = Element>
    NO_DISCARD const TElement &GetByHandle(ElementHandle handle) const
    {
      assert(handle.IsValid() && "Invalid handle");
      return static_cast<const TElement &>(*_elements.Get(handle).get());
    }

    /// @brief Destroys the element and removes it from the document. Child elements are also destroyed
    /// recursively.
    void DestroyElement(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &element = _elements.Get(handle);
      for (const auto &childHandle : element->_children)
      {
        DestroyElement(childHandle);
      }

      if (element->_parent.IsValid())
      {
        auto &parentElement = _elements.Get(element->_parent);
        auto it = std::find(parentElement->_children.begin(), parentElement->_children.end(), handle);
        if (it != parentElement->_children.end())
        {
          Layout::NodeRemoveChild(parentElement->_layoutNode, element->_layoutNode);
          parentElement->_children.erase(it);
        }
      }

      _elements.Remove(handle);
    }

    /// @brief Adds the element to the document's body.
    void AddToBody(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &body = _elements.Get(_body);
      body->_children.push_back(handle);

      auto &element = _elements.Get(handle);
      element->_parent = _body;

      Layout::NodeInsertChild(body->_layoutNode, element->_layoutNode, body->_children.size() - 1);
    }

    /// @brief Detaches the element from it's parent but does not destroy it. Child elements remain intact.
    void Remove(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");
      auto &element = _elements.Get(handle);
      if (!element->_parent.IsValid())
      {
        return;
      }
      auto &parentElement = _elements.Get(element->_parent);
      auto it = std::find(parentElement->_children.begin(), parentElement->_children.end(), handle);
      if (it != parentElement->_children.end())
      {
        parentElement->_children.erase(it);
        Layout::NodeRemoveChild(parentElement->_layoutNode, element->_layoutNode);
      }
      element->_parent = ElementHandle {};
    }

    /// @brief Inserts a child element at the specified index under the given parent element.
    void InsertChild(ElementHandle parent, ElementHandle child, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement->_children.insert(parentElement->_children.begin() + index, child);

      auto &childElement = _elements.Get(child);
      childElement->_parent = parent;

      Layout::NodeInsertChild(parentElement->_layoutNode, childElement->_layoutNode, index);
    }

    /// @brief Appends a child element under the given parent element.
    void AppendChild(ElementHandle parent, ElementHandle child)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement->_children.push_back(child);

      auto &childElement = _elements.Get(child);
      childElement->_parent = parent;

      Layout::NodeInsertChild(parentElement->_layoutNode, childElement->_layoutNode,
                              parentElement->_children.size() - 1);
    }

    /// @brief Removes the child element at the specified index from the given parent element.
    void RemoveChildAt(ElementHandle parent, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");

      auto &parentElement = _elements.Get(parent);
      assert(index < parentElement->_children.size() && "Index out of bounds");

      ElementHandle childHandle = parentElement->_children[index];
      parentElement->_children.erase(parentElement->_children.begin() + index);

      auto &childElement = _elements.Get(childHandle);
      childElement->_parent = ElementHandle {};

      Layout::NodeRemoveChild(parentElement->_layoutNode, childElement->_layoutNode);
    }

    void Layout(float width, float height) noexcept
    {
      Layout::NodeCalculateLayout(_elements.Get(_body)->_layoutNode, width, height, Styles::Direction::LTR);
    }
  };
}