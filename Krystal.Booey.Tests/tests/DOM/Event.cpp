#include "Krystal.Booey/DOM/Event.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::dom::tests
{
  using namespace std::chrono_literals;

  namespace
  {
    KRYS_NODISCARD Ref<Event> CancelableEvent() noexcept
    {
      return CreateRef<Event>(EventNames::Click, EventInit {.Cancelable = true});
    }

    KRYS_NODISCARD Ref<Element> NewElement(Document &document, DOMStringView localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    /// @brief The targets of @p event's composed path.
    KRYS_NODISCARD List<EventTarget *> ComposedPathOf(const Event &event) noexcept
    {
      List<EventTarget *> targets;
      for (auto &target : event.ComposedPath())
      {
        targets.push_back(target.get());
      }
      return targets;
    }
  }

  TEST_CASE("Event constructor: an untrusted, initialized event with the type and dictionary's values",
            "[HTML][Event][dom:concept-event-constructor][dom:inner-event-creation-steps]")
  {
    auto event =
      CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true, .Cancelable = true, .Composed = true});
    REQUIRE(event->Type() == EventNames::Click);
    REQUIRE(event->Bubbles());
    REQUIRE(event->Cancelable());
    REQUIRE(event->Composed());
    REQUIRE_FALSE(event->IsTrusted());
    REQUIRE(event->TimeStamp() == html::EventLoop::Current().Now());
    REQUIRE(event->Target() == nullptr);
    REQUIRE(event->EventPhase() == EventPhaseType::NONE);

    // Initialized, so dispatching it doesn't throw an InvalidStateError.
    auto document = CreateRef<Document>();
    auto element = NewElement(*document, u8"div");
    REQUIRE_FALSE(element->DispatchEvent(*event).HasException());
  }

  TEST_CASE("Event.type: the type it was initialized to", "[HTML][Event][dom:dom-event-type]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->Type() == EventNames::Click);
  }

  TEST_CASE("Event.target and srcElement: the event's target", "[HTML][Event][dom:dom-event-target]"
                                                               "[dom:dom-event-srcelement]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE(event->Target() == nullptr);
    REQUIRE(event->SrcElement() == nullptr);

    auto document = CreateRef<Document>();
    auto element = NewElement(*document, u8"div");
    REQUIRE_FALSE(element->DispatchEvent(*event).HasException());
    REQUIRE(event->Target().get() == element.get());
    REQUIRE(event->SrcElement().get() == element.get());
  }

  TEST_CASE("Event.currentTarget: the target whose listeners are being invoked, null outside dispatch",
            "[HTML][Event][dom:dom-event-currenttarget]")
  {
    auto document = CreateRef<Document>();
    auto parent = NewElement(*document, u8"div");
    auto child = NewElement(*document, u8"span");
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
    REQUIRE(event->CurrentTarget() == nullptr);

    List<EventTarget *> seen;
    auto onChild =
      child->AddEventListener(EventNames::Click, [&](Event &e) { seen.push_back(e.CurrentTarget().get()); });
    auto onParent =
      parent->AddEventListener(EventNames::Click, [&](Event &e) { seen.push_back(e.CurrentTarget().get()); });
    REQUIRE_FALSE(child->DispatchEvent(*event).HasException());

    REQUIRE(seen == List<EventTarget *> {child.get(), parent.get()});
    REQUIRE(event->CurrentTarget() == nullptr);
  }

  TEST_CASE("Event.composedPath(): the path seen from the current target, hiding closed shadow trees",
            "[HTML][Event][dom:dom-event-composedpath]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = NewElement(*document, u8"div");
    REQUIRE_FALSE(document->AppendChild(*host).HasException());

    SECTION("Empty outside dispatch")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(event->ComposedPath().empty());
      REQUIRE_FALSE(host->DispatchEvent(*event).HasException());
      REQUIRE(event->ComposedPath().empty());
    }

    SECTION("An open shadow tree is visible from outside it")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto inner = NewElement(*document, u8"span");
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

      List<EventTarget *> fromInner, fromHost;
      auto onInner =
        inner->AddEventListener(EventNames::Click, [&](Event &e) { fromInner = ComposedPathOf(e); });
      auto onHost =
        host->AddEventListener(EventNames::Click, [&](Event &e) { fromHost = ComposedPathOf(e); });
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true, .Composed = true});
      REQUIRE_FALSE(inner->DispatchEvent(*event).HasException());

      auto full = List<EventTarget *> {inner.get(), shadow.get(), host.get(), document.get()};
      REQUIRE(fromInner == full);
      REQUIRE(fromHost == full);
    }

    SECTION("A closed shadow tree is hidden from outside it")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
      auto inner = NewElement(*document, u8"span");
      REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

      List<EventTarget *> fromInner, fromHost;
      auto onInner =
        inner->AddEventListener(EventNames::Click, [&](Event &e) { fromInner = ComposedPathOf(e); });
      auto onHost =
        host->AddEventListener(EventNames::Click, [&](Event &e) { fromHost = ComposedPathOf(e); });
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true, .Composed = true});
      REQUIRE_FALSE(inner->DispatchEvent(*event).HasException());

      REQUIRE(fromInner == List<EventTarget *> {inner.get(), shadow.get(), host.get(), document.get()});
      REQUIRE(fromHost == List<EventTarget *> {host.get(), document.get()});
    }

    SECTION("A slot in a closed shadow tree is hidden from the slotted node")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
      auto slot = NewElement(*document, u8"slot");
      REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
      auto slotted = NewElement(*document, u8"span");
      REQUIRE_FALSE(host->AppendChild(*slotted).HasException());

      List<EventTarget *> fromSlotted, fromSlot;
      auto onSlotted =
        slotted->AddEventListener(EventNames::Click, [&](Event &e) { fromSlotted = ComposedPathOf(e); });
      auto onSlot =
        slot->AddEventListener(EventNames::Click, [&](Event &e) { fromSlot = ComposedPathOf(e); });
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
      REQUIRE_FALSE(slotted->DispatchEvent(*event).HasException());

      REQUIRE(fromSlotted == List<EventTarget *> {slotted.get(), host.get(), document.get()});
      REQUIRE(fromSlot
              == List<EventTarget *> {slotted.get(), slot.get(), shadow.get(), host.get(), document.get()});
    }
  }

  TEST_CASE("Event.eventPhase: NONE outside dispatch, then capturing, at target and bubbling",
            "[HTML][Event][dom:dom-event-eventphase]")
  {
    auto document = CreateRef<Document>();
    auto parent = NewElement(*document, u8"div");
    auto child = NewElement(*document, u8"span");
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
    REQUIRE(event->EventPhase() == EventPhaseType::NONE);

    List<EventPhaseType> phases;
    auto record = [&](Event &e)
    {
      phases.push_back(e.EventPhase());
    };
    auto capture = parent->AddEventListener(EventNames::Click, record, true);
    auto atTarget = child->AddEventListener(EventNames::Click, record);
    auto bubble = parent->AddEventListener(EventNames::Click, record);
    REQUIRE_FALSE(child->DispatchEvent(*event).HasException());

    REQUIRE(phases
            == List<EventPhaseType> {EventPhaseType::CAPTURING_PHASE, EventPhaseType::AT_TARGET,
                                     EventPhaseType::BUBBLING_PHASE});
    REQUIRE(event->EventPhase() == EventPhaseType::NONE);
  }

  TEST_CASE("Event.stopPropagation() and cancelBubble: the stop propagation flag",
            "[HTML][Event][dom:dom-event-stoppropagation][dom:dom-event-cancelbubble]")
  {
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(event->CancelBubble());

    SECTION("stopPropagation() sets it")
    {
      event->StopPropagation();
      REQUIRE(event->CancelBubble());
      event->StopPropagation();
      REQUIRE(event->CancelBubble());
    }

    SECTION("Setting cancelBubble to true sets it, and to false does nothing")
    {
      event->CancelBubble(false);
      REQUIRE_FALSE(event->CancelBubble());
      event->CancelBubble(true);
      REQUIRE(event->CancelBubble());
      event->CancelBubble(false);
      REQUIRE(event->CancelBubble());
    }
  }

  TEST_CASE("Event.stopImmediatePropagation(): sets both stop propagation flags",
            "[HTML][Event][dom:dom-event-stopimmediatepropagation]")
  {
    auto document = CreateRef<Document>();
    auto element = NewElement(*document, u8"div");
    int calls = 0;
    bool cancelBubble = false;
    auto first = element->AddEventListener(EventNames::Click,
                                           [&](Event &e)
                                           {
                                             ++calls;
                                             e.StopImmediatePropagation();
                                             cancelBubble = e.CancelBubble();
                                           });
    auto second = element->AddEventListener(EventNames::Click, [&](Event &) { ++calls; });
    auto event = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(element->DispatchEvent(*event).HasException());

    REQUIRE(calls == 1);
    REQUIRE(cancelBubble);
  }

  TEST_CASE("Event.bubbles and cancelable: the values they were initialized to",
            "[HTML][Event][dom:dom-event-bubbles][dom:dom-event-cancelable]")
  {
    auto plain = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(plain->Bubbles());
    REQUIRE_FALSE(plain->Cancelable());

    auto both = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true, .Cancelable = true});
    REQUIRE(both->Bubbles());
    REQUIRE(both->Cancelable());
  }

  TEST_CASE("Set the canceled flag: only for a cancelable event outside a passive listener",
            "[HTML][Event][dom:set-the-canceled-flag][dom:dom-event-preventdefault]"
            "[dom:dom-event-defaultprevented]")
  {
    SECTION("A cancelable event is canceled")
    {
      auto event = CancelableEvent();
      REQUIRE_FALSE(event->DefaultPrevented());
      event->PreventDefault();
      REQUIRE(event->DefaultPrevented());
    }

    SECTION("An event that isn't cancelable isn't")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      event->PreventDefault();
      REQUIRE_FALSE(event->DefaultPrevented());
    }

    SECTION("A passive listener can't cancel an event")
    {
      auto document = CreateRef<Document>();
      auto element = NewElement(*document, u8"div");
      AddEventListenerOptions passive;
      passive.Passive = true;
      auto listener =
        element->AddEventListener(EventNames::Click, [](Event &e) { e.PreventDefault(); }, passive);
      auto event = CancelableEvent();
      auto result = element->DispatchEvent(*event);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value());
      REQUIRE_FALSE(event->DefaultPrevented());
    }
  }

  TEST_CASE("Event.returnValue: false once canceled, and setting it to false cancels",
            "[HTML][Event][dom:dom-event-returnvalue]")
  {
    SECTION("True until the event is canceled")
    {
      auto event = CancelableEvent();
      REQUIRE(event->ReturnValue());
      event->PreventDefault();
      REQUIRE_FALSE(event->ReturnValue());
    }

    SECTION("Setting it to false sets the canceled flag, and to true does nothing")
    {
      auto event = CancelableEvent();
      event->ReturnValue(true);
      REQUIRE_FALSE(event->DefaultPrevented());
      event->ReturnValue(false);
      REQUIRE(event->DefaultPrevented());
      event->ReturnValue(true);
      REQUIRE(event->DefaultPrevented());
      REQUIRE_FALSE(event->ReturnValue());
    }

    SECTION("An event that isn't cancelable stays true")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      event->ReturnValue(false);
      REQUIRE(event->ReturnValue());
    }
  }

  TEST_CASE("Event.composed: the composed flag", "[HTML][Event][dom:dom-event-composed]")
  {
    REQUIRE_FALSE(CreateRef<Event>(EventNames::Click)->Composed());
    REQUIRE(CreateRef<Event>(EventNames::Click, EventInit {.Composed = true})->Composed());
  }

  TEST_CASE("Event.isTrusted: false for constructed events, true for the user agent's",
            "[HTML][Event][dom:dom-event-istrusted]")
  {
    REQUIRE_FALSE(CreateRef<Event>(EventNames::Click)->IsTrusted());
    REQUIRE(Event::Create(EventNames::Click, false, false)->IsTrusted());
  }

  TEST_CASE("Event::TimeStamp is the host's time at creation",
            "[HTML][Event][dom:inner-event-creation-steps]")
  {
    auto &loop = html::EventLoop::Current();
    loop.Run(loop.Now() + 7ms);
    auto event = CreateRef<Event>(EventNames::Click);
    CHECK(event->TimeStamp() == loop.Now());
  }

  TEST_CASE("Event.timeStamp: milliseconds since the window's time origin",
            "[HTML][Event][dom:dom-event-timestamp]")
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

  TEST_CASE("Event.initEvent(): initializes the event, unless it's being dispatched",
            "[HTML][Event][dom:dom-event-initevent][dom:concept-event-initialize]")
  {
    SECTION("Sets the type, bubbles and cancelable")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      event->InitEvent(EventNames::MouseWheel, true, true);
      REQUIRE(event->Type() == EventNames::MouseWheel);
      REQUIRE(event->Bubbles());
      REQUIRE(event->Cancelable());
    }

    SECTION("Clears the flags, isTrusted and the target")
    {
      auto document = CreateRef<Document>();
      auto element = NewElement(*document, u8"div");
      auto event = Event::Create(EventNames::Click, false, true);
      event->StopImmediatePropagation();
      event->PreventDefault();
      REQUIRE_FALSE(element->DispatchEvent(*event).HasException());
      REQUIRE(event->Target().get() == element.get());

      event->InitEvent(EventNames::Click, false, true);
      REQUIRE_FALSE(event->CancelBubble());
      REQUIRE_FALSE(event->DefaultPrevented());
      REQUIRE_FALSE(event->IsTrusted());
      REQUIRE(event->Target() == nullptr);

      // The stop immediate propagation flag is clear too: both listeners run.
      int calls = 0;
      auto first = element->AddEventListener(EventNames::Click, [&](Event &) { ++calls; });
      auto second = element->AddEventListener(EventNames::Click, [&](Event &) { ++calls; });
      REQUIRE_FALSE(element->DispatchEvent(*event).HasException());
      REQUIRE(calls == 2);
    }

    SECTION("Does nothing while the event is being dispatched")
    {
      auto document = CreateRef<Document>();
      auto element = NewElement(*document, u8"div");
      auto listener = element->AddEventListener(EventNames::Click, [](Event &e)
                                                { e.InitEvent(EventNames::MouseWheel, true, true); });
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE_FALSE(element->DispatchEvent(*event).HasException());
      REQUIRE(event->Type() == EventNames::Click);
      REQUIRE_FALSE(event->Bubbles());
      REQUIRE_FALSE(event->Cancelable());
    }
  }
}
