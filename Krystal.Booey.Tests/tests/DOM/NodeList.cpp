#include "Krystal.Booey/DOM/NodeList.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Text.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("Collections: a live one follows the tree in tree order, a static one keeps its snapshot",
            "[DOM][NodeList][dom:old-style-collections][dom:concept-collection][dom:concept-collection-live]"
            "[dom:concept-collection-static][dom:represented-by-the-collection]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    auto outer = document->CreateElement(u8"p").ReleaseValue();
    auto inner = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(outer->AppendChild(*inner).HasException());
    REQUIRE_FALSE(root->AppendChild(*outer).HasException());

    auto childNodes = root->ChildNodes();
    auto paragraphs = root->GetElementsByTagName(u8"p");
    auto selected = root->QuerySelectorAll(u8"p").ReleaseValue();

    // The view is of the root's subtree, filtered, in tree order.
    REQUIRE(childNodes->Length() == 1u);
    REQUIRE(paragraphs->Length() == 2u);
    CHECK(paragraphs->Item(0) == outer);
    CHECK(paragraphs->Item(1) == inner);

    auto text = document->CreateTextNode(u8"text");
    auto last = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(root->InsertBefore(*text, outer.get()).HasException());
    REQUIRE_FALSE(root->AppendChild(*last).HasException());

    // Live collections see the change.
    CHECK(childNodes->Length() == 3u);
    CHECK(childNodes->Item(0) == text);
    CHECK(paragraphs->Length() == 3u);
    CHECK(paragraphs->Item(2) == last);

    // A static one doesn't.
    CHECK(selected->Length() == 2u);
  }

  TEST_CASE("NodeList: length is the number of nodes, item() the indexth or null, and it iterates them",
            "[DOM][NodeList][dom:interface-nodelist][dom:dom-nodelist-length][dom:dom-nodelist-item]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    auto first = document->CreateElement(u8"p").ReleaseValue();
    auto second = document->CreateTextNode(u8"text");
    REQUIRE_FALSE(root->AppendChild(*first).HasException());
    REQUIRE_FALSE(root->AppendChild(*second).HasException());

    auto check = [&](NodeList &list, std::initializer_list<const Node *> nodes)
    {
      REQUIRE(list.Length() == nodes.size());
      size_t index = 0;
      for (auto *node : nodes)
      {
        CHECK(list.Item(index).get() == node);
        CHECK(list[index].get() == node);
        ++index;
      }
      CHECK(list.Item(nodes.size()) == nullptr);
      CHECK(list[nodes.size() + 1] == nullptr);

      List<const Node *> iterated;
      for (auto node : list)
      {
        iterated.push_back(node.get());
      }
      CHECK(iterated == List<const Node *>(nodes));
    };

    SECTION("Live")
    {
      check(*root->ChildNodes(), {first.get(), second.get()});
    }

    SECTION("Static")
    {
      check(*root->QuerySelectorAll(u8"p").ReleaseValue(), {first.get()});
    }

    SECTION("Empty")
    {
      check(*document->CreateElement(u8"span").ReleaseValue()->ChildNodes(), {});
    }
  }
}
