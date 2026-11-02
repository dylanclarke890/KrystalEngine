#include "Krystal.Booey/DOM/Element.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

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

    constexpr DOMStringView TestNamespace = u8"http://example.com/ns";
  }

  TEST_CASE("Node.nodeName: an Element's is its HTML-uppercased qualified name",
            "[DOM][Element][dom:dom-node-nodename]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeName() == u8"TEST-ELEMENT");
  }

  TEST_CASE("Node.nodeType: an Element is an ELEMENT_NODE",
            "[DOM][Element][dom:dom-node-nodetype][dom:dom-node-element_node]")
  {
    CommonTestData data;
    REQUIRE(data.Node->NodeType() == NodeType::ELEMENT_NODE);
  }

  TEST_CASE("Element: namespaceURI, prefix and localName are its qualified name's parts",
            "[DOM][Element][dom:interface-element][dom:concept-element][dom:concept-element-namespace]"
            "[dom:concept-element-namespace-prefix][dom:concept-element-local-name]"
            "[dom:concept-element-qualified-name][dom:dom-element-namespaceuri][dom:dom-element-prefix]"
            "[dom:dom-element-localname]")
  {
    auto document = CreateRef<HTMLDocument>();

    auto prefixed = document->CreateElementNS(u8"urn:x", u8"x:Thing").ReleaseValue();
    CHECK(prefixed->NamespaceURI() == u8"urn:x");
    CHECK(prefixed->Prefix() == u8"x");
    CHECK(prefixed->LocalName() == u8"Thing");

    auto div = document->CreateElement(u8"div").ReleaseValue();
    CHECK(div->NamespaceURI() == infra::Namespaces::HTML);
    CHECK(div->Prefix() == DOMStringAtom::Null());
    CHECK(div->LocalName() == u8"div");
  }

  TEST_CASE("Element.tagName: its HTML-uppercased qualified name",
            "[DOM][Element][dom:dom-element-tagname][dom:element-html-uppercased-qualified-name]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("Uppercased for an HTML element in an HTML document")
    {
      CHECK(document->CreateElement(u8"div").ReleaseValue()->TagName() == u8"DIV");
      CHECK(document->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), u8"h:sPan")
              .ReleaseValue()
              ->TagName()
            == u8"H:SPAN");
    }

    SECTION("Otherwise its qualified name as it is")
    {
      CHECK(document->CreateElementNS(DOMStringAtom(infra::Namespaces::SVG), u8"s:foreignObject")
              .ReleaseValue()
              ->TagName()
            == u8"s:foreignObject");
      auto xml = CreateRef<XMLDocument>();
      CHECK(xml->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), u8"div").ReleaseValue()->TagName()
            == u8"div");
    }
  }

  TEST_CASE("Element.id, className and slot: reflect their attributes",
            "[DOM][Element][dom:dom-element-id][dom:dom-element-classname][dom:dom-element-slot]"
            "[dom:concept-reflect]")
  {
    CommonTestData data;
    CHECK(data.Node->Id() == u8"");
    CHECK(data.Node->ClassName() == u8"");
    CHECK(data.Node->Slot() == u8"");

    data.Node->Id(u8"test-id");
    data.Node->ClassName(u8"a b");
    data.Node->Slot(u8"s");
    CHECK(data.Node->GetAttribute(u8"id") == u8"test-id");
    CHECK(data.Node->GetAttribute(u8"class") == u8"a b");
    CHECK(data.Node->GetAttribute(u8"slot") == u8"s");

    REQUIRE_FALSE(data.Node->SetAttribute(u8"id", u8"other").HasException());
    CHECK(data.Node->Id() == u8"other");
    // An attribute in a namespace isn't reflected.
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"class", u8"ns").HasException());
    CHECK(data.Node->ClassName() == u8"a b");
  }

  TEST_CASE("ID: an element's ID is its id attribute, and an empty one is none",
            "[DOM][Element][dom:concept-id]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*element).HasException());

    REQUIRE_FALSE(element->SetAttribute(u8"id", u8"a").HasException());
    CHECK(document->GetElementById(u8"a") == element.get());

    REQUIRE_FALSE(element->SetAttribute(u8"id", u8"b").HasException());
    CHECK(document->GetElementById(u8"a") == nullptr);
    CHECK(document->GetElementById(u8"b") == element.get());

    REQUIRE_FALSE(element->SetAttribute(u8"id", u8"").HasException());
    CHECK(document->GetElementById(u8"b") == nullptr);
    CHECK(document->GetElementById(u8"") == nullptr);

    REQUIRE_FALSE(element->SetAttribute(u8"id", u8"c").HasException());
    element->RemoveAttribute(u8"id");
    CHECK(document->GetElementById(u8"c") == nullptr);

    // An id attribute in a namespace isn't the ID.
    REQUIRE_FALSE(element->SetAttributeNS(TestNamespace, u8"id", u8"d").HasException());
    CHECK(document->GetElementById(u8"d") == nullptr);
  }

  TEST_CASE("Element.classList: a token list of the class attribute, whose tokens are the classes",
            "[DOM][Element][dom:dom-element-classlist][dom:concept-class]")
  {
    CommonTestData data;
    auto &classList = data.Node->ClassList();
    CHECK(&data.Node->ClassList() == &classList);
    REQUIRE(classList.Length() == 0uz);

    REQUIRE_FALSE(classList.Add(u8"foo").HasException());
    REQUIRE_FALSE(classList.Add(u8"bar").HasException());
    CHECK(data.Node->GetAttribute(u8"class") == u8"foo bar");
    CHECK(data.Node->ClassNames() == List<DOMStringAtom> {u8"foo", u8"bar"});

    REQUIRE_FALSE(classList.Remove(u8"foo").HasException());
    CHECK(data.Node->GetAttribute(u8"class") == u8"bar");

    SECTION("It follows the class attribute")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"class", u8"a b a").HasException());
      CHECK(classList.Length() == 2uz);
      CHECK(classList.Contains(u8"a"));
      CHECK(classList.Contains(u8"b"));

      data.Node->RemoveAttribute(u8"class");
      CHECK(classList.Length() == 0uz);
    }
  }

  TEST_CASE("Element.hasAttributes() and attributes: whether it has attributes, and their NamedNodeMap",
            "[DOM][Element][dom:dom-element-hasattributes][dom:dom-element-attributes]"
            "[dom:concept-element-attribute]")
  {
    CommonTestData data;
    auto &attributes = data.Node->Attributes();
    CHECK(&data.Node->Attributes() == &attributes);
    CHECK_FALSE(data.Node->HasAttributes());
    CHECK(attributes.Length() == 0uz);

    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    CHECK(data.Node->HasAttributes());
    REQUIRE(attributes.Length() == 1uz);
    CHECK(attributes.Item(0)->Name() == u8"test-attr");
    CHECK(attributes.Item(0)->Value() == u8"test-value");
  }

  TEST_CASE("Element.getAttributeNames(): the qualified names of its attributes, in order",
            "[DOM][Element][dom:dom-element-getattributenames]")
  {
    CommonTestData data;
    CHECK(data.Node->GetAttributeNames().empty());
    REQUIRE_FALSE(data.Node->SetAttribute(u8"b", u8"1").HasException());
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:a", u8"2").HasException());
    REQUIRE_FALSE(data.Node->SetAttribute(u8"b", u8"3").HasException());

    CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"b", u8"x:a"});
  }

  TEST_CASE("Element.getAttribute() and getAttributeNS(): the attribute's value, or null",
            "[DOM][Element][dom:dom-element-getattribute][dom:dom-element-getattributens]")
  {
    CommonTestData data;
    CHECK_FALSE(data.Node->GetAttribute(u8"test-attr").has_value());
    CHECK_FALSE(data.Node->GetAttributeNS(TestNamespace, u8"test-attr").has_value());

    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"").HasException());
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:test-attr", u8"ns").HasException());
    CHECK(data.Node->GetAttribute(u8"TEST-ATTR") == u8"");
    CHECK(data.Node->GetAttribute(u8"x:test-attr") == u8"ns");
    CHECK(data.Node->GetAttributeNS(DOMStringAtom::Empty(), u8"test-attr") == u8"");
    CHECK(data.Node->GetAttributeNS(TestNamespace, u8"test-attr") == u8"ns");
  }

  TEST_CASE("Element.setAttribute(): changes the first attribute with the name, or appends one",
            "[DOM][Element][dom:dom-element-setattribute]")
  {
    CommonTestData data;

    SECTION("A new attribute has no namespace or prefix")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"x:a", u8"1").HasException());
      auto *attr = data.Node->GetAttributeNode(u8"x:a");
      REQUIRE(attr != nullptr);
      CHECK(attr->NamespaceURI() == DOMStringAtom::Null());
      CHECK(attr->Prefix() == DOMStringAtom::Null());
      CHECK(attr->LocalName() == u8"x:a");
      CHECK(attr->Value() == u8"1");
    }

    SECTION("The first attribute with the qualified name is changed, whatever its namespace")
    {
      REQUIRE_FALSE(data.Node->SetAttributeNS(u8"urn:a", u8"x:a", u8"1").HasException());
      REQUIRE_FALSE(data.Node->SetAttributeNS(u8"urn:b", u8"x:a", u8"2").HasException());
      auto *first = data.Node->GetAttributeNodeNS(u8"urn:a", u8"a");
      REQUIRE_FALSE(data.Node->SetAttribute(u8"x:a", u8"3").HasException());
      CHECK(data.Node->GetAttributeNodeNS(u8"urn:a", u8"a") == first);
      CHECK(first->Value() == u8"3");
      CHECK(data.Node->GetAttributeNS(u8"urn:b", u8"a") == u8"2");
    }

    SECTION("The name is lowercased for an HTML element in an HTML document")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"Mixed", u8"1").HasException());
      CHECK(data.Node->GetAttributeNames() == List<DOMString> {u8"mixed"});

      auto xml = CreateRef<XMLDocument>();
      auto element = xml->CreateElementNS(DOMStringAtom(infra::Namespaces::HTML), u8"div").ReleaseValue();
      REQUIRE_FALSE(element->SetAttribute(u8"Mixed", u8"1").HasException());
      CHECK(element->GetAttributeNames() == List<DOMString> {u8"Mixed"});
    }

    SECTION("An invalid attribute local name throws an InvalidCharacterError")
    {
      for (auto name : {u8"", u8"invalid name", u8"a/b", u8"a>b", u8"a=b"})
      {
        auto result = data.Node->SetAttribute(DOMStringAtom(name), u8"value");
        REQUIRE(result.HasException());
        CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
      }
      CHECK_FALSE(data.Node->SetAttribute(u8"1:x", u8"value").HasException());
    }
  }

  TEST_CASE("Element.setAttributeNS(): validates and extracts the name, then sets the attribute value",
            "[DOM][Element][dom:dom-element-setattributens]")
  {
    CommonTestData data;

    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:a", u8"1").HasException());
    auto *attr = data.Node->GetAttributeNodeNS(TestNamespace, u8"a");
    REQUIRE(attr != nullptr);
    CHECK(attr->Prefix() == u8"x");
    CHECK(attr->Value() == u8"1");

    // The same namespace and local name changes it, keeping its prefix.
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"y:a", u8"2").HasException());
    CHECK(data.Node->GetAttributeNodeNS(TestNamespace, u8"a") == attr);
    CHECK(attr->Prefix() == u8"x");
    CHECK(attr->Value() == u8"2");

    SECTION("Validate and extract's exceptions are thrown")
    {
      auto result = data.Node->SetAttributeNS(TestNamespace, u8"invalid name", u8"value");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);

      result = data.Node->SetAttributeNS(DOMStringAtom::Null(), u8"x:a", u8"value");
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NamespaceError);
    }
  }

  TEST_CASE("Element.removeAttribute() and removeAttributeNS(): remove the attribute, if any",
            "[DOM][Element][dom:dom-element-removeattribute][dom:dom-element-removeattributens]")
  {
    CommonTestData data;
    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:test-attr", u8"ns").HasException());

    data.Node->RemoveAttribute(u8"missing");
    data.Node->RemoveAttribute(u8"TEST-ATTR");
    CHECK_FALSE(data.Node->GetAttribute(u8"test-attr").has_value());

    data.Node->RemoveAttributeNS(TestNamespace, u8"x:test-attr");
    CHECK(data.Node->HasAttributes());
    data.Node->RemoveAttributeNS(TestNamespace, u8"test-attr");
    CHECK_FALSE(data.Node->HasAttributes());
  }

  TEST_CASE("Element.toggleAttribute(): adds or removes the attribute, as force says",
            "[DOM][Element][dom:dom-element-toggleattribute]")
  {
    CommonTestData data;

    auto toggle = [&](DOMStringAtom name, Maybe<bool> force)
    {
      auto result = data.Node->ToggleAttribute(name, force);
      REQUIRE_FALSE(result.HasException());
      return result.Value();
    };

    SECTION("Without force, it flips")
    {
      CHECK(toggle(u8"Test-Attr", null));
      CHECK(data.Node->GetAttribute(u8"test-attr") == u8"");
      CHECK_FALSE(toggle(u8"test-attr", null));
      CHECK_FALSE(data.Node->HasAttribute(u8"test-attr"));
    }

    SECTION("Forced on, it's added or kept")
    {
      CHECK(toggle(u8"test-attr", true));
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value").HasException());
      CHECK(toggle(u8"test-attr", true));
      CHECK(data.Node->GetAttribute(u8"test-attr") == u8"value");
    }

    SECTION("Forced off, it's removed or left absent")
    {
      CHECK_FALSE(toggle(u8"test-attr", false));
      CHECK_FALSE(data.Node->HasAttribute(u8"test-attr"));
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value").HasException());
      CHECK_FALSE(toggle(u8"test-attr", false));
      CHECK_FALSE(data.Node->HasAttribute(u8"test-attr"));
    }

    SECTION("An invalid attribute local name throws an InvalidCharacterError")
    {
      auto result = data.Node->ToggleAttribute(u8"invalid name", true);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InvalidCharacterError);
    }
  }

  TEST_CASE("Element.hasAttribute() and hasAttributeNS(): whether it has the attribute",
            "[DOM][Element][dom:dom-element-hasattribute][dom:dom-element-hasattributens]"
            "[dom:concept-element-attribute-has]")
  {
    CommonTestData data;
    CHECK_FALSE(data.Node->HasAttribute(u8"test-attr"));
    CHECK_FALSE(data.Node->HasAttributeNS(TestNamespace, u8"test-attr"));

    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"").HasException());
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:other", u8"").HasException());
    CHECK(data.Node->HasAttribute(u8"TEST-ATTR"));
    CHECK(data.Node->HasAttribute(u8"x:other"));
    CHECK_FALSE(data.Node->HasAttribute(u8"other"));
    CHECK(data.Node->HasAttributeNS(DOMStringAtom::Empty(), u8"test-attr"));
    CHECK(data.Node->HasAttributeNS(TestNamespace, u8"other"));
    CHECK_FALSE(data.Node->HasAttributeNS(TestNamespace, u8"test-attr"));
  }

  TEST_CASE("Element.getAttributeNode() and getAttributeNodeNS(): the attribute, or null",
            "[DOM][Element][dom:dom-element-getattributenode][dom:dom-element-getattributenodens]")
  {
    CommonTestData data;
    CHECK(data.Node->GetAttributeNode(u8"test-attr") == nullptr);
    CHECK(data.Node->GetAttributeNodeNS(TestNamespace, u8"test-attr") == nullptr);

    REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"test-value").HasException());
    REQUIRE_FALSE(data.Node->SetAttributeNS(TestNamespace, u8"x:test-attr", u8"ns").HasException());

    auto *attr = data.Node->GetAttributeNode(u8"TEST-ATTR");
    REQUIRE(attr != nullptr);
    CHECK(attr->Value() == u8"test-value");
    CHECK(attr->OwnerElement() == data.Node.get());
    auto *nsAttr = data.Node->GetAttributeNodeNS(TestNamespace, u8"test-attr");
    REQUIRE(nsAttr != nullptr);
    CHECK(nsAttr->Value() == u8"ns");
  }

  TEST_CASE("Element.setAttributeNode() and setAttributeNodeNS(): set the attribute, returning the old one",
            "[DOM][Element][dom:dom-element-setattributenode][dom:dom-element-setattributenodens]")
  {
    CommonTestData data;
    using SetAttributeNode = ExceptionOr<RefPtr<Attr>> (Element::*)(Attr &) noexcept;
    auto set =
      GENERATE(SetAttributeNode(&Element::SetAttributeNode), SetAttributeNode(&Element::SetAttributeNodeNS));

    auto attr1 = data.Document->CreateAttributeNS(TestNamespace, u8"x:test-attr").ReleaseValue();
    attr1->Value(u8"value1");
    auto result = (data.Node.get()->*set)(*attr1);
    REQUIRE_FALSE(result.HasException());
    CHECK(result.Value() == nullptr);
    CHECK(data.Node->GetAttributeNodeNS(TestNamespace, u8"test-attr") == attr1.get());

    SECTION("One with the same namespace and local name replaces it")
    {
      auto attr2 = data.Document->CreateAttributeNS(TestNamespace, u8"y:test-attr").ReleaseValue();
      attr2->Value(u8"value2");
      result = (data.Node.get()->*set)(*attr2);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == attr1);
      CHECK(data.Node->GetAttributeNodeNS(TestNamespace, u8"test-attr") == attr2.get());
    }

    SECTION("Another element's attribute throws an InUseAttributeError")
    {
      auto anotherElement = data.Document->CreateElement(u8"another-element").ReleaseValue();
      result = (anotherElement.get()->*set)(*attr1);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InUseAttributeError);
    }
  }

  TEST_CASE("Element.removeAttributeNode(): removes and returns the attribute, if it's the element's",
            "[DOM][Element][dom:dom-element-removeattributenode]")
  {
    CommonTestData data;

    SECTION("One of its attributes")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value").HasException());
      Ref<Attr> attr = ShareRef(*data.Node->GetAttributeNode(u8"test-attr"));
      auto result = data.Node->RemoveAttributeNode(*attr);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == attr);
      CHECK(attr->OwnerElement() == nullptr);
      CHECK_FALSE(data.Node->HasAttributes());
    }

    SECTION("Another attribute, even with the same name, throws a NotFoundError")
    {
      REQUIRE_FALSE(data.Node->SetAttribute(u8"test-attr", u8"value").HasException());
      auto attr = data.Document->CreateAttribute(u8"test-attr").ReleaseValue();
      auto result = data.Node->RemoveAttributeNode(*attr);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotFoundError);
      CHECK(data.Node->HasAttribute(u8"test-attr"));
    }
  }

  TEST_CASE("Element.attachShadow(): attaches a shadow root with the init's options and registry",
            "[DOM][Element][dom:dom-element-attachshadow][dom:concept-element-shadow-root]"
            "[dom:element-shadow-host]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto global = html::CustomElementRegistry::Create(*document);
    document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(global));
    auto host = document->CreateElement(u8"div").ReleaseValue();

    SECTION("The document's registry by default, or a scoped one from the init")
    {
      auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Closed}).ReleaseValue();
      CHECK(shadow->Host() == host.get());
      CHECK(shadow->Mode() == ShadowRootMode::Closed);
      CHECK(shadow->CustomElementRegistry() == global);
      CHECK(TreeQueries::IsShadowHost(*host));

      auto scoped = html::CustomElementRegistry::CreateScoped();
      auto other = document->CreateElement(u8"div").ReleaseValue();
      CHECK(
        other->AttachShadow({.CustomElementRegistry = scoped.get()}).ReleaseValue()->CustomElementRegistry()
        == scoped);
    }

    SECTION("Another global registry throws a NotSupportedError")
    {
      auto otherDocument = CreateRef<HTMLDocument>();
      auto otherGlobal = html::CustomElementRegistry::Create(*otherDocument);
      auto result = host->AttachShadow({.CustomElementRegistry = otherGlobal.get()});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
      CHECK_FALSE(TreeQueries::IsShadowHost(*host));
    }

    SECTION("Attach a shadow root's exceptions are thrown")
    {
      auto img = document->CreateElement(u8"img").ReleaseValue();
      auto result = img->AttachShadow({});
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
    }
  }

  TEST_CASE("Element.shadowRoot: its shadow root if that's open, else null",
            "[DOM][Element][dom:dom-element-shadowroot]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    CHECK(host->ShadowRoot() == nullptr);

    auto mode = GENERATE(ShadowRootMode::Open, ShadowRootMode::Closed);
    auto shadow = host->AttachShadow({.Mode = mode}).ReleaseValue();
    CHECK(host->ShadowRoot() == (mode == ShadowRootMode::Open ? shadow.get() : nullptr));
  }

  TEST_CASE("Element.customElementRegistry: its custom element registry",
            "[DOM][Element][dom:dom-element-customelementregistry][dom:element-custom-element-registry]")
  {
    auto document = CreateRef<HTMLDocument>();
    CHECK(document->CreateElement(u8"div").ReleaseValue()->CustomElementRegistry() == nullptr);

    auto global = html::CustomElementRegistry::Create(*document);
    document->SetCustomElementRegistry(RefPtr<html::CustomElementRegistry>(global));
    CHECK(document->CreateElement(u8"div").ReleaseValue()->CustomElementRegistry() == global);

    auto scoped = html::CustomElementRegistry::CreateScoped();
    auto element = document->CreateElement(u8"div", ElementCreationOptions {.CustomElementRegistry = scoped});
    CHECK(element.ReleaseValue()->CustomElementRegistry() == scoped);
  }

  TEST_CASE("Element.closest(), matches() and webkitMatchesSelector(): match its inclusive ancestors or it",
            "[DOM][Element][dom:dom-element-closest][dom:dom-element-matches]"
            "[dom:dom-element-webkitmatchesselector]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto document =
      Document::ParseHTMLUnsafe(
        *context, u8"<!DOCTYPE html><section id=s class=c><div id=d><p id=p></p></div></section>")
        .ReleaseValue();
    auto p = document->GetElementById(u8"p");
    REQUIRE(p != nullptr);

    auto closestId = [&](DOMStringView selectors)
    {
      auto result = p->Closest(selectors);
      REQUIRE_FALSE(result.HasException());
      return result.Value() ? result.Value()->Id() : DOMString(u8"(none)");
    };
    CHECK(closestId(u8"p") == u8"p");
    CHECK(closestId(u8"div") == u8"d");
    CHECK(closestId(u8".c") == u8"s");
    CHECK(closestId(u8"section > div, span") == u8"d");
    CHECK(closestId(u8"article") == u8"(none)");
    CHECK(closestId(u8":scope") == u8"p");
    // The scoping root is the element, so :scope doesn't match its ancestors.
    CHECK(closestId(u8":scope > p, div:scope") == u8"(none)");

    CHECK(p->Matches(u8"div > p").ReleaseValue());
    CHECK_FALSE(p->Matches(u8"section > p").ReleaseValue());
    CHECK(p->WebkitMatchesSelector(u8"section p:only-child").ReleaseValue());

    for (auto invalid : {u8"", u8"a >", u8":unknown-pseudo"})
    {
      auto closest = p->Closest(invalid);
      REQUIRE(closest.HasException());
      CHECK(closest.GetException().Code() == ExceptionCode::SyntaxError);
      auto matches = p->Matches(invalid);
      REQUIRE(matches.HasException());
      CHECK(matches.GetException().Code() == ExceptionCode::SyntaxError);
    }
  }

  TEST_CASE("Element.getElementsByTagName(), getElementsByTagNameNS() and getElementsByClassName(): "
            "its descendants only",
            "[DOM][Element][dom:dom-element-getelementsbytagname][dom:dom-element-getelementsbytagnamens]"
            "[dom:dom-element-getelementsbyclassname]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto document =
      Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html><p id=outer class=a><p id=root class=a><span "
                                          u8"id=inner class=a>x</span></p>")
        .ReleaseValue();
    auto root = document->GetElementById(u8"root");
    REQUIRE(root != nullptr);

    auto tagNames = root->GetElementsByTagName(u8"span");
    REQUIRE(tagNames->Length() == 1u);
    CHECK(tagNames->Item(0)->Id() == u8"inner");
    CHECK(root->GetElementsByTagName(u8"p")->Length() == 0u);

    auto tagNamesNS = root->GetElementsByTagNameNS(DOMStringAtom(infra::Namespaces::HTML), u8"*");
    REQUIRE(tagNamesNS->Length() == 1u);
    CHECK(tagNamesNS->Item(0)->Id() == u8"inner");

    auto classNames = root->GetElementsByClassName(u8"a");
    REQUIRE(classNames->Length() == 1u);
    CHECK(classNames->Item(0)->Id() == u8"inner");
  }

  TEST_CASE("Element.insertAdjacentElement() and insertAdjacentText(): insert adjacent, given the node",
            "[DOM][Element][dom:dom-element-insertadjacentelement][dom:dom-element-insertadjacenttext]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto parent = document->CreateElement(u8"div").ReleaseValue();
    auto element = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(parent->AppendChild(*element).HasException());

    auto inserted = document->CreateElement(u8"b").ReleaseValue();
    auto result = element->InsertAdjacentElement(InsertAdjacentWhere::AfterBegin, *inserted);
    REQUIRE_FALSE(result.HasException());
    CHECK(result.Value() == inserted.get());

    REQUIRE_FALSE(element->InsertAdjacentText(InsertAdjacentWhere::BeforeBegin, u8"before").HasException());
    REQUIRE_FALSE(element->InsertAdjacentText(InsertAdjacentWhere::BeforeEnd, u8"end").HasException());
    CHECK(parent->InnerHTML() == u8"before<p><b></b>end</p>");
    REQUIRE(parent->FirstChild() != nullptr);
    CHECK(&parent->FirstChild()->NodeDocument() == document.get());

    SECTION("With no parent to insert beside, nothing is inserted")
    {
      auto orphan = document->CreateElement(u8"p").ReleaseValue();
      auto other = document->CreateElement(u8"a").ReleaseValue();
      result = orphan->InsertAdjacentElement(InsertAdjacentWhere::AfterEnd, *other);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == nullptr);
      REQUIRE_FALSE(orphan->InsertAdjacentText(InsertAdjacentWhere::BeforeBegin, u8"text").HasException());
      CHECK_FALSE(orphan->HasChildNodes());
    }
  }

  TEST_CASE("Node.textContent: an Element's is its descendant text, which setting it replaces",
            "[DOM][Element][dom:dom-node-textcontent][dom:get-text-content][dom:set-text-content]")
  {
    CommonTestData data;
    auto child = data.Document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(child->AppendChild(*data.Document->CreateTextNode(u8"b")).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*data.Document->CreateTextNode(u8"a")).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*data.Document->CreateComment(u8"not text")).HasException());
    REQUIRE_FALSE(data.Node->AppendChild(*child).HasException());

    CHECK(data.Node->TextContent() == u8"ab");

    REQUIRE_FALSE(data.Node->TextContent(u8"new").HasException());
    REQUIRE(data.Node->FirstChild() != nullptr);
    CHECK(data.Node->FirstChild() == data.Node->LastChild());
    CHECK(data.Node->FirstChild()->NodeValue() == u8"new");

    REQUIRE_FALSE(data.Node->TextContent(u8"").HasException());
    CHECK_FALSE(data.Node->HasChildNodes());
    CHECK(data.Node->TextContent() == u8"");
  }

  TEST_CASE("Update a slottable's name: a change to the slot attribute moves it to the matching slot",
            "[DOM][Element][dom:slotable-name]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*host).HasException());
    auto shadow = host->AttachShadow({.Mode = ShadowRootMode::Open}).ReleaseValue();
    auto defaultSlot = document->CreateElement(u8"slot").ReleaseValue();
    auto namedSlot = document->CreateElement(u8"slot").ReleaseValue();
    REQUIRE_FALSE(namedSlot->SetAttribute(u8"name", u8"a").HasException());
    REQUIRE_FALSE(shadow->AppendChild(*defaultSlot).HasException());
    REQUIRE_FALSE(shadow->AppendChild(*namedSlot).HasException());

    auto span = document->CreateElement(u8"span").ReleaseValue();
    REQUIRE_FALSE(host->AppendChild(*span).HasException());
    REQUIRE(SlotAlgorithms::GetAssignedSlot(*span) == defaultSlot.get());

    REQUIRE_FALSE(span->SetAttribute(u8"slot", u8"a").HasException());
    CHECK(SlotAlgorithms::GetAssignedSlot(*span) == namedSlot.get());

    SECTION("A name with no slot unassigns it")
    {
      REQUIRE_FALSE(span->SetAttribute(u8"slot", u8"b").HasException());
      CHECK(SlotAlgorithms::GetAssignedSlot(*span) == nullptr);
    }

    SECTION("Removed or emptied, it goes back to the default slot")
    {
      SECTION("Removed")
      {
        span->RemoveAttribute(u8"slot");
      }
      SECTION("Emptied")
      {
        REQUIRE_FALSE(span->SetAttribute(u8"slot", u8"").HasException());
      }
      CHECK(SlotAlgorithms::GetAssignedSlot(*span) == defaultSlot.get());
    }

    SECTION("A slot attribute in a namespace doesn't change it")
    {
      span->RemoveAttribute(u8"slot");
      REQUIRE_FALSE(span->SetAttributeNS(u8"http://example.com/", u8"slot", u8"a").HasException());
      CHECK(SlotAlgorithms::GetAssignedSlot(*span) == defaultSlot.get());
    }
  }

  TEST_CASE(
    "Element::ElementForAttributeInternal: the explicitly set element while it's in a shadow-including "
    "ancestor's tree, else the element with the ID",
    "[DOM][Element][html:attr-associated-element]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto root = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(document->AppendChild(*root).HasException());
    auto button = document->CreateElement(u8"button").ReleaseValue();
    auto target = document->CreateElement(u8"div").ReleaseValue();
    auto other = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(target->SetAttribute(u8"id", u8"t").HasException());
    REQUIRE_FALSE(root->Append({Ref<Node>(button), Ref<Node>(target)}).HasException());

    // By ID, in the element's tree.
    REQUIRE_FALSE(button->SetAttribute(u8"commandfor", u8"t").HasException());
    CHECK(button->ElementForAttributeInternal(u8"commandfor").get() == target.get());
    CHECK_FALSE(button->ElementForAttributeInternal(u8"popovertarget"));

    // Setting the element empties the content attribute; the element counts only in the same tree.
    button->SetElementAttribute(u8"commandfor", other.get());
    CHECK(button->GetAttribute(u8"commandfor") == u8"");
    CHECK_FALSE(button->ElementForAttributeInternal(u8"commandfor"));
    REQUIRE_FALSE(root->Append({Ref<Node>(other)}).HasException());
    CHECK(button->ElementForAttributeInternal(u8"commandfor").get() == other.get());

    // An element in a shadow tree can refer to one in its host's tree, but not the other way round.
    auto shadow = root->AttachShadow({}).ReleaseValue();
    auto inner = document->CreateElement(u8"button").ReleaseValue();
    REQUIRE_FALSE(shadow->Append({Ref<Node>(inner)}).HasException());
    inner->SetElementAttribute(u8"commandfor", other.get());
    CHECK(inner->ElementForAttributeInternal(u8"commandfor").get() == other.get());
    button->SetElementAttribute(u8"commandfor", inner.get());
    CHECK_FALSE(button->ElementForAttributeInternal(u8"commandfor"));

    // Changing the content attribute forgets the explicitly set element.
    REQUIRE_FALSE(button->SetAttribute(u8"commandfor", u8"t").HasException());
    CHECK(button->ElementForAttributeInternal(u8"commandfor").get() == target.get());

    // Setting null removes the content attribute.
    button->SetElementAttribute(u8"commandfor", nullptr);
    CHECK_FALSE(button->HasAttribute(u8"commandfor"));
    CHECK_FALSE(button->ElementForAttributeInternal(u8"commandfor"));
  }
}
