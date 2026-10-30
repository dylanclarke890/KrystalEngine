#include "Krystal.Booey/HTML/HashChangeEvent.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"

namespace krys::boo::html
{
  HashChangeEvent::HashChangeEvent(dom::DOMStringAtom type, const HashChangeEventInit &eventInitDict) noexcept
      : Event(type, eventInitDict), _oldURL(eventInitDict.OldURL), _newURL(eventInitDict.NewURL)
  {
  }

  Ref<HashChangeEvent> HashChangeEvent::Create(dom::USVString &&oldURL, dom::USVString &&newURL) noexcept
  {
    // https://html.spec.whatwg.org/#update-document-for-history-step-application: fire an event named
    // hashchange at document's relevant global object, using HashChangeEvent, with the oldURL attribute
    // initialized to oldURL and the newURL attribute initialized to newURL.
    HashChangeEventInit init;
    init.OldURL = krys::move(oldURL);
    init.NewURL = krys::move(newURL);
    auto event = CreateRef<HashChangeEvent>(dom::DOMStringAtom(EventNames::HashChange), init);
    event->_isTrusted = true;
    return event;
  }
}
