#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"

namespace krys::boo::dom::tests
{
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

  TEST_CASE("Create an attribute: the document, qualified name and value it's given, with no element",
            "[DOM][Attr][dom:create-an-attribute][dom:interface-attr][dom:concept-attribute]"
            "[dom:concept-attribute-element]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttributeNS(u8"urn:x", u8"x:a").ReleaseValue();
    CHECK(&attr->NodeDocument() == data.Document.get());
    CHECK(attr->Value() == u8"");
    CHECK(attr->OwnerElement() == nullptr);
    CHECK_FALSE(attr->HasChildNodes());

    // One created by setAttribute() is given its value.
    REQUIRE_FALSE(data.Node->SetAttribute(u8"b", u8"value").HasException());
    CHECK(data.Node->GetAttributeNode(u8"b")->Value() == u8"value");
  }

  TEST_CASE("Attr: namespaceURI, prefix, localName and name are its qualified name and its parts",
            "[DOM][Attr][dom:dom-attr-namespaceuri][dom:dom-attr-prefix][dom:dom-attr-localname]"
            "[dom:dom-attr-name][dom:concept-attribute-namespace][dom:concept-attribute-namespace-prefix]"
            "[dom:concept-attribute-local-name][dom:concept-attribute-qualified-name]"
            "[dom:concept-named-attribute]")
  {
    CommonTestData data;

    SECTION("With a prefix, the name is the prefix, a colon and the local name")
    {
      auto attr =
        data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"ns:test-attr").ReleaseValue();
      CHECK(attr->NamespaceURI() == u8"http://example.com/ns");
      CHECK(attr->Prefix() == u8"ns");
      CHECK(attr->LocalName() == u8"test-attr");
      CHECK(attr->Name() == u8"ns:test-attr");
    }

    SECTION("Without one, the name is the local name")
    {
      auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr").ReleaseValue();
      CHECK(attr->Prefix() == DOMStringAtom::Null());
      CHECK(attr->Name() == u8"test-attr");
    }

    SECTION("A named attribute has no namespace or prefix")
    {
      auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();
      CHECK(attr->NamespaceURI() == DOMStringAtom::Null());
      CHECK(attr->Prefix() == DOMStringAtom::Null());
      CHECK(attr->LocalName() == u8"test-attr");
    }
  }

  TEST_CASE(
    "Attr.value: its value, which is set as an existing attribute value",
    "[DOM][Attr][dom:dom-attr-value][dom:concept-attribute-value][dom:set-an-existing-attribute-value]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();

    SECTION("Without an element, it's just set")
    {
      attr->Value(u8"test-value");
      CHECK(attr->Value() == u8"test-value");
    }

    SECTION("With an element, the attribute is changed")
    {
      REQUIRE_FALSE(data.Node->SetAttributeNode(*attr).HasException());
      auto observer =
        CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
      REQUIRE_FALSE(
        observer->Observe(*data.Node, MutationObserverInit {.Attributes = true, .AttributeOldValue = true})
          .HasException());

      attr->Value(u8"test-value");
      CHECK(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1uz);
      CHECK(records[0]->AttributeName() == u8"test-attr");
      CHECK(records[0]->OldValue() == u8"");
    }
  }

  TEST_CASE("Attr.ownerElement: its element", "[DOM][Attr][dom:dom-attr-ownerelement]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();
    CHECK(attr->OwnerElement() == nullptr);

    REQUIRE_FALSE(data.Node->SetAttributeNode(*attr).HasException());
    CHECK(attr->OwnerElement() == data.Node.get());

    REQUIRE_FALSE(data.Node->RemoveAttributeNode(*attr).HasException());
    CHECK(attr->OwnerElement() == nullptr);
  }

  TEST_CASE("Attr.specified: always true", "[DOM][Attr][dom:dom-attr-specified]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();
    CHECK(attr->Specified());
    REQUIRE_FALSE(data.Node->SetAttributeNode(*attr).HasException());
    CHECK(attr->Specified());
  }

  TEST_CASE("Node.nodeName: an Attr's is its qualified name", "[DOM][Attr][dom:dom-node-nodename]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"ns:test-attr").ReleaseValue();
    CHECK(attr->NodeName() == u8"ns:test-attr");
  }

  TEST_CASE("Node.nodeValue: an Attr's is its value, which setting it sets",
            "[DOM][Attr][dom:dom-node-nodevalue]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();
    CHECK(attr->NodeValue() == u8"");
    REQUIRE_FALSE(data.Node->SetAttributeNode(*attr).HasException());
    REQUIRE_FALSE(attr->NodeValue(u8"test-value").HasException());
    CHECK(attr->NodeValue() == u8"test-value");
    CHECK(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
  }

  TEST_CASE("Node.textContent: an Attr's is its value, which setting it sets",
            "[DOM][Attr][dom:dom-node-textcontent][dom:get-text-content][dom:set-text-content]")
  {
    CommonTestData data;
    auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();
    CHECK(attr->TextContent() == u8"");
    REQUIRE_FALSE(data.Node->SetAttributeNode(*attr).HasException());
    REQUIRE_FALSE(attr->TextContent(u8"test-value").HasException());
    CHECK(attr->TextContent() == u8"test-value");
    CHECK(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
  }
}
