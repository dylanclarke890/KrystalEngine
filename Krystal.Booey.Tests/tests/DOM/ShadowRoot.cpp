#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE(
    "ShadowRoot: its getters return what it was attached with",
    "[DOM][ShadowRoot][dom:interface-shadowroot][dom:concept-shadow-root][dom:shadowroot-mode]"
    "[dom:shadowroot-delegates-focus][dom:shadowroot-serializable][dom:shadowroot-slot-assignment]"
    "[dom:shadowroot-clonable][dom:dom-shadowroot-mode][dom:dom-shadowroot-delegatesfocus]"
    "[dom:dom-shadowroot-serializable][dom:dom-shadowroot-slotassignment][dom:dom-shadowroot-clonable]"
    "[dom:dom-shadowroot-host][dom:shadowroot-available-to-element-internals]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();

    SECTION("the defaults")
    {
      auto shadow = host->AttachShadow({}).ReleaseValue();
      CHECK(shadow->Mode() == ShadowRootMode::Open);
      CHECK_FALSE(shadow->DelegatesFocus());
      CHECK_FALSE(shadow->Serialisable());
      CHECK(shadow->SlotAssignment() == SlotAssignmentMode::Named);
      CHECK_FALSE(shadow->Clonable());
      CHECK(shadow->Host() == host.get());
      CHECK_FALSE(shadow->IsAvailableToElementInternals());
    }

    SECTION("each option")
    {
      auto mode = GENERATE(ShadowRootMode::Open, ShadowRootMode::Closed);
      auto shadow = host
                      ->AttachShadow({.Mode = mode,
                                      .DelegatesFocus = DelegatesFocus(true),
                                      .SlotAssignment = SlotAssignmentMode::Manual,
                                      .Clonable = Clonable(true),
                                      .Serialisable = Serialisable(true)})
                      .ReleaseValue();
      CHECK(shadow->Mode() == mode);
      CHECK(shadow->DelegatesFocus());
      CHECK(shadow->Serialisable());
      CHECK(shadow->SlotAssignment() == SlotAssignmentMode::Manual);
      CHECK(shadow->Clonable());
      CHECK(shadow->Host() == host.get());
    }
  }

  TEST_CASE("ShadowRoot: a declarative shadow root is emptied and kept by a later attachShadow()",
            "[DOM][ShadowRoot][dom:shadowroot-declarative]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto document =
      Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html><div id=host><template "
                                          u8"shadowrootmode=open><p>shadow</p></template></div>")
        .ReleaseValue();
    auto host = document->GetElementById(u8"host");
    REQUIRE(host != nullptr);
    auto shadow = host->ShadowRoot();
    REQUIRE(shadow != nullptr);
    REQUIRE(shadow->HasChildNodes());

    auto attached = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    CHECK(attached == shadow);
    CHECK_FALSE(shadow->HasChildNodes());

    // It's no longer declarative, so attaching again throws.
    auto again = host->AttachShadow({.Mode = ShadowRootMode::Open});
    REQUIRE(again.HasException());
    CHECK(again.GetException().Code() == ExceptionCode::NotSupportedError);
  }

  TEST_CASE("ShadowRoot: its custom element registry is the one it was attached with",
            "[DOM][ShadowRoot][dom:shadowroot-custom-element-registry]"
            "[dom:shadowroot-keep-custom-element-registry-null]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto global = html::CustomElementRegistry::Create(*document);
    document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(global));
    auto scoped = html::CustomElementRegistry::CreateScoped();

    auto host = document->CreateElement(u8"div").ReleaseValue();
    CHECK(host->AttachShadow({}).ReleaseValue()->CustomElementRegistry() == global);

    auto scopedHost = document->CreateElement(u8"div").ReleaseValue();
    CHECK(scopedHost->AttachShadow({.CustomElementRegistry = scoped.get()})
            .ReleaseValue()
            ->CustomElementRegistry()
          == scoped);
  }

  TEST_CASE("ShadowRoot.onslotchange: an event handler for the slotchange event, which bubbles to it",
            "[DOM][ShadowRoot][dom:dom-shadowroot-onslotchange][dom:shadowroot-onslotchange]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto slot = document->CreateElement(u8"slot").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK_FALSE(shadow->OnSlotChange());

    int calls = 0;
    shadow->SetOnSlotChange(
      [&](Event &event)
      {
        ++calls;
        CHECK(event.Type() == EventNames::SlotChange);
        CHECK(event.Target() == slot.get());
        CHECK(event.Bubbles());
      });
    CHECK(shadow->OnSlotChange());

    REQUIRE_FALSE(host->AppendChild(*document->CreateTextNode(u8"text")).HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK(calls == 1);
  }
}
