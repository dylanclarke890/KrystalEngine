#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/HTML/CustomElementTestUtils.hpp"
#include "Krystal.Booey/CSS/CSSStyleProperties.hpp"
#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElement.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/HTMLUnknownElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLDocumentParser.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"

namespace krys::boo::html::tests
{

  TEST_CASE_METHOD(CustomElementFixture, "CustomElementRegistry defines elements", "[HTML][CustomElement]")
  {
    SECTION("names must be valid custom element names")
    {
      auto factory = [](HTMLElement &)
      {
        return UniquePtr<CustomElement> {};
      };
      CHECK(Registry().Define(u8"div", factory).HasException());
      CHECK(Registry().Define(u8"Upper-case", factory).HasException());
      CHECK(Registry().Define(u8"font-face", factory).HasException());
      CHECK_FALSE(Registry().Define(u8"x-a", factory).HasException());
    }

    SECTION("a name is defined once")
    {
      Define(u8"x-a");
      auto again = Registry().Define(u8"x-a", [](HTMLElement &) { return UniquePtr<CustomElement> {}; });
      REQUIRE(again.HasException());
      CHECK(again.GetException().Code() == dom::ExceptionCode::NotSupportedError);
    }

    SECTION("get and getName")
    {
      CHECK(Registry().Get(u8"x-a") == nullptr);
      Define(u8"x-a");
      auto *definition = Registry().Get(u8"x-a");
      REQUIRE(definition);
      CHECK(Registry().GetName(*definition) == u8"x-a");
      auto other = CustomElementRegistry::CreateScoped();
      CHECK(other->GetName(*definition) == dom::DOMStringAtom::Null());
    }

    SECTION("whenDefined calls back in a microtask once the name is defined")
    {
      int32 calls = 0;
      REQUIRE_FALSE(
        Registry().WhenDefined(u8"x-a", [&](CustomElementInterface &) { ++calls; }).HasException());
      CHECK(Registry().WhenDefined(u8"nohyphen", [&](CustomElementInterface &) {}).HasException());
      Define(u8"x-a");
      CHECK(calls == 0);
      EventLoop::Current().PerformMicrotaskCheckpoint();
      CHECK(calls == 1);

      REQUIRE_FALSE(
        Registry().WhenDefined(u8"x-a", [&](CustomElementInterface &) { ++calls; }).HasException());
      EventLoop::Current().PerformMicrotaskCheckpoint();
      CHECK(calls == 2);
    }
  }

  TEST_CASE_METHOD(CustomElementFixture, "Custom elements are created by the parser and createElement",
                   "[HTML][CustomElement]")
  {
    Define(u8"x-widget", {u8"value"});

    SECTION("the parser constructs them synchronously, then sets their attributes and inserts them")
    {
      Parse(u8"<x-widget id=a value=1 other=2></x-widget>");
      auto &element = ById(u8"a");
      CHECK(Is<HTMLMaybeFormAssociatedCustomElement>(element));
      CHECK(element.IsDefinedCustomElement());
      CHECK(dynamic_cast<LoggingElement *>(element.GetCustomElement()) != nullptr);
      CHECK(element.Matches(u8":defined").ReleaseValue());
      CHECK(TakeCalls() == Log {u8"constructed", u8"attribute value null 1", u8"connected a"});
    }

    SECTION("createElement constructs them synchronously; connecting them calls back before the call returns")
    {
      auto element = Create(u8"x-widget");
      CHECK(element->IsDefinedCustomElement());
      CHECK(TakeCalls() == Log {u8"constructed"});

      (void)element->SetAttribute(u8"id", u8"b");
      (void)element->SetAttribute(u8"value", u8"3");
      CHECK(TakeCalls() == Log {u8"attribute value null 3"});

      REQUIRE_FALSE(Body().AppendChild(*element).HasException());
      CHECK(TakeCalls() == Log {u8"connected b"});

      REQUIRE_FALSE(Body().RemoveChild(*element).HasException());
      CHECK(TakeCalls() == Log {u8"disconnected b"});
    }

    SECTION("cloning upgrades the clone")
    {
      Parse(u8"<x-widget id=a value=1></x-widget>");
      (void)TakeCalls();
      auto clone = ById(u8"a").CloneNode(false);
      REQUIRE_FALSE(clone.HasException());
      auto &cloned = Downcast<dom::Element>(*clone.Value());
      CHECK(cloned.IsDefinedCustomElement());
      CHECK(TakeCalls() == Log {u8"constructed", u8"attribute value null 1"});
    }

    SECTION("elements of other names aren't custom")
    {
      Parse(u8"<x-other id=a></x-other><div id=b></div>");
      CHECK(ById(u8"a").IsCustomElementUpgradeCandidate());
      CHECK_FALSE(ById(u8"a").Matches(u8":defined").ReleaseValue());
      CHECK(ById(u8"b").GetCustomElementState() == dom::CustomElementState::Uncustomized);
      CHECK(TakeCalls().empty());
    }
  }

  TEST_CASE_METHOD(CustomElementFixture, "Custom elements are upgraded when they're defined late",
                   "[HTML][CustomElement]")
  {
    SECTION("defining a name upgrades the document's elements in tree order")
    {
      Parse(u8"<x-late id=a value=1><x-late id=b></x-late></x-late><div><x-late id=c></x-late></div>");
      Ref<dom::Element> first = ShareRef(ById(u8"a"));
      CHECK(first->IsCustomElementUpgradeCandidate());
      CHECK_FALSE(first->Matches(u8":defined").ReleaseValue());

      Define(u8"x-late", {u8"value"});
      CHECK(TakeCalls()
            == Log {u8"constructed", u8"attribute value null 1", u8"connected a", u8"constructed",
                    u8"connected b", u8"constructed", u8"connected c"});
      CHECK(&ById(u8"a") == first.get());
      CHECK(first->IsDefinedCustomElement());
      CHECK(first->Matches(u8":defined").ReleaseValue());
    }

    SECTION("an element that isn't connected upgrades when it's inserted")
    {
      auto element = Create(u8"x-late");
      Define(u8"x-late");
      CHECK(element->IsCustomElementUpgradeCandidate());
      CHECK(TakeCalls().empty());

      REQUIRE_FALSE(Body().AppendChild(*element).HasException());
      CHECK(element->IsDefinedCustomElement());
      CHECK(TakeCalls() == Log {u8"constructed", u8"connected "});
    }

    SECTION("upgrade() upgrades a tree that isn't connected")
    {
      auto element = Create(u8"x-late");
      Define(u8"x-late");
      Registry().Upgrade(*element);
      CHECK(element->IsDefinedCustomElement());
      CHECK(TakeCalls() == Log {u8"constructed"});
    }
  }

  TEST_CASE_METHOD(CustomElementFixture, "Custom elements fail when their factory gives no behaviour",
                   "[HTML][CustomElement]")
  {
    REQUIRE_FALSE(Registry()
                    .Define(u8"x-broken", [](HTMLElement &) { return UniquePtr<CustomElement> {}; })
                    .HasException());

    SECTION("created synchronously, they're unknown elements")
    {
      auto element = Create(u8"x-broken");
      CHECK(Is<HTMLUnknownElement>(element));
      CHECK(element->GetCustomElementState() == dom::CustomElementState::Failed);
      CHECK(element->LocalName() == u8"x-broken");
      CHECK_FALSE(element->Matches(u8":defined").ReleaseValue());
    }

    SECTION("upgraded, they fail")
    {
      auto registry = CustomElementRegistry::CreateScoped();
      auto element = Document->CreateElement(u8"x-broken",
                                             dom::ElementCreationOptions {.CustomElementRegistry = registry});
      REQUIRE_FALSE(element.HasException());
      CHECK(element.Value()->IsCustomElementUpgradeCandidate());
      REQUIRE_FALSE(registry->Define(u8"x-broken", [](HTMLElement &) { return UniquePtr<CustomElement> {}; })
                      .HasException());
      registry->Upgrade(*element.Value());
      CHECK(element.Value()->GetCustomElementState() == dom::CustomElementState::Failed);
    }
  }

  TEST_CASE_METHOD(CustomElementFixture, "Custom element callbacks", "[HTML][CustomElement]")
  {
    Define(u8"x-widget", {u8"value", u8"style"});
    Parse(u8"<div id=parent><x-widget id=a></x-widget><p id=p></p></div>");
    (void)TakeCalls();
    auto &element = ById(u8"a");

    SECTION("only observed attributes are reported")
    {
      (void)element.SetAttribute(u8"value", u8"1");
      (void)element.SetAttribute(u8"title", u8"2");
      element.RemoveAttribute(u8"value");
      CHECK(TakeCalls() == Log {u8"attribute value null 1", u8"attribute value 1 null"});
    }

    SECTION("changes to the inline style through its CSSOM view are reported")
    {
      REQUIRE_FALSE(
        Downcast<HTMLElement>(element).Style().SetProperty(u8"color", u8"red", u8"").HasException());
      CHECK(TakeCalls() == Log {u8"attribute style null color: red;"});
    }

    SECTION("removing an ancestor disconnects the element")
    {
      REQUIRE_FALSE(Body().RemoveChild(ById(u8"parent")).HasException());
      CHECK(TakeCalls() == Log {u8"disconnected a"});
    }

    SECTION("moveBefore keeps it connected")
    {
      REQUIRE_FALSE(ById(u8"parent").MoveBefore(element, nullptr).HasException());
      CHECK(TakeCalls() == Log {u8"moved a"});
    }

    SECTION("adopting it into another document")
    {
      auto other = CreateRef<dom::HTMLDocument>();
      REQUIRE_FALSE(other->AdoptNode(element).HasException());
      CHECK(TakeCalls() == Log {u8"disconnected a", u8"adopted a"});
    }

    SECTION("a callback's own changes call back before the methods it calls return")
    {
      Calls.clear();
      struct Reenter : dom::EventListenerCallback
      {
        dom::Element *Target {nullptr};
        Log *Calls {nullptr};
        void HandleEvent(dom::Event &) noexcept override
        {
          (void)Target->SetAttribute(u8"value", u8"from-listener");
          Calls->push_back(u8"returned");
        }
      };
      auto listener = CreateRef<Reenter>();
      listener->Target = &element;
      listener->Calls = &Calls;
      element.AddEventListener(u8"custom", Ref<dom::EventListenerCallback> {listener}, false);
      auto event = CreateRef<dom::Event>(dom::DOMStringAtom(u8"custom"));
      (void)element.DispatchEvent(*event);
      CHECK(TakeCalls() == Log {u8"attribute value null from-listener", u8"returned"});
    }

    SECTION("changes made outside a reaction scope call back at the next microtask checkpoint")
    {
      dom::ElementAlgorithms::SetAttributeValue(element, u8"value", u8"1");
      CHECK(Calls.empty());
      EventLoop::Current().PerformMicrotaskCheckpoint();
      CHECK(TakeCalls() == Log {u8"attribute value null 1"});
    }

    SECTION("a reaction scope defers callbacks until it ends")
    {
      {
        CEReactionsScope scope;
        (void)element.SetAttribute(u8"value", u8"1");
        REQUIRE_FALSE(Body().RemoveChild(ById(u8"parent")).HasException());
        CHECK(Calls.empty());
      }
      CHECK(TakeCalls() == Log {u8"attribute value null 1", u8"disconnected a"});
    }
  }

  TEST_CASE_METHOD(CustomElementFixture, "Custom element definitions can disable shadow roots",
                   "[HTML][CustomElement]")
  {
    REQUIRE_FALSE(Registry()
                    .Define(u8"x-closed", [this](HTMLElement &element)
                            { return CreateUnique<LoggingElement>(element, Calls); }, {.DisableShadow = true})
                    .HasException());
    auto element = Create(u8"x-closed");
    auto shadow = element->AttachShadow({});
    REQUIRE(shadow.HasException());
    CHECK(shadow.GetException().Code() == dom::ExceptionCode::NotSupportedError);

    SECTION("an element with a shadow root fails to upgrade")
    {
      auto registry = CustomElementRegistry::CreateScoped();
      auto candidate = Document->CreateElement(
        u8"x-closed", dom::ElementCreationOptions {.CustomElementRegistry = registry});
      REQUIRE_FALSE(candidate.HasException());
      REQUIRE_FALSE(candidate.Value()->AttachShadow({}).HasException());
      REQUIRE_FALSE(registry
                      ->Define(u8"x-closed", [this](HTMLElement &element)
                               { return CreateUnique<LoggingElement>(element, Calls); },
                               {.DisableShadow = true})
                      .HasException());
      registry->Upgrade(*candidate.Value());
      CHECK(candidate.Value()->GetCustomElementState() == dom::CustomElementState::Failed);
    }
  }

  TEST_CASE_METHOD(CustomElementFixture, "Scoped custom element registries", "[HTML][CustomElement]")
  {
    auto scoped = CustomElementRegistry::CreateScoped();
    REQUIRE_FALSE(scoped
                    ->Define(u8"x-scoped", [this](HTMLElement &element)
                             { return CreateUnique<LoggingElement>(element, Calls); })
                    .HasException());

    auto inScope =
      Document->CreateElement(u8"x-scoped", dom::ElementCreationOptions {.CustomElementRegistry = scoped});
    REQUIRE_FALSE(inScope.HasException());
    CHECK(inScope.Value()->IsDefinedCustomElement());

    auto global = Create(u8"x-scoped");
    CHECK(global->IsCustomElementUpgradeCandidate());

    SECTION("a definition added later upgrades the scope's connected elements")
    {
      auto later =
        Document->CreateElement(u8"x-later", dom::ElementCreationOptions {.CustomElementRegistry = scoped});
      REQUIRE_FALSE(later.HasException());
      REQUIRE_FALSE(Body().AppendChild(*later.Value()).HasException());
      (void)TakeCalls();
      REQUIRE_FALSE(scoped
                      ->Define(u8"x-later", [this](HTMLElement &element)
                               { return CreateUnique<LoggingElement>(element, Calls); })
                      .HasException());
      CHECK(later.Value()->IsDefinedCustomElement());
      CHECK(TakeCalls() == Log {u8"constructed", u8"connected "});
    }
  }
}
