#include "Krystal.Booey/DOM/Algorithms/ElementAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationObservers/MutationRecord.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Nodes/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    struct AttributeChange
    {
      DOMStringAtom LocalName;
      DOMStringAtom NamespaceURI;
      Maybe<DOMString> OldValue;
      Maybe<DOMString> Value;
    };

    /// @brief Records the attribute change steps run for it.
    class AttributeChangeRecordingElement : public Element
    {
      KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(AttributeChangeRecordingElement);

    public:
      List<AttributeChange> Changes;

      explicit AttributeChangeRecordingElement(dom::Document &document) noexcept
          : Element(document, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"test-element"},
                    NodeFlags::None)
      {
      }

    protected:
      void OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                              Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept override
      {
        Element::OnAttributeChanged(localName, oldValue, value, namespaceURI);

        auto toString = [](DOMStringView view)
        {
          return DOMString(view);
        };
        Changes.push_back({localName, namespaceURI, oldValue.transform(toString), value.transform(toString)});
      }
    };

    Ref<MutationObserver> ObserveAttributes(Node &node) noexcept
    {
      auto observer =
        CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
      REQUIRE_FALSE(
        observer->Observe(node, MutationObserverInit {.Attributes = true, .AttributeOldValue = true})
          .HasException());
      return observer;
    }

    Ref<Attr> NewAttribute(Document &document, DOMStringAtom localName, DOMString &&value) noexcept
    {
      auto attr = document.CreateAttribute(localName).ReleaseValue();
      attr->Value(krys::move(value));
      return attr;
    }
  }

  TEST_CASE(
    "Handle attribute changes: queues a record and runs the attribute change steps",
    "[DOM][ElementAlgorithms][dom:handle-attribute-changes][dom:concept-element-attributes-change-ext]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateRef<AttributeChangeRecordingElement>(*document);
    auto observer = ObserveAttributes(*element);

    REQUIRE_FALSE(element->SetAttributeNS(u8"urn:x", u8"x:a", u8"1").HasException());

    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->Type() == u8"attributes");
    CHECK(records[0]->Target().get() == element.get());
    CHECK(records[0]->AttributeName() == u8"a");
    CHECK(records[0]->AttributeNamespace() == u8"urn:x");
    CHECK(records[0]->OldValue() == DOMStringAtom::Null());

    REQUIRE(element->Changes.size() == 1uz);
    CHECK(element->Changes[0].LocalName == u8"a");
    CHECK(element->Changes[0].NamespaceURI == u8"urn:x");
    CHECK_FALSE(element->Changes[0].OldValue.has_value());
    CHECK(element->Changes[0].Value == u8"1");

    SECTION("An empty value isn't null")
    {
      REQUIRE_FALSE(element->SetAttribute(u8"title", u8"").HasException());
      element->RemoveAttribute(u8"title");
      REQUIRE(element->Changes.size() == 3uz);
      CHECK(element->Changes[1].Value == u8"");
      CHECK(element->Changes[2].OldValue == u8"");
      CHECK_FALSE(element->Changes[2].Value.has_value());
    }
  }

  TEST_CASE("Change an attribute: sets its value and handles the change from the old value",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-change]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateRef<AttributeChangeRecordingElement>(*document);
    REQUIRE_FALSE(element->SetAttribute(u8"title", u8"a").HasException());
    auto *attr = element->GetAttributeNode(u8"title");
    REQUIRE(attr != nullptr);
    auto observer = ObserveAttributes(*element);

    ElementAlgorithms::ChangeAttribute(*attr, u8"b");

    CHECK(attr->Value() == u8"b");
    CHECK(element->GetAttributeNode(u8"title") == attr);
    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->OldValue() == u8"a");
    REQUIRE(element->Changes.size() == 2uz);
    CHECK(element->Changes[1].OldValue == u8"a");
    CHECK(element->Changes[1].Value == u8"b");
  }

  TEST_CASE("Append an attribute: adds it last, sets its element and node document, from a null old value",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-append]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto other = CreateRef<HTMLDocument>();
    auto element = CreateRef<AttributeChangeRecordingElement>(*document);
    REQUIRE_FALSE(element->SetAttribute(u8"first", u8"").HasException());
    auto attr = NewAttribute(*other, u8"second", u8"v");

    ElementAlgorithms::AppendAttribute(*attr, *element);

    CHECK(element->GetAttributeNames() == List<DOMString> {u8"first", u8"second"});
    CHECK(attr->OwnerElement() == element.get());
    CHECK(&attr->NodeDocument() == document.get());
    REQUIRE(element->Changes.size() == 2uz);
    CHECK_FALSE(element->Changes[1].OldValue.has_value());
    CHECK(element->Changes[1].Value == u8"v");
  }

  TEST_CASE("Remove an attribute: takes it from its element's list and handles the change to null",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-remove]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateRef<AttributeChangeRecordingElement>(*document);
    REQUIRE_FALSE(element->SetAttribute(u8"a", u8"1").HasException());
    REQUIRE_FALSE(element->SetAttribute(u8"b", u8"2").HasException());
    Ref<Attr> attr = ShareRef(*element->GetAttributeNode(u8"a"));
    auto observer = ObserveAttributes(*element);

    ElementAlgorithms::RemoveAttribute(*attr);

    auto records = observer->TakeRecords();
    REQUIRE(records.size() == 1uz);
    CHECK(records[0]->AttributeName() == u8"a");
    CHECK(records[0]->OldValue() == u8"1");
    CHECK(element->GetAttributeNames() == List<DOMString> {u8"b"});
    CHECK(attr->OwnerElement() == nullptr);
    CHECK(attr->Value() == u8"1");
    REQUIRE(element->Changes.size() == 3uz);
    CHECK(element->Changes[2].LocalName == u8"a");
    CHECK(element->Changes[2].OldValue == u8"1");
    CHECK_FALSE(element->Changes[2].Value.has_value());
  }

  TEST_CASE("Replace an attribute: the new one takes the old one's place, and the old one is detached",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-replace]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto other = CreateRef<HTMLDocument>();
    auto element = CreateRef<AttributeChangeRecordingElement>(*document);
    REQUIRE_FALSE(element->SetAttribute(u8"a", u8"1").HasException());
    REQUIRE_FALSE(element->SetAttribute(u8"b", u8"2").HasException());
    Ref<Attr> oldAttr = ShareRef(*element->GetAttributeNode(u8"a"));
    auto newAttr = NewAttribute(*other, u8"a", u8"3");

    ElementAlgorithms::ReplaceAttribute(*oldAttr, *newAttr);

    CHECK(element->GetAttributeNames() == List<DOMString> {u8"a", u8"b"});
    CHECK(element->GetAttributeNode(u8"a") == newAttr.get());
    CHECK(newAttr->OwnerElement() == element.get());
    CHECK(&newAttr->NodeDocument() == document.get());
    CHECK(oldAttr->OwnerElement() == nullptr);
    REQUIRE(element->Changes.size() == 3uz);
    CHECK(element->Changes[2].OldValue == u8"1");
    CHECK(element->Changes[2].Value == u8"3");
  }

  TEST_CASE("Get an attribute by name: lowercased for an HTML element in an HTML document",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-get-by-name]")
  {
    SECTION("An HTML element in an HTML document")
    {
      auto document = CreateRef<HTMLDocument>();
      auto element = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(element->SetAttributeNS(DOMStringAtom::Null(), u8"Mixed", u8"upper").HasException());
      REQUIRE_FALSE(element->SetAttribute(u8"mixed", u8"lower").HasException());
      auto *attr = ElementAlgorithms::GetAttributeByName(u8"MIXED", *element);
      REQUIRE(attr != nullptr);
      CHECK(attr->Value() == u8"lower");
    }

    SECTION("A non-HTML element, or an XML document, matches case-sensitively")
    {
      auto check = [](Document &document, DOMStringAtom namespaceURI)
      {
        auto element = document.CreateElementNS(namespaceURI, u8"g").ReleaseValue();
        REQUIRE_FALSE(element->SetAttribute(u8"viewBox", u8"0 0 1 1").HasException());
        CHECK(ElementAlgorithms::GetAttributeByName(u8"viewbox", *element) == nullptr);
        CHECK(ElementAlgorithms::GetAttributeByName(u8"viewBox", *element) != nullptr);
      };
      check(*CreateRef<HTMLDocument>(), DOMStringAtom(infra::Namespaces::SVG));
      check(*CreateRef<XMLDocument>(), DOMStringAtom(infra::Namespaces::HTML));
    }

    SECTION("The first with the qualified name")
    {
      auto document = CreateRef<HTMLDocument>();
      auto element = document->CreateElement(u8"div").ReleaseValue();
      REQUIRE_FALSE(element->SetAttributeNS(u8"urn:a", u8"x:y", u8"a").HasException());
      REQUIRE_FALSE(element->SetAttributeNS(u8"urn:b", u8"x:y", u8"b").HasException());
      auto *attr = ElementAlgorithms::GetAttributeByName(u8"x:y", *element);
      REQUIRE(attr != nullptr);
      CHECK(attr->Value() == u8"a");
      CHECK(ElementAlgorithms::GetAttributeByName(u8"y", *element) == nullptr);
    }
  }

  TEST_CASE("Get an attribute by namespace and local name: the empty namespace is null",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-get-by-namespace]"
            "[dom:concept-element-attributes-get-value]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(element->SetAttribute(u8"a", u8"none").HasException());
    REQUIRE_FALSE(element->SetAttributeNS(u8"urn:x", u8"x:a", u8"x").HasException());

    CHECK(ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom::Empty(), u8"a", *element)->Value()
          == u8"none");
    CHECK(ElementAlgorithms::GetAttributeByNamespace(DOMStringAtom::Null(), u8"a", *element)->Value()
          == u8"none");
    CHECK(ElementAlgorithms::GetAttributeByNamespace(u8"urn:x", u8"a", *element)->Value() == u8"x");
    CHECK(ElementAlgorithms::GetAttributeByNamespace(u8"urn:y", u8"a", *element) == nullptr);

    // Get an attribute value: the empty string if there's none.
    CHECK(ElementAlgorithms::GetAttributeValue(*element, u8"a") == u8"none");
    CHECK(ElementAlgorithms::GetAttributeValue(*element, u8"a", u8"urn:x") == u8"x");
    CHECK(ElementAlgorithms::GetAttributeValue(*element, u8"b") == u8"");
  }

  TEST_CASE("Set an attribute: appends it, replaces the one with its name, or throws if it's in use",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-set]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div").ReleaseValue();
    auto first = NewAttribute(*document, u8"a", u8"1");

    auto result = ElementAlgorithms::SetAttribute(*first, *element);
    REQUIRE_FALSE(result.HasException());
    CHECK(result.Value() == nullptr);
    CHECK(first->OwnerElement() == element.get());

    SECTION("Setting it again returns it and changes nothing")
    {
      result = ElementAlgorithms::SetAttribute(*first, *element);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == first);
      CHECK(element->GetAttributeNode(u8"a") == first.get());
    }

    SECTION("One with the same namespace and local name replaces it, returning it")
    {
      auto second = NewAttribute(*document, u8"a", u8"2");
      result = ElementAlgorithms::SetAttribute(*second, *element);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == first);
      CHECK(element->GetAttributeNode(u8"a") == second.get());
      CHECK(first->OwnerElement() == nullptr);
    }

    SECTION("Another element's attribute throws an InUseAttributeError")
    {
      auto other = document->CreateElement(u8"div").ReleaseValue();
      result = ElementAlgorithms::SetAttribute(*first, *other);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::InUseAttributeError);
      CHECK_FALSE(other->HasAttributes());
    }
  }

  TEST_CASE("Set an attribute value: changes the attribute, or appends a new one with the prefix",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-set-value]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div").ReleaseValue();

    ElementAlgorithms::SetAttributeValue(*element, u8"a", u8"1", u8"x", u8"urn:x");
    auto *attr = element->GetAttributeNodeNS(u8"urn:x", u8"a");
    REQUIRE(attr != nullptr);
    CHECK(attr->Prefix() == u8"x");
    CHECK(attr->Value() == u8"1");

    // The prefix of an existing attribute is kept.
    ElementAlgorithms::SetAttributeValue(*element, u8"a", u8"2", u8"y", u8"urn:x");
    CHECK(element->GetAttributeNodeNS(u8"urn:x", u8"a") == attr);
    CHECK(attr->Prefix() == u8"x");
    CHECK(attr->Value() == u8"2");
  }

  TEST_CASE("Remove an attribute by name or by namespace and local name: removes and returns it, if any",
            "[DOM][ElementAlgorithms][dom:concept-element-attributes-remove-by-name]"
            "[dom:concept-element-attributes-remove-by-namespace]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = document->CreateElement(u8"div").ReleaseValue();
    REQUIRE_FALSE(element->SetAttribute(u8"a", u8"1").HasException());
    REQUIRE_FALSE(element->SetAttributeNS(u8"urn:x", u8"x:b", u8"2").HasException());

    CHECK(ElementAlgorithms::RemoveAttributeByName(u8"c", *element) == nullptr);
    auto removed = ElementAlgorithms::RemoveAttributeByName(u8"A", *element);
    REQUIRE(removed != nullptr);
    CHECK(removed->Value() == u8"1");
    CHECK_FALSE(element->HasAttribute(u8"a"));

    CHECK(ElementAlgorithms::RemoveAttributeByNamespace(DOMStringAtom::Null(), u8"b", *element) == nullptr);
    removed = ElementAlgorithms::RemoveAttributeByNamespace(u8"urn:x", u8"b", *element);
    REQUIRE(removed != nullptr);
    CHECK(removed->Value() == u8"2");
    CHECK_FALSE(element->HasAttributes());
  }

  TEST_CASE("Attach a shadow root: only to HTML elements with a valid shadow host name",
            "[DOM][ElementAlgorithms][dom:concept-attach-a-shadow-root][dom:valid-shadow-host-name]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto attach = [](Element &element)
    {
      return ElementAlgorithms::AttachShadowRoot(element, ShadowRootMode::Open, Clonable(false),
                                                 Serialisable(false), DelegatesFocus(false),
                                                 SlotAssignmentMode::Named, null, nullptr);
    };
    auto requireNotSupported = [](const ExceptionOr<void> &result)
    {
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
    };

    requireNotSupported(
      attach(*document->CreateElementNS(DOMStringAtom(infra::Namespaces::SVG), u8"div").ReleaseValue()));
    requireNotSupported(attach(*document->CreateElement(u8"img").ReleaseValue()));
    requireNotSupported(attach(*document->CreateElement(u8"input").ReleaseValue()));

    for (auto name : {u8"article", u8"aside", u8"blockquote", u8"body", u8"div", u8"footer", u8"h1", u8"h6",
                      u8"header", u8"main", u8"nav", u8"p", u8"section", u8"span", u8"x-custom"})
    {
      auto element = document->CreateElement(DOMStringAtom(name)).ReleaseValue();
      CHECK_FALSE(attach(*element).HasException());
      CHECK(element->ShadowRoot() != nullptr);
    }
  }

  TEST_CASE("Attach a shadow root: the new shadow root has the options it's attached with",
            "[DOM][ElementAlgorithms][dom:concept-attach-a-shadow-root]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = document->CreateElement(u8"div").ReleaseValue();
    auto registry = html::CustomElementRegistry::CreateScoped();

    REQUIRE_FALSE(ElementAlgorithms::AttachShadowRoot(
                    *host, ShadowRootMode::Open, Clonable(true), Serialisable(true), DelegatesFocus(true),
                    SlotAssignmentMode::Manual, DOMString(u8"inner"), registry.get())
                    .HasException());

    auto shadow = host->ShadowRoot();
    REQUIRE(shadow != nullptr);
    CHECK(shadow->Host() == host.get());
    CHECK(&shadow->NodeDocument() == document.get());
    CHECK(shadow->Mode() == ShadowRootMode::Open);
    CHECK(shadow->DelegatesFocus());
    CHECK(shadow->Clonable());
    CHECK(shadow->Serialisable());
    CHECK(shadow->SlotAssignment() == SlotAssignmentMode::Manual);
    CHECK(shadow->ReferenceTarget() == DOMString(u8"inner"));
    CHECK(shadow->CustomElementRegistry() == registry);
    CHECK_FALSE(shadow->IsAvailableToElementInternals());
    CHECK_FALSE(shadow->HasChildNodes());

    // A second attach throws, as the shadow root isn't declarative.
    auto again = ElementAlgorithms::AttachShadowRoot(*host, ShadowRootMode::Open, Clonable(true),
                                                     Serialisable(true), DelegatesFocus(true),
                                                     SlotAssignmentMode::Manual, null, registry.get());
    REQUIRE(again.HasException());
    CHECK(again.GetException().Code() == ExceptionCode::NotSupportedError);
  }

  TEST_CASE("Attach a shadow root: a declarative one of another mode isn't reused",
            "[DOM][ElementAlgorithms][dom:concept-attach-a-shadow-root]")
  {
    auto context = CreateRef<HTMLDocument>();
    auto document =
      Document::ParseHTMLUnsafe(*context, u8"<!DOCTYPE html><div id=host><template "
                                          u8"shadowrootmode=closed><p>shadow</p></template></div>")
        .ReleaseValue();
    auto host = document->GetElementById(u8"host");
    REQUIRE(host != nullptr);

    auto result = host->AttachShadow({.Mode = ShadowRootMode::Open});
    REQUIRE(result.HasException());
    CHECK(result.GetException().Code() == ExceptionCode::NotSupportedError);
  }

  TEST_CASE("Insert adjacent: pre-inserts the node around or inside the element",
            "[DOM][ElementAlgorithms][dom:insert-adjacent]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto parent = document->CreateElement(u8"div").ReleaseValue();
    auto element = document->CreateElement(u8"p").ReleaseValue();
    REQUIRE_FALSE(parent->AppendChild(*element).HasException());
    REQUIRE_FALSE(element->AppendChild(*document->CreateTextNode(u8"child")).HasException());

    auto insert = [&](InsertAdjacentWhere where, DOMStringAtom name)
    {
      auto node = document->CreateElement(name).ReleaseValue();
      auto result = ElementAlgorithms::InsertAdjacent(*element, where, *node);
      REQUIRE_FALSE(result.HasException());
      CHECK(result.Value() == node.get());
    };
    insert(InsertAdjacentWhere::BeforeBegin, u8"a");
    insert(InsertAdjacentWhere::AfterBegin, u8"b");
    insert(InsertAdjacentWhere::BeforeEnd, u8"i");
    insert(InsertAdjacentWhere::AfterEnd, u8"s");

    CHECK(parent->InnerHTML().ReleaseValue() == u8"<a></a><p><b></b>child<i></i></p><s></s>");

    SECTION("Before or after an element without a parent gives null")
    {
      auto orphan = document->CreateElement(u8"p").ReleaseValue();
      auto node = document->CreateElement(u8"a").ReleaseValue();
      for (auto where : {InsertAdjacentWhere::BeforeBegin, InsertAdjacentWhere::AfterEnd})
      {
        auto result = ElementAlgorithms::InsertAdjacent(*orphan, where, *node);
        REQUIRE_FALSE(result.HasException());
        CHECK(result.Value() == nullptr);
      }
      CHECK(node->ParentNode() == nullptr);
    }

    SECTION("Pre-insert's exceptions are thrown")
    {
      auto result = ElementAlgorithms::InsertAdjacent(*element, InsertAdjacentWhere::BeforeEnd, *parent);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::HierarchyRequestError);
    }

    SECTION("Any other value throws a SyntaxError")
    {
      auto node = document->CreateElement(u8"a").ReleaseValue();
      auto result = ElementAlgorithms::InsertAdjacent(*element, static_cast<InsertAdjacentWhere>(4), *node);
      REQUIRE(result.HasException());
      CHECK(result.GetException().Code() == ExceptionCode::SyntaxError);
    }
  }
}
