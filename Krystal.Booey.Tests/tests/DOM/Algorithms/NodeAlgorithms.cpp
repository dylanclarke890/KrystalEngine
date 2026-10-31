#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "catch2/catch_test_macros.hpp"
#include "catch2/generators/catch_generators.hpp"
#include "Krystal.Booey.Tests/DOM/TestElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/MutationObserver.hpp"
#include "Krystal.Booey/DOM/MutationRecord.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/XMLDocument.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"

namespace krys::boo::dom::tests
{
  namespace
  {
    KRYS_NODISCARD Ref<Element> CreateElement(Document &document, DOMStringAtom name) noexcept
    {
      return document.CreateElement(name).ReleaseValue();
    }

    KRYS_NODISCARD Ref<Element> CreateElementNS(Document &document, DOMStringAtom namespaceURI,
                                                DOMStringAtom qualifiedName) noexcept
    {
      return document.CreateElementNS(namespaceURI, qualifiedName).ReleaseValue();
    }

    void Append(Node &parent, Node &child) noexcept
    {
      REQUIRE_FALSE(parent.AppendChild(child).HasException());
    }

    void SetAttribute(Element &element, DOMStringAtom name, DOMString &&value) noexcept
    {
      REQUIRE_FALSE(element.SetAttribute(name, krys::move(value)).HasException());
    }

    void SetAttributeNS(Element &element, DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                        DOMString &&value) noexcept
    {
      REQUIRE_FALSE(element.SetAttributeNS(namespaceURI, qualifiedName, krys::move(value)).HasException());
    }

    KRYS_NODISCARD List<const Element *> Elements(HTMLCollection &collection) noexcept
    {
      List<const Element *> elements;
      for (size_t i = 0; i < collection.Length(); ++i)
      {
        elements.push_back(collection.Item(i).get());
      }
      return elements;
    }
  }

  TEST_CASE("String replace all: the children become one text node, or none for the empty string",
            "[DOM][NodeAlgorithms][dom:string-replace-all]")
  {
    auto &loop = html::EventLoop::Current();
    loop.RunUntilIdle();
    auto document = CreateRef<HTMLDocument>();
    auto parent = CreateElement(*document, u8"div");
    Append(*parent, *CreateElement(*document, u8"p"));
    Append(*parent, *document->CreateTextNode(u8"old"));
    auto observer = CreateRef<MutationObserver>([](const List<Ref<MutationRecord>> &, MutationObserver &) {});
    REQUIRE_FALSE(observer->Observe(*parent, {.ChildList = true}).HasException());

    SECTION("a string")
    {
      REQUIRE_FALSE(NodeAlgorithms::StringReplaceAll(u8"new", *parent).HasException());
      REQUIRE(parent->FirstChild() != nullptr);
      CHECK(parent->FirstChild() == parent->LastChild());
      REQUIRE(Is<Text>(parent->FirstChild()));
      CHECK(Downcast<Text>(parent->FirstChild())->Data() == u8"new");

      // One record, for the removals and the insertion together.
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->RemovedNodes()->Length() == 2u);
      CHECK(records[0]->AddedNodes()->Length() == 1u);
    }

    SECTION("the empty string")
    {
      REQUIRE_FALSE(NodeAlgorithms::StringReplaceAll(u8"", *parent).HasException());
      CHECK_FALSE(parent->HasChildNodes());
      auto records = observer->TakeRecords();
      REQUIRE(records.size() == 1);
      CHECK(records[0]->AddedNodes()->Length() == 0u);
    }
  }

  TEST_CASE(
    "Clone a node: the subtree is cloned only when asked, into the given document and parent",
    "[DOM][NodeAlgorithms][dom:concept-node-clone][dom:clone-a-node-document][dom:clone-a-node-subtree]"
    "[dom:clone-a-node-parent]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto element = CreateElement(*document, u8"div");
    auto child = CreateElement(*document, u8"p");
    Append(*child, *document->CreateTextNode(u8"text"));
    Append(*element, *child);

    SECTION("shallow")
    {
      auto copy = NodeAlgorithms::CloneNode(*element).ReleaseValue();
      CHECK(&copy->NodeDocument() == document.get());
      CHECK_FALSE(copy->HasChildNodes());
    }

    SECTION("deep")
    {
      auto copy = NodeAlgorithms::CloneNode(*element, nullptr, true).ReleaseValue();
      REQUIRE(copy->FirstChild() != nullptr);
      CHECK(copy->FirstChild() != child.get());
      CHECK(copy->IsEqualNode(element.get()));
    }

    SECTION("into another document, appended to a parent")
    {
      auto other = CreateRef<HTMLDocument>();
      auto parent = CreateElement(*other, u8"section");
      auto copy = NodeAlgorithms::CloneNode(*element, other.get(), true, parent.get()).ReleaseValue();
      CHECK(&copy->NodeDocument() == other.get());
      CHECK(&copy->FirstChild()->FirstChild()->NodeDocument() == other.get());
      CHECK(copy->ParentNode() == parent.get());
    }
  }

  TEST_CASE("Clone a node: a clonable shadow root is cloned with all its children, even for a shallow clone",
            "[DOM][NodeAlgorithms][dom:concept-node-clone]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto host = CreateElement(*document, u8"div");
    Append(*host, *CreateElement(*document, u8"span"));

    SECTION("clonable")
    {
      auto mode = GENERATE(ShadowRootMode::Open, ShadowRootMode::Closed);
      auto shadow = host
                      ->AttachShadow({.Mode = mode,
                                      .DelegatesFocus = DelegatesFocus(true),
                                      .SlotAssignment = SlotAssignmentMode::Manual,
                                      .Clonable = Clonable(true),
                                      .Serialisable = Serialisable(true)})
                      .ReleaseValue();
      auto inner = CreateElement(*document, u8"p");
      Append(*inner, *document->CreateTextNode(u8"deep"));
      Append(*shadow, *inner);

      auto copyNode = NodeAlgorithms::CloneNode(*host).ReleaseValue();
      auto *copy = Downcast<Element>(copyNode.get());
      CHECK_FALSE(copy->HasChildNodes());
      auto *copyShadow = TreeQueries::GetShadowRoot(*copy);
      REQUIRE(copyShadow != nullptr);
      CHECK(copyShadow->Mode() == mode);
      CHECK(copyShadow->DelegatesFocus());
      CHECK(copyShadow->SlotAssignment() == SlotAssignmentMode::Manual);
      CHECK(copyShadow->Clonable());
      CHECK(copyShadow->Serialisable());

      // The shadow tree is cloned in full.
      REQUIRE(copyShadow->FirstChild() != nullptr);
      CHECK(copyShadow->FirstChild()->IsEqualNode(inner.get()));
    }

    SECTION("not clonable")
    {
      (void)host->AttachShadow({}).ReleaseValue();
      auto copyNode = NodeAlgorithms::CloneNode(*host, nullptr, true).ReleaseValue();
      auto *copy = Downcast<Element>(copyNode.get());
      CHECK(TreeQueries::GetShadowRoot(*copy) == nullptr);
    }
  }

  TEST_CASE("Clone a single node: the copy has the node's interface and its data",
            "[DOM][NodeAlgorithms][dom:clone-a-single-node]")
  {
    auto document = CreateRef<HTMLDocument>();

    SECTION("Element, with its attributes")
    {
      auto element = CreateElementNS(*document, u8"urn:x", u8"p:e");
      SetAttribute(*element, u8"a", u8"1");
      SetAttributeNS(*element, infra::Namespaces::XML, u8"xml:lang", u8"en");

      auto copyNode = NodeAlgorithms::CloneSingleNode(*element, *document);
      auto *copy = Downcast<Element>(copyNode.get());
      CHECK(copy->NamespaceURI() == u8"urn:x");
      CHECK(copy->Prefix() == u8"p");
      CHECK(copy->LocalName() == u8"e");
      CHECK(copy->GetAttribute(u8"a") == u8"1");
      CHECK(copy->GetAttributeNS(infra::Namespaces::XML, u8"lang") == u8"en");
      CHECK(copy->IsEqualNode(element.get()));
    }

    SECTION("Document")
    {
      auto source = document->Implementation().CreateHTMLDocument(u8"title").ReleaseValue();
      auto copy = NodeAlgorithms::CloneNode(*source, nullptr, true).ReleaseValue();
      auto &documentCopy = Downcast<Document>(*copy);
      REQUIRE(Is<HTMLDocument>(documentCopy));
      CHECK(&documentCopy.NodeDocument() == &documentCopy);
      CHECK(documentCopy.ContentType() == source->ContentType());
      CHECK(documentCopy.URL() == source->URL());
      CHECK(documentCopy.CompatMode() == source->CompatMode());
      CHECK(documentCopy.IsEqualNode(source.get()));
      CHECK(&documentCopy.DocumentElement()->NodeDocument() == &documentCopy);
    }

    SECTION("DocumentType")
    {
      auto doctype = *document->Implementation().CreateDocumentType(u8"html", u8"public-id", u8"system-id");
      auto copyNode = NodeAlgorithms::CloneSingleNode(*doctype, *document);
      auto &copy = Downcast<DocumentType>(*copyNode);
      CHECK(copy.Name() == u8"html");
      CHECK(copy.PublicId() == u8"public-id");
      CHECK(copy.SystemId() == u8"system-id");
    }

    SECTION("Attr")
    {
      auto attribute = document->CreateAttributeNS(infra::Namespaces::XML, u8"xml:name").ReleaseValue();
      attribute->Value(u8"value");
      auto copyNode = NodeAlgorithms::CloneSingleNode(*attribute, *document);
      auto &copy = Downcast<Attr>(*copyNode);
      CHECK(copy.NamespaceURI() == infra::Namespaces::XML);
      CHECK(copy.LocalName() == u8"name");
      CHECK(copy.Prefix() == u8"xml");
      CHECK(copy.Value() == u8"value");
    }

    SECTION("Text, CDATASection and Comment")
    {
      auto text = document->CreateTextNode(u8"text");
      auto cdata = CreateRef<CDATASection>(*document, u8"cdata");
      auto comment = document->CreateComment(u8"comment");

      auto textCopy = NodeAlgorithms::CloneSingleNode(*text, *document);
      CHECK(textCopy->NodeType() == NodeType::TEXT_NODE);
      CHECK(Downcast<Text>(*textCopy).Data() == u8"text");

      auto cdataCopy = NodeAlgorithms::CloneSingleNode(*cdata, *document);
      CHECK(cdataCopy->NodeType() == NodeType::CDATA_SECTION_NODE);
      CHECK(Is<CDATASection>(cdataCopy.get()));
      CHECK(Downcast<Text>(*cdataCopy).Data() == u8"cdata");

      auto commentCopy = NodeAlgorithms::CloneSingleNode(*comment, *document);
      CHECK(Downcast<Comment>(*commentCopy).Data() == u8"comment");
    }

    SECTION("ProcessingInstruction")
    {
      auto instruction = document->CreateProcessingInstruction(u8"target", u8"data").ReleaseValue();
      auto copyNode = NodeAlgorithms::CloneSingleNode(*instruction, *document);
      auto &copy = Downcast<ProcessingInstruction>(*copyNode);
      CHECK(copy.Target() == u8"target");
      CHECK(copy.Data() == u8"data");
    }

    SECTION("DocumentFragment")
    {
      auto fragment = document->CreateDocumentFragment();
      Append(*fragment, *document->CreateTextNode(u8"text"));
      auto copy = NodeAlgorithms::CloneNode(*fragment, nullptr, true).ReleaseValue();
      CHECK(copy->NodeType() == NodeType::DOCUMENT_FRAGMENT_NODE);
      CHECK(copy->IsEqualNode(fragment.get()));
    }

    SECTION("into another document")
    {
      auto other = CreateRef<XMLDocument>();
      auto copy = NodeAlgorithms::CloneSingleNode(*document->CreateTextNode(u8"text"), *other);
      CHECK(&copy->NodeDocument() == other.get());
    }
  }

  TEST_CASE("Equals: the same interface and data, with equal attributes and children",
            "[DOM][NodeAlgorithms][dom:concept-node-equals][dom:dom-node-isequalnode]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto create = [&]
    {
      return CreateElement(*document, u8"div");
    };

    SECTION("Attributes in a different order")
    {
      auto a = create();
      auto b = create();
      SetAttribute(*a, u8"x", u8"1");
      SetAttribute(*a, u8"y", u8"2");
      SetAttribute(*b, u8"y", u8"2");
      SetAttribute(*b, u8"x", u8"1");
      CHECK(NodeAlgorithms::Equals(*a, *b));
      CHECK(NodeAlgorithms::Equals(*b, *a));
    }

    SECTION("An attribute with a different value")
    {
      auto a = create();
      auto b = create();
      SetAttribute(*a, u8"x", u8"1");
      SetAttribute(*b, u8"x", u8"2");
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *b));
    }

    SECTION("An extra attribute")
    {
      auto a = create();
      auto b = create();
      SetAttribute(*a, u8"x", u8"1");
      SetAttribute(*b, u8"x", u8"1");
      SetAttribute(*b, u8"y", u8"2");
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *b));
      CHECK_FALSE(NodeAlgorithms::Equals(*b, *a));
    }

    SECTION("An attribute in another namespace")
    {
      auto a = create();
      auto b = create();
      SetAttribute(*a, u8"x", u8"1");
      SetAttributeNS(*b, u8"urn:x", u8"p:x", u8"1");
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *b));
      CHECK_FALSE(NodeAlgorithms::Equals(*b, *a));
    }

    SECTION("Elements with another namespace, prefix or local name")
    {
      auto a = CreateElementNS(*document, u8"urn:x", u8"p:e");
      CHECK(NodeAlgorithms::Equals(*a, *CreateElementNS(*document, u8"urn:x", u8"p:e")));
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *CreateElementNS(*document, u8"urn:y", u8"p:e")));
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *CreateElementNS(*document, u8"urn:x", u8"q:e")));
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *CreateElementNS(*document, u8"urn:x", u8"p:f")));
    }

    SECTION("An extra child")
    {
      auto a = create();
      auto b = create();
      Append(*a, *create());
      Append(*b, *create());
      Append(*a, *document->CreateTextNode(u8"t"));
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *b));
      CHECK_FALSE(NodeAlgorithms::Equals(*b, *a));
    }

    SECTION("Children's attributes")
    {
      auto a = create();
      auto b = create();
      auto aChild = create();
      auto bChild = create();
      SetAttribute(*aChild, u8"x", u8"1");
      SetAttribute(*bChild, u8"x", u8"1");
      Append(*a, *aChild);
      Append(*b, *bChild);
      CHECK(NodeAlgorithms::Equals(*a, *b));
      SetAttribute(*bChild, u8"x", u8"2");
      CHECK_FALSE(NodeAlgorithms::Equals(*a, *b));
    }

    SECTION("Other interfaces compare their own data")
    {
      auto &implementation = document->Implementation();
      CHECK(NodeAlgorithms::Equals(*implementation.CreateDocumentType(u8"a", u8"b", u8"c").ReleaseValue(),
                                   *implementation.CreateDocumentType(u8"a", u8"b", u8"c").ReleaseValue()));
      CHECK_FALSE(
        NodeAlgorithms::Equals(*implementation.CreateDocumentType(u8"a", u8"b", u8"c").ReleaseValue(),
                               *implementation.CreateDocumentType(u8"a", u8"b", u8"d").ReleaseValue()));

      auto attribute = [&](DOMString &&value)
      {
        auto attr = document->CreateAttribute(u8"a").ReleaseValue();
        attr->Value(krys::move(value));
        return attr;
      };
      CHECK(NodeAlgorithms::Equals(*attribute(u8"1"), *attribute(u8"1")));
      CHECK_FALSE(NodeAlgorithms::Equals(*attribute(u8"1"), *attribute(u8"2")));

      auto instruction = [&](DOMString &&target, DOMString &&data)
      {
        return document->CreateProcessingInstruction(krys::move(target), krys::move(data)).ReleaseValue();
      };
      CHECK(NodeAlgorithms::Equals(*instruction(u8"t", u8"d"), *instruction(u8"t", u8"d")));
      CHECK_FALSE(NodeAlgorithms::Equals(*instruction(u8"t", u8"d"), *instruction(u8"u", u8"d")));
      CHECK_FALSE(NodeAlgorithms::Equals(*instruction(u8"t", u8"d"), *instruction(u8"t", u8"e")));

      CHECK(NodeAlgorithms::Equals(*document->CreateTextNode(u8"a"), *document->CreateTextNode(u8"a")));
      CHECK_FALSE(NodeAlgorithms::Equals(*document->CreateTextNode(u8"a"), *document->CreateTextNode(u8"b")));
      CHECK_FALSE(NodeAlgorithms::Equals(*document->CreateTextNode(u8"a"), *document->CreateComment(u8"a")));
      CHECK_FALSE(
        NodeAlgorithms::Equals(*document->CreateTextNode(u8"a"), *CreateRef<CDATASection>(*document, u8"a")));
    }
  }

  TEST_CASE(
    "Locate a namespace prefix: the element's own prefix, an xmlns attribute's name, or an ancestor's",
    "[DOM][NodeAlgorithms][dom:locate-a-namespace-prefix]")
  {
    auto document = CreateRef<XMLDocument>();
    auto parent = CreateElementNS(*document, u8"urn:parent", u8"p:parent");
    auto element = CreateElementNS(*document, u8"urn:element", u8"e:element");
    Append(*parent, *element);

    // 1. Its own namespace prefix.
    CHECK(NodeAlgorithms::LocateNamespacePrefix(*element, u8"urn:element") == u8"e");

    // 2. The first xmlns attribute declaring the namespace.
    SetAttributeNS(*element, infra::Namespaces::XMLNS, u8"xmlns:x", u8"urn:declared");
    SetAttributeNS(*element, infra::Namespaces::XMLNS, u8"xmlns:y", u8"urn:declared");
    CHECK(NodeAlgorithms::LocateNamespacePrefix(*element, u8"urn:declared") == u8"x");

    // 3. The parent element's.
    CHECK(NodeAlgorithms::LocateNamespacePrefix(*element, u8"urn:parent") == u8"p");

    // 4. Null.
    CHECK(NodeAlgorithms::LocateNamespacePrefix(*element, u8"urn:other") == DOMStringAtom::Null());

    // An element without a prefix doesn't give one.
    auto unprefixed = CreateElementNS(*document, u8"urn:element", u8"element");
    CHECK(NodeAlgorithms::LocateNamespacePrefix(*unprefixed, u8"urn:element") == DOMStringAtom::Null());
  }

  TEST_CASE("Locate a namespace: per interface, from the element, its xmlns attributes and its ancestors",
            "[DOM][NodeAlgorithms][dom:locate-a-namespace][dom:dom-node-lookupnamespaceuri]"
            "[dom:dom-node-isdefaultnamespace]")
  {
    auto document = CreateRef<XMLDocument>();
    auto parent = CreateElementNS(*document, u8"urn:parent", u8"p:parent");
    auto element = CreateElementNS(*document, u8"urn:element", u8"e:element");
    Append(*parent, *element);

    SECTION("Element")
    {
      // 1. and 2. The reserved prefixes.
      CHECK(NodeAlgorithms::LocateNamespace(*element, u8"xml") == infra::Namespaces::XML);
      CHECK(NodeAlgorithms::LocateNamespace(*element, u8"xmlns") == infra::Namespaces::XMLNS);

      // 3. Its own prefix.
      CHECK(NodeAlgorithms::LocateNamespace(*element, u8"e") == u8"urn:element");

      // 4. An xmlns attribute declaring the prefix, or the default namespace; an empty one undeclares it.
      SetAttributeNS(*element, infra::Namespaces::XMLNS, u8"xmlns:x", u8"urn:x");
      SetAttributeNS(*element, infra::Namespaces::XMLNS, u8"xmlns", u8"urn:default");
      SetAttributeNS(*element, infra::Namespaces::XMLNS, u8"xmlns:p", u8"");
      CHECK(NodeAlgorithms::LocateNamespace(*element, u8"x") == u8"urn:x");
      CHECK(NodeAlgorithms::LocateNamespace(*element, DOMStringAtom::Null()) == u8"urn:default");
      CHECK(NodeAlgorithms::LocateNamespace(*element, u8"p") == DOMStringAtom::Null());
      CHECK(element->LookupNamespaceURI(DOMStringAtom::Empty()) == u8"urn:default");
      CHECK(element->IsDefaultNamespace(u8"urn:default"));
      CHECK_FALSE(element->IsDefaultNamespace(DOMStringAtom::Empty()));

      // 5. and 6. The parent element's, or null.
      auto child = CreateElementNS(*document, u8"urn:child", u8"child");
      Append(*element, *child);
      CHECK(NodeAlgorithms::LocateNamespace(*child, u8"x") == u8"urn:x");
      CHECK(NodeAlgorithms::LocateNamespace(*child, u8"other") == DOMStringAtom::Null());
      CHECK(NodeAlgorithms::LocateNamespace(*child, DOMStringAtom::Null()) == u8"urn:child");
    }

    SECTION("Document")
    {
      CHECK(NodeAlgorithms::LocateNamespace(*document, u8"p") == DOMStringAtom::Null());
      Append(*document, *parent);
      CHECK(NodeAlgorithms::LocateNamespace(*document, u8"p") == u8"urn:parent");
    }

    SECTION("DocumentType and DocumentFragment")
    {
      auto doctype = document->Implementation().CreateDocumentType(u8"test", u8"", u8"").ReleaseValue();
      CHECK(NodeAlgorithms::LocateNamespace(*doctype, u8"xml") == DOMStringAtom::Null());
      auto fragment = document->CreateDocumentFragment();
      Append(*fragment, *parent);
      CHECK(NodeAlgorithms::LocateNamespace(*fragment, u8"p") == DOMStringAtom::Null());
    }

    SECTION("Attr")
    {
      auto attribute = document->CreateAttribute(u8"a").ReleaseValue();
      CHECK(NodeAlgorithms::LocateNamespace(*attribute, u8"e") == DOMStringAtom::Null());
      REQUIRE_FALSE(element->SetAttributeNode(*attribute).HasException());
      CHECK(NodeAlgorithms::LocateNamespace(*attribute, u8"e") == u8"urn:element");
    }

    SECTION("Otherwise")
    {
      auto text = document->CreateTextNode(u8"text");
      CHECK(NodeAlgorithms::LocateNamespace(*text, u8"e") == DOMStringAtom::Null());
      Append(*element, *text);
      CHECK(NodeAlgorithms::LocateNamespace(*text, u8"e") == u8"urn:element");
    }
  }

  TEST_CASE("List of elements with qualified name: every descendant for *, else by qualified name",
            "[DOM][NodeAlgorithms][dom:concept-getelementsbytagname]")
  {
    SECTION("in an HTML document, HTML elements match the name in ASCII lowercase")
    {
      auto document = CreateRef<HTMLDocument>();
      auto root = CreateElement(*document, u8"div");
      auto html = CreateElement(*document, u8"p");
      auto svg = CreateElementNS(*document, infra::Namespaces::SVG, u8"P");
      auto prefixed = CreateElementNS(*document, u8"urn:x", u8"x:p");
      Append(*root, *html);
      Append(*root, *svg);
      Append(*root, *prefixed);

      CHECK(Elements(*NodeAlgorithms::GetElementsByTagName(*root, u8"*"))
            == List<const Element *> {html.get(), svg.get(), prefixed.get()});
      CHECK(Elements(*NodeAlgorithms::GetElementsByTagName(*root, u8"P"))
            == List<const Element *> {html.get(), svg.get()});
      CHECK(Elements(*NodeAlgorithms::GetElementsByTagName(*root, u8"p"))
            == List<const Element *> {html.get()});
      CHECK(Elements(*NodeAlgorithms::GetElementsByTagName(*root, u8"x:p"))
            == List<const Element *> {prefixed.get()});
    }

    SECTION("elsewhere, by qualified name exactly")
    {
      auto document = CreateRef<XMLDocument>();
      auto root = CreateElementNS(*document, infra::Namespaces::HTML, u8"div");
      auto lower = CreateElementNS(*document, infra::Namespaces::HTML, u8"p");
      auto upper = CreateElementNS(*document, infra::Namespaces::HTML, u8"P");
      Append(*root, *lower);
      Append(*root, *upper);

      CHECK(Elements(*NodeAlgorithms::GetElementsByTagName(*root, u8"P"))
            == List<const Element *> {upper.get()});
    }

    SECTION("live, and never the root")
    {
      auto document = CreateRef<HTMLDocument>();
      auto root = CreateElement(*document, u8"p");
      auto elements = NodeAlgorithms::GetElementsByTagName(*root, u8"p");
      CHECK(elements->Length() == 0u);
      auto child = CreateElement(*document, u8"p");
      Append(*root, *child);
      CHECK(Elements(*elements) == List<const Element *> {child.get()});
      REQUIRE_FALSE(root->RemoveChild(*child).HasException());
      CHECK(elements->Length() == 0u);
    }
  }

  TEST_CASE(
    "List of elements with namespace and local name: either may be *, and the empty namespace is null",
    "[DOM][NodeAlgorithms][dom:concept-getelementsbytagnamens]")
  {
    auto document = CreateRef<XMLDocument>();
    auto root = CreateElementNS(*document, u8"urn:x", u8"root");
    auto a = CreateElementNS(*document, u8"urn:x", u8"a");
    auto b = CreateElementNS(*document, u8"urn:x", u8"x:b");
    auto noNamespace = CreateElementNS(*document, DOMStringAtom::Null(), u8"a");
    Append(*root, *a);
    Append(*root, *b);
    Append(*root, *noNamespace);

    CHECK(Elements(*NodeAlgorithms::GetElementsByTagNameNS(*root, u8"*", u8"*"))
          == List<const Element *> {a.get(), b.get(), noNamespace.get()});
    CHECK(Elements(*NodeAlgorithms::GetElementsByTagNameNS(*root, u8"*", u8"a"))
          == List<const Element *> {a.get(), noNamespace.get()});
    CHECK(Elements(*NodeAlgorithms::GetElementsByTagNameNS(*root, u8"urn:x", u8"*"))
          == List<const Element *> {a.get(), b.get()});
    CHECK(Elements(*NodeAlgorithms::GetElementsByTagNameNS(*root, u8"urn:x", u8"b"))
          == List<const Element *> {b.get()});
    CHECK(Elements(*NodeAlgorithms::GetElementsByTagNameNS(*root, DOMStringAtom::Empty(), u8"a"))
          == List<const Element *> {noNamespace.get()});
    CHECK(Elements(*NodeAlgorithms::GetElementsByTagNameNS(*root, DOMStringAtom::Null(), u8"*"))
          == List<const Element *> {noNamespace.get()});
  }

  TEST_CASE(
    "List of elements with class names: elements with all the classes, case-insensitively in quirks mode",
    "[DOM][NodeAlgorithms][dom:concept-getelementsbyclassname]")
  {
    auto build = [](Document &document)
    {
      auto root = CreateElement(document, u8"div");
      for (auto *classes : {u8"foo", u8"foo bar", u8"bar", u8"FOO"})
      {
        auto element = CreateElement(document, u8"p");
        SetAttribute(*element, u8"class", classes);
        Append(*root, *element);
      }
      return root;
    };
    auto children = [](Element &root)
    {
      List<const Element *> elements;
      for (auto *child = root.FirstChild(); child; child = child->NextSibling())
      {
        elements.push_back(Downcast<Element>(child));
      }
      return elements;
    };

    SECTION("no-quirks: identical to")
    {
      auto document = CreateRef<HTMLDocument>();
      auto root = build(*document);
      auto all = children(*root);
      CHECK(Elements(*NodeAlgorithms::GetElementsByClassName(*root, u8"foo"))
            == List<const Element *> {all[0], all[1]});
      CHECK(Elements(*NodeAlgorithms::GetElementsByClassName(*root, u8" bar\tfoo "))
            == List<const Element *> {all[1]});
      CHECK(Elements(*NodeAlgorithms::GetElementsByClassName(*root, u8"FOO"))
            == List<const Element *> {all[3]});
      CHECK(NodeAlgorithms::GetElementsByClassName(*root, u8"baz")->Length() == 0u);

      // 2. No classes, no elements.
      CHECK(NodeAlgorithms::GetElementsByClassName(*root, u8" \t")->Length() == 0u);

      // Live.
      auto foo = NodeAlgorithms::GetElementsByClassName(*root, u8"foo");
      SetAttribute(const_cast<Element &>(*all[2]), u8"class", u8"bar foo");
      CHECK(Elements(*foo) == List<const Element *> {all[0], all[1], all[2]});
    }

    SECTION("quirks: ASCII case-insensitive")
    {
      auto context = CreateRef<HTMLDocument>();
      auto document = Document::ParseHTMLUnsafe(*context, u8"<body>").ReleaseValue();
      REQUIRE(document->CompatMode() == u8"BackCompat");
      auto root = build(*document);
      auto all = children(*root);
      CHECK(Elements(*NodeAlgorithms::GetElementsByClassName(*root, u8"Foo"))
            == List<const Element *> {all[0], all[1], all[3]});
    }
  }

  TEST_CASE("Clone a node: an element without a registry is given the fallback registry",
            "[DOM][NodeAlgorithms][dom:concept-node-clone][dom:clone-a-node-fallbackregistry]"
            "[dom:clone-a-single-node]")
  {
    auto document = CreateRef<HTMLDocument>();
    auto scoped = html::CustomElementRegistry::CreateScoped();
    auto withoutRegistry = CreateRef<TestElement>(*document);
    REQUIRE(withoutRegistry->CustomElementRegistry() == nullptr);

    auto copy =
      NodeAlgorithms::CloneNode(*withoutRegistry, nullptr, false, nullptr, scoped.get()).ReleaseValue();
    CHECK(Downcast<Element>(*copy).CustomElementRegistry().get() == scoped.get());

    // An element with a registry keeps it.
    auto registry = html::CustomElementRegistry::CreateScoped();
    auto withRegistry =
      document->CreateElement(u8"div", ElementCreationOptions {.CustomElementRegistry = registry})
        .ReleaseValue();
    REQUIRE(withRegistry->CustomElementRegistry().get() == registry.get());
    auto other =
      NodeAlgorithms::CloneNode(*withRegistry, nullptr, false, nullptr, scoped.get()).ReleaseValue();
    CHECK(Downcast<Element>(*other).CustomElementRegistry().get() == registry.get());
  }
}
