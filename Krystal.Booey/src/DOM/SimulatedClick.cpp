#include "Krystal.Booey/DOM/SimulatedClick.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Core/Types/HashSet.hpp"

namespace krys::boo::dom
{
  namespace
  {
    void SimulateMouseEvent(DOMStringAtom eventType, Element &element, Event *underlyingEvent,
                            SimulatedClickSource source) noexcept
    {
      auto event = MouseEvent::CreateSimulated(eventType, underlyingEvent, element,
                                               source == SimulatedClickSource::UserAgent);
      (void)EventDispatcher::DispatchToTarget(*event, &element);
    }
  }

  bool SimulateClick(Element &element, Event *underlyingEvent,
                     SimulatedClickMouseEventOptions mouseEventOptions,
                     SimulatedClickVisualOptions visualOptions, SimulatedClickSource source) noexcept
  {
    if (element.IsDisabledFormControl())
    {
      return false;
    }

    static HashSet<Element *> elementsDispatchingSimulatedClicks;
    if (!elementsDispatchingSimulatedClicks.insert(&element).second)
    {
      return false;
    }

    Ref<Element> protectedElement = ShareRef(element);

    if (mouseEventOptions != SimulatedClickMouseEventOptions::SendNoEvents)
    {
      SimulateMouseEvent(DOMStringAtom(EventNames::MouseDown), element, underlyingEvent, source);
    }
    if (mouseEventOptions != SimulatedClickMouseEventOptions::SendNoEvents
        || visualOptions == SimulatedClickVisualOptions::ShowPressedLook)
    {
      element.SetActive(true);
    }
    if (mouseEventOptions != SimulatedClickMouseEventOptions::SendNoEvents)
    {
      SimulateMouseEvent(DOMStringAtom(EventNames::MouseUp), element, underlyingEvent, source);
    }
    element.SetActive(false);

    // FIXME: A pointer event (PointerEvent) for the click, once there are pointer events.
    SimulateMouseEvent(DOMStringAtom(EventNames::Click), element, underlyingEvent, source);

    elementsDispatchingSimulatedClicks.erase(&element);
    return true;
  }
}
