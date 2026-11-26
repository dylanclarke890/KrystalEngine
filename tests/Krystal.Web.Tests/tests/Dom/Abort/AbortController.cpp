#include "Krystal.Web/Dom/Abort/AbortController.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Abort/AbortAlgorithms.hpp"
#include "Krystal.Web/Dom/Abort/AbortSignal.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "Krystal.Web/Dom/Events/EventNames.hpp"
#include "Krystal.Web/Dom/Types/Exception.hpp"

namespace krys::web::dom::tests
{
  TEST_CASE("new AbortController(): a controller with a new signal that isn't aborted",
            "[DOM][AbortController][dom:dom-abortcontroller-abortcontroller][dom:dom-abortcontroller-signal]")
  {
    AbortController controller;
    REQUIRE_FALSE(controller.Signal().Aborted());
    REQUIRE(&controller.Signal() == &controller.Signal());

    AbortController other;
    REQUIRE(&other.Signal() != &controller.Signal());
  }

  TEST_CASE("AbortController.abort(): signals abort on its signal, with the reason if given",
            "[DOM][AbortController][dom:dom-abortcontroller-abort][dom:abortcontroller-signal-abort]")
  {
    AbortController controller;
    int aborts = 0;
    auto listener = controller.Signal().AddEventListener(event_names::Abort, [&](Event &) { ++aborts; });

    SECTION("Without a reason, an AbortError")
    {
      controller.Abort();
      REQUIRE(controller.Signal().Aborted());
      const auto &reason = controller.Signal().Reason();
      REQUIRE(reason);
      REQUIRE(reason->Code() == ExceptionCode::AbortError);
      REQUIRE(aborts == 1);
    }

    SECTION("With a reason")
    {
      controller.Abort(Exception(ExceptionCode::AbortError, u8"42"));
      REQUIRE(controller.Signal().Reason()->Message() == u8"42");
    }

    SECTION("Only once")
    {
      controller.Abort(Exception(ExceptionCode::AbortError, u8"1"));
      controller.Abort(Exception(ExceptionCode::AbortError, u8"2"));
      REQUIRE(controller.Signal().Reason()->Message() == u8"1");
      REQUIRE(aborts == 1);
    }

    SECTION("Signal abort on the controller does the same")
    {
      AbortAlgorithms::SignalAbort(controller, Exception(ExceptionCode::AbortError, u8"7"));
      REQUIRE(controller.Signal().Aborted());
      REQUIRE(controller.Signal().Reason()->Message() == u8"7");
      REQUIRE(aborts == 1);
    }
  }
}
