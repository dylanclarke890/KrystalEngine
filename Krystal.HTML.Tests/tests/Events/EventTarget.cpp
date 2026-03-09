#include "Krystal.HTML/Events/EventTarget.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Events/EventNames.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  class TestEventTarget : public EventTarget
  {
  protected:
  };

  TEST_CASE("EventTarget::AddEventListener", "[EventTarget]")
  {
    RefPtr<TestEventTarget> eventTarget = CreateRefPtr<TestEventTarget>();

    SECTION("null callback")
    {
      RefPtr<EventListener> nullCallback;
      bool result = eventTarget->AddEventListener(EventNames::Click, Krys::Move(nullCallback), {});
      REQUIRE(result == false);
    }

    SECTION("duplicate callback with same capture")
    {
      class TestEventListener : public EventListener
      {
      public:
        void HandleEvent(Event &event) noexcept override
        {
        }
      };

      AddEventListenerOptions options {
        true,
      };

      RefPtr<EventListener> callback = CreateRefPtr<TestEventListener>();
      RefPtr<EventListener> duplicateCallback = callback;
      REQUIRE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(callback), options));
      REQUIRE_FALSE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(duplicateCallback), options));
    }

    SECTION("different callback or different capture")
    {
      class TestEventListenerA : public EventListener
      {
      public:
        void HandleEvent(Event &event) noexcept override
        {
        }
      };
      class TestEventListenerB : public EventListener
      {
      public:
        void HandleEvent(Event &event) noexcept override
        {
        }
      };
      AddEventListenerOptions optionsCaptureTrue {
        true,
      };
      AddEventListenerOptions optionsCaptureFalse {
        false,
      };
      RefPtr<EventListener> callbackA = CreateRefPtr<TestEventListenerA>();
      RefPtr<EventListener> callbackB = CreateRefPtr<TestEventListenerB>();
      RefPtr<EventListener> callbackC = callbackA;
      REQUIRE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(callbackA), optionsCaptureTrue));
      REQUIRE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(callbackB), optionsCaptureTrue));
      REQUIRE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(callbackC), optionsCaptureFalse));
    }
  }

  TEST_CASE("EventTarget::RemoveEventListener", "[EventTarget]")
  {
    class TestEventListener : public EventListener
    {
    public:
      void HandleEvent(Event &event) noexcept override
      {
      }
    };

    RefPtr<TestEventTarget> eventTarget = CreateRefPtr<TestEventTarget>();

    AddEventListenerOptions addOptions {
      true,
    };

    EventListenerOptions options {
      true,
    };

    SECTION("callback which doesn't exist")
    {
      RefPtr<EventListener> callback = CreateRefPtr<TestEventListener>();
      bool result = eventTarget->RemoveEventListener(EventNames::Click, Krys::Move(callback), options);
      REQUIRE(result == false);
    }

    SECTION("existing callback")
    {
      RefPtr<EventListener> callback = CreateRefPtr<TestEventListener>();
      RefPtr<EventListener> removeCallback = callback;

      REQUIRE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(callback), addOptions));
      REQUIRE(eventTarget->RemoveEventListener(EventNames::Click, Krys::Move(removeCallback), options));
    }

    SECTION("existing callback with different capture")
    {
      RefPtr<EventListener> callback = CreateRefPtr<TestEventListener>();
      RefPtr<EventListener> removeCallback = callback;
      REQUIRE(eventTarget->AddEventListener(EventNames::Click, Krys::Move(callback), addOptions));
      EventListenerOptions differentOptions {
        false,
      };
      REQUIRE_FALSE(
        eventTarget->RemoveEventListener(EventNames::Click, Krys::Move(removeCallback), differentOptions));
    }
  }

  // TODO(impl): test DispatchEvent and RemoveAllEventListeners.
}