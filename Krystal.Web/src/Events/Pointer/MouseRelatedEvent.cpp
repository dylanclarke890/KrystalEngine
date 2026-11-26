#include "Krystal.Web/Events/Pointer/MouseRelatedEvent.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/Node.hpp"
#include "Krystal.Web/Events/Pointer/MouseEvent.hpp"
#include "Krystal.Web/Html/Window.hpp"
#include "Krystal.Web/Page/Frame.hpp"
#include "Krystal.Web/Page/FrameView.hpp"
#include "Krystal.Web/Rendering/Layers/RenderLayer.hpp"
#include "Krystal.Web/Rendering/RenderBox.hpp"
#include "Krystal.Web/Rendering/RenderObject.hpp"
#include <cmath>

namespace krys::web
{
  namespace
  {
    /// @brief Whether events of @p eventType keep their movement; the others' is 0.
    KRYS_NODISCARD bool IsMoveEventType(const DOMStringAtom &eventType) noexcept
    {
      return eventType == event_names::MouseMove || eventType == event_names::PointerMove
             || eventType == event_names::PointerRawUpdate || eventType == event_names::TouchMove;
    }

    /// @brief A coordinate with infinities and NaN as 0.
    KRYS_NODISCARD double FiniteValue(double value) noexcept
    {
      return std::isfinite(value) ? value : 0;
    }

    KRYS_NODISCARD web::FrameView *FrameViewOf(const Window *window) noexcept
    {
      auto *frame = window ? window->Frame() : nullptr;
      return frame ? frame->View() : nullptr;
    }
  }

  MouseRelatedEvent::MouseRelatedEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                                       web::FrameView *view, const PointD &screenLocation,
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

  web::FrameView *MouseRelatedEvent::FrameView() const noexcept
  {
    return _view.get();
  }

  void MouseRelatedEvent::UpdateFrameView() noexcept
  {
    _view = ShareRefPtr(FrameViewOf(View()));
  }

  void MouseRelatedEvent::Init(bool isSimulated, const PointD &windowLocation) noexcept
  {
    // A simulated event stays at 0; a real one goes from the window to the view's contents, the document and
    // the viewport.
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
    // The page location stands in for the target-relative ones until they're worked out for a target.
    _layerLocation = LayoutPoint(_pageLocation);
    _offsetLocation = _pageLocation;

    ComputeAbsoluteLocation();
    _relativePositionTarget = nullptr;
  }

  void MouseRelatedEvent::InitCoordinates(const PointD &clientLocation) noexcept
  {
    SizeD documentToClientOffset;
    if (_view)
    {
      documentToClientOffset = SizeD(_view->DocumentToClientOffset());
    }

    _clientLocation = clientLocation;
    _pageLocation = clientLocation - documentToClientOffset;

    _layerLocation = LayoutPoint(_pageLocation);
    _offsetLocation = _pageLocation;

    ComputeAbsoluteLocation();
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

  void MouseRelatedEvent::ComputeAbsoluteLocation() noexcept
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

    _layerLocation = LayoutPoint(_pageLocation);
    _offsetLocation = _pageLocation;

    // The target's boxes have to be where they'll be drawn.
    (void)targetNode->NodeDocument().UpdateLayoutIgnorePendingStylesheets();

    // The offset is from the target's padding box, in the document's (unzoomed) coordinates.
    // https://drafts.csswg.org/cssom-view/#dom-mouseevent-offsetx
    if (auto *renderer = targetNode->Renderer())
    {
      auto local = renderer->AbsoluteToLocal(
        PointF(static_cast<float>(_absoluteLocation.X()), static_cast<float>(_absoluteLocation.Y())),
        {MapCoordinatesMode::UseTransforms});
      _offsetLocation = PointD(local.X(), local.Y());
      if (auto *box = DynamicDowncast<RenderBox>(*renderer))
      {
        _offsetLocation -= SizeD(box->BorderLeft().ToDouble(), box->BorderTop().ToDouble());
      }
      float scaleFactor = 1 / DocumentToAbsoluteScaleFactor();
      if (scaleFactor != 1.0f)
      {
        _offsetLocation = _offsetLocation.Scaled(scaleFactor);
      }
    }

    // The layer location is from the nearest render layer of the target, or of its nearest ancestor with a
    // renderer. Render layers are roughly the positioned boxes and stacking contexts that Pointer Events
    // measures layerX and layerY from.
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

  // The screen and client coordinates are floored to whole pixels, as Chrome gives them.

  double MouseRelatedEvent::ScreenX() const noexcept
  {
    return std::floor(FiniteValue(_screenLocation.X()));
  }

  double MouseRelatedEvent::ScreenY() const noexcept
  {
    return std::floor(FiniteValue(_screenLocation.Y()));
  }

  double MouseRelatedEvent::ClientX() const noexcept
  {
    return std::floor(FiniteValue(_clientLocation.X()));
  }

  double MouseRelatedEvent::ClientY() const noexcept
  {
    return std::floor(FiniteValue(_clientLocation.Y()));
  }
}
