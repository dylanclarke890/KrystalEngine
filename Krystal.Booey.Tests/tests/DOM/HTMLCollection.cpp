#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Internals/HTMLCollection/StaticHTMLCollection.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    auto DefaultFilter = [](const Element &)
    {
      return true;
    };

    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Element;
      Ref<dom::Text> TextChild1;
      Ref<dom::Text> TextChild2;
      Ref<TestElement> ElementChild1;
      Ref<TestElement> ElementChild2;
      Ref<HTMLCollection> Collection;

      CommonTestData(LiveHTMLCollectionFilterFunc &&filter = DefaultFilter) noexcept
          : Document(CreateRef<HTMLDocument>()), Element(CreateRef<TestElement>(*Document)),
            TextChild1(CreateRef<dom::Text>(*Document, u8"")),
            TextChild2(CreateRef<dom::Text>(*Document, u8"")),
            ElementChild1(CreateRef<TestElement>(*Document)),
            ElementChild2(CreateRef<TestElement>(*Document)),
            Collection(CreateRef<LiveHTMLCollection>(*Element, krys::move(filter)))
      {
        REQUIRE_FALSE(Document->AppendChild(*Element).HasException());
        REQUIRE_FALSE(Element->AppendChild(*TextChild1).HasException());
        REQUIRE_FALSE(Element->AppendChild(*TextChild2).HasException());
        REQUIRE_FALSE(Element->AppendChild(*ElementChild1).HasException());
        REQUIRE_FALSE(Element->AppendChild(*ElementChild2).HasException());

        REQUIRE_FALSE(ElementChild1->SetAttribute(u8"name", u8"element-name-1").HasException());
        REQUIRE_FALSE(ElementChild1->SetAttribute(u8"id", u8"element-id-1").HasException());

        REQUIRE_FALSE(ElementChild2->SetAttribute(u8"name", u8"element-name-2").HasException());
        REQUIRE_FALSE(ElementChild2->SetAttribute(u8"id", u8"element-id-2").HasException());
      }

      ~CommonTestData() noexcept
      {
        if (Element->ParentNode())
        {
          REQUIRE_FALSE(Element->ParentNode()->RemoveChild(*Element).HasException());
        }
        if (TextChild1->ParentNode())
        {
          REQUIRE_FALSE(TextChild1->ParentNode()->RemoveChild(*TextChild1).HasException());
        }

        if (TextChild2->ParentNode())
        {
          REQUIRE_FALSE(TextChild2->ParentNode()->RemoveChild(*TextChild2).HasException());
        }

        if (ElementChild1->ParentNode())
        {
          REQUIRE_FALSE(ElementChild1->ParentNode()->RemoveChild(*ElementChild1).HasException());
        }

        if (ElementChild2->ParentNode())
        {
          REQUIRE_FALSE(ElementChild2->ParentNode()->RemoveChild(*ElementChild2).HasException());
        }
      }
    };
  }

  TEST_CASE("HTMLCollection.length: the number of elements",
            "[DOM][HTMLCollection][dom:interface-htmlcollection][dom:dom-htmlcollection-length]")
  {
    CommonTestData data;

    REQUIRE(data.Collection->Length() == 2uz);
    REQUIRE_FALSE(data.Element->RemoveChild(*data.ElementChild1).HasException());
    REQUIRE(data.Collection->Length() == 1uz);
  }

  TEST_CASE("HTMLCollection.item(): the indexth element, or null",
            "[DOM][HTMLCollection][dom:dom-htmlcollection-item]")
  {
    CommonTestData data;

    auto result = data.Collection->Item(0uz);
    REQUIRE(result != nullptr);
    REQUIRE(result == data.ElementChild1);

    result = data.Collection->Item(1uz);
    REQUIRE(result != nullptr);
    REQUIRE(result == data.ElementChild2);

    result = data.Collection->Item(2uz);
    REQUIRE(result == nullptr);
  }

  TEST_CASE("HTMLCollection[index]: as item()", "[DOM][HTMLCollection][dom:dom-htmlcollection-item]")
  {
    CommonTestData data;
    auto &collection = *data.Collection;

    auto result = collection[0uz];
    REQUIRE(result != nullptr);
    REQUIRE(result == data.ElementChild1);

    result = collection[1uz];
    REQUIRE(result != nullptr);
    REQUIRE(result == data.ElementChild2);

    result = collection[2uz];
    REQUIRE(result == nullptr);
  }

  TEST_CASE("HTMLCollection.namedItem(): the first element with the key as its ID or name",
            "[DOM][HTMLCollection][dom:dom-htmlcollection-nameditem-key]")
  {
    CommonTestData data;

    SECTION("Search by name")
    {
      auto result = data.Collection->NamedItem(u8"element-name-1");
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild1);

      result = data.Collection->NamedItem(u8"element-name-2");
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild2);

      result = data.Collection->NamedItem(u8"non-existent-name");
      REQUIRE(result == nullptr);
    }

    SECTION("Search by id")
    {
      auto result = data.Collection->NamedItem(u8"element-id-1");
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild1);

      result = data.Collection->NamedItem(u8"element-id-2");
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild2);

      result = data.Collection->NamedItem(u8"non-existent-id");
      REQUIRE(result == nullptr);
    }
  }

  TEST_CASE("HTMLCollection[name]: as namedItem()",
            "[DOM][HTMLCollection][dom:dom-htmlcollection-nameditem-key]")
  {
    CommonTestData data;
    auto &collection = *data.Collection;

    SECTION("Search by name")
    {
      auto result = collection[u8"element-name-1"];
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild1);

      result = collection[u8"element-name-2"];
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild2);

      result = collection[u8"non-existent-name"];
      REQUIRE(result == nullptr);
    }

    SECTION("Search by id")
    {
      auto result = collection[u8"element-id-1"];
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild1);

      result = collection[u8"element-id-2"];
      REQUIRE(result != nullptr);
      REQUIRE(result == data.ElementChild2);

      result = collection[u8"non-existent-id"];
      REQUIRE(result == nullptr);
    }
  }

  TEST_CASE("HTMLCollection.namedItem(): null for the empty string, and name only in the HTML namespace",
            "[DOM][HTMLCollection][dom:dom-htmlcollection-nameditem-key]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    auto unnamed = document->CreateElement(u8"span").ReleaseValue();
    auto svg = document->CreateElementNS(infra::Namespaces::SVG, u8"svg").ReleaseValue();
    REQUIRE_FALSE(svg->SetAttribute(u8"name", u8"n").HasException());
    REQUIRE_FALSE(svg->SetAttribute(u8"id", u8"i").HasException());
    REQUIRE_FALSE(root->AppendChild(*unnamed).HasException());
    REQUIRE_FALSE(root->AppendChild(*svg).HasException());

    auto live = CreateRef<LiveHTMLCollection>(*root, DefaultFilter);
    auto staticCollection = CreateRef<StaticHTMLCollection>(SmallElementList {unnamed, svg});
    for (HTMLCollection *collection :
         {static_cast<HTMLCollection *>(live.get()), static_cast<HTMLCollection *>(staticCollection.get())})
    {
      CHECK(collection->NamedItem(u8"") == nullptr);
      CHECK(collection->NamedItem(u8"n") == nullptr);
      CHECK(collection->NamedItem(u8"i") == svg);
    }
  }

  TEST_CASE(
    "HTMLCollection: the supported property names are the IDs and HTML names, once each, in tree order",
    "[DOM][HTMLCollection][dom:dom-htmlcollection-item]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    auto newElement = [&](DOMStringView id, DOMStringView name)
    {
      auto element = document->CreateElement(u8"span").ReleaseValue();
      if (!id.empty())
      {
        REQUIRE_FALSE(element->SetAttribute(u8"id", DOMString(id)).HasException());
      }
      REQUIRE_FALSE(element->SetAttribute(u8"name", DOMString(name)).HasException());
      REQUIRE_FALSE(root->AppendChild(*element).HasException());
      return element;
    };
    auto first = newElement(u8"a", u8"b");
    auto second = newElement(u8"b", u8"c");
    auto third = newElement(u8"", u8"");

    auto live = CreateRef<LiveHTMLCollection>(*root, DefaultFilter);
    auto staticCollection = CreateRef<StaticHTMLCollection>(SmallElementList {first, second, third});
    CHECK(live->SupportedPropertyNames() == List<DOMString> {u8"a", u8"b", u8"c"});
    CHECK(staticCollection->SupportedPropertyNames() == List<DOMString> {u8"a", u8"b", u8"c"});
  }
}
