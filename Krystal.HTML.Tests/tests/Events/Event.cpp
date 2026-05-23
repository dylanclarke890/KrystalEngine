#include "Krystal.HTML/Events/Event.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include "Krystal.HTML/Events/EventTarget.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  TEST_CASE("Event::IsTrusted should be false for events created via constructor", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(event->IsTrusted());
  }

  TEST_CASE("Event::Type", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->Type() == EventNames::Click);
  }

  TEST_CASE("Event::Target/SrcElement", "[HTML][Event]")
  {
    SECTION("Without target")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->Target() == nullptr);
      REQUIRE(event->SrcElement() == nullptr);
    }

    // TODO(test): more unit test coverage
  }

  TEST_CASE("Event::CurrentTarget", "[HTML][Event]")
  {
    SECTION("Without target")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->CurrentTarget() == nullptr);
    }

    // TODO(test): more unit test coverage
  }

  TEST_CASE("Event::ComposedPath", "[HTML][Event]")
  {
    SECTION("With empty path")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      auto composedPath = event->ComposedPath();
      REQUIRE(composedPath.empty());
    }

    // TODO(test): more unit test coverage
  }

  TEST_CASE("Event::EventPhase", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->EventPhase() == EventPhaseType::NONE);
  }

  TEST_CASE("Event::StopPropagation/CancelBubble", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->CancelBubble() == false);

    event->StopPropagation();
    REQUIRE(event->CancelBubble() == true);

    event->StopPropagation();
    REQUIRE(event->CancelBubble() == true);
  }

  TEST_CASE("Event::StopImmediatePropagation/IsImmediatePropagationStopped", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->IsImmediatePropagationStopped() == false);
    REQUIRE(event->CancelBubble() == false);

    event->StopImmediatePropagation();
    REQUIRE(event->IsImmediatePropagationStopped() == true);
    REQUIRE(event->CancelBubble() == true);

    event->StopImmediatePropagation();
    REQUIRE(event->IsImmediatePropagationStopped() == true);
    REQUIRE(event->CancelBubble() == true);
  }

  TEST_CASE("Event::Bubbles", "[HTML][Event]")
  {
    SECTION("Default value")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->Bubbles() == false);
    }

    SECTION("Set value")
    {
      EventInit init {};
      init.Bubbles = true;
      auto event = CreateRef<Event>(EventNames::Click, init);
      REQUIRE(event->Bubbles() == true);
    }
  }

  TEST_CASE("Event::Cancellable", "[HTML][Event]")
  {
    SECTION("Default value")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->Cancellable() == false);
    }
    SECTION("Set value")
    {
      EventInit init {};
      init.Cancellable = true;
      auto event = CreateRef<Event>(EventNames::Click, init);
      REQUIRE(event->Cancellable() == true);
    }
  }

  TEST_CASE("Event::ReturnValue/PreventDefault/DefaultPrevented", "[HTML][Event]")
  {
    SECTION("Default value")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->ReturnValue() == false);
      REQUIRE(event->DefaultPrevented() == false);
    }

    SECTION("Prevent default on non-cancellable event")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      event->PreventDefault();
      REQUIRE(event->ReturnValue() == false);
      REQUIRE(event->DefaultPrevented() == false);
    }

    SECTION("Prevent default on cancellable event")
    {
      EventInit init {};
      init.Cancellable = true;
      auto event = CreateRef<Event>(EventNames::Click, init);
      event->PreventDefault();
      REQUIRE(event->ReturnValue() == true);
      REQUIRE(event->DefaultPrevented() == true);
    }
  }

  TEST_CASE("Event::Composed", "[HTML][Event]")
  {
    SECTION("Default value")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->Composed() == false);
    }

    SECTION("Set value")
    {
      EventInit init {};
      init.Composed = true;
      auto event = CreateRef<Event>(EventNames::Click, init);
      REQUIRE(event->Composed() == true);
    }
  }

  TEST_CASE("Event::IsTrusted", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->IsTrusted() == false);
  }

  TEST_CASE("Event::InitEvent", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    event->InitEvent(EventNames::MouseWheel, true, true);

    REQUIRE(event->Type() == EventNames::MouseWheel);
    REQUIRE(event->Bubbles() == true);
    REQUIRE(event->Cancellable() == true);
  }

  TEST_CASE("Event::RelatedTarget", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->RelatedTarget() == nullptr);

    RefPtr<EventTarget> eventTarget = CreateRefPtr<EventTarget>();
    event->RelatedTarget(ShareRefPtr(eventTarget.get()));
    REQUIRE(event->RelatedTarget() != eventTarget.get());
  }

  TEST_CASE("Event::TouchTargetList", "[HTML][Event]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->TouchTargetList().empty());
  }
}