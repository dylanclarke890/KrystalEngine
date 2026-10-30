#include "Krystal.Booey/DOM/Event.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::dom::tests
{
  using namespace std::chrono_literals;

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

    // FIXME: More unit test coverage
  }

  TEST_CASE("Event::CurrentTarget", "[HTML][Event]")
  {
    SECTION("Without target")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->CurrentTarget() == nullptr);
    }

    // FIXME: More unit test coverage
  }

  TEST_CASE("Event::ComposedPath", "[HTML][Event]")
  {
    SECTION("With empty path")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      auto composedPath = event->ComposedPath();
      REQUIRE(composedPath.empty());
    }

    // FIXME: More unit test coverage
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

  TEST_CASE("Event::Cancelable", "[HTML][Event]")
  {
    SECTION("Default value")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->Cancelable() == false);
    }
    SECTION("Set value")
    {
      EventInit init {};
      init.Cancelable = true;
      auto event = CreateRef<Event>(EventNames::Click, init);
      REQUIRE(event->Cancelable() == true);
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

    SECTION("Prevent default on non-Cancelable event")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      event->PreventDefault();
      REQUIRE(event->ReturnValue() == false);
      REQUIRE(event->DefaultPrevented() == false);
    }

    SECTION("Prevent default on Cancelable event")
    {
      EventInit init {};
      init.Cancelable = true;
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
    REQUIRE(event->Cancelable() == true);
  }

  TEST_CASE("Event::TimeStamp is the host's time at creation", "[HTML][Event]")
  {
    auto &loop = html::EventLoop::Current();
    loop.Run(loop.Now() + 7ms);
    auto event = CreateRef<Event>(EventNames::Click);
    CHECK(event->TimeStamp() == loop.Now());
  }

  TEST_CASE("Event::TimeStampForBindings", "[HTML][Event]")
  {
    auto &loop = html::EventLoop::Current();

    SECTION("Milliseconds since the window's time origin, with their fraction")
    {
      boo::tests::PaintFixture fixture(u8"<!DOCTYPE html>", {200, 100}, true);
      auto *window = fixture.Document->Window();
      REQUIRE(window);
      loop.Run(window->TimeOrigin() + 2'500us);
      auto event = CreateRef<Event>(EventNames::Click);
      CHECK(event->TimeStampForBindings(*fixture.Document) == 2.5);
    }

    SECTION("Never before the time origin")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      loop.Run(loop.Now() + 10ms);
      boo::tests::PaintFixture fixture(u8"<!DOCTYPE html>", {200, 100}, true);
      REQUIRE(fixture.Document->Window());
      CHECK(event->TimeStampForBindings(*fixture.Document) == 0);
    }

    SECTION("Zero without a window")
    {
      loop.Run(loop.Now() + 10ms);
      auto document = CreateRef<Document>();
      auto event = CreateRef<Event>(EventNames::Click);
      CHECK(event->TimeStampForBindings(*document) == 0);
    }
  }
}
