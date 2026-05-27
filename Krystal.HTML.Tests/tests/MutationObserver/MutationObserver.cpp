#include "Krystal.HTML/MutationObserver/MutationObserver.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    auto CreateEmptyObserver = []()
    {
      return CreateRef<MutationObserver>(
        [](const List<MutationRecord> &records, MutationObserver &observer) {});
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
                                           .AttributeFilter = List<DOMStringAtom> {u8"data-test"}};
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
}