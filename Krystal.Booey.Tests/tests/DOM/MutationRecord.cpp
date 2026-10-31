#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<MutationRecord> OnlyRecord(MutationObserver &observer) noexcept
    {
      auto records = observer.TakeRecords();
      REQUIRE(records.size() == 1);
      return records[0];
    }
  }

  TEST_CASE(
    "MutationRecord: the attributes return the values the record was initialised to",
    "[DOM][MutationRecord][dom:interface-mutationrecord][dom:dom-mutationrecord-type]"
    "[dom:dom-mutationrecord-target][dom:dom-mutationrecord-addednodes][dom:dom-mutationrecord-removednodes]"
    "[dom:dom-mutationrecord-previoussibling][dom:dom-mutationrecord-nextsibling]"
    "[dom:dom-mutationrecord-attributename][dom:dom-mutationrecord-attributenamespace]"
    "[dom:dom-mutationrecord-oldvalue]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto element = document->CreateElement(u8"div").ReleaseValue();
    auto text = document->CreateTextNode(u8"a");
    REQUIRE_FALSE(element->SetAttributeNS(u8"urn:x", u8"x:title", u8"old").HasException());
    auto observer = CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});

    SECTION("attributes")
    {
      REQUIRE_FALSE(observer->Observe(*element, {.AttributeOldValue = true}).HasException());
      REQUIRE_FALSE(element->SetAttributeNS(u8"urn:x", u8"x:title", u8"new").HasException());

      auto record = OnlyRecord(*observer);
      CHECK(record->Type() == u8"attributes");
      CHECK(record->Target().get() == element.get());
      CHECK(record->AttributeName() == u8"title");
      CHECK(record->AttributeNamespace() == u8"urn:x");
      CHECK(record->OldValue() == u8"old");
      CHECK(record->PreviousSibling() == nullptr);
      CHECK(record->NextSibling() == nullptr);

      // Empty node lists, the same each time.
      REQUIRE(record->AddedNodes() != nullptr);
      CHECK(record->AddedNodes()->Length() == 0u);
      CHECK(record->AddedNodes() == record->AddedNodes());
      REQUIRE(record->RemovedNodes() != nullptr);
      CHECK(record->RemovedNodes()->Length() == 0u);
      CHECK(record->RemovedNodes() == record->RemovedNodes());
    }

    SECTION("characterData")
    {
      REQUIRE_FALSE(observer->Observe(*text, {.CharacterDataOldValue = true}).HasException());
      REQUIRE_FALSE(text->AppendData(u8"b").HasException());

      auto record = OnlyRecord(*observer);
      CHECK(record->Type() == u8"characterData");
      CHECK(record->Target().get() == text.get());
      CHECK(record->OldValue() == u8"a");
      CHECK(record->AttributeName() == DOMStringAtom::Null());
      CHECK(record->AttributeNamespace() == DOMStringAtom::Null());
      CHECK(record->PreviousSibling() == nullptr);
      CHECK(record->NextSibling() == nullptr);
      REQUIRE(record->AddedNodes() != nullptr);
      CHECK(record->AddedNodes()->Length() == 0u);
      CHECK(record->AddedNodes() == record->AddedNodes());
      REQUIRE(record->RemovedNodes() != nullptr);
      CHECK(record->RemovedNodes()->Length() == 0u);
    }

    SECTION("childList")
    {
      auto first = document->CreateElement(u8"a").ReleaseValue();
      auto removed = document->CreateElement(u8"b").ReleaseValue();
      auto last = document->CreateElement(u8"c").ReleaseValue();
      REQUIRE_FALSE(element->AppendChild(*first).HasException());
      REQUIRE_FALSE(element->AppendChild(*removed).HasException());
      REQUIRE_FALSE(element->AppendChild(*last).HasException());
      REQUIRE_FALSE(observer->Observe(*element, {.ChildList = true}).HasException());
      REQUIRE_FALSE(element->RemoveChild(*removed).HasException());

      auto record = OnlyRecord(*observer);
      CHECK(record->Type() == u8"childList");
      CHECK(record->Target().get() == element.get());
      REQUIRE(record->AddedNodes() != nullptr);
      CHECK(record->AddedNodes()->Length() == 0u);
      CHECK(record->AddedNodes() == record->AddedNodes());
      REQUIRE(record->RemovedNodes() != nullptr);
      REQUIRE(record->RemovedNodes()->Length() == 1u);
      CHECK(record->RemovedNodes()->Item(0).get() == removed.get());
      CHECK(record->RemovedNodes() == record->RemovedNodes());
      CHECK(record->PreviousSibling().get() == first.get());
      CHECK(record->NextSibling().get() == last.get());
      CHECK(record->AttributeName() == DOMStringAtom::Null());
      CHECK(record->AttributeNamespace() == DOMStringAtom::Null());
      CHECK(record->OldValue() == DOMStringAtom::Null());
    }
  }
}
