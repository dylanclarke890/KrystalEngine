#include "Krystal.HTML/DOM/HTMLCollection.hpp"
#include "Krystal.HTML.Tests/DOM/TestElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/HTMLDocument.hpp"
#include "Krystal.HTML/DOM/Internals/HTMLCollection/LiveHTMLCollection.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/DOM/Text.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

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
      Ref<HTML::Text> TextChild1;
      Ref<HTML::Text> TextChild2;
      Ref<TestElement> ElementChild1;
      Ref<TestElement> ElementChild2;
      Ref<HTMLCollection> Collection;

      CommonTestData(LiveHTMLCollectionFilterFunc &&filter = DefaultFilter) noexcept
          : Document(CreateRef<HTMLDocument>()), Element(CreateRef<TestElement>(*Document)),
            TextChild1(CreateRef<HTML::Text>(*Document, u8"")),
            TextChild2(CreateRef<HTML::Text>(*Document, u8"")),
            ElementChild1(CreateRef<TestElement>(*Document)),
            ElementChild2(CreateRef<TestElement>(*Document)),
            Collection(CreateRef<LiveHTMLCollection>(*Element, Krys::Move(filter)))
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

  TEST_CASE("HTMLCollection::Length", "[HTML][Node][HTMLCollection]")
  {
    CommonTestData data;

    REQUIRE(data.Collection->Length() == 2uz);
    REQUIRE_FALSE(data.Element->RemoveChild(*data.ElementChild1).HasException());
    REQUIRE(data.Collection->Length() == 1uz);
  }

  TEST_CASE("HTMLCollection::Item", "[HTML][Node][HTMLCollection]")
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

  TEST_CASE("HTMLCollection::operator[](size_t index)", "[HTML][Node][HTMLCollection]")
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

  TEST_CASE("HTMLCollection::NamedItem", "[HTML][Node][HTMLCollection]")
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

  TEST_CASE("HTMLCollection::operator[](DOMStringView name)", "[HTML][Node][HTMLCollection]")
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
}