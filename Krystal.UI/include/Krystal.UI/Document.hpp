#pragma once

#include "Krystal.Gfx.Lib/ResourceManager.hpp"
#include "Krystal.Lib/Concepts.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI/Elements/Element.hpp"
#include "Krystal.UI/Layout/LayoutEngine.hpp"
#include <cassert>

namespace Krys::UI
{
  class Document
  {
    NO_COPY_MOVE(Document)

    using ElementManager = Gfx::ResourceManager<Unique<Element>, ElementHandle>;

  private:
    ConfigRef _layoutConfig;
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
      for (const auto &childHandle : element->Children)
      {
        DestroyElement(childHandle);
      }

      if (element->Parent.IsValid())
      {
        auto &parentElement = _elements.Get(element->Parent);
        auto it = std::find(parentElement->Children.begin(), parentElement->Children.end(), handle);
        if (it != parentElement->Children.end())
        {
          NodeRemoveChild(parentElement->LayoutNode, element->LayoutNode);
          parentElement->Children.erase(it);
        }
      }

      _elements.Remove(handle);
    }

    /// @brief Adds the element to the document's body.
    void AddToBody(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");

      auto &body = _elements.Get(_body);
      body->Children.push_back(handle);

      auto &element = _elements.Get(handle);
      element->Parent = _body;

      NodeInsertChild(body->LayoutNode, element->LayoutNode, body->Children.size() - 1);
    }

    /// @brief Detaches the element from it's parent but does not destroy it. Child elements remain intact.
    void Remove(ElementHandle handle)
    {
      assert(handle.IsValid() && "Invalid handle");
      auto &element = _elements.Get(handle);
      if (!element->Parent.IsValid())
      {
        return;
      }
      auto &parentElement = _elements.Get(element->Parent);
      auto it = std::find(parentElement->Children.begin(), parentElement->Children.end(), handle);
      if (it != parentElement->Children.end())
      {
        parentElement->Children.erase(it);
        NodeRemoveChild(parentElement->LayoutNode, element->LayoutNode);
      }
      element->Parent = ElementHandle {};
    }

    /// @brief Inserts a child element at the specified index under the given parent element.
    void InsertChild(ElementHandle parent, ElementHandle child, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement->Children.insert(parentElement->Children.begin() + index, child);

      auto &childElement = _elements.Get(child);
      childElement->Parent = parent;

      NodeInsertChild(parentElement->LayoutNode, childElement->LayoutNode, index);
    }

    /// @brief Appends a child element under the given parent element.
    void AppendChild(ElementHandle parent, ElementHandle child)
    {
      assert(parent.IsValid() && "Invalid parent handle");
      assert(child.IsValid() && "Invalid child handle");

      auto &parentElement = _elements.Get(parent);
      parentElement->Children.push_back(child);

      auto &childElement = _elements.Get(child);
      childElement->Parent = parent;

      NodeInsertChild(parentElement->LayoutNode, childElement->LayoutNode,
                      parentElement->Children.size() - 1);
    }

    /// @brief Removes the child element at the specified index from the given parent element.
    void RemoveChildAt(ElementHandle parent, size_t index)
    {
      assert(parent.IsValid() && "Invalid parent handle");

      auto &parentElement = _elements.Get(parent);
      assert(index < parentElement->Children.size() && "Index out of bounds");

      ElementHandle childHandle = parentElement->Children[index];
      parentElement->Children.erase(parentElement->Children.begin() + index);

      auto &childElement = _elements.Get(childHandle);
      childElement->Parent = ElementHandle {};

      NodeRemoveChild(parentElement->LayoutNode, childElement->LayoutNode);
    }

    void Layout(float width, float height) noexcept
    {
      NodeCalculateLayout(_elements.Get(_body)->LayoutNode, width, height, Direction::LTR);
    }
  };
}