#include "Krystal.Booey/Page/NavigationCurrentEntryChangeEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo
{
  NavigationCurrentEntryChangeEvent::NavigationCurrentEntryChangeEvent(
    dom::DOMStringAtom type, const NavigationCurrentEntryChangeEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _navigationType(eventInitDict.NavigationType), _from(eventInitDict.From)
  {
  }

  NavigationCurrentEntryChangeEvent::~NavigationCurrentEntryChangeEvent() noexcept = default;

  Ref<NavigationCurrentEntryChangeEvent>
    NavigationCurrentEntryChangeEvent::Create(Maybe<NavigationNavigationType> navigationType,
                                              NavigationHistoryEntry &from) noexcept
  {
    NavigationCurrentEntryChangeEventInit init;
    init.NavigationType = navigationType;
    init.From = ShareRefPtr(&from);
    auto event =
      CreateRef<NavigationCurrentEntryChangeEvent>(dom::DOMStringAtom(EventNames::CurrentEntryChange), init);
    event->_isTrusted = true;
    return event;
  }
}
