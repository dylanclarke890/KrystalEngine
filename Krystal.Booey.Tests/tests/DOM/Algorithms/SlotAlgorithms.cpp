#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
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
}
