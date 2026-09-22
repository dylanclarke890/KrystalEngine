#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "catch2/catch_test_macros.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("EventDispatcher::FireEvent", "[EventDispatcher]")
  {
    struct TestEvent : public Event
    {
      using dictionary_type = EventInit;

      TestEvent(dom::DOMStringAtom type, const dictionary_type &init) noexcept : Event(type, init)
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