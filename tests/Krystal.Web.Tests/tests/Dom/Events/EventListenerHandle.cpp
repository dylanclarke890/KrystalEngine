#include "Krystal.Web/Dom/Events/EventListenerHandle.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Abort/AbortController.hpp"
#include "Krystal.Web/Dom/Abort/AbortSignal.hpp"
#include "Krystal.Web/Dom/Events/Event.hpp"
#include "Krystal.Web/Dom/Events/EventListener.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Input/PointerEvents/MouseEvent.hpp"
#include "Krystal.Web/Input/UIEvents/KeyboardEvent.hpp"

namespace krys::web::dom::tests
{
  namespace
  {
    struct HandleFixture
    {
      Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
      Ref<Element> Target {Document->CreateElement(u8"div").ReleaseValue()};

      void Fire(DOMStringAtom type) noexcept
      {
        (void)Target->DispatchEvent(*Event::Create(krys::move(type), false, false));
      }
    };
  }

  TEST_CASE_METHOD(HandleFixture, "EventListenerHandle owns a lambda listener", "[DOM][EventListenerHandle]")
  {
    int32 calls = 0;

    SECTION("the lambda is called while the handle lives")
    {
      {
        auto handle = Target->AddEventListener(u8"ping", [&](Event &) { ++calls; });
        CHECK(handle.IsActive());
        Fire(u8"ping");
        Fire(u8"pong");
        CHECK(calls == 1);
      }
      Fire(u8"ping");
      CHECK(calls == 1);
    }

    SECTION("Remove() removes it now, once")
    {
      auto handle = Target->AddEventListener(u8"ping", [&](Event &) { ++calls; });
      handle.Remove();
      CHECK_FALSE(handle);
      handle.Remove();
      Fire(u8"ping");
      CHECK(calls == 0);
    }

    SECTION("assigning over a handle removes its listener; moving keeps it")
    {
      auto first = Target->AddEventListener(u8"ping", [&](Event &) { calls += 1; });
      first = Target->AddEventListener(u8"ping", [&](Event &) { calls += 10; });
      Fire(u8"ping");
      CHECK(calls == 10);

      auto moved = krys::move(first);
      CHECK_FALSE(first.IsActive()); // NOLINT(clang-analyzer-cplusplus.Move)
      CHECK(moved.IsActive());
      Fire(u8"ping");
      CHECK(calls == 20);
    }

    SECTION("Release() gives the listener to its target")
    {
      {
        auto handle = Target->AddEventListener(u8"ping", [&](Event &) { ++calls; });
        handle.Release();
        CHECK_FALSE(handle.IsActive());
      }
      Fire(u8"ping");
      Fire(u8"ping");
      CHECK(calls == 2);
    }

    SECTION("an empty handle owns nothing")
    {
      EventListenerHandle handle;
      CHECK_FALSE(handle.IsActive());
      handle.Remove();
    }
  }

  TEST_CASE_METHOD(HandleFixture, "EventListenerHandle after other removals", "[DOM][EventListenerHandle]")
  {
    int32 calls = 0;

    SECTION("a once listener is gone after it runs")
    {
      auto handle =
        Target->AddEventListener(u8"ping", [&](Event &) { ++calls; }, AddEventListenerOptions {.Once = true});
      Fire(u8"ping");
      CHECK_FALSE(handle.IsActive());
      Fire(u8"ping");
      CHECK(calls == 1);
    }

    SECTION("a signal removes released listeners together")
    {
      AbortController controller;
      AddEventListenerOptions options;
      options.Signal = &controller.Signal();
      Target->AddEventListener(u8"ping", [&](Event &) { ++calls; }, options).Release();
      auto kept = Target->AddEventListener(u8"pong", [&](Event &) { ++calls; }, options);
      controller.Abort();
      CHECK(controller.Signal().Aborted());
      CHECK_FALSE(kept.IsActive());
      Fire(u8"ping");
      Fire(u8"pong");
      CHECK(calls == 0);
    }

    SECTION("an aborted signal adds nothing, and the handle is empty")
    {
      AbortController controller;
      controller.Abort();
      AddEventListenerOptions options;
      options.Signal = &controller.Signal();
      auto handle = Target->AddEventListener(u8"ping", [&](Event &) { ++calls; }, options);
      CHECK_FALSE(handle.IsActive());
    }

    SECTION("the target going away")
    {
      auto element = Document->CreateElement(u8"span").ReleaseValue();
      auto handle = element->AddEventListener(u8"ping", [&](Event &) { ++calls; });
      element = Document->CreateElement(u8"b").ReleaseValue();
      CHECK_FALSE(handle.IsActive());
      handle.Remove();
    }

    SECTION("a listener destroying its own handle as it runs")
    {
      EventListenerHandle handle;
      handle = Target->AddEventListener(u8"ping",
                                        [&](Event &)
                                        {
                                          ++calls;
                                          handle.Remove();
                                        });
      auto other = Target->AddEventListener(u8"ping", [&](Event &) { calls += 10; });
      Fire(u8"ping");
      Fire(u8"ping");
      CHECK(calls == 21);
    }
  }

  TEST_CASE_METHOD(HandleFixture, "EventListenerHandle's typed listeners", "[DOM][EventListenerHandle]")
  {
    int32 mouse = 0;
    auto handle = Target->AddEventListener<MouseEvent>(u8"click", [&](MouseEvent &event)
                                                       { mouse += event.Button() + 1; });
    (void)Target->DispatchEvent(*Event::Create(u8"click", false, false));
    CHECK(mouse == 0);
    (void)Target->DispatchEvent(*CreateRef<MouseEvent>(u8"click"));
    CHECK(mouse == 1);
  }
}
