#include "Krystal.HTML/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("EventDispatcher::FireEvent", "[EventDispatcher]")
  {
    struct TestEvent : public Event
    {
      using dictionary_type = EventInit;

      TestEvent(DOMStringAtom type, const dictionary_type &init) noexcept : Event(type, init)
      {
      }
    };

    struct TestTarget : public EventTarget
    {
      int value = 0;
    };

    struct TestEventListener : public EventListenerCallback
    {
      TestTarget &_target;

      TestEventListener(TestTarget &target) noexcept : EventListenerCallback(), _target(target)
      {
      }

      void HandleEvent(Event &e) noexcept override
      {
        _target.value = 1;
      }
    };

    auto target = CreateRef<TestTarget>();
    target->AddEventListener(u8"test", CreateRef<TestEventListener>(*target), true);
    REQUIRE(EventDispatcher::FireEvent<TestEvent>(u8"test", *target));
    REQUIRE(target->value == 1);
  }
}