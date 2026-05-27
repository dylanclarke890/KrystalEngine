#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML.Tests/TestElement.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/DOM/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
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

  TEST_CASE("Element::NodeName", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeName() == u8"TEST-ELEMENT");
  }

  TEST_CASE("Element::NodeType", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeType() == NodeType::ELEMENT_NODE);
  }

  TEST_CASE("Element::TagName", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->TagName() == u8"TEST-ELEMENT");
  }

  TEST_CASE("Element::Id", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->Id() == u8"");
    data.Node->Id(u8"test-id");
    REQUIRE(data.Node->Id() == u8"test-id");
  }

  TEST_CASE("Element::ClassName", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->ClassName() == u8"");
    data.Node->ClassName(u8"test-class");
    REQUIRE(data.Node->ClassName() == u8"test-class");
  }

  TEST_CASE("Element::ClassList", "[HTML][Element]")
  {
    CommonTestData data;
    auto &classList = data.Node->ClassList();
    REQUIRE(classList.Length() == 0uz);

    REQUIRE_FALSE(classList.Add(u8"foo").HasException());
    REQUIRE(classList.Length() == 1uz);
    REQUIRE(classList.Contains(u8"foo"));

    REQUIRE_FALSE(classList.Add(u8"bar").HasException());
    REQUIRE(classList.Length() == 2uz);
    REQUIRE(classList.Contains(u8"bar"));

    REQUIRE_FALSE(classList.Remove(u8"foo").HasException());
    REQUIRE(classList.Length() == 1uz);
    REQUIRE_FALSE(classList.Contains(u8"foo"));

    REQUIRE_FALSE(classList.Remove(u8"bar").HasException());
    REQUIRE(classList.Length() == 0uz);
    REQUIRE_FALSE(classList.Contains(u8"bar"));
  }

  // TODO(test): SLOTTABLES
  // TEST_CASE("Element::Slot", "[HTML][Element]")

  TEST_CASE("Element::HasAttributes", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->HasAttributes());
    REQUIRE_FALSE(data.Node->GetAttribute(u8"test-attr").has_value());
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->HasAttributes());
    REQUIRE(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
  }

  TEST_CASE("Element::Attributes", "[HTML][Element]")
  {
    CommonTestData data;

    auto &attributes = data.Node->Attributes();
    REQUIRE(attributes.Length() == 0uz);

    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE(attributes.Length() == 1uz);
    REQUIRE(attributes.Item(0)->Name() == u8"test-attr");
    REQUIRE(attributes.Item(0)->Value() == u8"test-value");
  }

  TEST_CASE("Element::GetAttributeNames", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE(data.Node->GetAttributeNames().empty());
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr1", u8"value1").HasException());
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr2", u8"value2").HasException());

    auto attributeNames = data.Node->GetAttributeNames();
    REQUIRE(attributeNames.size() == 2uz);
    REQUIRE(attributeNames[0] == u8"test-attr1");
    REQUIRE(attributeNames[1] == u8"test-attr2");
  }

  TEST_CASE("Element::GetAttribute", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->GetAttribute(u8"test-attr").has_value());
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
  }

  TEST_CASE("Element::GetAttributeNS", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr").has_value());
    REQUIRE_FALSE(
      data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr") == u8"test-value");
  }

  TEST_CASE("Element::SetAttribute", "[HTML][Element]")
  {
    CommonTestData data;

    SECTION("new attribute")
    {
      REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
      REQUIRE(data.Node->HasAttribute(u8"test-attr"));
      REQUIRE(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
    }

    SECTION("replacing existing attribute")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value1").HasException());
      REQUIRE(data.Node->GetAttribute(u8"test-attr") == u8"value1");
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value2").HasException());
      REQUIRE(data.Node->GetAttribute(u8"test-attr") == u8"value2");
    }

    SECTION("invalid attribute name")
    {
      auto result = data.Node->SetAttribute(u8"invalid name", u8"value");
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
  }

  TEST_CASE("Element::SetAttributeNS", "[HTML][Element]")
  {
    CommonTestData data;

    SECTION("new attribute")
    {
      REQUIRE_FALSE(data.Node->HasAttributeNS(u8"http://example.com/ns", u8"test-attr"));
      REQUIRE_FALSE(
        data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"test-value").HasException());
      REQUIRE(data.Node->HasAttributeNS(u8"http://example.com/ns", u8"test-attr"));
      REQUIRE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr") == u8"test-value");
    }

    SECTION("replacing existing attribute")
    {
      REQUIRE_FALSE(
        data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"value1").HasException());
      REQUIRE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr") == u8"value1");
      REQUIRE_FALSE(
        data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"value2").HasException());
      REQUIRE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr") == u8"value2");
    }

    SECTION("invalid attribute name")
    {
      auto result = data.Node->SetAttributeNS(u8"http://example.com/ns", u8"invalid name", u8"value");
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
  }

  TEST_CASE("Element::RemoveAttribute", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->GetAttribute(u8"test-attr") == u8"test-value");
    data.Node->RemoveAttribute(u8"test-attr");
    REQUIRE_FALSE(data.Node->GetAttribute(u8"test-attr").has_value());
  }

  TEST_CASE("Element::RemoveAttributeNS", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(
      data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr") == u8"test-value");
    data.Node->RemoveAttributeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE_FALSE(data.Node->GetAttributeNS(u8"http://example.com/ns", u8"test-attr").has_value());
  }

  TEST_CASE("Element::ToggleAttribute", "[HTML][Element]")
  {
    CommonTestData data;

    SECTION("toggling on")
    {
      REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
      auto result = data.Node->ToggleAttribute(u8"test-attr", true);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value());
      REQUIRE(data.Node->HasAttribute(u8"test-attr"));
    }

    SECTION("toggling off")
    {
      REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value").HasException());
      REQUIRE(data.Node->HasAttribute(u8"test-attr"));
      auto result = data.Node->ToggleAttribute(u8"test-attr", false);
      REQUIRE_FALSE(result.HasException());
      REQUIRE_FALSE(result.Value());
      REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
    }

    SECTION("toggling without force")
    {
      REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
      auto result = data.Node->ToggleAttribute(u8"test-attr", Null);
      REQUIRE_FALSE(result.HasException());
      REQUIRE(result.Value());
      REQUIRE(data.Node->HasAttribute(u8"test-attr"));
      result = data.Node->ToggleAttribute(u8"test-attr", Null);
      REQUIRE_FALSE(result.HasException());
      REQUIRE_FALSE(result.Value());
      REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
    }

    SECTION("invalid attribute name")
    {
      auto result = data.Node->ToggleAttribute(u8"invalid name", true);
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
  }

  TEST_CASE("Element::HasAttribute", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->HasAttribute(u8"test-attr"));
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->HasAttribute(u8"test-attr"));
  }

  TEST_CASE("Element::HasAttributeNS", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->HasAttributeNS(u8"http://example.com/ns", u8"test-attr"));
    REQUIRE_FALSE(
      data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"test-value").HasException());
    REQUIRE(data.Node->HasAttributeNS(u8"http://example.com/ns", u8"test-attr"));
  }

  TEST_CASE("Element::GetAttributeNode", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->GetAttributeNode(u8"test-attr"));
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    auto attrNode = data.Node->GetAttributeNode(u8"test-attr");
    REQUIRE(attrNode);
    REQUIRE(attrNode->Name() == u8"test-attr");
    REQUIRE(attrNode->Value() == u8"test-value");
  }

  TEST_CASE("Element::GetAttributeNodeNS", "[HTML][Element]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->GetAttributeNodeNS(u8"http://example.com/ns", u8"test-attr"));
    REQUIRE_FALSE(
      data.Node->SetAttributeNS(u8"http://example.com/ns", u8"test-attr", u8"test-value").HasException());

    auto attrNode = data.Node->GetAttributeNodeNS(u8"http://example.com/ns", u8"test-attr");
    REQUIRE(attrNode);
    REQUIRE(attrNode->Name() == u8"test-attr");
    REQUIRE(attrNode->Value() == u8"test-value");
  }

  TEST_CASE("Element::SetAttributeNode", "[HTML][Element]")
  {
    CommonTestData data;

    SECTION("new attribute")
    {
      auto attr = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr.HasException());
      attr->Value(u8"test-value");
      REQUIRE_FALSE(data.Node->SetAttributeNode(*attr.Value()).HasException());

      auto attrNode = data.Node->GetAttributeNode(u8"test-attr");
      REQUIRE(attrNode);
      REQUIRE(attrNode->Name() == u8"test-attr");
      REQUIRE(attrNode->Value() == u8"test-value");
    }

    SECTION("replacing existing attribute")
    {
      auto attr1 = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr1.HasException());
      attr1.Value()->Value(u8"value1");
      REQUIRE_FALSE(data.Node->SetAttributeNode(*attr1.Value()).HasException());

      auto attr2 = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr2.HasException());
      attr2.Value()->Value(u8"value2");

      auto result = data.Node->SetAttributeNode(*attr2.Value());
      REQUIRE_FALSE(result.HasException());

      auto &replacedAttr = *result;
      REQUIRE(replacedAttr);
      REQUIRE(replacedAttr->Name() == u8"test-attr");
      REQUIRE(replacedAttr->Value() == u8"value1");

      auto attrNode = data.Node->GetAttributeNode(u8"test-attr");
      REQUIRE(attrNode);
      REQUIRE(attrNode->Name() == u8"test-attr");
      REQUIRE(attrNode->Value() == u8"value2");
    }

    SECTION("InUseAttributeError if attr has different owner element")
    {
      auto attr1 = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr1.HasException());
      attr1.Value()->Value(u8"value1");
      REQUIRE_FALSE(data.Node->SetAttributeNode(*attr1.Value()).HasException());

      auto anotherElement = data.Document->CreateElement(u8"another-element");
      REQUIRE_FALSE(anotherElement.HasException());
      auto result = anotherElement->SetAttributeNode(*attr1.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InUseAttributeError);
    }
  }

  TEST_CASE("Element::SetAttributeNodeNS", "[HTML][Element]")
  {
    CommonTestData data;

    SECTION("new attribute")
    {
      auto attr = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
      REQUIRE_FALSE(attr.HasException());
      attr->Value(u8"test-value");
      REQUIRE_FALSE(data.Node->SetAttributeNodeNS(*attr.Value()).HasException());

      auto attrNode = data.Node->GetAttributeNodeNS(u8"http://example.com/ns", u8"test-attr");
      REQUIRE(attrNode);
      REQUIRE(attrNode->Name() == u8"test-attr");
      REQUIRE(attrNode->Value() == u8"test-value");
    }

    SECTION("replacing existing attribute")
    {
      auto attr1 = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
      REQUIRE_FALSE(attr1.HasException());
      attr1.Value()->Value(u8"value1");
      REQUIRE_FALSE(data.Node->SetAttributeNodeNS(*attr1.Value()).HasException());

      auto attr2 = data.Document->CreateAttributeNS(u8"http://example.com/ns", u8"test-attr");
      REQUIRE_FALSE(attr2.HasException());
      attr2.Value()->Value(u8"value2");

      auto result = data.Node->SetAttributeNodeNS(*attr2.Value());
      REQUIRE_FALSE(result.HasException());

      auto &replacedAttr = result.Value();
      REQUIRE(replacedAttr);
      REQUIRE(replacedAttr->Name() == u8"test-attr");
      REQUIRE(replacedAttr->Value() == u8"value1");

      auto attrNode = data.Node->GetAttributeNodeNS(u8"http://example.com/ns", u8"test-attr");
      REQUIRE(attrNode);
      REQUIRE(attrNode->Name() == u8"test-attr");
      REQUIRE(attrNode->Value() == u8"value2");
    }

    SECTION("InUseAttributeError if attr has different owner element")
    {
      auto attr1 = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr1.HasException());
      attr1.Value()->Value(u8"value1");
      REQUIRE_FALSE(data.Node->SetAttributeNode(*attr1.Value()).HasException());

      auto anotherElement = data.Document->CreateElement(u8"another-element");
      REQUIRE_FALSE(anotherElement.HasException());
      auto result = anotherElement->SetAttributeNodeNS(*attr1.Value());
      REQUIRE(result.HasException());
      REQUIRE(result.GetException().Code() == ExceptionCode::InUseAttributeError);
    }
  }

  TEST_CASE("Element::RemoveAttributeNode", "[HTML][Element]")
  {
    CommonTestData data;

    SECTION("Existing attribute node")
    {
      auto attr = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr.HasException());
      REQUIRE_FALSE(data.Node->SetAttributeNode(*attr.Value()).HasException());

      auto attrNode = data.Node->GetAttributeNode(u8"test-attr");
      REQUIRE(attrNode);
      REQUIRE_FALSE(data.Node->RemoveAttributeNode(*attrNode).HasException());
      REQUIRE_FALSE(data.Node->GetAttribute(u8"test-attr").has_value());
    }

    SECTION("Non-existing attribute node")
    {
      auto attr = data.Document->CreateAttribute(u8"test-attr");
      REQUIRE_FALSE(attr.HasException());

      auto removeResult = data.Node->RemoveAttributeNode(*attr.Value());
      REQUIRE(removeResult.HasException());
      REQUIRE(removeResult.GetException().Code() == ExceptionCode::NotFoundError);
    }
  }

  // TODO(test): SHADOW-ROOT
  // TEST_CASE("Element::AttachShadow", "[HTML][Element]")
  // TEST_CASE("Element::ShadowRoot", "[HTML][Element]")

  // TODO(test): CUSTOM-ELEMENTS
  // TEST_CASE("Element::CustomElementRegistry", "[HTML][Element]")

  // TODO(test): CSS-SELECTORS:
  // TEST_CASE("Element::Matches", "[HTML][Element]")
  // TEST_CASE("Element::Closest", "[HTML][Element]")
  // TEST_CASE("Element::WebkitMatchesSelector", "[HTML][Element]")

  TEST_CASE("Element::GetElementsByTagName", "[HTML][Element]")
  {
    CommonTestData data;

    auto element1 = data.Document->CreateElement(u8"test-element");
    REQUIRE_FALSE(element1.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element1.Value()).HasException());

    auto element2 = data.Document->CreateElement(u8"test-element");
    REQUIRE_FALSE(element2.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element2.Value()).HasException());

    auto element3 = data.Document->CreateElement(u8"another-element");
    REQUIRE_FALSE(element3.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element3.Value()).HasException());

    auto elements = data.Node->GetElementsByTagName(u8"test-element");
    REQUIRE(elements->Length() == 2uz);

    REQUIRE(elements->Item(0)->TagName() == u8"TEST-ELEMENT");
    REQUIRE(elements->Item(0) == element1.Value());

    REQUIRE(elements->Item(1)->TagName() == u8"TEST-ELEMENT");
    REQUIRE(elements->Item(1) == element2.Value());

    REQUIRE_FALSE(data.Node->RemoveChild(*element1.Value()).HasException());

    REQUIRE(elements->Length() == 1uz);
    REQUIRE(elements->Item(0)->TagName() == u8"TEST-ELEMENT");
    REQUIRE(elements->Item(0) == element2.Value());

    REQUIRE_FALSE(data.Node->RemoveChild(*element2.Value()).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*element3.Value()).HasException());
  }

  TEST_CASE("Element::GetElementsByTagNameNS", "[HTML][Element]")
  {
    CommonTestData data;
    auto element1 = data.Document->CreateElementNS(u8"http://example.com/ns", u8"test-element");
    REQUIRE_FALSE(element1.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element1.Value()).HasException());

    auto element2 = data.Document->CreateElementNS(u8"http://example.com/ns", u8"test-element");
    REQUIRE_FALSE(element2.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element2.Value()).HasException());

    auto element3 = data.Document->CreateElementNS(u8"http://example.com/ns", u8"another-element");
    REQUIRE_FALSE(element3.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element3.Value()).HasException());

    auto elements = data.Node->GetElementsByTagNameNS(u8"http://example.com/ns", u8"test-element");
    REQUIRE(elements->Length() == 2uz);
    REQUIRE(elements->Item(0)->NamespaceURI() == u8"http://example.com/ns");
    REQUIRE(elements->Item(0)->LocalName() == u8"test-element");
    REQUIRE(elements->Item(0) == element1.Value());
    REQUIRE(elements->Item(1)->NamespaceURI() == u8"http://example.com/ns");
    REQUIRE(elements->Item(1)->LocalName() == u8"test-element");
    REQUIRE(elements->Item(1) == element2.Value());

    REQUIRE_FALSE(data.Node->RemoveChild(*element1.Value()).HasException());
    REQUIRE(elements->Length() == 1uz);
    REQUIRE(elements->Item(0)->NamespaceURI() == u8"http://example.com/ns");
    REQUIRE(elements->Item(0)->LocalName() == u8"test-element");
    REQUIRE(elements->Item(0) == element2.Value());

    REQUIRE_FALSE(data.Node->RemoveChild(*element2.Value()).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*element3.Value()).HasException());
  }

  TEST_CASE("Element::GetElementsByClassName", "[HTML][Element]")
  {
    CommonTestData data;

    auto element1 = data.Document->CreateElement(u8"test-element");
    REQUIRE_FALSE(element1.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element1.Value()).HasException());
    element1.Value()->ClassName(u8"foo");

    auto element2 = data.Document->CreateElement(u8"test-element");
    REQUIRE_FALSE(element2.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element2.Value()).HasException());
    element2.Value()->ClassName(u8"foo bar");

    auto element3 = data.Document->CreateElement(u8"test-element");
    REQUIRE_FALSE(element3.HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*element3.Value()).HasException());
    element3.Value()->ClassName(u8"bar");

    auto elements = data.Node->GetElementsByClassName(u8"foo");
    REQUIRE(elements->Length() == 2uz);
    REQUIRE(elements->Item(0) == element1.Value());
    REQUIRE(elements->Item(1) == element2.Value());

    elements = data.Node->GetElementsByClassName(u8"bar");
    REQUIRE(elements->Length() == 2uz);
    REQUIRE(elements->Item(0) == element2.Value());
    REQUIRE(elements->Item(1) == element3.Value());

    elements = data.Node->GetElementsByClassName(u8"baz");
    REQUIRE(elements->Length() == 0uz);

    REQUIRE_FALSE(data.Node->RemoveChild(*element1.Value()).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*element2.Value()).HasException());
    REQUIRE_FALSE(data.Node->RemoveChild(*element3.Value()).HasException());
  }
}