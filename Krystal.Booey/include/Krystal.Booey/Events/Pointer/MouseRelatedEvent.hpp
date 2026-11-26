#pragma once

#include "Krystal.Booey/Events/UI/UIEventWithKeyState.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo
{
  class FrameView;
  struct MouseEventInit;

  /// @brief The base of the events at a point (mouse, wheel, pointer and touch-driven ones), which give that
  /// point in several coordinate spaces: the screen, the viewport (client), the document (page), the target's
  /// padding box (offset) and its layer.
  class MouseRelatedEvent : public UIEventWithKeyState
  {
  public:
    /// @brief Whether the event stands in for another (a click for a key press, say), so it isn't really at
    /// a point: its offsets are 0.
    enum class Simulated : bool
    {
      No,
      Yes,
    };

  private:
    RefPtr<boo::FrameView> _view;
    double _movementX {0};
    double _movementY {0};
    /// @brief Where the host put the event, in its window.
    PointD _windowLocation;
    PointD _screenLocation;
    PointD _clientLocation;
    PointD _pageLocation;
    /// @brief The page location with the page's zoom applied, in the render tree's coordinates.
    PointD _absoluteLocation;
    // The layer and offset locations depend on the target, so they're worked out when first read.
    mutable LayoutPoint _layerLocation;
    mutable PointD _offsetLocation;
    /// @brief The target the relative position was computed for, if it's cached.
    mutable const EventTarget *_relativePositionTarget {nullptr};
    bool _isSimulated {false};

  public:
    ~MouseRelatedEvent() noexcept override;

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-screenx
    KRYS_NODISCARD virtual double ScreenX() const noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-screeny
    KRYS_NODISCARD virtual double ScreenY() const noexcept;

    /// @brief The unrounded screen coordinates.
    KRYS_NODISCARD const PointD &ScreenLocation() const noexcept
    {
      return _screenLocation;
    }

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-clientx
    KRYS_NODISCARD virtual double ClientX() const noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-clienty
    KRYS_NODISCARD virtual double ClientY() const noexcept;

    /// @brief The unrounded client coordinates.
    KRYS_NODISCARD const PointD &ClientLocation() const noexcept
    {
      return _clientLocation;
    }

    /// @see https://w3c.github.io/pointerlock/#dom-mouseevent-movementx
    KRYS_NODISCARD double MovementX() const noexcept
    {
      return _movementX;
    }

    /// @see https://w3c.github.io/pointerlock/#dom-mouseevent-movementy
    KRYS_NODISCARD double MovementY() const noexcept
    {
      return _movementY;
    }

    /// @brief Where the host put a trusted event, in its window's coordinates.
    KRYS_NODISCARD const PointD &WindowLocation() const noexcept
    {
      return _windowLocation;
    }

    /// @brief The point relative to the target's nearest enclosing layer.
    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-layerx
    KRYS_NODISCARD int32 LayerX() const noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-mouseevent-layery
    KRYS_NODISCARD int32 LayerY() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-offsetx
    KRYS_NODISCARD virtual double OffsetX() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-offsety
    KRYS_NODISCARD virtual double OffsetY() const noexcept;

    /// @brief See `Simulated`.
    KRYS_NODISCARD bool IsSimulated() const noexcept
    {
      return _isSimulated;
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-pagex
    KRYS_NODISCARD virtual double PageX() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-pagey
    KRYS_NODISCARD virtual double PageY() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-x
    KRYS_NODISCARD double X() const noexcept
    {
      return ClientX();
    }

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-y
    KRYS_NODISCARD double Y() const noexcept
    {
      return ClientY();
    }

    /// @brief See `_absoluteLocation`: the point to map into a renderer's coordinates (`AbsoluteToLocal`).
    KRYS_NODISCARD const PointD &AbsoluteLocation() const noexcept
    {
      return _absoluteLocation;
    }

    /// @brief The frame view of the event's view, which its coordinates are relative to.
    KRYS_NODISCARD boo::FrameView *FrameView() const noexcept;

  protected:
    /// @brief A trusted event the host put at @p windowLocation in @p view.
    MouseRelatedEvent(DOMStringAtom type, const UIEventInit &eventInitDict, boo::FrameView *view,
                      const PointD &screenLocation, const PointD &windowLocation, double movementX,
                      double movementY, EnumSet<Modifier> modifiers, Simulated isSimulated) noexcept;

    /// @brief An event from a page's `MouseEventInit`.
    MouseRelatedEvent(DOMStringAtom type, const MouseEventInit &initializer) noexcept;

    /// @brief Puts the event at @p clientLocation, working out its page and absolute locations from it.
    void InitCoordinates(const PointD &clientLocation) noexcept;

    /// @brief Makes the frame view the one of the event's view, after that changed.
    void UpdateFrameView() noexcept;

    void SetScreenLocation(const PointD &screenLocation) noexcept
    {
      _screenLocation = screenLocation;
    }

    void SetIsSimulated(bool value) noexcept
    {
      _isSimulated = value;
    }

    KRYS_NODISCARD const PointD &PageLocation() const noexcept
    {
      return _pageLocation;
    }

    KRYS_NODISCARD const PointD &OffsetLocation() const noexcept
    {
      return _offsetLocation;
    }

    /// @brief Works out the offset and layer locations for the current target.
    void ComputeRelativePosition() const noexcept;

    /// @brief Whether the offset and layer locations were worked out for the current target.
    KRYS_NODISCARD bool HasCachedRelativePosition() const noexcept;

  private:
    void Init(bool isSimulated, const PointD &windowLocation) noexcept;

    void InitCoordinates() noexcept;

    void ComputeAbsoluteLocation() noexcept;

    KRYS_NODISCARD float DocumentToAbsoluteScaleFactor() const noexcept;
  };
}
