#include "Krystal.Booey/DOM/Collections/LiveCollection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/HTML/Collections/HTMLOptionsCollection.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptGroupElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLOptionElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSelectElement.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct LiveCollectionFixture
    {
      Ref<HTMLDocument> Document {CreateRef<HTMLDocument>()};
      Ref<Element> Root {Create(u8"div")};

      Ref<Element> Create(DOMStringAtom name) noexcept
      {
        return Document->CreateElement(name).ReleaseValue();
      }

      /// @brief Appends a new element to @p parent.
      Ref<Element> Add(Node &parent, DOMStringAtom name) noexcept
      {
        auto element = Create(name);
        REQUIRE_FALSE(parent.AppendChild(*element).HasException());
        return element;
      }

      /// @brief Puts @p Root in the document, with an `html` element above it.
      void Connect() noexcept
      {
        auto html = Add(*Document, u8"html");
        REQUIRE_FALSE(html->AppendChild(*Root).HasException());
      }
    };
  }

  TEST_CASE_METHOD(LiveCollectionFixture, "LiveHTMLCollection: follows its items by index in any order",
                   "[DOM][LiveCollection][dom:concept-collection-live]")
  {
    auto first = Add(*Root, u8"p");
    auto second = Add(*Root, u8"p");
    auto third = Add(*Root, u8"p");
    auto collection = Root->GetElementsByTagName(u8"p");

    SECTION("by index, before its length is known")
    {
      CHECK(collection->Item(1).get() == second.get());
      CHECK(collection->Item(2).get() == third.get());
      CHECK(collection->Item(0).get() == first.get());
      CHECK(collection->Item(2).get() == third.get());
    }

    SECTION("past its end, which tells it its length")
    {
      CHECK(collection->Item(5).get() == nullptr);
      CHECK(collection->Length() == 3uz);
      CHECK(collection->Item(2).get() == third.get());
      CHECK(collection->Item(3).get() == nullptr);
    }

    SECTION("by index after its length")
    {
      CHECK(collection->Length() == 3uz);
      CHECK(collection->Item(0).get() == first.get());
      CHECK(collection->Item(2).get() == third.get());
      CHECK(collection->Item(3).get() == nullptr);
    }

    SECTION("when it has no items")
    {
      auto none = Root->GetElementsByTagName(u8"span");
      CHECK(none->Item(0).get() == nullptr);
      CHECK(none->Length() == 0uz);
      CHECK(none->NamedItem(u8"a").get() == nullptr);
    }
  }

  TEST_CASE_METHOD(LiveCollectionFixture,
                   "LiveHTMLCollection: a change to the tree changes what it has cached",
                   "[DOM][LiveCollection][dom:concept-collection-live]")
  {
    auto middle = Add(*Root, u8"div");
    auto first = Add(*middle, u8"p");
    auto collection = Root->GetElementsByTagName(u8"p");
    CHECK(collection->Length() == 1uz);
    CHECK(collection->Item(0).get() == first.get());

    SECTION("insertion anywhere below the root")
    {
      auto second = Add(*middle, u8"p");
      CHECK(collection->Length() == 2uz);
      CHECK(collection->Item(1).get() == second.get());
    }

    SECTION("insertion above the cached position")
    {
      auto earlier = Create(u8"p");
      REQUIRE_FALSE(middle->InsertBefore(*earlier, first.get()).HasException());
      CHECK(collection->Item(0).get() == earlier.get());
      CHECK(collection->Item(1).get() == first.get());
    }

    SECTION("insertion of a subtree holding items")
    {
      auto subtree = Create(u8"section");
      auto inner = Add(*subtree, u8"p");
      REQUIRE_FALSE(Root->AppendChild(*subtree).HasException());
      CHECK(collection->Length() == 2uz);
      CHECK(collection->Item(1).get() == inner.get());
    }

    SECTION("removal, of the item and of an ancestor of it")
    {
      REQUIRE_FALSE(middle->RemoveChild(*first).HasException());
      CHECK(collection->Length() == 0uz);
      CHECK(collection->Item(0).get() == nullptr);

      REQUIRE_FALSE(middle->AppendChild(*first).HasException());
      CHECK(collection->Length() == 1uz);
      REQUIRE_FALSE(Root->RemoveChild(*middle).HasException());
      CHECK(collection->Length() == 0uz);
    }

    SECTION("a move into, out of and within it")
    {
      Connect();
      auto outside = Add(*Root->ParentNode(), u8"p");
      auto other = Add(*Root, u8"p");
      CHECK(collection->Item(1).get() == other.get());

      REQUIRE_FALSE(Root->MoveBefore(*other, middle.get()).HasException());
      CHECK(collection->Item(0).get() == other.get());

      REQUIRE_FALSE(Root->MoveBefore(*outside, nullptr).HasException());
      CHECK(collection->Length() == 3uz);
      CHECK(collection->Item(2).get() == outside.get());

      REQUIRE_FALSE(Downcast<Element>(*Root->ParentNode()).MoveBefore(*outside, nullptr).HasException());
      CHECK(collection->Length() == 2uz);
    }

    SECTION("a change outside it leaves it alone")
    {
      Connect();
      auto outside = Add(*Root->ParentNode(), u8"p");
      CHECK(collection->Length() == 1uz);
      CHECK(collection->Item(0).get() == first.get());
      REQUIRE_FALSE(Root->ParentNode()->RemoveChild(*outside).HasException());
      CHECK(collection->Length() == 1uz);
    }

    SECTION("a change in a shadow tree below it isn't in it")
    {
      auto shadow = middle->AttachShadow({}).ReleaseValue();
      (void)Add(*shadow, u8"p");
      CHECK(collection->Length() == 1uz);

      auto own = shadow->Children();
      CHECK(own->Length() == 1uz);
      (void)Add(*shadow, u8"p");
      CHECK(own->Length() == 2uz);
      CHECK(collection->Length() == 1uz);
    }

    SECTION("a text node doesn't change an element collection, but is still seen by later changes")
    {
      REQUIRE_FALSE(middle->AppendChild(*Document->CreateTextNode(u8"text")).HasException());
      CHECK(collection->Length() == 1uz);
      (void)Add(*middle, u8"p");
      CHECK(collection->Length() == 2uz);
    }
  }

  TEST_CASE_METHOD(LiveCollectionFixture,
                   "LiveHTMLCollection: the attribute its filter reads changes what it has cached",
                   "[DOM][LiveCollection][dom:concept-getelementsbyclassname]")
  {
    auto a = Add(*Root, u8"p");
    auto b = Add(*Root, u8"p");
    REQUIRE_FALSE(a->SetAttribute(u8"class", u8"on").HasException());
    auto collection = Root->GetElementsByClassName(u8"on");
    CHECK(collection->Length() == 1uz);

    SECTION("setting and changing")
    {
      REQUIRE_FALSE(b->SetAttribute(u8"class", u8"on other").HasException());
      CHECK(collection->Length() == 2uz);
      REQUIRE_FALSE(b->SetAttribute(u8"class", u8"other").HasException());
      CHECK(collection->Length() == 1uz);
    }

    SECTION("removing")
    {
      a->RemoveAttribute(u8"class");
      CHECK(collection->Length() == 0uz);
    }

    SECTION("through the class list and the attribute node")
    {
      (void)b->ClassList().Add(List<DOMString> {u8"on"});
      CHECK(collection->Length() == 2uz);
      a->GetAttributeNode(u8"class")->Value(u8"off");
      CHECK(collection->Length() == 1uz);
      CHECK(collection->Item(0).get() == b.get());
    }

    SECTION("of a descendant of an item")
    {
      auto inner = Add(*a, u8"span");
      CHECK(collection->Length() == 1uz);
      REQUIRE_FALSE(inner->SetAttribute(u8"class", u8"on").HasException());
      CHECK(collection->Length() == 2uz);
      CHECK(collection->Item(1).get() == inner.get());
    }
  }

  TEST_CASE_METHOD(LiveCollectionFixture, "LiveHTMLCollection: a named item follows IDs and names",
                   "[DOM][LiveCollection][dom:dom-htmlcollection-nameditem-key]")
  {
    auto first = Add(*Root, u8"p");
    auto second = Add(*Root, u8"p");
    REQUIRE_FALSE(first->SetAttribute(u8"id", u8"a").HasException());
    REQUIRE_FALSE(second->SetAttribute(u8"name", u8"b").HasException());
    auto collection = Root->GetElementsByTagName(u8"p");

    CHECK(collection->NamedItem(u8"a").get() == first.get());
    CHECK(collection->NamedItem(u8"b").get() == second.get());
    CHECK(collection->NamedItem(u8"c").get() == nullptr);
    CHECK(collection->NamedItem(u8"").get() == nullptr);

    SECTION("the first in tree order with an ID or a name wins")
    {
      REQUIRE_FALSE(second->SetAttribute(u8"id", u8"a").HasException());
      CHECK(collection->NamedItem(u8"a").get() == first.get());
      REQUIRE_FALSE(first->SetAttribute(u8"name", u8"b").HasException());
      CHECK(collection->NamedItem(u8"b").get() == first.get());
    }

    SECTION("a new ID or name, though its filter doesn't read them")
    {
      REQUIRE_FALSE(first->SetAttribute(u8"id", u8"c").HasException());
      CHECK(collection->NamedItem(u8"a").get() == nullptr);
      CHECK(collection->NamedItem(u8"c").get() == first.get());

      second->RemoveAttribute(u8"name");
      CHECK(collection->NamedItem(u8"b").get() == nullptr);

      // The items themselves stay cached, and are right.
      CHECK(collection->Length() == 2uz);
      CHECK(collection->Item(1).get() == second.get());
    }

    SECTION("a change to the items")
    {
      REQUIRE_FALSE(Root->RemoveChild(*first).HasException());
      CHECK(collection->NamedItem(u8"a").get() == nullptr);

      auto earlier = Create(u8"p");
      REQUIRE_FALSE(earlier->SetAttribute(u8"id", u8"b").HasException());
      REQUIRE_FALSE(Root->InsertBefore(*earlier, second.get()).HasException());
      CHECK(collection->NamedItem(u8"b").get() == earlier.get());
    }

    SECTION("the names it supports")
    {
      CHECK(collection->SupportedPropertyNames() == List<DOMString> {u8"a", u8"b"});
      REQUIRE_FALSE(second->SetAttribute(u8"id", u8"d").HasException());
      CHECK(collection->SupportedPropertyNames() == List<DOMString> {u8"a", u8"d", u8"b"});
    }
  }

  TEST_CASE_METHOD(LiveCollectionFixture, "LiveCollection: childNodes and children hold only children",
                   "[DOM][LiveCollection][dom:dom-node-childnodes][dom:dom-parentnode-children]")
  {
    auto element = Add(*Root, u8"p");
    (void)Add(*element, u8"span");
    auto text = Document->CreateTextNode(u8"text");
    auto childNodes = Root->ChildNodes();
    auto children = Root->Children();
    CHECK(childNodes->Length() == 1uz);
    CHECK(children->Length() == 1uz);

    SECTION("a text node changes childNodes only")
    {
      REQUIRE_FALSE(Root->AppendChild(*text).HasException());
      CHECK(childNodes->Length() == 2uz);
      CHECK(childNodes->Item(1).get() == text.get());
      CHECK(children->Length() == 1uz);

      REQUIRE_FALSE(Root->RemoveChild(*text).HasException());
      CHECK(childNodes->Length() == 1uz);
      CHECK(childNodes->Item(1).get() == nullptr);
    }

    SECTION("an element changes both")
    {
      auto next = Add(*Root, u8"p");
      CHECK(childNodes->Item(1).get() == next.get());
      CHECK(children->Item(1).get() == next.get());
    }

    SECTION("a grandchild changes neither")
    {
      (void)Add(*element, u8"span");
      CHECK(childNodes->Length() == 1uz);
      CHECK(children->Length() == 1uz);
    }
  }

  TEST_CASE_METHOD(LiveCollectionFixture,
                   "LiveCollection: a document's collections follow the attributes they read",
                   "[DOM][LiveCollection][html:dom-document-links][html:dom-document-anchors]"
                   "[html:dom-document-getelementsbyname]")
  {
    Connect();
    auto link = Add(*Root, u8"a");
    auto links = Document->Links();
    auto anchors = Document->Anchors();
    auto named = Document->GetElementsByName(u8"n");
    CHECK(links->Length() == 0uz);
    CHECK(anchors->Length() == 0uz);
    CHECK(named->Length() == 0uz);

    REQUIRE_FALSE(link->SetAttribute(u8"href", u8"#").HasException());
    CHECK(links->Length() == 1uz);
    CHECK(anchors->Length() == 0uz);

    REQUIRE_FALSE(link->SetAttribute(u8"name", u8"n").HasException());
    CHECK(anchors->Length() == 1uz);
    CHECK(named->Length() == 1uz);

    link->RemoveAttribute(u8"href");
    CHECK(links->Length() == 0uz);

    REQUIRE_FALSE(link->SetAttribute(u8"name", u8"m").HasException());
    CHECK(anchors->Length() == 1uz);
    CHECK(named->Length() == 0uz);
  }

  TEST_CASE_METHOD(LiveCollectionFixture, "LiveCollection: goes on working after its root is adopted",
                   "[DOM][LiveCollection][dom:concept-node-adopt]")
  {
    Connect();
    auto first = Add(*Root, u8"p");
    REQUIRE_FALSE(first->SetAttribute(u8"class", u8"on").HasException());
    auto byTag = Root->GetElementsByTagName(u8"p");
    auto byClass = Root->GetElementsByClassName(u8"on");
    CHECK(byTag->Length() == 1uz);
    CHECK(byClass->Length() == 1uz);
    CHECK(byTag->NamedItem(u8"a").get() == nullptr);

    auto other = CreateRef<HTMLDocument>();
    REQUIRE_FALSE(other->AdoptNode(*Root).HasException());
    CHECK(byTag->Length() == 1uz);
    CHECK(byClass->Item(0).get() == first.get());

    // The new document knows the caches are there, and the old one that they aren't.
    CHECK(Document->HasLiveCollectionCaches() == false);
    CHECK(other->HasLiveCollectionCaches());
    auto second = other->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(Root->AppendChild(*second).HasException());
    CHECK(byTag->Length() == 2uz);
    REQUIRE_FALSE(second->SetAttribute(u8"class", u8"on").HasException());
    CHECK(byClass->Length() == 2uz);
    REQUIRE_FALSE(first->SetAttribute(u8"id", u8"a").HasException());
    CHECK(byTag->NamedItem(u8"a").get() == first.get());
  }

  TEST_CASE_METHOD(LiveCollectionFixture, "LiveCollection: the document counts only the caches that exist",
                   "[DOM][LiveCollection]")
  {
    Connect();
    auto first = Add(*Root, u8"p");
    CHECK_FALSE(Document->HasLiveCollectionCaches());

    {
      auto collection = Root->GetElementsByTagName(u8"p");
      CHECK_FALSE(Document->HasLiveCollectionCaches());
      CHECK(collection->Length() == 1uz);
      CHECK(Document->HasLiveCollectionCaches());
      CHECK_FALSE(Document->HasLiveCollectionCachesForAttribute(u8"id"));

      (void)collection->NamedItem(u8"a");
      CHECK(Document->HasLiveCollectionCachesForAttribute(u8"id"));
      CHECK(Document->HasLiveCollectionCachesForAttribute(u8"name"));
      CHECK_FALSE(Document->HasLiveCollectionCachesForAttribute(u8"class"));

      // A change forgets the cache until it's next asked for.
      (void)Add(*Root, u8"p");
      CHECK_FALSE(Document->HasLiveCollectionCaches());
      CHECK(collection->Item(1));
      CHECK(Document->HasLiveCollectionCaches());
    }

    // As does destroying the collection.
    CHECK_FALSE(Document->HasLiveCollectionCaches());
    CHECK(first->ParentNode() == Root.get());
  }

  TEST_CASE_METHOD(LiveCollectionFixture, "LiveCollection: selectedOptions follows the options' selectedness",
                   "[DOM][LiveCollection][html:dom-select-selectedoptions]")
  {
    auto select = Downcast<html::HTMLSelectElement>(Add(*Root, u8"select").get());
    REQUIRE_FALSE(select->SetAttribute(u8"multiple", u8"").HasException());
    auto firstRef = Add(*select, u8"option");
    auto &first = Downcast<html::HTMLOptionElement>(*firstRef);
    auto secondRef = Add(*select, u8"option");
    auto &second = Downcast<html::HTMLOptionElement>(*secondRef);
    auto selected = select->SelectedOptions();
    auto options = select->Options();
    CHECK(selected->Length() == 0uz);
    CHECK(options->Length() == 2uz);

    SECTION("setting selectedness")
    {
      second.Selected(true);
      CHECK(selected->Length() == 1uz);
      CHECK(selected->Item(0).get() == &second);
      first.Selected(true);
      CHECK(selected->Length() == 2uz);
      second.Selected(false);
      CHECK(selected->Length() == 1uz);
      CHECK(selected->Item(0).get() == &first);
    }

    SECTION("the selected attribute of an unchanged option")
    {
      REQUIRE_FALSE(first.SetAttribute(u8"selected", u8"").HasException());
      CHECK(selected->Length() == 1uz);
    }

    SECTION("an option leaving and joining, directly and with an optgroup")
    {
      second.Selected(true);
      CHECK(selected->Length() == 1uz);
      REQUIRE_FALSE(select->RemoveChild(second).HasException());
      CHECK(selected->Length() == 0uz);
      CHECK(options->Length() == 1uz);

      auto group = Create(u8"optgroup");
      REQUIRE_FALSE(group->AppendChild(second).HasException());
      REQUIRE_FALSE(select->AppendChild(*group).HasException());
      CHECK(selected->Length() == 1uz);
      CHECK(options->Length() == 2uz);

      REQUIRE_FALSE(select->RemoveChild(*group).HasException());
      CHECK(selected->Length() == 0uz);
      CHECK(options->Length() == 1uz);
    }

    SECTION("a single select selects by itself as its options change")
    {
      auto single = Downcast<html::HTMLSelectElement>(Add(*Root, u8"select").get());
      auto singleSelected = single->SelectedOptions();
      CHECK(singleSelected->Length() == 0uz);
      auto option = Add(*single, u8"option");
      CHECK(singleSelected->Length() == 1uz);
      CHECK(singleSelected->Item(0).get() == option.get());
      REQUIRE_FALSE(single->RemoveChild(*option).HasException());
      CHECK(singleSelected->Length() == 0uz);
    }
  }
}
