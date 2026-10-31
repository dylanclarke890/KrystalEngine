#include "Krystal.Booey/DOM/Internals/EventFactory.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct TestEventInit : public EventInit
    {
      int32 Value {7};
    };

    class TestEvent : public Event
    {
    private:
      int32 _value {0};

    public:
      using dictionary_type = TestEventInit;

      TestEvent(DOMStringAtom type, const TestEventInit &init) noexcept
          : Event(type, init), _value(init.Value)
      {
      }

      KRYS_NODISCARD int32 Value() const noexcept
      {
        return _value;
      }
    };
  }

  TEST_CASE("Create an event: a trusted event with its dictionary's defaults and the time now",
            "[HTML][EventFactory][dom:concept-event-create]")
  {
    auto event = EventFactory::Create<TestEvent>();
    REQUIRE(event->IsTrusted());
    REQUIRE(event->Type() == DOMStringAtom::Null());
    REQUIRE_FALSE(event->Bubbles());
    REQUIRE_FALSE(event->Cancelable());
    REQUIRE_FALSE(event->Composed());
    REQUIRE(event->Value() == 7);
    REQUIRE(event->TimeStamp() == html::EventLoop::Current().Now());

    SECTION("For any interface that inherits from Event")
    {
      auto mouseEvent = EventFactory::Create<MouseEvent>();
      REQUIRE(mouseEvent->IsTrusted());
      REQUIRE(mouseEvent->ClientX() == 0);
      REQUIRE(mouseEvent->Button() == 0);
    }
  }
}
