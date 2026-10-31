#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::html::tests
{
  namespace
  {
    /// @brief A connected host with an open shadow root.
    struct SlotTestData
    {
      Ref<dom::HTMLDocument> Document {CreateRef<dom::HTMLDocument>()};
      Ref<dom::Element> Host {Document->CreateElement(u8"div").ReleaseValue()};
      RefPtr<dom::ShadowRoot> Shadow;

      SlotTestData() noexcept
      {
        (void)Document->AppendChild(*Host);
        Shadow = Host->AttachShadow({.Mode = dom::ShadowRootMode::Open}).ReleaseValue();
      }

      KRYS_NODISCARD Ref<HTMLSlotElement> NewSlot() noexcept
      {
        auto slot = Document->CreateElement(u8"slot").ReleaseValue();
        return ShareRef(Downcast<HTMLSlotElement>(*slot));
      }

      /// @brief Whether @p slot's assigned nodes are exactly @p nodes.
      KRYS_NODISCARD static bool AssignedNodesAre(const HTMLSlotElement &slot,
                                                  std::initializer_list<const dom::Node *> nodes) noexcept
      {
        auto *assigned = slot.AssignedNodes();
        if (!assigned)
        {
          return nodes.size() == 0;
        }
        return std::ranges::equal(*assigned, nodes,
                                  [](const auto &weak, const dom::Node *node) { return weak.get() == node; });
      }
    };
  }

  TEST_CASE("Slots: a slot is HTML's slot element, with no assigned nodes to begin with",
            "[HTML][HTMLSlotElement][dom:shadow-tree-slots][dom:concept-slot][dom:slot-assigned-nodes]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto slot = CreateRef<HTMLSlotElement>(*document);

    CHECK(Is<HTMLSlotElement>(static_cast<dom::Node &>(*slot)));
    CHECK(slot->Name().empty());
    CHECK(slot->AssignedNodes() == nullptr);
  }

  TEST_CASE("Slots: the default slot is the first slot whose name is the empty string",
            "[HTML][HTMLSlotElement][dom:shadow-tree-slots][dom:slot-assigned-nodes]")
  {
    SlotTestData data;
    auto first = data.NewSlot();
    auto second = data.NewSlot();
    REQUIRE_FALSE(data.Shadow->AppendChild(*first).HasException());
    REQUIRE_FALSE(data.Shadow->AppendChild(*second).HasException());

    auto text = data.Document->CreateTextNode(u8"text");
    auto span = data.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(data.Host->AppendChild(*text).HasException());
    REQUIRE_FALSE(data.Host->AppendChild(*span).HasException());

    CHECK(SlotTestData::AssignedNodesAre(*first, {text.get(), span.get()}));
    CHECK(SlotTestData::AssignedNodesAre(*second, {}));
  }

  TEST_CASE("Update a slot's name: a change to the name attribute reassigns the tree's slottables",
            "[HTML][HTMLSlotElement][dom:slot-name]")
  {
    SlotTestData data;
    auto slot = data.NewSlot();
    REQUIRE_FALSE(slot->SetAttribute(u8"name", u8"a").HasException());
    REQUIRE_FALSE(data.Shadow->AppendChild(*slot).HasException());

    auto named = data.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(named->SetAttribute(u8"slot", u8"a").HasException());
    auto unnamed = data.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(data.Host->AppendChild(*named).HasException());
    REQUIRE_FALSE(data.Host->AppendChild(*unnamed).HasException());
    REQUIRE(SlotTestData::AssignedNodesAre(*slot, {named.get()}));

    SECTION("Renamed")
    {
      REQUIRE_FALSE(slot->SetAttribute(u8"name", u8"b").HasException());
      CHECK(slot->Name() == u8"b");
      CHECK(SlotTestData::AssignedNodesAre(*slot, {}));
    }

    SECTION("Removed or emptied, it becomes the empty string")
    {
      SECTION("Removed")
      {
        slot->RemoveAttribute(u8"name");
      }
      SECTION("Emptied")
      {
        REQUIRE_FALSE(slot->SetAttribute(u8"name", u8"").HasException());
      }
      CHECK(slot->Name().empty());
      CHECK(SlotTestData::AssignedNodesAre(*slot, {unnamed.get()}));
    }

    SECTION("A name attribute in a namespace doesn't change it")
    {
      slot->RemoveAttribute(u8"name");
      REQUIRE_FALSE(slot->SetAttributeNS(u8"http://example.com/", u8"name", u8"a").HasException());
      CHECK(slot->Name().empty());
      CHECK(SlotTestData::AssignedNodesAre(*slot, {unnamed.get()}));
    }
  }
}
