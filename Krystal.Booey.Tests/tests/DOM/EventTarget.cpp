#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief An event whose initialized flag is unset, as only an event created without a constructor would
    /// be.
    class UninitializedEvent : public Event
    {
    public:
      UninitializedEvent() noexcept : Event(EventNames::Click)
      {
        _initialized = false;
      }
    };
  }

  TEST_CASE("new EventTarget(): a target with no listeners that events can be dispatched to",
            "[DOM][EventTarget][dom:dom-eventtarget-eventtarget]")
  {
    auto target = CreateRef<EventTarget>();
    REQUIRE(target->EventListeners().empty());

    int calls = 0;
    auto listener = target->AddEventListener(EventNames::Click, [&](Event &) { ++calls; });
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(target->DispatchEvent(*event).HasException());
    REQUIRE(calls == 1);
  }

  TEST_CASE(
    "addEventListener() and removeEventListener(): match listeners by type, callback and capture",
    "[DOM][EventTarget][dom:dom-eventtarget-addeventlistener][dom:dom-eventtarget-removeeventlistener]")
  {
    auto target = CreateRef<EventTarget>();
    int calls = 0;
    Ref<EventListenerCallback> callback = CreateRef<FunctionEventListenerCallback>([&](Event &) { ++calls; });

    SECTION("The options are flattened")
    {
      AddEventListenerOptions options;
      options.Capture = true;
      options.Once = true;
      options.Passive = true;
      target->AddEventListener(EventNames::Click, Ref<EventListenerCallback>(callback), options);
      REQUIRE(target->EventListeners().size() == 1);
      auto &listener = *target->EventListeners().front();
      REQUIRE(listener.Capture());
      REQUIRE(listener.Once());
      REQUIRE(listener.Passive() == true);
      target->RemoveEventListener(EventNames::Click, Ref<EventListenerCallback>(callback), true);
      REQUIRE(target->EventListeners().empty());
    }

    SECTION("Adding the same listener twice adds it once")
    {
      target->AddEventListener(EventNames::Click, Ref<EventListenerCallback>(callback), false);
      target->AddEventListener(EventNames::Click, Ref<EventListenerCallback>(callback), false);
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE_FALSE(target->DispatchEvent(*event).HasException());
      REQUIRE(calls == 1);
    }

    SECTION("Removing needs the same type, callback and capture")
    {
      target->AddEventListener(EventNames::Click, Ref<EventListenerCallback>(callback), true);
      target->RemoveEventListener(EventNames::Click, Ref<EventListenerCallback>(callback), false);
      target->RemoveEventListener(DOMStringAtom(u8"other"), Ref<EventListenerCallback>(callback), true);
      target->RemoveEventListener(EventNames::Click, CreateRef<FunctionEventListenerCallback>([](Event &) {}),
                                  true);
      REQUIRE(target->EventListeners().size() == 1);

      target->RemoveEventListener(EventNames::Click, Ref<EventListenerCallback>(callback),
                                  EventListenerOptions {.Capture = true});
      REQUIRE(target->EventListeners().empty());
    }
  }

  TEST_CASE(
    "dispatchEvent(): throws for an event being dispatched or not initialized, else dispatches it untrusted",
    "[DOM][EventTarget][dom:dom-eventtarget-dispatchevent]")
  {
    auto target = CreateRef<EventTarget>();

    SECTION("An event that's being dispatched")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      Maybe<ExceptionCode> nested;
      auto listener = target->AddEventListener(EventNames::Click,
                                               [&](Event &e)
                                               {
                                                 auto result = target->DispatchEvent(e);
                                                 if (result.HasException())
                                                 {
                                                   nested = result.GetException().Code();
                                                 }
                                               });
      REQUIRE_FALSE(target->DispatchEvent(*event).HasException());
      REQUIRE(nested == ExceptionCode::InvalidStateError);
    }

    SECTION("An event that isn't initialized")
    {
      auto event = CreateRef<UninitializedEvent>();
      auto result = target->DispatchEvent(*event);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidStateError);
    }

    SECTION("A trusted event becomes untrusted, and the result is whether it wasn't canceled")
    {
      auto event = Event::Create(EventNames::Click, false, true);
      REQUIRE(event->IsTrusted());
      auto listener = target->AddEventListener(EventNames::Click, [](Event &e) { e.PreventDefault(); });
      auto result = target->DispatchEvent(*event);
      REQUIRE_FALSE(result.HasException());
      REQUIRE_FALSE(result.Value());
      REQUIRE_FALSE(event->IsTrusted());
    }
  }
}
