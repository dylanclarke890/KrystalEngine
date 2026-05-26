#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include <catch_all.hpp>

namespace Krys::Tests
{
  using namespace Krys::HTML;

  namespace
  {
    struct CommonTestData
    {
      Ref<Document> Document;
      Ref<TestElement> Node;
      CommonTestData() : Document(CreateRef<HTMLDocument>()), Node(CreateRef<TestElement>(*Document))
      {
      }
    };
  }

  TEST_CASE("Attr::Name and Attr::Value", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttribute(u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->Name() == u8"test-attr");
    REQUIRE(attr->Value() == u8"");

    attr->Value(u8"test-value");
    REQUIRE(attr->Value() == u8"test-value");
  }

  TEST_CASE("Attr::OwnerElement", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttribute(u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->OwnerElement() == nullptr);
    REQUIRE_FALSE(data.Node->SetAttributeNode(*attr.Value()).HasException());
    REQUIRE(attr->OwnerElement() == data.Node.get());
  }

  TEST_CASE("Attr::Specified", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttribute(u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->Specified());
  }

  TEST_CASE("Attr::NamespaceURI and Attr::Prefix", "[HTML][Attr]")
  {
    CommonTestData data;

    SECTION("empty prefix")
    {
      auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
      REQUIRE_FALSE(attr.HasException());

      REQUIRE(attr->NamespaceURI() == u8"http://example.com/ns");
      REQUIRE(attr->Prefix() == DOMStringAtom::Null());
    }

    SECTION("non-empty prefix")
    {
      auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"ns:test-attr");
      REQUIRE_FALSE(attr.HasException());

      REQUIRE(attr->NamespaceURI() == u8"http://example.com/ns");
      REQUIRE(attr->Prefix() == u8"ns");
    }
  }

  TEST_CASE("Attr::LocalName", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->LocalName() == u8"test-attr");
  }

  TEST_CASE("Attr::NodeName", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->NodeName() == u8"test-attr");
  }

  TEST_CASE("Attr::NodeValue", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttribute(u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->NodeValue() == u8"");
    attr->NodeValue(u8"test-value");
    REQUIRE(attr->NodeValue() == u8"test-value");
  }

  TEST_CASE("Attr::TextContent", "[HTML][Attr]")
  {
    CommonTestData data;

    auto attr = data.Document->CreateAttribute(u8"test-attr");
    REQUIRE_FALSE(attr.HasException());

    REQUIRE(attr->TextContent() == u8"");
    attr->TextContent(u8"test-value");
    REQUIRE(attr->TextContent() == u8"test-value");
  }
}
