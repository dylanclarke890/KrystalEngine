/*
 * Copyright (C) 2001 Peter Kelly (pmk@post.com)
 * Copyright (C) 2001 Tobias Anton (anton@stud.fbi.fh-darmstadt.de)
 * Copyright (C) 2006 Samuel Weinig (sam.weinig@gmail.com)
 * Copyright (C) 2003, 2005, 2006, 2008, 2013 Apple Inc. All rights reserved.
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
 */

#include "Krystal.Booey/DOM/Events/MouseRelatedEvent.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/DOM/Nodes/Node.hpp"
#include "Krystal.Booey/HTML/Window.hpp"
#include "Krystal.Booey/Page/Frame.hpp"
#include "Krystal.Booey/Page/FrameView.hpp"
#include "Krystal.Booey/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include <cmath>

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool IsMoveEventType(const DOMStringAtom &eventType) noexcept
    {
      return eventType == event_names::MouseMove || eventType == event_names::PointerMove
             || eventType == event_names::PointerRawUpdate || eventType == event_names::TouchMove;
    }

    KRYS_NODISCARD double FiniteValue(double value) noexcept
    {
      return std::isfinite(value) ? value : 0;
    }
  }

  namespace
  {
    KRYS_NODISCARD boo::FrameView *FrameViewOf(const Window *window) noexcept
    {
      auto *frame = window ? window->Frame() : nullptr;
      return frame ? frame->View() : nullptr;
    }
  }

  MouseRelatedEvent::MouseRelatedEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                                       boo::FrameView *view, const PointD &screenLocation,
                                       const PointD &windowLocation, double movementX, double movementY,
                                       EnumSet<Modifier> modifiers, Simulated isSimulated) noexcept
      : UIEventWithKeyState(krys::move(type), eventInitDict, modifiers), _view(ShareRefPtr(view)),
        _movementX(movementX), _movementY(movementY), _windowLocation(windowLocation),
        _screenLocation(screenLocation), _isSimulated(isSimulated == Simulated::Yes)
  {
    if (view)
    {
      SetView(view->Frame().Window());
    }
    Init(_isSimulated, windowLocation);
  }

  MouseRelatedEvent::MouseRelatedEvent(DOMStringAtom type, const MouseEventInit &initializer) noexcept
      : UIEventWithKeyState(krys::move(type), initializer), _view(ShareRefPtr(FrameViewOf(initializer.View))),
        _movementX(initializer.MovementX), _movementY(initializer.MovementY),
        _screenLocation(initializer.ScreenX, initializer.ScreenY)
  {
    SetScreenLocation({FiniteValue(initializer.ScreenX), FiniteValue(initializer.ScreenY)});
    InitCoordinates();
  }

  MouseRelatedEvent::~MouseRelatedEvent() noexcept = default;

  boo::FrameView *MouseRelatedEvent::FrameView() const noexcept
  {
    return _view.get();
  }

  void MouseRelatedEvent::UpdateFrameView() noexcept
  {
    _view = ShareRefPtr(FrameViewOf(View()));
  }

  void MouseRelatedEvent::Init(bool isSimulated, const PointD &windowLocation) noexcept
  {
    if (!isSimulated && _view)
    {
      PointD absolutePoint = _view->WindowToContents(windowLocation);
      _pageLocation = _view->AbsoluteToDocumentPoint(absolutePoint);
      _clientLocation = _view->DocumentToClientPoint(_pageLocation);
    }

    InitCoordinates();

    if (!IsMoveEventType(Type()))
    {
      _movementX = 0;
      _movementY = 0;
    }
  }

  void MouseRelatedEvent::InitCoordinates() noexcept
  {
    // Set up initial values for coordinates.
    // Correct values are computed lazily, see ComputeRelativePosition.
    _layerLocation = LayoutPoint(_pageLocation);
    _offsetLocation = _pageLocation;

    ComputePageLocation();
    _relativePositionTarget = nullptr;
  }

  void MouseRelatedEvent::InitCoordinates(const PointD &clientLocation) noexcept
  {
    // Set up initial values for coordinates.
    // Correct values are computed lazily, see ComputeRelativePosition.
    SizeD documentToClientOffset;
    if (_view)
    {
      documentToClientOffset = SizeD(_view->DocumentToClientOffset());
    }

    _clientLocation = clientLocation;
    _pageLocation = clientLocation - documentToClientOffset;

    _layerLocation = LayoutPoint(_pageLocation);
    _offsetLocation = _pageLocation;

    ComputePageLocation();
    _relativePositionTarget = nullptr;
  }

  float MouseRelatedEvent::DocumentToAbsoluteScaleFactor() const noexcept
  {
    if (_view)
    {
      return _view->DocumentToAbsoluteScaleFactor();
    }
    return 1;
  }

  void MouseRelatedEvent::ComputePageLocation() noexcept
  {
    _absoluteLocation = _pageLocation.Scaled(DocumentToAbsoluteScaleFactor());
  }

  bool MouseRelatedEvent::HasCachedRelativePosition() const noexcept
  {
    // The position is relative to the target, so it's computed again when the event has a new target.
    return _relativePositionTarget && _relativePositionTarget == Target().get();
  }

  void MouseRelatedEvent::ComputeRelativePosition() const noexcept
  {
    RefPtr<EventTarget> target = Target();
    RefPtr<Node> targetNode = DynamicDowncast<Node>(target);
    if (!targetNode)
    {
      return;
    }

    // Compute coordinates that are based on the target.
    _layerLocation = LayoutPoint(_pageLocation);
    _offsetLocation = _pageLocation;

    // Must have an updated render tree for this math to work correctly.
    (void)targetNode->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    // Adjust offsetLocation to be relative to the target's position.
    if (auto *renderer = targetNode->Renderer())
    {
      auto local = renderer->AbsoluteToLocal(
        PointF(static_cast<float>(_absoluteLocation.X()), static_cast<float>(_absoluteLocation.Y())),
        {MapCoordinatesMode::UseTransforms});
      _offsetLocation = PointD(local.X(), local.Y());
      float scaleFactor = 1 / DocumentToAbsoluteScaleFactor();
      if (scaleFactor != 1.0f)
      {
        _offsetLocation = _offsetLocation.Scaled(scaleFactor);
      }
    }

    // Adjust layerLocation to be relative to the layer.
    // FIXME: event.layerX and event.layerY are poorly defined, and probably don't always correspond to
    // RenderLayer offsets (https://bugs.webkit.org/show_bug.cgi?id=21868).
    RefPtr<Node> node = targetNode;
    while (node && !node->Renderer())
    {
      node = ShareRefPtr(node->ParentNode());
    }

    if (node)
    {
      for (auto *layer = node->Renderer()->EnclosingLayer(); layer; layer = layer->Parent())
      {
        _layerLocation -= ToLayoutSize(layer->Location());
      }
    }

    _relativePositionTarget = target.get();
  }

  int32 MouseRelatedEvent::LayerX() const noexcept
  {
    if (!HasCachedRelativePosition())
    {
      ComputeRelativePosition();
    }
    return _layerLocation.X().ToInt();
  }

  int32 MouseRelatedEvent::LayerY() const noexcept
  {
    if (!HasCachedRelativePosition())
    {
      ComputeRelativePosition();
    }
    return _layerLocation.Y().ToInt();
  }

  double MouseRelatedEvent::OffsetX() const noexcept
  {
    if (IsSimulated())
    {
      return 0;
    }

    if (!HasCachedRelativePosition())
    {
      ComputeRelativePosition();
    }

    return std::round(_offsetLocation.X());
  }

  double MouseRelatedEvent::OffsetY() const noexcept
  {
    if (IsSimulated())
    {
      return 0;
    }

    if (!HasCachedRelativePosition())
    {
      ComputeRelativePosition();
    }

    return std::round(_offsetLocation.Y());
  }

  double MouseRelatedEvent::PageX() const noexcept
  {
    return std::floor(_pageLocation.X());
  }

  double MouseRelatedEvent::PageY() const noexcept
  {
    return std::floor(_pageLocation.Y());
  }

  double MouseRelatedEvent::ScreenX() const noexcept
  {
    // FIXME: Determine whether or not we should restrict the range of this value
    // now that screenX is a double instead of an integer.
    return std::floor(FiniteValue(_screenLocation.X()));
  }

  double MouseRelatedEvent::ScreenY() const noexcept
  {
    // FIXME: Determine whether or not we should restrict the range of this value
    // now that screenY is a double instead of an integer.
    return std::floor(FiniteValue(_screenLocation.Y()));
  }

  double MouseRelatedEvent::ClientX() const noexcept
  {
    // FIXME: Determine whether or not we should restrict the range of this value
    // now that clientX is a double instead of an integer.
    return std::floor(FiniteValue(_clientLocation.X()));
  }

  double MouseRelatedEvent::ClientY() const noexcept
  {
    // FIXME: Determine whether or not we should restrict the range of this value
    // now that clientY is a double instead of an integer.
    return std::floor(FiniteValue(_clientLocation.Y()));
  }
}
