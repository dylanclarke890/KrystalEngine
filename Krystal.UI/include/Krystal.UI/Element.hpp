#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Properties.hpp"

namespace Krys::UI
{
  struct ElementHandle : public Gfx::Handle<ElementHandle>
  {
  };

  class Element
  {
    NO_COPY_MOVE(Element)

    friend class Document;
    friend class LayoutEngine;
    friend class ElementPool;

  protected:
    ElementHandle _id;
    ElementHandle _parent;
    List<ElementHandle> _children;
    Properties _properties;
    ComputedLayout _layout;

  public:
    Element() noexcept = default;

    virtual ~Element() = default;

    ElementHandle GetId() const noexcept
    {
      return _id;
    }

    const List<ElementHandle> &GetChildren() const noexcept
    {
      return _children;
    }

    ElementHandle GetParent() const noexcept
    {
      return _parent;
    }

    const Properties &GetProperties() const noexcept
    {
      return _properties;
    }

    Properties &Properties() noexcept
    {
      return _properties;
    }

    const ComputedLayout &GetComputedLayout() const noexcept
    {
      return _layout;
    }

    ComputedLayout &ComputedLayout() noexcept
    {
      return _layout;
    }

    void AppendChild(ElementHandle child)
    {
      _children.push_back(child);
    }

    void AppendChild(Element &child)
    {
      _children.push_back(child._id);
    }
  };

  class Body final : public Element
  {
    NO_COPY_MOVE(Body)

  public:
    Body() noexcept : Element()
    {
      using namespace UI::Literals;

      _properties.Width = 100._pct;
      _properties.Height = 100._pct;
      _properties.BackgroundColour = Gfx::Colours::White;
      _properties.TextStyles.Colour = Gfx::Colours::Black;
    }

    ~Body() noexcept override = default;
  };
}