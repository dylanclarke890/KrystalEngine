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

  TEST_CASE("HTMLSlotElement.assignedNodes() and assignedElements(): the assigned nodes, or flattened",
            "[HTML][HTMLSlotElement][html:dom-slot-assignednodes][html:dom-slot-assignedelements]")
  {
    SlotTestData data;
    auto outer = data.NewSlot();
    REQUIRE_FALSE(data.Shadow->AppendChild(*outer).HasException());
    auto text = data.Document->CreateTextNode(u8"text");
    auto span = data.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(data.Host->AppendChild(*text).HasException());
    REQUIRE_FALSE(data.Host->AppendChild(*span).HasException());

    SECTION("Not flattened")
    {
      auto nodes = outer->AssignedNodes({});
      REQUIRE(nodes.size() == 2);
      CHECK(nodes[0].get() == text.get());
      CHECK(nodes[1].get() == span.get());

      auto elements = outer->AssignedElements();
      REQUIRE(elements.size() == 1);
      CHECK(elements[0].get() == span.get());
    }

    SECTION("Flattened, a slot with nothing assigned gives its children")
    {
      // A slot in the span's shadow tree, which has no children, so nothing is assigned to it.
      auto innerShadow = span->AttachShadow({.Mode = dom::ShadowRootMode::Open}).ReleaseValue();
      auto inner = data.NewSlot();
      auto fallback = data.Document->CreateElement(u8"b").ReleaseValue();
      REQUIRE_FALSE(inner->AppendChild(*fallback).HasException());
      REQUIRE_FALSE(innerShadow->AppendChild(*inner).HasException());
      CHECK(inner->AssignedNodes({}).empty());
      auto innerFlattened = inner->AssignedNodes({.Flatten = true});
      REQUIRE(innerFlattened.size() == 1);
      CHECK(innerFlattened[0].get() == fallback.get());
    }

    SECTION("Flattened, a slot assigned to a slot is replaced by its flattened slottables")
    {
      // The host's child x goes to relay, a slot in the host's shadow tree that is itself the child of
      // nestedHost, whose shadow tree's slot receives relay.
      SlotTestData nesting;
      auto x = nesting.Document->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(nesting.Host->AppendChild(*x).HasException());
      auto nestedHost = nesting.Document->CreateElement(u8"div").ReleaseValue();
      auto relay = nesting.NewSlot();
      REQUIRE_FALSE(nestedHost->AppendChild(*relay).HasException());
      REQUIRE_FALSE(nesting.Shadow->AppendChild(*nestedHost).HasException());
      auto nestedShadow = nestedHost->AttachShadow({.Mode = dom::ShadowRootMode::Open}).ReleaseValue();
      auto inner = nesting.NewSlot();
      REQUIRE_FALSE(nestedShadow->AppendChild(*inner).HasException());

      CHECK(SlotTestData::AssignedNodesAre(*inner, {relay.get()}));
      auto flattened = inner->AssignedNodes({.Flatten = true});
      REQUIRE(flattened.size() == 1);
      CHECK(flattened[0].get() == x.get());
      auto flattenedElements = inner->AssignedElements({.Flatten = true});
      REQUIRE(flattenedElements.size() == 1);
      CHECK(flattenedElements[0].get() == x.get());
    }

    SECTION("Flattened, a slot outside a shadow tree has none")
    {
      auto detached = data.NewSlot();
      REQUIRE_FALSE(
        detached->AppendChild(*data.Document->CreateElement(u8"b").ReleaseValue()).HasException());
      CHECK(detached->AssignedNodes({.Flatten = true}).empty());
    }
  }

  TEST_CASE("HTMLSlotElement.assign(): sets the manually assigned nodes, which a manual shadow root assigns",
            "[HTML][HTMLSlotElement][html:dom-slot-assign]")
  {
    auto document = CreateRef<dom::HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow =
      host
        ->AttachShadow({.Mode = dom::ShadowRootMode::Open, .SlotAssignment = dom::SlotAssignmentMode::Manual})
        .ReleaseValue();
    auto newSlot = [&]
    {
      return ShareRef(Downcast<HTMLSlotElement>(*document->CreateElement(u8"slot").ReleaseValue()));
    };
    auto first = newSlot();
    auto second = newSlot();
    REQUIRE_FALSE(shadow->AppendChild(*first).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*second).HasException());

    auto a = document->CreateElement(u8"span").ReleaseValue();
    auto b = document->CreateTextNode(u8"b");
    auto notAChild = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(host->AppendChild(*a).HasException());
    REQUIRE_FALSE(host->AppendChild(*b).HasException());

    // Nothing is assigned by name in a manual shadow root.
    CHECK(SlotTestData::AssignedNodesAre(*first, {}));

    // In the order given, once each; nodes that aren't the host's children are kept but not assigned.
    first->Assign(
      {Ref<dom::Text>(b), Ref<dom::Element>(a), Ref<dom::Element>(a), Ref<dom::Element>(notAChild)});
    CHECK(SlotTestData::AssignedNodesAre(*first, {b.get(), a.get()}));

    SECTION("Assigning to another slot moves the node")
    {
      second->Assign({Ref<dom::Element>(a)});
      CHECK(SlotTestData::AssignedNodesAre(*first, {b.get()}));
      CHECK(SlotTestData::AssignedNodesAre(*second, {a.get()}));
    }

    SECTION("A manually assigned node that becomes the host's child is assigned")
    {
      REQUIRE_FALSE(host->AppendChild(*notAChild).HasException());
      CHECK(SlotTestData::AssignedNodesAre(*first, {b.get(), a.get(), notAChild.get()}));
    }

    SECTION("Assigning nothing clears it")
    {
      first->Assign({});
      CHECK(SlotTestData::AssignedNodesAre(*first, {}));
    }
  }
}
