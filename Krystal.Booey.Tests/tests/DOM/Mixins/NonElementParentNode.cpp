#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom::tests
{
  TEST_CASE("getElementById(): the first descendant in tree order whose ID matches",
            "[DOM][NonElementParentNode][dom:interface-nonelementparentnode][dom:get-an-element-by-id]"
            "[dom:dom-nonelementparentnode-getelementbyid]")
  {
    auto document = CreateRef<HTMLDocument>();

    auto parent = document->CreateElement(u8"parent");
    REQUIRE_FALSE(parent.HasException());

    auto div = document->CreateElement(u8"div");
    REQUIRE_FALSE(div.HasException());

    div->Id(u8"test-id");

    auto span = document->CreateElement(u8"span");
    REQUIRE_FALSE(span.HasException());

    SECTION("With Document node")
    {
      REQUIRE_FALSE(document->AppendChild(*parent.Value()).HasException());

      REQUIRE(document->GetElementById(u8"test-id") == nullptr);

      REQUIRE(parent->AppendChild(*div.Value()).HasValue());
      REQUIRE(document->GetElementById(u8"test-id") == div.Value());

      REQUIRE(parent->InsertBefore(*span.Value(), div.Value().get()).HasValue());
      REQUIRE(document->GetElementById(u8"test-id") == div.Value());

      span->Id(u8"test-id");
      REQUIRE(document->GetElementById(u8"test-id") == span.Value());

      REQUIRE_FALSE(document->RemoveChild(*parent.Value()).HasException());
      REQUIRE_FALSE(parent->RemoveChild(*div.Value()).HasException());
      REQUIRE_FALSE(parent->RemoveChild(*span.Value()).HasException());
    }

    SECTION("With DocumentFragment node")
    {
      auto fragment = document->CreateDocumentFragment();

      REQUIRE_FALSE(fragment->AppendChild(*parent.Value()).HasException());

      REQUIRE(fragment->GetElementById(u8"test-id") == nullptr);

      REQUIRE(parent->AppendChild(*div.Value()).HasValue());
      REQUIRE(fragment->GetElementById(u8"test-id") == div.Value());

      REQUIRE(parent->InsertBefore(*span.Value(), div.Value().get()).HasValue());
      REQUIRE(fragment->GetElementById(u8"test-id") == div.Value());

      span->Id(u8"test-id");
      REQUIRE(fragment->GetElementById(u8"test-id") == span.Value());

      REQUIRE_FALSE(fragment->RemoveChild(*parent.Value()).HasException());
    }
  }

  TEST_CASE(
    "getElementById(): nothing for the empty string, as no element has it as its ID",
    "[DOM][NonElementParentNode][dom:get-an-element-by-id][dom:dom-nonelementparentnode-getelementbyid]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div");
    REQUIRE_FALSE(element.HasException());

    SECTION("An element without an id")
    {
      REQUIRE_FALSE(document->AppendChild(*element.Value()).HasException());
      CHECK(document->GetElementById(u8"") == nullptr);
    }

    SECTION("An element whose id is empty")
    {
      element->Id(u8"");
      REQUIRE_FALSE(document->AppendChild(*element.Value()).HasException());
      CHECK(document->GetElementById(u8"") == nullptr);
    }

    SECTION("In a document fragment")
    {
      auto fragment = document->CreateDocumentFragment();
      REQUIRE_FALSE(fragment->AppendChild(*element.Value()).HasException());
      CHECK(fragment->GetElementById(u8"") == nullptr);
    }
  }

  TEST_CASE("getElementById(): descendants only, not shadow trees, and only the id attribute in no namespace",
            "[DOM][NonElementParentNode][dom:get-an-element-by-id]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*host).HasException());

    SECTION("An element in a shadow tree")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
      auto inShadow = document->CreateElement(u8"span").ReleaseValue();
      inShadow->Id(u8"a");
      REQUIRE_FALSE(shadow->AppendChild(*inShadow).HasException());
      CHECK(document->GetElementById(u8"a") == nullptr);
    }

    SECTION("An id attribute in a namespace")
    {
      auto element = document->CreateElement(u8"span").ReleaseValue();
      REQUIRE_FALSE(element->SetAttributeNS(u8"http://example.com/", u8"id", u8"a").HasException());
      REQUIRE_FALSE(host->AppendChild(*element).HasException());
      CHECK(document->GetElementById(u8"a") == nullptr);
    }
  }
}
