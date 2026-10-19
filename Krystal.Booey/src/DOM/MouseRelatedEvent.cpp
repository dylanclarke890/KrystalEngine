#include "Krystal.Booey/DOM/MouseRelatedEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include <cmath>

namespace krys::boo::dom
{
  namespace
  {
    KRYS_NODISCARD bool IsMoveEventType(DOMStringAtom eventType) noexcept
    {
      return eventType == EventNames::MouseMove || eventType == EventNames::PointerMove
             || eventType == EventNames::TouchMove;
    }

    KRYS_NODISCARD double FiniteValue(double value) noexcept
    {
      return std::isfinite(value) ? value : 0;
    }
  }

  MouseRelatedEvent::MouseRelatedEvent(DOMStringAtom type, const UIEventInit &eventInitDict,
                                       LocalFrameView *view, const DoublePoint &screenLocation,
                                       const DoublePoint &windowLocation, double movementX, double movementY,
                                       EnumSet<Modifier> modifiers, Simulated isSimulated) noexcept
      : UIEventWithKeyState(type, eventInitDict, modifiers), _view(ShareRefPtr(view)), _movementX(movementX),
        _movementY(movementY), _windowLocation(windowLocation), _screenLocation(screenLocation),
        _isSimulated(isSimulated == Simulated::Yes)
  {
    Init(_isSimulated, windowLocation);
  }

  MouseRelatedEvent::MouseRelatedEvent(DOMStringAtom type, const MouseEventInit &initializer) noexcept
      : UIEventWithKeyState(type, initializer), _movementX(initializer.MovementX),
        _movementY(initializer.MovementY), _screenLocation(initializer.ScreenX, initializer.ScreenY)
  {
    SetScreenLocation({FiniteValue(initializer.ScreenX), FiniteValue(initializer.ScreenY)});
    InitCoordinates();
  }

  MouseRelatedEvent::~MouseRelatedEvent() noexcept = default;

  LocalFrameView *MouseRelatedEvent::View() const noexcept
  {
    return _view.get();
  }

  void MouseRelatedEvent::Init(bool isSimulated, const DoublePoint &windowLocation) noexcept
  {
    if (!isSimulated && _view)
    {
      DoublePoint absolutePoint = _view->WindowToContents(windowLocation);
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

  void MouseRelatedEvent::InitCoordinates(const DoublePoint &clientLocation) noexcept
  {
    // Set up initial values for coordinates.
    // Correct values are computed lazily, see ComputeRelativePosition.
    DoubleSize documentToClientOffset;
    if (_view)
    {
      documentToClientOffset = DoubleSize(_view->DocumentToClientOffset());
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
    RefPtr<Node> targetNode = ShareRefPtr(DynamicDowncast<Node>(target.get()));
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
        FloatPoint(static_cast<float>(_absoluteLocation.X()), static_cast<float>(_absoluteLocation.Y())),
        {MapCoordinatesMode::UseTransforms});
      _offsetLocation = DoublePoint(local.X(), local.Y());
      float scaleFactor = 1 / DocumentToAbsoluteScaleFactor();
      if (scaleFactor != 1.0f)
      {
        _offsetLocation = _offsetLocation.Scaled(scaleFactor);
      }
    }

    // Adjust layerLocation to be relative to the layer.
    // FIXME: event.layerX and event.layerY are poorly defined, and probably don't always correspond to RenderLayer
    // offsets (https://bugs.webkit.org/show_bug.cgi?id=21868).
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
