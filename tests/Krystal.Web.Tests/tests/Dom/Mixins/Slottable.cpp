#include "Krystal.Web/Dom/Mixins/Slottable.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Web/Dom/Nodes/Attr.hpp"
#include "Krystal.Web/Dom/Nodes/CharacterData.hpp"
#include "Krystal.Web/Dom/Nodes/Document.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentFragment.hpp"
#include "Krystal.Web/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Dom/Nodes/HTMLDocument.hpp"
#include "Krystal.Web/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Web/Dom/Nodes/Text.hpp"
#include "Krystal.Web/Html/Elements/HTMLSlotElement.hpp"

namespace krys::web::dom::tests
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
      CHECK(Downcast<HTMLSlotElement>(*slot).AssignedNodes());
    }
  }

  template <typename T>
  concept SlottableMixin = requires(T &node) {
    { node.AssignedSlot() } -> SameType<RefPtr<HTMLSlotElement>>;
  };

  TEST_CASE("Mixin::Slottable", "[HTML][Mixins][dom:mixin-slotable]")
  {
    STATIC_REQUIRE(SlottableMixin<Element>);
    STATIC_REQUIRE(SlottableMixin<krys::web::Text>);
  }
}
