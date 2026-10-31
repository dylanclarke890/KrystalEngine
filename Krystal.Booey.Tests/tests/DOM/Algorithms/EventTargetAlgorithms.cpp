#include "Krystal.Booey/DOM/Algorithms/EventTargetAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortController.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<EventListenerCallback> Callback(Func<void(Event &)> &&function = [](Event &) {})
    {
      return CreateRef<FunctionEventListenerCallback>(krys::move(function));
    }

    KRYS_NODISCARD Ref<EventListener> Listener(DOMStringAtom type, Ref<EventListenerCallback> callback,
                                               AddEventListenerOptions options = {})
    {
      return CreateRef<EventListener>(type, RefPtr<EventListenerCallback>(callback), krys::move(options));
    }

    KRYS_NODISCARD AddEventListenerOptions WithCapture(bool capture)
    {
      AddEventListenerOptions options;
      options.Capture = capture;
      return options;
    }
  }

  TEST_CASE("Flatten: a boolean is capture, otherwise options' capture",
            "[DOM][EventTargetAlgorithms][dom:concept-flatten-options]")
  {
    REQUIRE(EventTargetAlgorithms::FlattenOptions(true).Capture);
    REQUIRE_FALSE(EventTargetAlgorithms::FlattenOptions(false).Capture);
    REQUIRE(EventTargetAlgorithms::FlattenOptions(EventListenerOptions {.Capture = true}).Capture);
    REQUIRE_FALSE(EventTargetAlgorithms::FlattenOptions(EventListenerOptions {}).Capture);
  }

  TEST_CASE("Flatten more: capture, with once false and passive and signal null unless the options give them",
            "[DOM][EventTargetAlgorithms][dom:event-flatten-more]")
  {
    SECTION("A boolean")
    {
      auto options = EventTargetAlgorithms::FlattenMoreOptions(true);
      REQUIRE(options.Capture);
      REQUIRE_FALSE(options.Once);
      REQUIRE_FALSE(options.Passive.has_value());
      REQUIRE(options.Signal == nullptr);
    }

    SECTION("A dictionary")
    {
      AbortController controller;
      AddEventListenerOptions given = WithCapture(true);
      given.Once = true;
      given.Passive = false;
      given.Signal = ShareRefPtr(controller.Signal());

      auto options = EventTargetAlgorithms::FlattenMoreOptions(given);
      REQUIRE(options.Capture);
      REQUIRE(options.Once);
      REQUIRE(options.Passive == false);
      REQUIRE(options.Signal.get() == controller.Signal());
    }
  }

  TEST_CASE(
    "Default passive value: true for wheel and touch listeners on the window, document, its element and body",
    "[DOM][EventTargetAlgorithms][dom:default-passive-value]")
  {
    auto wheel = DOMStringAtom(EventNames::Wheel);

    SECTION("The window, the document, its document element and its body")
    {
      boo::tests::PaintFixture fixture(u8"<!DOCTYPE html><body><div id=box></div></body>", {200, 100}, true);
      auto *window = fixture.Document->Window();
      REQUIRE(window);

      CHECK(EventTargetAlgorithms::DefaultPassiveValue(wheel, *window));
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(DOMStringAtom(EventNames::TouchStart), *window));
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(DOMStringAtom(EventNames::TouchMove), *window));
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(DOMStringAtom(EventNames::MouseWheel), *window));
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(wheel, *fixture.Document));
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(wheel, *fixture.Document->DocumentElement()));
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(wheel, *fixture.Document->Body()));
      CHECK_FALSE(
        EventTargetAlgorithms::DefaultPassiveValue(wheel, *fixture.Document->GetElementById(u8"box")));
      CHECK_FALSE(EventTargetAlgorithms::DefaultPassiveValue(DOMStringAtom(EventNames::Click), *window));
      CHECK_FALSE(EventTargetAlgorithms::DefaultPassiveValue(DOMStringAtom(EventNames::TouchEnd), *window));
    }

    SECTION("A frameset is the body element")
    {
      boo::tests::PaintFixture fixture(u8"<!DOCTYPE html><frameset></frameset>");
      auto *frameset = fixture.Document->BodyOrFrameset();
      REQUIRE(frameset);
      CHECK(EventTargetAlgorithms::DefaultPassiveValue(wheel, *frameset));
    }
  }

  TEST_CASE(
    "Add an event listener: once per type, callback and capture, unless aborted, removed when aborted",
    "[DOM][EventTargetAlgorithms][dom:add-an-event-listener]")
  {
    auto document = CreateRef<Document>();
    auto target = document->CreateElement(u8"div").ReleaseValue();
    auto click = DOMStringAtom(EventNames::Click);
    auto callback = Callback();

    SECTION("A listener with an aborted signal isn't added")
    {
      AbortController controller;
      controller.Abort();
      AddEventListenerOptions options;
      options.Signal = ShareRefPtr(controller.Signal());
      REQUIRE_FALSE(EventTargetAlgorithms::AddEventListener(*target, *Listener(click, callback, options)));
      REQUIRE(target->EventListeners().empty());
    }

    SECTION("A listener without a callback isn't added")
    {
      auto listener =
        CreateRef<EventListener>(click, RefPtr<EventListenerCallback>(), AddEventListenerOptions {});
      REQUIRE_FALSE(EventTargetAlgorithms::AddEventListener(*target, *listener));
      REQUIRE(target->EventListeners().empty());
    }

    SECTION("A listener without passive gets the default passive value")
    {
      auto listener = Listener(click, callback);
      REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *listener));
      REQUIRE(listener->Passive() == false);
    }

    SECTION("The same type, callback and capture is added once")
    {
      REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *Listener(click, callback)));
      REQUIRE_FALSE(EventTargetAlgorithms::AddEventListener(*target, *Listener(click, callback)));
      REQUIRE(
        EventTargetAlgorithms::AddEventListener(*target, *Listener(click, callback, WithCapture(true))));
      REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *Listener(click, Callback())));
      REQUIRE(
        EventTargetAlgorithms::AddEventListener(*target, *Listener(DOMStringAtom(u8"other"), callback)));
      REQUIRE(target->EventListeners().size() == 4);
    }

    SECTION("Aborting the signal removes the listener")
    {
      AbortController controller;
      AddEventListenerOptions options;
      options.Signal = ShareRefPtr(controller.Signal());
      auto listener = Listener(click, callback, options);
      REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *listener));
      controller.Abort();
      REQUIRE(target->EventListeners().empty());
      REQUIRE(listener->Removed());
    }
  }

  TEST_CASE("Remove an event listener: marks it removed, so a dispatch already under way skips it",
            "[DOM][EventTargetAlgorithms][dom:remove-an-event-listener]")
  {
    auto document = CreateRef<Document>();
    auto target = document->CreateElement(u8"div").ReleaseValue();
    auto click = DOMStringAtom(EventNames::Click);

    int laterCalls = 0;
    auto later = Listener(click, Callback([&](Event &) { ++laterCalls; }));
    auto earlier = Listener(
      click, Callback([&](Event &) { EventTargetAlgorithms::RemoveEventListener(*target, *later); }));
    REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *earlier));
    REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *later));

    auto event = CreateRef<Event>(click);
    REQUIRE_FALSE(target->DispatchEvent(*event).HasException());
    REQUIRE(laterCalls == 0);
    REQUIRE(later->Removed());
    REQUIRE(target->EventListeners().size() == 1);
  }

  TEST_CASE("Remove all event listeners", "[DOM][EventTargetAlgorithms][dom:remove-all-event-listeners]")
  {
    auto document = CreateRef<Document>();
    auto target = document->CreateElement(u8"div").ReleaseValue();
    auto first = Listener(DOMStringAtom(EventNames::Click), Callback());
    auto second = Listener(DOMStringAtom(u8"other"), Callback());
    REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *first));
    REQUIRE(EventTargetAlgorithms::AddEventListener(*target, *second));

    EventTargetAlgorithms::RemoveAllEventListeners(*target);
    REQUIRE(target->EventListeners().empty());
    REQUIRE(first->Removed());
    REQUIRE(second->Removed());
  }
}
