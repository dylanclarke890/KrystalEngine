#include "Krystal.Booey/DOM/Internals/EventFactory.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include <catch_all.hpp>

namespace krys::boo::dom::tests
{
  namespace
  {
    class TestEvent : public Event
    {
      friend class EventFactory;
      using dictionary_type = EventInit;

    public:
      TestEvent(const dictionary_type &init) noexcept : Event(u8"test", init)
      {
      }
    };
  }

  TEST_CASE("EventFactory::Create", "[HTML][EventFactory]")
  {
    auto event = EventFactory::Create<TestEvent>();

    REQUIRE(event->Type() == u8"test");
    REQUIRE_FALSE(event->Bubbles());
    REQUIRE_FALSE(event->Cancelable());
    REQUIRE_FALSE(event->Composed());
    REQUIRE(event->IsTrusted());
  }
}