#include "Krystal.Booey/DOM/DragEvent.hpp"
#include "Krystal.Booey/DOM/DataTransfer.hpp"

namespace krys::boo::dom
{
  DragEvent::DragEvent(DOMStringAtom type, const DragEventInit &eventInitDict) noexcept
      : MouseEvent(type, eventInitDict), _dataTransfer(eventInitDict.DataTransfer)
  {
  }

  DragEvent::DragEvent(DOMStringAtom type, const UIEventInit &eventInitDict, LocalFrameView *view,
                       const DoublePoint &screenLocation, const DoublePoint &windowLocation, double movementX,
                       double movementY, EnumSet<Modifier> modifiers, MouseButton button, uint16 buttons,
                       EventTarget *relatedTarget, double force, SyntheticClickType syntheticClickType,
                       DataTransfer *dataTransfer) noexcept
      : MouseEvent(type, eventInitDict, view, screenLocation, windowLocation, movementX, movementY, modifiers,
                   button, buttons, relatedTarget, force, syntheticClickType, {}, {}, Simulated::No),
        _dataTransfer(ShareRefPtr(dataTransfer))
  {
  }

  DragEvent::~DragEvent() noexcept = default;

  Ref<DragEvent> DragEvent::Create(DOMStringAtom type, bool canBubble, bool isCancelable, bool isComposed,
                                   MonotonicTime::duration timestamp, LocalFrameView *view, int32 detail,
                                   const DoublePoint &screenLocation, const DoublePoint &windowLocation,
                                   double movementX, double movementY, EnumSet<Modifier> modifiers,
                                   MouseButton button, uint16 buttons, EventTarget *relatedTarget,
                                   double force, SyntheticClickType syntheticClickType,
                                   DataTransfer *dataTransfer) noexcept
  {
    UIEventInit init;
    init.Bubbles = canBubble;
    init.Cancelable = isCancelable;
    init.Composed = isComposed;
    init.Detail = detail;

    Ref<DragEvent> event = AdoptRef(*new DragEvent(type, init, view, screenLocation, windowLocation,
                                                   movementX, movementY, modifiers, button, buttons,
                                                   relatedTarget, force, syntheticClickType, dataTransfer));
    event->_createTime = timestamp;
    return event;
  }
}
