#include "Krystal.Booey/DOM/Events/ScopedEventQueue.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include <string>

namespace krys::boo::dom::tests
{
  TEST_CASE("ScopedEventQueue: events wait for the outermost scope", "[DOM][Events]")
  {
    auto target = CreateRef<EventTarget>();
    List<std::string> dispatched;
    auto first =
      target->AddEventListener(DOMStringAtom(u8"first"), [&](Event &) { dispatched.push_back("first"); });
    auto second =
      target->AddEventListener(DOMStringAtom(u8"second"), [&](Event &) { dispatched.push_back("second"); });

    SECTION("Without a scope an event is dispatched straight away")
    {
      ScopedEventQueue::DispatchOrQueue(Event::Create(DOMStringAtom(u8"first"), false, false), target);
      CHECK(dispatched == List<std::string> {"first"});
    }

    SECTION("Nested scopes dispatch the events in order when the outermost one ends")
    {
      {
        EventQueueScope outer;
        ScopedEventQueue::DispatchOrQueue(Event::Create(DOMStringAtom(u8"first"), false, false), target);
        {
          EventQueueScope inner;
          ScopedEventQueue::DispatchOrQueue(Event::Create(DOMStringAtom(u8"second"), false, false), target);
        }
        CHECK(dispatched.empty());
      }
      CHECK(dispatched == List<std::string> {"first", "second"});
    }
  }
}
