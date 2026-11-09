/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2004, 2005, 2006, 2013 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Events/UIEventWithKeyState.hpp"
#include "Krystal.Booey/Graphics/Geometry/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutPoint.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class LocalFrameView;
}

namespace krys::boo::dom
{
  struct MouseEventInit;

  /// @brief An event at a point on the screen: its coordinates in the screen, the viewport, the page, the
  /// target and its layer.
  class MouseRelatedEvent : public UIEventWithKeyState
  {
  public:
    enum class Simulated : bool
    {
      No,
      Yes
    };

  private:
    RefPtr<LocalFrameView> _view;
    double _movementX {0};
    double _movementY {0};
    DoublePoint _windowLocation;
    DoublePoint _screenLocation;
    DoublePoint _clientLocation;
    DoublePoint _pageLocation;
    DoublePoint _absoluteLocation;
    mutable LayoutPoint _layerLocation;
    mutable DoublePoint _offsetLocation;
    /// @brief The target the relative position was computed for, if it's cached.
    mutable const EventTarget *_relativePositionTarget {nullptr};
    bool _isSimulated {false};

  public:
    ~MouseRelatedEvent() noexcept override;

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-screenx
    KRYS_NODISCARD virtual double ScreenX() const noexcept;
    /// @see https://w3c.github.io/uievents/#dom-mouseevent-screeny
    KRYS_NODISCARD virtual double ScreenY() const noexcept;
    KRYS_NODISCARD const DoublePoint &ScreenLocation() const noexcept
    {
      return _screenLocation;
    }

    /// @see https://w3c.github.io/uievents/#dom-mouseevent-clientx
    KRYS_NODISCARD virtual double ClientX() const noexcept;
    /// @see https://w3c.github.io/uievents/#dom-mouseevent-clienty
    KRYS_NODISCARD virtual double ClientY() const noexcept;
    KRYS_NODISCARD const DoublePoint &ClientLocation() const noexcept
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

    KRYS_NODISCARD const DoublePoint &WindowLocation() const noexcept
    {
      return _windowLocation;
    }

    /// @note Non-standard.
    KRYS_NODISCARD int32 LayerX() const noexcept;
    /// @note Non-standard.
    KRYS_NODISCARD int32 LayerY() const noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-offsetx
    KRYS_NODISCARD virtual double OffsetX() const noexcept;
    /// @see https://drafts.csswg.org/cssom-view/#dom-mouseevent-offsety
    KRYS_NODISCARD virtual double OffsetY() const noexcept;

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

    KRYS_NODISCARD const DoublePoint &AbsoluteLocation() const noexcept
    {
      return _absoluteLocation;
    }

    /// @brief The frame view of the event's view, which its coordinates are relative to.
    KRYS_NODISCARD LocalFrameView *FrameView() const noexcept;

  protected:
    MouseRelatedEvent(DOMStringAtom type, const UIEventInit &eventInitDict, LocalFrameView *view,
                      const DoublePoint &screenLocation, const DoublePoint &windowLocation, double movementX,
                      double movementY, EnumSet<Modifier> modifiers, Simulated isSimulated) noexcept;

    MouseRelatedEvent(DOMStringAtom type, const MouseEventInit &initializer) noexcept;

    void InitCoordinates(const DoublePoint &clientLocation) noexcept;
    /// @brief Makes the frame view the one of the event's view, after that changed.
    void UpdateFrameView() noexcept;

    void SetScreenLocation(const DoublePoint &screenLocation) noexcept
    {
      _screenLocation = screenLocation;
    }

    void SetIsSimulated(bool value) noexcept
    {
      _isSimulated = value;
    }

    KRYS_NODISCARD const DoublePoint &PageLocation() const noexcept
    {
      return _pageLocation;
    }

    KRYS_NODISCARD const DoublePoint &OffsetLocation() const noexcept
    {
      return _offsetLocation;
    }

    void ComputeRelativePosition() const noexcept;
    KRYS_NODISCARD bool HasCachedRelativePosition() const noexcept;

  private:
    void Init(bool isSimulated, const DoublePoint &windowLocation) noexcept;
    void InitCoordinates() noexcept;
    void ComputePageLocation() noexcept;
    KRYS_NODISCARD float DocumentToAbsoluteScaleFactor() const noexcept;
  };
}
