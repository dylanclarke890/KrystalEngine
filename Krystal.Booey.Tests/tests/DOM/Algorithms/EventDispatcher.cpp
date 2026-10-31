#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/Rendering/PaintTestUtils.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/EventTarget.hpp"
#include "Krystal.Booey/DOM/FocusEvent.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include <string>

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<Element> NewElement(Document &document, DOMStringView localName)
    {
      auto element = document.CreateElement(localName);
      REQUIRE_FALSE(element.HasException());
      return element.ReleaseValue();
    }

    /// @brief A document holding `root > parent > child`, for following an event along its path.
    struct PathFixture
    {
      Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
      Ref<Element> Root {NewElement(*Document, u8"div")};
      Ref<Element> Parent {NewElement(*Document, u8"div")};
      Ref<Element> Child {NewElement(*Document, u8"span")};
      List<std::string> Log;
      List<EventListenerHandle> Listeners;

      PathFixture()
      {
        REQUIRE_FALSE(Document->AppendChild(*Root).HasException());
        REQUIRE_FALSE(Root->AppendChild(*Parent).HasException());
        REQUIRE_FALSE(Parent->AppendChild(*Child).HasException());
      }

      void Listen(EventTarget &target, std::string name, bool capture)
      {
        Listeners.push_back(target.AddEventListener(
          EventNames::Click, [this, name](Event &) { Log.push_back(name); }, capture));
      }
    };
  }

  TEST_CASE("Dispatch: capturing from the root to the target, then bubbling back up",
            "[EventDispatcher][dom:concept-event-dispatch]")
  {
    PathFixture fixture;
    fixture.Listen(*fixture.Root, "root bubble", false);
    fixture.Listen(*fixture.Root, "root capture", true);
    fixture.Listen(*fixture.Child, "child bubble", false);
    fixture.Listen(*fixture.Child, "child capture", true);
    fixture.Listen(*fixture.Parent, "parent capture", true);
    fixture.Listen(*fixture.Parent, "parent bubble", false);

    SECTION("A bubbling event")
    {
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
      REQUIRE(fixture.Child->DispatchEvent(*event).Value());
      REQUIRE(fixture.Log
              == List<std::string> {"root capture", "parent capture", "child capture", "child bubble",
                                    "parent bubble", "root bubble"});
    }

    SECTION("An event that doesn't bubble reaches the target's own listeners only")
    {
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE(fixture.Child->DispatchEvent(*event).Value());
      REQUIRE(fixture.Log
              == List<std::string> {"root capture", "parent capture", "child capture", "child bubble"});
    }

    SECTION("The result is false once a listener cancels it")
    {
      auto listener =
        fixture.Parent->AddEventListener(EventNames::Click, [](Event &e) { e.PreventDefault(); });
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true, .Cancelable = true});
      REQUIRE_FALSE(fixture.Child->DispatchEvent(*event).Value());
    }
  }

  TEST_CASE("Dispatch: targets are retargeted out of shadow trees, and cleared afterwards",
            "[EventDispatcher][dom:concept-event-dispatch]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = NewElement(*document, u8"div");
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto inner = NewElement(*document, u8"span");
    REQUIRE_FALSE(shadow->AppendChild(*inner).HasException());

    EventTarget *targetAtInner = nullptr;
    EventTarget *targetAtHost = nullptr;
    auto onInner =
      inner->AddEventListener(EventNames::Click, [&](Event &e) { targetAtInner = e.Target().get(); });
    auto onHost =
      host->AddEventListener(EventNames::Click, [&](Event &e) { targetAtHost = e.Target().get(); });

    SECTION("A composed event leaves with the host as its target")
    {
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true, .Composed = true});
      REQUIRE_FALSE(inner->DispatchEvent(*event).HasException());

      REQUIRE(targetAtInner == inner.get());
      REQUIRE(targetAtHost == host.get());
      REQUIRE(event->Target().get() == host.get());
    }

    SECTION("An event that stays in the shadow tree has its target cleared")
    {
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
      REQUIRE_FALSE(inner->DispatchEvent(*event).HasException());

      REQUIRE(targetAtInner == inner.get());
      REQUIRE(targetAtHost == nullptr);
      REQUIRE(event->Target() == nullptr);
    }
  }

  TEST_CASE("Dispatch: relatedTarget is retargeted, and an event doesn't leave a shadow tree it stays within",
            "[EventDispatcher][dom:concept-event-dispatch]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = NewElement(*document, u8"div");
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto from = NewElement(*document, u8"span");
    auto to = NewElement(*document, u8"span");
    REQUIRE_FALSE(shadow->AppendChild(*from).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*to).HasException());

    bool hostHeard = false;
    EventTarget *relatedAtTo = nullptr;
    auto onHost = host->AddEventListener(EventNames::MouseOver, [&](Event &) { hostHeard = true; });
    auto onTo = to->AddEventListener(EventNames::MouseOver, [&](Event &e)
                                     { relatedAtTo = Downcast<MouseEvent>(e).RelatedTarget().get(); });

    SECTION("Moving within the shadow tree stops at its root")
    {
      MouseEventInit init;
      init.Bubbles = true;
      init.Composed = true;
      init.RelatedTarget = from;
      auto event = CreateRef<MouseEvent>(EventNames::MouseOver, init);
      REQUIRE_FALSE(to->DispatchEvent(*event).HasException());

      REQUIRE(relatedAtTo == from.get());
      REQUIRE_FALSE(hostHeard);
      REQUIRE(event->Target() == nullptr);
      REQUIRE(event->RelatedTarget() == nullptr);
    }

    SECTION("A target that is its relatedTarget once retargeted gets nothing")
    {
      MouseEventInit init;
      init.Bubbles = true;
      init.RelatedTarget = from;
      auto event = CreateRef<MouseEvent>(EventNames::MouseOver, init);
      REQUIRE(host->DispatchEvent(*event).Value());
      REQUIRE_FALSE(hostHeard);
    }
  }

  TEST_CASE("Dispatch: a click runs the activation target's activation behaviour, or undoes it when canceled",
            "[EventDispatcher][dom:concept-event-dispatch]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = NewElement(*document, u8"input");
    auto *checkbox = &Downcast<html::HTMLInputElement>(*element);
    REQUIRE_FALSE(checkbox->SetAttribute(u8"type", u8"checkbox").HasException());
    REQUIRE_FALSE(document->AppendChild(*checkbox).HasException());

    bool checkedInListener = false;
    bool cancel = false;
    auto listener = checkbox->AddEventListener(EventNames::Click,
                                               [&](Event &e)
                                               {
                                                 checkedInListener = checkbox->Checked();
                                                 if (cancel)
                                                 {
                                                   e.PreventDefault();
                                                 }
                                               });

    SECTION("The legacy-pre-activation behaviour checks it before the listeners run")
    {
      MouseEventInit init;
      init.Bubbles = true;
      init.Cancelable = true;
      auto event = CreateRef<MouseEvent>(EventNames::Click, init);
      REQUIRE_FALSE(checkbox->DispatchEvent(*event).HasException());
      REQUIRE(checkedInListener);
      REQUIRE(checkbox->Checked());
    }

    SECTION("The legacy-canceled-activation behaviour unchecks it again")
    {
      cancel = true;
      MouseEventInit init;
      init.Bubbles = true;
      init.Cancelable = true;
      auto event = CreateRef<MouseEvent>(EventNames::Click, init);
      REQUIRE_FALSE(checkbox->DispatchEvent(*event).HasException());
      REQUIRE(checkedInListener);
      REQUIRE_FALSE(checkbox->Checked());
    }

    SECTION("Only a MouseEvent is an activation event")
    {
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
      REQUIRE_FALSE(checkbox->DispatchEvent(*event).HasException());
      REQUIRE_FALSE(checkedInListener);
      REQUIRE_FALSE(checkbox->Checked());
    }
  }

  TEST_CASE("Invoke: a stopped event reaches no more targets, and trusted events reach legacy type listeners",
            "[EventDispatcher][dom:concept-event-listener-invoke]")
  {
    PathFixture fixture;

    SECTION("stopPropagation() lets the current target's other listeners run")
    {
      auto stopper = fixture.Child->AddEventListener(EventNames::Click,
                                                     [&](Event &e)
                                                     {
                                                       fixture.Log.push_back("child stops");
                                                       e.StopPropagation();
                                                     });
      fixture.Listen(*fixture.Child, "child", false);
      fixture.Listen(*fixture.Parent, "parent", false);
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Bubbles = true});
      REQUIRE_FALSE(fixture.Child->DispatchEvent(*event).HasException());
      REQUIRE(fixture.Log == List<std::string> {"child stops", "child"});
    }

    SECTION("A trusted animationend with no listeners of its own reaches webkitAnimationEnd listeners")
    {
      DOMStringAtom typeInListener = DOMStringAtom::Null();
      auto legacy = fixture.Child->AddEventListener(DOMStringAtom(EventNames::Legacy::AnimationEnd),
                                                    [&](Event &e) { typeInListener = e.Type(); });

      auto untrusted = CreateRef<Event>(DOMStringAtom(EventNames::AnimationEnd));
      REQUIRE_FALSE(fixture.Child->DispatchEvent(*untrusted).HasException());
      REQUIRE(typeInListener == DOMStringAtom::Null());

      auto trusted = Event::Create(DOMStringAtom(EventNames::AnimationEnd), false, false);
      REQUIRE(EventDispatcher::DispatchToTarget(*trusted, fixture.Child.get()));
      REQUIRE(typeInListener == EventNames::Legacy::AnimationEnd);
      REQUIRE(trusted->Type() == EventNames::AnimationEnd);

      SECTION("but not when there is one")
      {
        typeInListener = DOMStringAtom::Null();
        auto own = fixture.Child->AddEventListener(DOMStringAtom(EventNames::AnimationEnd), [](Event &) {});
        auto again = Event::Create(DOMStringAtom(EventNames::AnimationEnd), false, false);
        REQUIRE(EventDispatcher::DispatchToTarget(*again, fixture.Child.get()));
        REQUIRE(typeInListener == DOMStringAtom::Null());
      }
    }
  }

  TEST_CASE("Inner invoke: the listeners as dispatch reached the target, matched by type and phase",
            "[EventDispatcher][dom:concept-event-listener-inner-invoke]")
  {
    PathFixture fixture;

    SECTION("A listener added during dispatch isn't called")
    {
      Maybe<EventListenerHandle> added;
      auto adder = fixture.Child->AddEventListener(EventNames::Click,
                                                   [&](Event &)
                                                   {
                                                     added = fixture.Child->AddEventListener(
                                                       EventNames::Click,
                                                       [&](Event &) { fixture.Log.push_back("added"); });
                                                   });
      auto event = CreateRef<Event>(EventNames::Click);
      REQUIRE_FALSE(fixture.Child->DispatchEvent(*event).HasException());
      REQUIRE(fixture.Log.empty());
    }

    SECTION("A once listener is removed before it's called")
    {
      AddEventListenerOptions once;
      once.Once = true;
      auto listener = fixture.Child->AddEventListener(
        EventNames::Click, [&](Event &) { fixture.Log.push_back("once"); }, once);
      for (int i = 0; i < 2; ++i)
      {
        auto event = CreateRef<Event>(EventNames::Click);
        REQUIRE_FALSE(fixture.Child->DispatchEvent(*event).HasException());
      }
      REQUIRE(fixture.Log == List<std::string> {"once"});
      REQUIRE(fixture.Child->EventListeners().empty());
    }

    SECTION("A passive listener runs with the in passive listener flag set, and only it")
    {
      AddEventListenerOptions passive;
      passive.Passive = true;
      bool canceledInPassive = true;
      bool canceledAfter = false;
      auto first = fixture.Child->AddEventListener(
        EventNames::Click,
        [&](Event &e)
        {
          e.PreventDefault();
          canceledInPassive = e.DefaultPrevented();
        },
        passive);
      auto second = fixture.Child->AddEventListener(EventNames::Click,
                                                    [&](Event &e)
                                                    {
                                                      e.PreventDefault();
                                                      canceledAfter = e.DefaultPrevented();
                                                    });
      auto event = CreateRef<Event>(EventNames::Click, EventInit {.Cancelable = true});
      REQUIRE_FALSE(fixture.Child->DispatchEvent(*event).Value());
      REQUIRE_FALSE(canceledInPassive);
      REQUIRE(canceledAfter);
    }
  }

  TEST_CASE("Inner invoke: listeners in a document that isn't fully active aren't called",
            "[EventDispatcher][dom:concept-event-listener-inner-invoke]")
  {
    boo::tests::PaintFixture fixture(u8"<!DOCTYPE html><p id=p></p>", {200, 100}, true);
    Ref<dom::Document> old = ShareRef(*fixture.Document);
    auto p = old->GetElementById(u8"p");
    REQUIRE(p);
    REQUIRE(old->Window());
    int calls = 0;
    auto listener = p->AddEventListener(EventNames::Click, [&](Event &) { ++calls; });

    auto first = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(p->DispatchEvent(*first).HasException());
    REQUIRE(calls == 1);

    fixture.Frame->SetDocument(CreateRef<HTMLDocument>());
    REQUIRE_FALSE(old->IsFullyActive());
    REQUIRE(old->Window());
    auto second = CreateRef<Event>(EventNames::Click);
    REQUIRE_FALSE(p->DispatchEvent(*second).HasException());
    REQUIRE(calls == 1);
  }

  TEST_CASE("Fire an event: a trusted event of the type, with the attributes it's given",
            "[EventDispatcher][dom:concept-event-fire]")
  {
    PathFixture fixture;

    SECTION("An Event by default")
    {
      bool trusted = false;
      bool bubbles = false;
      bool cancelable = false;
      DOMStringAtom type = DOMStringAtom::Null();
      auto listener = fixture.Parent->AddEventListener(EventNames::Click,
                                                       [&](Event &e)
                                                       {
                                                         trusted = e.IsTrusted();
                                                         bubbles = e.Bubbles();
                                                         cancelable = e.Cancelable();
                                                         type = e.Type();
                                                         e.PreventDefault();
                                                       });
      bool result = EventDispatcher::FireEvent(EventNames::Click, *fixture.Child,
                                               EventInit {.Bubbles = true, .Cancelable = true});
      REQUIRE(trusted);
      REQUIRE(bubbles);
      REQUIRE(cancelable);
      REQUIRE(type == EventNames::Click);
      REQUIRE_FALSE(result);
    }

    SECTION("Another interface, with its own attributes")
    {
      EventTarget *relatedTarget = nullptr;
      auto listener = fixture.Child->AddEventListener<FocusEvent>(
        DOMStringAtom(u8"focus"), [&](FocusEvent &e) { relatedTarget = e.RelatedTarget().get(); });
      FocusEventInit attributes;
      attributes.RelatedTarget = fixture.Parent;
      REQUIRE(EventDispatcher::FireEvent<FocusEvent>(DOMStringAtom(u8"focus"), *fixture.Child, attributes));
      REQUIRE(relatedTarget == fixture.Parent.get());
    }

    SECTION("Events the user agent fires with attributes are trusted (invalid)")
    {
      auto element = NewElement(*fixture.Document, u8"input");
      auto &input = Downcast<html::HTMLInputElement>(*element);
      REQUIRE_FALSE(input.SetAttribute(u8"required", u8"").HasException());
      REQUIRE_FALSE(fixture.Root->AppendChild(input).HasException());
      bool trusted = false;
      bool cancelable = false;
      auto listener = input.AddEventListener(DOMStringAtom(EventNames::Invalid),
                                             [&](Event &e)
                                             {
                                               trusted = e.IsTrusted();
                                               cancelable = e.Cancelable();
                                             });
      REQUIRE_FALSE(input.CheckValidity());
      REQUIRE(trusted);
      REQUIRE(cancelable);
    }
  }

  TEST_CASE("Fire an event at a custom target", "[EventDispatcher][dom:concept-event-fire]")
  {
    struct TestEvent : public Event
    {
      using dictionary_type = EventInit;

      TestEvent(dom::DOMStringAtom type, const dictionary_type &init) noexcept : Event(type, init)
      {
      }
    };

    struct TestTarget : public EventTarget
    {
      int value = 0;
    };

    struct TestEventListener : public EventListenerCallback
    {
      TestTarget &_target;

      TestEventListener(TestTarget &target) noexcept : EventListenerCallback(), _target(target)
      {
      }

      void HandleEvent(Event &) noexcept override
      {
        _target.value = 1;
      }
    };

    auto target = CreateRef<TestTarget>();
    target->AddEventListener(u8"test", CreateRef<TestEventListener>(*target), true);
    REQUIRE(EventDispatcher::FireEvent<TestEvent>(u8"test", *target));
    REQUIRE(target->value == 1);
  }

  TEST_CASE("Action versus occurrence: keyboard events a script dispatches don't type or move focus",
            "[EventDispatcher][dom:action-versus-occurrence]")
  {
    boo::tests::PaintFixture fixture(u8"<!DOCTYPE html><input id=a><input id=b>", {200, 100}, true);
    auto &a = Downcast<html::HTMLInputElement>(*fixture.Document->GetElementById(u8"a"));
    a.Focus();
    REQUIRE(fixture.Document->FocusedElement() == &a);

    auto dispatch = [&](DOMStringView type, DOMStringView key, uint32 charCode, uint32 keyCode)
    {
      KeyboardEventInit init;
      init.Bubbles = true;
      init.Cancelable = true;
      init.Key = DOMString(key);
      init.CharCode = charCode;
      init.KeyCode = keyCode;
      auto event = CreateRef<KeyboardEvent>(DOMStringAtom(type), init);
      REQUIRE_FALSE(a.DispatchEvent(*event).HasException());
    };
    dispatch(u8"keydown", u8"x", 'x', 'X');
    dispatch(u8"keypress", u8"x", 'x', 'X');
    dispatch(u8"keydown", u8"Tab", 0, 9);

    CHECK(a.Value().empty());
    CHECK(fixture.Document->FocusedElement() == &a);
  }
}
