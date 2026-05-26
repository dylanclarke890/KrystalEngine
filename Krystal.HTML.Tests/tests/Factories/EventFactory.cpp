#include "Krystal.HTML/Factories/EventFactory.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Event/Event.hpp"
#include "Krystal.HTML/Constants/EventNames.hpp"
#include "Krystal.HTML/DOM/EventTarget.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

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