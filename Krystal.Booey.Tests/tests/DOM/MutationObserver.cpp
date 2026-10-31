#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<MutationObserver> CreateEmptyObserver() noexcept
    {
      return CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    }

    KRYS_NODISCARD Ref<Element> CreateDiv(Document &document) noexcept
    {
      return document.CreateElement(u8"div").ReleaseValue();
    }

    KRYS_NODISCARD bool ThrowsTypeError(const ExceptionOr<void> &result) noexcept
    {
      return result.HasException() && result.GetException().Code() == ExceptionCode::TypeError;
    }
  }

  TEST_CASE(
    "new MutationObserver(callback): the callback gets the records and the observer, in creation order",
    "[DOM][MutationObserver][dom:interface-mutationobserver][dom:dom-mutationobserver-mutationobserver]"
    "[dom:concept-mo-callback][dom:mutationobserver-creation-index]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto target = CreateDiv(*document);

    List<const MutationObserver *> calls;
    size_t recordCount = 0;
    auto callback = [&](const List<Ref<MutationRecord>> &records, MutationObserver &observer)
    {
      recordCount += records.size();
      calls.push_back(&observer);
    };
    auto first = CreateRef<MutationObserver>(callback);
    auto second = CreateRef<MutationObserver>(callback);

    // The later observer observes, and so is pending, first.
    REQUIRE_FALSE(second->Observe(*target, {.Attributes = true}).HasException());
    REQUIRE_FALSE(first->Observe(*target, {.Attributes = true}).HasException());
    REQUIRE_FALSE(target->SetAttribute(u8"title", u8"x").HasException());
    loop.PerformMicrotaskCheckpoint();

    CHECK(calls == List<const MutationObserver *> {first.get(), second.get()});
    CHECK(recordCount == 2);
  }

  TEST_CASE("MutationObserver.observe(): invalid options throw a TypeError",
            "[DOM][MutationObserver][dom:dom-mutationobserver-observe]")
  {
    auto document = CreateRef<Document>();
    auto observer = CreateEmptyObserver();

    // 3. Nothing to observe.
    CHECK(ThrowsTypeError(observer->Observe(*document, {})));
    CHECK(ThrowsTypeError(observer->Observe(*document, {.Attributes = false, .CharacterData = false})));
    CHECK(ThrowsTypeError(observer->Observe(*document, {.Subtree = true})));

    // 4. to 6. An old value or filter for what isn't observed.
    CHECK(ThrowsTypeError(
      observer->Observe(*document, {.ChildList = true, .Attributes = false, .AttributeOldValue = true})));
    CHECK(ThrowsTypeError(observer->Observe(
      *document, {.ChildList = true, .Attributes = false, .AttributeFilter = List<DOMStringAtom> {u8"id"}})));
    CHECK(ThrowsTypeError(observer->Observe(
      *document, {.ChildList = true, .CharacterData = false, .CharacterDataOldValue = true})));

    // A false old value with its type turned off is fine.
    CHECK_FALSE(
      observer->Observe(*document, {.ChildList = true, .Attributes = false, .AttributeOldValue = false})
        .HasException());
  }

  TEST_CASE("MutationObserver.observe(): an old value or filter option implies its type, even when false",
            "[DOM][MutationObserver][dom:dom-mutationobserver-observe]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto element = CreateDiv(*document);
    auto text = document->CreateTextNode(u8"a");
    auto observer = CreateEmptyObserver();

    SECTION("attributeOldValue")
    {
      auto option = GENERATE(true, false);
      REQUIRE_FALSE(observer->Observe(*element, {.AttributeOldValue = option}).HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"title", u8"x").HasException());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->Type() == u8"attributes");
    }

    SECTION("attributeFilter")
    {
      REQUIRE_FALSE(
        observer->Observe(*element, {.AttributeFilter = List<DOMStringAtom> {u8"title"}}).HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"id", u8"x").HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"title", u8"x").HasException());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->AttributeName() == u8"title");
    }

    SECTION("characterDataOldValue")
    {
      auto option = GENERATE(true, false);
      REQUIRE_FALSE(observer->Observe(*text, {.CharacterDataOldValue = option}).HasException());
      REQUIRE_FALSE(text->AppendData(u8"b").HasException());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->Type() == u8"characterData");
      CHECK(records[0]->OldValue() == (option ? DOMStringAtom(u8"a") : DOMStringAtom::Null()));
    }
  }

  TEST_CASE("MutationObserver.observe(): observing a node again replaces the options",
            "[DOM][MutationObserver][dom:dom-mutationobserver-observe][dom:mutationobserver-node-list]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto element = CreateDiv(*document);
    auto observer = CreateEmptyObserver();

    REQUIRE_FALSE(observer->Observe(*element, {.Attributes = true}).HasException());
    REQUIRE_FALSE(observer->Observe(*element, {.ChildList = true}).HasException());

    // One registration, with the new options.
    REQUIRE_FALSE(element->SetAttribute(u8"title", u8"x").HasException());
    REQUIRE_FALSE(element->AppendChild(*CreateDiv(*document)).HasException());
    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1);
    CHECK(records[0]->Type() == u8"childList");
  }

  TEST_CASE(
    "MutationObserver.observe(): observing a node again drops the transient observers it was the source of",
    "[DOM][MutationObserver][dom:dom-mutationobserver-observe][dom:mutationobserver-transient-node-set]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto parent = CreateDiv(*document);
    auto child = CreateDiv(*document);
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());
    auto observer = CreateEmptyObserver();
    REQUIRE_FALSE(observer->Observe(*parent, {.ChildList = true, .Subtree = true}).HasException());

    // The removed child is observed through a transient observer...
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    REQUIRE_FALSE(child->AppendChild(*CreateDiv(*document)).HasException());
    CHECK(observer->TakeRecords().size() == 2);

    // ...until the parent is observed again.
    REQUIRE_FALSE(observer->Observe(*parent, {.ChildList = true, .Subtree = true}).HasException());
    REQUIRE_FALSE(child->AppendChild(*CreateDiv(*document)).HasException());
    CHECK(observer->TakeRecords().empty());
  }

  TEST_CASE("MutationObserver.disconnect(): stops observing every node and drops the queued records",
            "[DOM][MutationObserver][dom:dom-mutationobserver-disconnect][dom:mutationobserver-node-list]"
            "[dom:concept-mo-queue]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto parent = CreateDiv(*document);
    auto child = CreateDiv(*document);
    auto other = CreateDiv(*document);
    REQUIRE_FALSE(parent->AppendChild(*child).HasException());

    size_t calls = 0;
    auto observer =
      CreateRef<MutationObserver>([&](const List<Ref<MutationRecord>> &, MutationObserver &) { ++calls; });
    REQUIRE_FALSE(observer->Observe(*parent, {.ChildList = true, .Subtree = true}).HasException());
    REQUIRE_FALSE(observer->Observe(*other, {.Attributes = true}).HasException());

    // A queued record, and a transient observer on the removed child.
    REQUIRE_FALSE(parent->RemoveChild(*child).HasException());
    observer->Disconnect();

    CHECK(observer->TakeRecords().empty());
    REQUIRE_FALSE(parent->AppendChild(*CreateDiv(*document)).HasException());
    REQUIRE_FALSE(child->AppendChild(*CreateDiv(*document)).HasException());
    REQUIRE_FALSE(other->SetAttribute(u8"title", u8"x").HasException());
    CHECK(observer->TakeRecords().empty());
    loop.PerformMicrotaskCheckpoint();
    CHECK(calls == 0);

    // It can observe again.
    REQUIRE_FALSE(observer->Observe(*other, {.Attributes = true}).HasException());
    REQUIRE_FALSE(other->SetAttribute(u8"title", u8"y").HasException());
    CHECK(observer->TakeRecords().size() == 1);
  }

  TEST_CASE("MutationObserver.takeRecords(): returns the queued records and empties the queue",
            "[DOM][MutationObserver][dom:dom-mutationobserver-takerecords][dom:concept-mo-queue]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<Document>();
    auto element = CreateDiv(*document);

    size_t calls = 0;
    auto observer =
      CreateRef<MutationObserver>([&](const List<Ref<MutationRecord>> &, MutationObserver &) { ++calls; });
    REQUIRE_FALSE(observer->Observe(*element, {.ChildList = true, .Attributes = true}).HasException());

    CHECK(observer->TakeRecords().empty());

    REQUIRE_FALSE(element->SetAttribute(u8"title", u8"x").HasException());
    REQUIRE_FALSE(element->AppendChild(*CreateDiv(*document)).HasException());
    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 2);
    CHECK(records[0]->Type() == u8"attributes");
    CHECK(records[1]->Type() == u8"childList");
    CHECK(records[0]->Target().get() == element.get());

    // Nothing is left to take or deliver.
    CHECK(observer->TakeRecords().empty());
    loop.PerformMicrotaskCheckpoint();
    CHECK(calls == 0);
  }
}
