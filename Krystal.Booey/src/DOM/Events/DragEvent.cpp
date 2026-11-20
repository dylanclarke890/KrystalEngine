#include "Krystal.Booey/DOM/Events/DragEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer/DataTransfer.hpp"

namespace krys::boo
{
  DragEvent::DragEvent(DOMStringAtom type, const DragEventInit &eventInitDict) noexcept
      : MouseEvent(krys::move(type), eventInitDict), _dataTransfer(eventInitDict.DataTransfer)
  {
  }

  DragEvent::DragEvent(DOMStringAtom type, const UIEventInit &eventInitDict, LocalFrameView *view,
                       const PointD &screenLocation, const PointD &windowLocation, double movementX,
                       double movementY, EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons,
                       EventTarget *relatedTarget, double force, SyntheticClickType syntheticClickType,
                       dom::DataTransfer *dataTransfer) noexcept
      : MouseEvent(krys::move(type), eventInitDict, view, screenLocation, windowLocation, movementX,
                   movementY, modifiers, button, buttons, relatedTarget, force, syntheticClickType, {}, {},
                   Simulated::No),
        _dataTransfer(ShareRefPtr(dataTransfer))
  {
  }

  DragEvent::~DragEvent() noexcept = default;

  Ref<DragEvent> DragEvent::Create(DOMStringAtom type, bool canBubble, bool isCancelable, bool isComposed,
                                   MonotonicTime::duration timestamp, LocalFrameView *view, int32 detail,
                                   const PointD &screenLocation, const PointD &windowLocation,
                                   double movementX, double movementY, EnumSet<Modifier> modifiers,
                                   MouseButton button, uint16 buttons, EventTarget *relatedTarget,
                                   double force, SyntheticClickType syntheticClickType,
                                   dom::DataTransfer *dataTransfer) noexcept
  {
    UIEventInit init;
    init.Bubbles = canBubble;
    init.Cancelable = isCancelable;
    init.Composed = isComposed;
    init.Detail = detail;

    Ref<DragEvent> event = AdoptRef(
      *new DragEvent(krys::move(type), init, view, screenLocation, windowLocation, movementX, movementY,
                     modifiers, button, buttons, relatedTarget, force, syntheticClickType, dataTransfer));
    event->_createTime = timestamp;
    return event;
  }
}
