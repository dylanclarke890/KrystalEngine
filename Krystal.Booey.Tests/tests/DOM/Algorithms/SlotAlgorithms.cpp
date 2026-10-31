#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/EventListenerHandle.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Slottables: elements and text, including slots", "[DOM][SlotAlgorithms][dom:concept-slotable]")
  {
    auto document = CreateRef<HTMLDocument>();
    CHECK(SlotAlgorithms::IsSlottable(
      static_cast<const Node &>(*document->CreateElement(u8"span").ReleaseValue())));
    CHECK(SlotAlgorithms::IsSlottable(
      static_cast<const Node &>(*document->CreateElement(u8"slot").ReleaseValue())));
    CHECK(SlotAlgorithms::IsSlottable(static_cast<const Node &>(*document->CreateTextNode(u8"text"))));
    CHECK_FALSE(
      SlotAlgorithms::IsSlottable(static_cast<const Node &>(*document->CreateComment(u8"comment"))));
    CHECK_FALSE(SlotAlgorithms::IsSlottable(static_cast<const Node &>(*document)));
  }

  TEST_CASE("Assigned: a slottable whose assigned slot is non-null",
            "[DOM][SlotAlgorithms][dom:slotable-assigned][dom:slotable-assigned-slot]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();

    auto element = document->CreateElement(u8"span").ReleaseValue();
    auto text = document->CreateTextNode(u8"text");
    REQUIRE_FALSE(host->AppendChild(*element).HasException());
    REQUIRE_FALSE(host->AppendChild(*text).HasException());
    CHECK_FALSE(SlotAlgorithms::IsAssigned(static_cast<const Node &>(*element)));
    CHECK_FALSE(SlotAlgorithms::IsAssigned(static_cast<const Node &>(*text)));
    CHECK(SlotAlgorithms::GetAssignedSlot(static_cast<const Node &>(*element)) == nullptr);

    auto slot = document->CreateElement(u8"slot").ReleaseValue();
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
    CHECK(SlotAlgorithms::IsAssigned(static_cast<const Node &>(*element)));
    CHECK(SlotAlgorithms::IsAssigned(static_cast<const Node &>(*text)));
    CHECK(SlotAlgorithms::GetAssignedSlot(static_cast<const Node &>(*element)) == slot.get());
    CHECK(SlotAlgorithms::GetAssignedSlot(static_cast<const Node &>(*text)) == slot.get());

    REQUIRE_FALSE(element->Remove().HasException());
    CHECK_FALSE(SlotAlgorithms::IsAssigned(static_cast<const Node &>(*element)));
  }

  TEST_CASE("Manual slot assignment: the slot a slottable was given to by assign()",
            "[DOM][SlotAlgorithms][dom:slottable-manual-slot-assignment]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"span").ReleaseValue();
    auto text = document->CreateTextNode(u8"text");
    CHECK(SlotAlgorithms::GetManualSlotAssignment(*element) == nullptr);

    auto newSlot = [&]
    {
      return ShareRef(Downcast<html::HTMLSlotElement>(*document->CreateElement(u8"slot").ReleaseValue()));
    };
    auto first = newSlot();
    auto second = newSlot();

    first->Assign({Ref<Element>(element), Ref<Text>(text)});
    CHECK(SlotAlgorithms::GetManualSlotAssignment(*element) == first.get());
    CHECK(SlotAlgorithms::GetManualSlotAssignment(*text) == first.get());

    // Assigning to another slot moves it; reassigning the first slot clears the rest.
    second->Assign({Ref<Element>(element)});
    CHECK(SlotAlgorithms::GetManualSlotAssignment(*element) == second.get());
    first->Assign({});
    CHECK(SlotAlgorithms::GetManualSlotAssignment(*text) == nullptr);
    CHECK(SlotAlgorithms::GetManualSlotAssignment(*element) == second.get());
  }

  namespace
  {
    KRYS_NODISCARD Ref<html::HTMLSlotElement> NewSlotNamed(Document &document, DOMString &&name) noexcept
    {
      auto slot = ShareRef(Downcast<html::HTMLSlotElement>(*document.CreateElement(u8"slot").ReleaseValue()));
      if (!name.empty())
      {
        (void)slot->SetAttribute(u8"name", krys::move(name));
      }
      return slot;
    }
  }

  TEST_CASE("Find a slot: the first slot in the parent's shadow root with the slottable's name",
            "[DOM][SlotAlgorithms][dom:finding-slots-and-slotables][dom:find-a-slot]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto span = document->CreateElement(u8"span").ReleaseValue();

    SECTION("Without a parent, or a parent without a shadow root, none")
    {
      CHECK(SlotAlgorithms::FindSlot(*span) == nullptr);
      auto parent = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(parent->AppendChild(*span).HasException());
      CHECK(SlotAlgorithms::FindSlot(*span) == nullptr);
    }

    SECTION("Named: the first matching slot in tree order, or none")
    {
      auto host = document->CreateElement(u8"div").ReleaseValue();
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
      auto wrapper = document->CreateElement(u8"div").ReleaseValue();
      auto nestedA = NewSlotNamed(*document, u8"a");
      auto laterA = NewSlotNamed(*document, u8"a");
      REQUIRE_FALSE(wrapper->AppendChild(*nestedA).HasException());
      REQUIRE_FALSE(shadow->AppendChild(*wrapper).HasException());
      REQUIRE_FALSE(shadow->AppendChild(*laterA).HasException());
      REQUIRE_FALSE(host->AppendChild(*span).HasException());

      // No slot is named the empty string.
      CHECK(SlotAlgorithms::FindSlot(*span) == nullptr);

      REQUIRE_FALSE(span->SetAttribute(u8"slot", u8"a").HasException());
      CHECK(SlotAlgorithms::FindSlot(*span) == nestedA.get());

      // With open, a closed shadow root gives none.
      CHECK(SlotAlgorithms::FindSlot(*span, true) == nullptr);
    }

    SECTION("Manual: the slot whose manually assigned nodes contain it, whatever its name")
    {
      auto host = document->CreateElement(u8"div").ReleaseValue();
      auto shadow =
        host->AttachShadow({.Mode = ShadowRootMode::Open, .SlotAssignment = SlotAssignmentMode::Manual})
          .ReleaseValue();
      auto first = NewSlotNamed(*document, u8"");
      auto second = NewSlotNamed(*document, u8"");
      REQUIRE_FALSE(shadow->AppendChild(*first).HasException());
      REQUIRE_FALSE(shadow->AppendChild(*second).HasException());
      REQUIRE_FALSE(host->AppendChild(*span).HasException());
      CHECK(SlotAlgorithms::FindSlot(*span) == nullptr);

      second->Assign({Ref<Element>(span)});
      CHECK(SlotAlgorithms::FindSlot(*span) == second.get());
      CHECK(SlotAlgorithms::FindSlot(*span, true) == second.get());
    }
  }

  TEST_CASE(
    "Find slottables: the host's slottable children that find the slot, or its manually assigned nodes",
    "[DOM][SlotAlgorithms][dom:find-slotables]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    auto text = document->CreateTextNode(u8"text");
    auto comment = document->CreateComment(u8"comment");
    auto named = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(named->SetAttribute(u8"slot", u8"a").HasException());
    auto span = document->CreateElement(u8"span").ReleaseValue();

    SECTION("A slot outside a shadow tree, none")
    {
      auto slot = NewSlotNamed(*document, u8"");
      REQUIRE_FALSE(host->AppendChild(*slot).HasException());
      CHECK(SlotAlgorithms::FindSlottables(*slot).empty());
    }

    SECTION("Named: the slottable children whose slot it is, in tree order")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto slot = NewSlotNamed(*document, u8"");
      REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
      for (Node *child : {static_cast<Node *>(text.get()), static_cast<Node *>(comment.get()),
                          static_cast<Node *>(named.get()), static_cast<Node *>(span.get())})
      {
        REQUIRE_FALSE(host->AppendChild(*child).HasException());
      }

      auto slottables = SlotAlgorithms::FindSlottables(*slot);
      REQUIRE(slottables.size() == 2);
      CHECK(slottables[0].get() == text.get());
      CHECK(slottables[1].get() == span.get());
    }

    SECTION("Manual: the manually assigned nodes that are the host's children, in their order")
    {
      auto shadow =
        host->AttachShadow({.Mode = ShadowRootMode::Open, .SlotAssignment = SlotAssignmentMode::Manual})
          .ReleaseValue();
      auto slot = NewSlotNamed(*document, u8"");
      REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
      REQUIRE_FALSE(host->AppendChild(*text).HasException());
      REQUIRE_FALSE(host->AppendChild(*named).HasException());

      slot->Assign({Ref<Element>(named), Ref<Element>(span), Ref<Text>(text)});
      auto slottables = SlotAlgorithms::FindSlottables(*slot);
      REQUIRE(slottables.size() == 2);
      CHECK(slottables[0].get() == named.get());
      CHECK(slottables[1].get() == text.get());
    }
  }

  TEST_CASE(
    "Assign slottables: sets the assigned nodes and their assigned slot, signalling a change only if any",
    "[DOM][SlotAlgorithms][dom:assigning-slotables-and-slots][dom:assign-slotables]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto slot = NewSlotNamed(*document, u8"");
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());

    int changes = 0;
    auto listener = slot->AddEventListener(DOMStringAtom(u8"slotchange"), [&](Event &) { ++changes; });

    auto span = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(host->AppendChild(*span).HasException());
    loop.PerformMicrotaskCheckpoint();
    REQUIRE(changes == 1);

    // Nothing changed, so no slot change.
    SlotAlgorithms::AssignSlottables(*slot);
    loop.PerformMicrotaskCheckpoint();
    CHECK(changes == 1);
    REQUIRE(slot->AssignedNodes());
    CHECK(slot->AssignedNodes()->size() == 1);
    CHECK(SlotAlgorithms::GetAssignedSlot(*span) == slot.get());

    REQUIRE_FALSE(span->SetAttribute(u8"slot", u8"elsewhere").HasException());
    loop.PerformMicrotaskCheckpoint();
    CHECK(changes == 2);
    CHECK(slot->AssignedNodes() == nullptr);
    CHECK(SlotAlgorithms::GetAssignedSlot(*span) == nullptr);
  }

  TEST_CASE("Assign slottables for a tree: every slot among the root's inclusive descendants",
            "[DOM][SlotAlgorithms][dom:assign-slotables-for-a-tree]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto a = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(a->SetAttribute(u8"slot", u8"a").HasException());
    auto b = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(b->SetAttribute(u8"slot", u8"b").HasException());
    REQUIRE_FALSE(host->AppendChild(*a).HasException());
    REQUIRE_FALSE(host->AppendChild(*b).HasException());

    // Inserting a subtree holding two slots assigns both, through its root.
    auto wrapper = document->CreateElement(u8"div").ReleaseValue();
    auto slotA = NewSlotNamed(*document, u8"a");
    auto slotB = NewSlotNamed(*document, u8"b");
    REQUIRE_FALSE(wrapper->AppendChild(*slotA).HasException());
    REQUIRE_FALSE(wrapper->AppendChild(*slotB).HasException());
    CHECK(slotA->AssignedNodes() == nullptr);
    REQUIRE_FALSE(shadow->AppendChild(*wrapper).HasException());

    CHECK(SlotAlgorithms::GetAssignedSlot(*a) == slotA.get());
    CHECK(SlotAlgorithms::GetAssignedSlot(*b) == slotB.get());

    // Removing the subtree reassigns the slots it holds, which find nothing outside a shadow tree.
    REQUIRE_FALSE(wrapper->Remove().HasException());
    CHECK(slotA->AssignedNodes() == nullptr);
    CHECK(SlotAlgorithms::GetAssignedSlot(*a) == nullptr);
  }

  TEST_CASE("Assign a slot: assigns the slottables of the slot the slottable finds, if any",
            "[DOM][SlotAlgorithms][dom:assign-a-slot]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto slot = NewSlotNamed(*document, u8"a");
    REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());

    auto unnamed = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(host->AppendChild(*unnamed).HasException());
    CHECK(SlotAlgorithms::GetAssignedSlot(*unnamed) == nullptr);

    auto named = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(named->SetAttribute(u8"slot", u8"a").HasException());
    REQUIRE_FALSE(host->AppendChild(*named).HasException());
    CHECK(SlotAlgorithms::GetAssignedSlot(*named) == slot.get());

    // Called directly, it leaves things as they are.
    SlotAlgorithms::AssignSlot(*named);
    SlotAlgorithms::AssignSlot(*unnamed);
    CHECK(SlotAlgorithms::GetAssignedSlot(*named) == slot.get());
    CHECK(SlotAlgorithms::GetAssignedSlot(*unnamed) == nullptr);
  }
}
