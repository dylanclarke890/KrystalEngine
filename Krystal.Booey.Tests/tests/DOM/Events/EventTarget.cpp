#include "Krystal.Booey/DOM/Events/EventTarget.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Abort/AbortController.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Events/CustomEvent.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventListener.hpp"
#include "Krystal.Booey/DOM/Events/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

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

  TEST_CASE("Events: the introduction's examples",
            "[DOM][EventTarget][dom:events][dom:introduction-to-dom-events]")
  {
    SECTION("A listener is added, removed with the same arguments, or removed by aborting its signal")
    {
      auto target = CreateRef<EventTarget>();
      int calls = 0;
      Ref<EventListenerCallback> imgFetched = CreateRef<FunctionEventListenerCallback>(
        [&](Event &event)
        {
          ++calls;
          CHECK(event.Type() == EventNames::Load);
          CHECK(event.Target() == target.get());
        });

      target->AddEventListener(EventNames::Load, Ref<EventListenerCallback> {imgFetched}, false);
      REQUIRE_FALSE(target->DispatchEvent(*CreateRef<Event>(EventNames::Load)).HasException());
      CHECK(calls == 1);

      target->RemoveEventListener(EventNames::Load, Ref<EventListenerCallback> {imgFetched}, false);
      REQUIRE_FALSE(target->DispatchEvent(*CreateRef<Event>(EventNames::Load)).HasException());
      CHECK(calls == 1);

      AbortController controller;
      AddEventListenerOptions options;
      options.Signal = ShareRefPtr(controller.Signal());
      target->AddEventListener(EventNames::Load, Ref<EventListenerCallback> {imgFetched}, options);
      controller.Abort();
      REQUIRE_FALSE(target->DispatchEvent(*CreateRef<Event>(EventNames::Load)).HasException());
      CHECK(calls == 1);
    }

    SECTION("A synthetic event carries its detail, and dispatchEvent() says whether it was canceled")
    {
      auto target = CreateRef<EventTarget>();
      Maybe<uint32> detail;
      auto listener =
        target->AddEventListener(EventNames::Click, [&](Event &event)
                                 { detail = static_cast<CustomEvent<uint32> &>(event).Detail(); });
      auto event =
        CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {{.Cancelable = true}, 7u});
      CHECK(target->DispatchEvent(*event).ReleaseValue());
      CHECK(detail == 7u);

      auto canceler =
        target->AddEventListener(EventNames::Click, [](Event &event) { event.PreventDefault(); });
      auto canceled =
        CreateRef<CustomEvent<uint32>>(EventNames::Click, CustomEventInit<uint32> {{.Cancelable = true}, 8u});
      CHECK_FALSE(target->DispatchEvent(*canceled).ReleaseValue());
    }

    SECTION(
      "In a tree, capturing listeners on the inclusive ancestors run first in tree order, then bubbling ones")
    {
      auto document = Document::ParseHTMLUnsafe(*CreateRef<HTMLDocument>(),
                                                u8"<!DOCTYPE html><body><div id=d><b id=b></b></div>")
                        .ReleaseValue();
      Ref<Element> div = ShareRef(*document->GetElementById(u8"d"));
      Ref<Element> b = ShareRef(*document->GetElementById(u8"b"));
      List<DOMString> order;
      List<EventListenerHandle> listeners;
      for (auto *node : List<EventTarget *> {document.get(), div.get(), b.get()})
      {
        DOMString name = node == document.get() ? u8"document" : node == div.get() ? u8"div" : u8"b";
        listeners.push_back(node->AddEventListener(
          EventNames::Click, [&order, name](Event &) { order.push_back(name + u8" capture"); },
          AddEventListenerOptions {{.Capture = true}}));
        listeners.push_back(node->AddEventListener(EventNames::Click, [&order, name](Event &)
                                                   { order.push_back(name + u8" bubble"); }));
      }

      REQUIRE_FALSE(b->DispatchEvent(*Event::Create(EventNames::Click, true, false)).HasException());
      CHECK(order
            == List<DOMString> {u8"document capture", u8"div capture", u8"b capture", u8"b bubble",
                                u8"div bubble", u8"document bubble"});

      // Without bubbles, only the capturing listeners and the target's.
      order.clear();
      REQUIRE_FALSE(b->DispatchEvent(*Event::Create(EventNames::Click, false, false)).HasException());
      CHECK(order == List<DOMString> {u8"document capture", u8"div capture", u8"b capture", u8"b bubble"});
    }
  }
}
