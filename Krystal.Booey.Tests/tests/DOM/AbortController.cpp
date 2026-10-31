#include "Krystal.Booey/DOM/AbortController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/AbortAlgorithms.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Types/Exception.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("new AbortController(): a controller with a new signal that isn't aborted",
            "[DOM][AbortController][dom:dom-abortcontroller-abortcontroller][dom:dom-abortcontroller-signal]")
  {
    AbortController controller;
    REQUIRE(controller.Signal());
    REQUIRE_FALSE(controller.Signal()->Aborted());
    REQUIRE(controller.Signal() == controller.Signal());

    AbortController other;
    REQUIRE(other.Signal() != controller.Signal());
  }

  TEST_CASE("AbortController.abort(): signals abort on its signal, with the reason if given",
            "[DOM][AbortController][dom:dom-abortcontroller-abort][dom:abortcontroller-signal-abort]")
  {
    AbortController controller;
    int aborts = 0;
    auto listener = controller.Signal()->AddEventListener(EventNames::Abort, [&](Event &) { ++aborts; });

    SECTION("Without a reason, an AbortError")
    {
      controller.Abort();
      REQUIRE(controller.Signal()->Aborted());
      auto *reason = std::any_cast<Exception>(&controller.Signal()->Reason());
      REQUIRE(reason);
      REQUIRE(reason->Code() == ExceptionCode::AbortError);
      REQUIRE(aborts == 1);
    }

    SECTION("With a reason")
    {
      controller.Abort(Any(42));
      REQUIRE(std::any_cast<int>(controller.Signal()->Reason()) == 42);
    }

    SECTION("Only once")
    {
      controller.Abort(Any(1));
      controller.Abort(Any(2));
      REQUIRE(std::any_cast<int>(controller.Signal()->Reason()) == 1);
      REQUIRE(aborts == 1);
    }

    SECTION("Signal abort on the controller does the same")
    {
      AbortAlgorithms::SignalAbort(controller, Any(7));
      REQUIRE(controller.Signal()->Aborted());
      REQUIRE(std::any_cast<int>(controller.Signal()->Reason()) == 7);
      REQUIRE(aborts == 1);
    }
  }
}
