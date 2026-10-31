#include "Krystal.Booey/DOM/Algorithms/MutationObserverAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/CharacterData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    /// @brief An observer that keeps each delivery's record types (one string a delivery, `childList` and so
    /// on, comma-separated).
    struct RecordingObserver
    {
      List<std::string> Deliveries;
      Ref<MutationObserver> Observer;

      RecordingObserver() noexcept
          : Observer(CreateRef<MutationObserver>(
              [this](const List<Ref<MutationRecord>> &records, MutationObserver &)
              {
                std::string delivery;
                for (auto &record : records)
                {
                  auto type = record->Type().View();
                  delivery += (delivery.empty() ? "" : ",") + std::string(type.begin(), type.end());
                }
                Deliveries.push_back(delivery);
              }))
      {
      }
    };

    KRYS_NODISCARD Ref<Element> NewElement(Document &document, DOMStringView name) noexcept
    {
      return document.CreateElement(DOMStringAtom(name)).ReleaseValue();
    }
  }

  TEST_CASE("Notify mutation observers: records are delivered at the microtask checkpoint",
            "[DOM][MutationObserverAlgorithms][dom:mutation-observers]"
            "[dom:queue-a-mutation-observer-compound-microtask][dom:notify-mutation-observers]"
            "[dom:mutation-observer-compound-microtask-queued-flag][dom:mutation-observer-list]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto parent = NewElement(*document, u8"div");
    REQUIRE_FALSE(document->AppendChild(*parent).HasException());
    RecordingObserver recorder;
    REQUIRE_FALSE(
      recorder.Observer->Observe(*parent, {.ChildList = true, .Attributes = true}).HasException());

    SECTION("in one delivery for every mutation since the last")
    {
      REQUIRE_FALSE(parent->AppendChild(*NewElement(*document, u8"span")).HasException());
      REQUIRE_FALSE(parent->SetAttribute(u8"title", u8"x").HasException());
      CHECK(recorder.Deliveries.empty());
      loop.PerformMicrotaskCheckpoint();
      CHECK(recorder.Deliveries == List<std::string> {"childList,attributes"});

      // Nothing new, nothing delivered.
      loop.PerformMicrotaskCheckpoint();
      CHECK(recorder.Deliveries.size() == 1);
    }

    SECTION("takeRecords() and disconnect() leave nothing to deliver")
    {
      REQUIRE_FALSE(parent->SetAttribute(u8"title", u8"x").HasException());
      CHECK(recorder.Observer->TakeRecords().size() == 1);
      REQUIRE_FALSE(parent->SetAttribute(u8"title", u8"y").HasException());
      recorder.Observer->Disconnect();
      REQUIRE_FALSE(parent->SetAttribute(u8"title", u8"z").HasException());
      loop.PerformMicrotaskCheckpoint();
      CHECK(recorder.Deliveries.empty());
    }
  }

  TEST_CASE("Add transient registered observers: a removed node is observed until the observer is notified",
            "[DOM][MutationObserverAlgorithms][dom:add-transient-registered-observers]"
            "[dom:remove-transient-registered-observers][dom:transient-registered-observer]"
            "[dom:registered-observer-list]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto parent = NewElement(*document, u8"div");
    auto child = NewElement(*document, u8"div");
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());
    RecordingObserver recorder;
    REQUIRE_FALSE(recorder.Observer->Observe(*parent, {.ChildList = true, .Subtree = true}).HasException());

    // The removed child is still observed until the observer is notified.
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    REQUIRE_FALSE(child->AppendChild(*NewElement(*document, u8"span")).HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK(recorder.Deliveries == List<std::string> {"childList,childList"});

    // Then it isn't.
    REQUIRE_FALSE(child->AppendChild(*NewElement(*document, u8"span")).HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK(recorder.Deliveries.size() == 1);
  }

  TEST_CASE("Notify mutation observers: slotchange fires, bubbling, at the signal slots",
            "[DOM][MutationObserverAlgorithms][dom:notify-mutation-observers]"
            "[dom:eventdef-htmlslotelement-slotchange]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<HTMLDocument>();
    auto host = NewElement(*document, u8"div");
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({}).ReleaseValue();
    auto slot = NewElement(*document, u8"slot");
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());

    struct Counter : EventListenerCallback
    {
      int32 Count {0};
      bool Bubbles {false};

      void HandleEvent(Event &event) noexcept override
      {
        ++Count;
        Bubbles = event.Bubbles();
      }
    };
    auto counter = CreateRef<Counter>();
    slot->AddEventListener(DOMStringAtom(u8"slotchange"), Ref<EventListenerCallback> {counter}, false);

    // A slottable joining the slot signals a slot change, which fires at the next notification.
    REQUIRE_FALSE(host->AppendChild(*NewElement(*document, u8"span")).HasException());
    CHECK(counter->Count == 0);
    loop.PerformMicrotaskCheckpoint();
    CHECK(counter->Count == 1);
    CHECK(counter->Bubbles);
  }

  TEST_CASE("Notify mutation observers: in the order the observers were created",
            "[DOM][MutationObserverAlgorithms][dom:notify-mutation-observers][dom:mutation-observer-counter]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto first = NewElement(*document, u8"div");
    auto second = NewElement(*document, u8"div");

    List<std::string> order;
    auto older = CreateRef<MutationObserver>([&](const List<Ref<MutationRecord>> &, MutationObserver &)
                                             { order.push_back("older"); });
    auto newer = CreateRef<MutationObserver>([&](const List<Ref<MutationRecord>> &, MutationObserver &)
                                             { order.push_back("newer"); });
    REQUIRE_FALSE(older->Observe(*first, {.ChildList = true}).HasException());
    REQUIRE_FALSE(newer->Observe(*second, {.ChildList = true}).HasException());

    // The newer observer gets its record first.
    REQUIRE_FALSE(second->AppendChild(*NewElement(*document, u8"span")).HasException());
    REQUIRE_FALSE(first->AppendChild(*NewElement(*document, u8"span")).HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK(order == List<std::string> {"older", "newer"});
  }

  TEST_CASE("Add transient registered observers: removed at the next notification even without records",
            "[DOM][MutationObserverAlgorithms][dom:add-transient-registered-observers]"
            "[dom:remove-transient-registered-observers][dom:transient-registered-observer-source]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto parent = NewElement(*document, u8"div");
    auto child = NewElement(*document, u8"div");
    auto text = document->CreateTextNode(u8"a");
    REQUIRE_FALSE(child->AppendChild(*text).HasException());
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    // Only character data is observed, so removing the child queues no record for the observer.
    RecordingObserver recorder;
    REQUIRE_FALSE(
      recorder.Observer->Observe(*parent, {.CharacterData = true, .Subtree = true}).HasException());
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());

    // The removal makes the observer pending, so its transient observers go at the next notification.
    loop.PerformMicrotaskCheckpoint();
    CHECK(recorder.Deliveries.empty());
    text->Data(u8"b");
    loop.PerformMicrotaskCheckpoint();
    CHECK(recorder.Deliveries.empty());
  }

  TEST_CASE(
    "Add transient registered observers: a node removed from a removed node is observed too",
    "[DOM][MutationObserverAlgorithms][dom:add-transient-registered-observers][dom:registered-observer]"
    "[dom:registered-observer-observer][dom:registered-observer-options]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto parent = NewElement(*document, u8"div");
    auto child = NewElement(*document, u8"div");
    auto grandchild = NewElement(*document, u8"div");
    REQUIRE_FALSE(child->AppendChild(*grandchild).HasException());
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());
    RecordingObserver recorder;
    REQUIRE_FALSE(recorder.Observer->Observe(*parent, {.ChildList = true, .Subtree = true}).HasException());

    // The child's transient observer gives the grandchild one of its own as it's removed.
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    REQUIRE_FALSE(child->RemoveChild(*grandchild).HasException());
    REQUIRE_FALSE(grandchild->AppendChild(*NewElement(*document, u8"span")).HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK(recorder.Deliveries == List<std::string> {"childList,childList,childList"});
  }
}
