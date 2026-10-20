#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Callbacks/EventListenerCallback.hpp"
#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    auto CreateEmptyObserver = []()
    {
      return CreateRef<MutationObserver>(
        [](const List<Ref<MutationRecord>> &records, MutationObserver &observer)
        {
          (void)records;
          (void)observer;
        });
    };
  }

  TEST_CASE("MutationObserver::Observe", "[HTML][MutationObserver]")
  {
    auto document = CreateRef<Document>();

    SECTION("Should return TypeError when childList, attributes and characterData is false or not set")
    {
      auto observer = CreateEmptyObserver();

      MutationObserverInit invalidOptions {};
      auto result = observer->Observe(*document, invalidOptions);

      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::TypeError);
    }

    SECTION("Should return TypeError when attributeOldValue is true but attributes is false")
    {
      auto observer = CreateEmptyObserver();

      MutationObserverInit invalidOptions {.Attributes = false, .AttributeOldValue = true};
      auto result = observer->Observe(*document, invalidOptions);

      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::TypeError);
    }

    SECTION("Should return TypeError when attributeFilter is set but attributes is false")
    {
      auto observer = CreateEmptyObserver();

      MutationObserverInit invalidOptions {.Attributes = false,
                                           .AttributeFilter = List<dom::DOMStringAtom> {u8"data-test"}};
      auto result = observer->Observe(*document, invalidOptions);

      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::TypeError);
    }

    SECTION("Should return TypeError when characterDataOldValue is true but characterData is false")
    {
      auto observer = CreateEmptyObserver();

      MutationObserverInit invalidOptions {.CharacterData = false, .CharacterDataOldValue = true};
      auto result = observer->Observe(*document, invalidOptions);

      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::TypeError);
    }

    SECTION("Should succeed when valid options are provided")
    {
      auto observer = CreateEmptyObserver();

      MutationObserverInit validOptions {.ChildList = true};

      auto result = observer->Observe(*document, validOptions);
      REQUIRE_FALSE(result.HasException());
    }
  }

  TEST_CASE("MutationObserver::TakeRecords", "[HTML][MutationObserver]")
  {
    auto document = CreateRef<Document>();

    SECTION("Returns empty list when queue is empty")
    {
      auto observer = CreateEmptyObserver();

      MutationObserverInit options {.ChildList = true};
      REQUIRE_FALSE(observer->Observe(*document, options).HasException());

      auto records = observer->TakeRecords();
      REQUIRE(records.empty());
    }
  }

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

  TEST_CASE("MutationObserver: records are delivered at the microtask checkpoint", "[DOM][MutationObserver]")
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

  TEST_CASE("MutationObserver: transient registered observers", "[DOM][MutationObserver]")
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

  TEST_CASE("MutationObserver: slotchange", "[DOM][MutationObserver][Slot]")
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
}
