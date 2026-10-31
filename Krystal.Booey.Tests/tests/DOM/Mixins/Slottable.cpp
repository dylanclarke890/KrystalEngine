#include "Krystal.Booey/DOM/Mixins/Slottable.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("assignedSlot: the slot the slottable finds, only in an open shadow root",
            "[DOM][Slottable][dom:mixin-slotable][dom:dom-slotable-assignedslot]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    auto element = document->CreateElement(u8"span").ReleaseValue();
    auto text = document->CreateTextNode(u8"text");
    REQUIRE_FALSE(host->AppendChild(*element).HasException());
    REQUIRE_FALSE(host->AppendChild(*text).HasException());

    SECTION("Without a shadow root, none")
    {
      CHECK(element->AssignedSlot() == nullptr);
      CHECK(text->AssignedSlot() == nullptr);
    }

    SECTION("In an open shadow root, the slot")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto slot = document->CreateElement(u8"slot").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
      CHECK(element->AssignedSlot().get() == slot.get());
      CHECK(text->AssignedSlot().get() == slot.get());
    }

    SECTION("In a closed shadow root, none, though it's assigned")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
      auto slot = document->CreateElement(u8"slot").ReleaseValue();
      REQUIRE_FALSE(shadow->AppendChild(*slot).HasException());
      CHECK(element->AssignedSlot() == nullptr);
      CHECK(text->AssignedSlot() == nullptr);
      CHECK(Downcast<html::HTMLSlotElement>(*slot).AssignedNodes());
    }
  }
}
