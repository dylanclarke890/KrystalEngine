#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx.Lib/Handle.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI.Layout/LayoutEngine.hpp"
#include "Krystal.UI.Styles/Values/UnitValue.hpp"
#include "Krystal.UI/UnitLiterals.hpp"

namespace Krys::UI
{
  struct ElementHandle : public Gfx::Handle<ElementHandle>
  {
  };

  struct ComputedBounds
  {
    float X;
    float Y;
    float Width;
    float Height;

    NO_DISCARD bool Contains(float x, float y) const noexcept
    {
      return x >= X && x <= (X + Width) && y >= Y && y <= (Y + Height);
    }

    NO_DISCARD float MinX() const noexcept
    {
      return X;
    }

    NO_DISCARD float MinY() const noexcept
    {
      return Y;
    }

    NO_DISCARD float MaxX() const noexcept
    {
      return X + Width;
    }

    NO_DISCARD float MaxY() const noexcept
    {
      return Y + Height;
    }
  };

  class Element
  {
    NO_COPY_MOVE(Element)

    friend class Document;

  protected:
    ElementHandle _handle;
    ElementHandle _parent;
    List<ElementHandle> _children;
    Layout::NodeRef _layoutNode {nullptr};
    Gfx::Colour _backgroundColour {Gfx::Colours::Transparent};
    Gfx::Colour _foregroundColour {Gfx::Colours::Black};
    Gfx::Colour _borderColour {Gfx::Colours::Transparent};
    float _borderRadius {0.0f};

  public:
    Element(ElementHandle handle, Layout::ConfigRef);
    virtual ~Element();

    void SetWidth(const Styles::UnitValue &value) const
    {
      if (value.Type == Styles::Unit::Point)
      {
        Layout::NodeStyleSetWidth(_layoutNode, value.Value);
      }
      else if (value.Type == Styles::Unit::Percent)
      {
        Layout::NodeStyleSetWidthPercent(_layoutNode, value.Value);
      }
      else
      {
        throw std::invalid_argument("Only 'px' or 'pct' is supported for width.");
      }
    }

    ElementHandle GetHandle() const noexcept
    {
      return _handle;
    }

    ElementHandle GetParent() const noexcept
    {
      return _parent;
    }

    float GetBorderRadius() const
    {
      return _borderRadius;
    }

    ComputedBounds GetComputedBounds() const
    {
      return {
        .X = Layout::NodeLayoutGetLeft(_layoutNode),
        .Y = Layout::NodeLayoutGetTop(_layoutNode),
        .Width = Layout::NodeLayoutGetWidth(_layoutNode),
        .Height = Layout::NodeLayoutGetHeight(_layoutNode),
      };
    }

    NO_DISCARD bool CreatesStackingContext() const noexcept
    {
      return _backgroundColour.a < 1.0f || _borderColour.a > 0.0f;
    }

    const List<ElementHandle> &GetChildren() const noexcept
    {
      return _children;
    }
  };
}