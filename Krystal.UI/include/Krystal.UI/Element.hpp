#pragma once

#include "Krystal.Gfx/Handle.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/SmartPointers.hpp"
#include "Krystal.Maths/Vector.hpp"
#include "Krystal.UI/Properties.hpp"

namespace Krys::UI
{
  class Element
  {
    NO_COPY_MOVE(Element)

    friend class Document;
    friend class LayoutEngine;

  protected:
    WeakRef<Element> _parent;
    List<Unique<Element>> _children;
    Properties _properties;
    ComputedLayout _layout;

  public:
    Element() noexcept = default;

    virtual ~Element() = default;
  };

  class Body final : public Element
  {
    NO_COPY_MOVE(Body)

  public:
    Body() noexcept : Element()
    {
      _properties.Width = 100._pct;
      _properties.Height = 100._pct;
      _properties.BackgroundColour = Gfx::Colours::White;
      _properties.Text.Colour = Gfx::Colours::Black;
    }

    ~Body() noexcept override = default;

    const Properties &GetProperties() const noexcept
    {
      return _properties;
    }

    const ComputedLayout &GetComputedLayout() const noexcept
    {
      return _layout;
    }
  };
}