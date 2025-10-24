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

  enum class Position
  {
    Static,
    Relative,
    Absolute,
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

    float GetWidth() const
    {
      return Layout::NodeStyleGetWidth(_layoutNode).Value;
    }

    void SetHeight(const Styles::UnitValue &value) const
    {
      if (value.Type == Styles::Unit::Point)
      {
        Layout::NodeStyleSetHeight(_layoutNode, value.Value);
      }
      else if (value.Type == Styles::Unit::Percent)
      {
        Layout::NodeStyleSetHeightPercent(_layoutNode, value.Value);
      }
      else
      {
        throw std::invalid_argument("Only 'px' or 'pct' is supported for height.");
      }
    }

    float GetHeight() const
    {
      return Layout::NodeStyleGetHeight(_layoutNode).Value;
    }

    void SetBackgroundColour(const Gfx::Colour &colour)
    {
      _backgroundColour = colour;
    }

    Gfx::Colour GetBackgroundColor() const
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

    void SetBorderColour(const Gfx::Colour &colour)
    {
      _borderColour = colour;
    }

    Gfx::Colour GetBorderColor() const
    {
      return _borderColour;
    }

    void SetPadding(const Styles::UnitValue &value) const
    {
      if (value.Type == Styles::Unit::Point)
      {
        Layout::NodeStyleSetPadding(_layoutNode, Styles::Edge::All, value.Value);
      }
      else if (value.Type == Styles::Unit::Percent)
      {
        Layout::NodeStyleSetPaddingPercent(_layoutNode, Styles::Edge::All, value.Value);
      }
      else
      {
        throw std::invalid_argument("Only 'px' or 'pct' is supported for padding.");
      }
    }

    void SetMargin(const Styles::UnitValue &value) const
    {
      if (value.Type == Styles::Unit::Point)
      {
        Layout::NodeStyleSetMargin(_layoutNode, Styles::Edge::All, value.Value);
      }
      else if (value.Type == Styles::Unit::Percent)
      {
        Layout::NodeStyleSetMarginPercent(_layoutNode, Styles::Edge::All, value.Value);
      }
      else
      {
        throw std::invalid_argument("Only 'px' or 'pct' is supported for margin.");
      }
    }

    void SetBorderWidth(const Styles::UnitValue &value) const
    {
      if (value.Type == Styles::Unit::Point)
      {
        Layout::NodeStyleSetBorder(_layoutNode, Styles::Edge::All, value.Value);
      }
      else
      {
        throw std::invalid_argument("Only 'px' is supported for border width.");
      }
    }

    float GetBorderWidth() const
    {
      return Layout::NodeStyleGetBorder(_layoutNode, Styles::Edge::All);
    }

    void SetBorderRadius(const Styles::UnitValue &radius)
    {
      if (radius.Type == Styles::Unit::Point)
      {
        _borderRadius = radius.Value;
      }
      else
      {
        throw std::invalid_argument("Only 'px' is supported for border radius.");
      }
    }

    float GetBorderRadius() const
    {
      return _borderRadius;
    }

    void SetFlexDirection(FlexDirection direction) const
    {
      Styles::FlexDirection layoutDirection = Styles::FlexDirection::Column;
      if (direction == FlexDirection::Row)
      {
        layoutDirection = Styles::FlexDirection::Row;
      }
      Layout::NodeStyleSetFlexDirection(_layoutNode, layoutDirection);
    }

    FlexDirection GetFlexDirection() const
    {
      Styles::FlexDirection layoutDirection = Layout::NodeStyleGetFlexDirection(_layoutNode);
      if (layoutDirection == Styles::FlexDirection::Row)
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
      Styles::Wrap layoutWrap = Styles::Wrap::NoWrap;
      if (wrap == Wrap::Wrap)
      {
        layoutWrap = Styles::Wrap::Wrap;
      }
      Layout::NodeStyleSetFlexWrap(_layoutNode, layoutWrap);
    }

    Wrap GetFlexWrap() const
    {
      Styles::Wrap layoutWrap = Layout::NodeStyleGetFlexWrap(_layoutNode);
      if (layoutWrap == Styles::Wrap::NoWrap)
      {
        return Wrap::NoWrap;
      }
      else if (layoutWrap == Styles::Wrap::Wrap)
      {
        return Wrap::Wrap;
      }
      else
      {
        return Wrap::WrapReverse;
      }
    }

    void SetPosition(Position position) const
    {
      Styles::PositionType layoutPosition = Styles::PositionType::Static;
      if (position == Position::Relative)
      {
        layoutPosition = Styles::PositionType::Relative;
      }
      else if (position == Position::Absolute)
      {
        layoutPosition = Styles::PositionType::Absolute;
      }
      Layout::NodeStyleSetPositionType(_layoutNode, layoutPosition);
    }

    Position GetPosition() const
    {
      Styles::PositionType layoutPosition = Layout::NodeStyleGetPositionType(_layoutNode);
      if (layoutPosition == Styles::PositionType::Relative)
      {
        return Position::Relative;
      }
      else if (layoutPosition == Styles::PositionType::Absolute)
      {
        return Position::Absolute;
      }
      else
      {
        return Position::Static;
      }
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

  class GroupElement : public Element
  {
    NO_COPY_MOVE(GroupElement)

  public:
    ~GroupElement() override = default;
  };
}