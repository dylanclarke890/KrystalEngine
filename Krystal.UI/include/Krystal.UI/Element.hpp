#pragma once

#include "Krystal.Gfx.Lib/Colour.hpp"
#include "Krystal.Gfx.Lib/Handle.hpp"
#include "Krystal.Lib/List.hpp"
#include "Krystal.Lib/Macros.hpp"
#include "Krystal.Lib/Types.hpp"
#include "Krystal.UI.Layout/LayoutEngine.hpp"

namespace Krys::UI
{
  enum class FlexDirection
  {
    Column,
    Row,
  };

  enum class Wrap
  {
    NoWrap,
    Wrap,
    WrapReverse,
  };

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

  protected:
    ElementHandle _parent;
    List<ElementHandle> _children;
    Layout::NodeRef _layoutNode {nullptr};
    Gfx::Colour _backgroundColour {Gfx::Colours::Transparent};
    Gfx::Colour _foregroundColour {Gfx::Colours::Black};

  public:
    Element() = default;
    virtual ~Element();

    void SetWidth(float width) const
    {
      Layout::NodeStyleSetWidth(_layoutNode, width);
    }

    float GetWidth() const
    {
      return Layout::NodeStyleGetWidth(_layoutNode).Value;
    }
    void SetHeight(float height) const
    {
      Layout::NodeStyleSetHeight(_layoutNode, height);
    }

    float GetHeight() const
    {
      return Layout::NodeStyleGetHeight(_layoutNode).Value;
    }

    void SetBackgroundColour(const Gfx::Colour &colour)
    {
      _backgroundColour = colour;
    }

    void SetPadding(uint32 p) const
    {
      Layout::NodeStyleSetPadding(_layoutNode, Layout::Edge::All, static_cast<float>(p));
    }

    void SetMargin(uint32 p) const
    {
      Layout::NodeStyleSetMargin(_layoutNode, Layout::Edge::All, static_cast<float>(p));
    }

    void SetFlexDirection(FlexDirection direction) const
    {
      Layout::FlexDirection layoutDirection = Layout::FlexDirection::Column;
      if (direction == FlexDirection::Row)
      {
        layoutDirection = Layout::FlexDirection::Row;
      }
      Layout::NodeStyleSetFlexDirection(_layoutNode, layoutDirection);
    }

    FlexDirection GetFlexDirection() const
    {
      Layout::FlexDirection layoutDirection = Layout::NodeStyleGetFlexDirection(_layoutNode);
      if (layoutDirection == Layout::FlexDirection::Row)
      {
        return FlexDirection::Row;
      }
      else
      {
        return FlexDirection::Column;
      }
    }

    void SetWrap(Wrap wrap) const
    {
      Layout::Wrap layoutWrap = Layout::Wrap::NoWrap;
      if (wrap == Wrap::Wrap)
      {
        layoutWrap = Layout::Wrap::Wrap;
      }
      Layout::NodeStyleSetFlexWrap(_layoutNode, layoutWrap);
    }

    Wrap GetFlexWrap() const
    {
      Layout::Wrap layoutWrap = Layout::NodeStyleGetFlexWrap(_layoutNode);
      if (layoutWrap == Layout::Wrap::NoWrap)
      {
        return Wrap::NoWrap;
      }
      else if (layoutWrap == Layout::Wrap::Wrap)
      {
        return Wrap::Wrap;
      }
      else
      {
        return Wrap::WrapReverse;
      }
    }
     

    Gfx::Colour
      GetBackgroundColor() const
    {
      return _backgroundColour;
    }

    void SetForegroundColour(const Gfx::Colour &colour)
    {
      _foregroundColour = colour;
    }

    Gfx::Colour GetForegroundColor() const
    {
      return _foregroundColour;
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

    const List<ElementHandle> &GetChildren() const noexcept
    {
      return _children;
    }

  private:
    friend class Document;

    void CreateLayoutNode(Layout::ConfigRef layoutConfig);
  };

  class GroupElement : public Element
  {
    NO_COPY_MOVE(GroupElement)

  public:
    ~GroupElement() override = default;
  };
}