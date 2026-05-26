#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/HTMLCollectionAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.HTML/Algorithms/SubtreeRanges.hpp"
#include "Krystal.HTML/Algorithms/TreeTraversal.hpp"
#include "Krystal.HTML/Constants/Namespaces.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/DOM/Algorithms/OrderedSet.hpp"
#include "Krystal.HTML/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Event/Event.hpp"
#include "Krystal.HTML/Factories/ElementFactory.hpp"
#include "Krystal.HTML/HTMLElement/HTMLBodyElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLHeadElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLHtmlElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/HTMLElement/HTMLTitleElement.hpp"
#include "Krystal.HTML/Mixins/NonElementParentNode.hpp"
#include "Krystal.HTML/Mixins/ParentNode.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CDATASection.hpp"
#include "Krystal.HTML/Node/Comment.hpp"
#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/DOMImplementation.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/HTMLCollection.hpp"
#include "Krystal.HTML/Node/HTMLDocument.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ProcessingInstruction.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/Node/Text.hpp"
#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.Text/ASCII.hpp"
#include <cassert>

namespace Krys::HTML
{
  Document::Document(Type documentType) noexcept : Document()
  {
    _documentType = documentType;

    if (documentType == Type::HTML)
    {
      _contentType = u8"text/html";
    }
  }

#pragma region Document

  Document::Document() noexcept : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlag::IsContainerNode)
  {
    SetEventTargetFlag(EventTargetFlags::IsConnected);
  }

  DOMImplementation &Document::Implementation() noexcept
  {
    if (_implementation == nullptr)
    {
      _implementation = UniquePtr<DOMImplementation>(new DOMImplementation(*this));
    }

    return *_implementation;
  }

  DOMString Document::URL() const noexcept
  {
    // TODO(impl): URL - return the url, serialized.
    return {};
  }

  DOMString Document::DocumentURI() const noexcept
  {
    return URL();
  }

  DOMString Document::CompatMode() const noexcept
  {
    return _quirksMode == QuirksMode::Quirks ? u8"BackCompat" : u8"CSS1Compat";
  }

  DOMString Document::CharacterSet() const noexcept
  {
    // TODO(impl): MINOR - pass encoding to document
    return u8"UTF-8";
  }

  DOMString Document::Charset() const noexcept
  {
    return CharacterSet();
  }

  DOMString Document::InputEncoding() const noexcept
  {
    return CharacterSet();
  }

  DOMString Document::ContentType() const noexcept
  {
    return _contentType;
  }

  RefPtr<DocumentType> Document::DocType() noexcept
  {
    auto children = ChildNodeRange(*this);
    auto it = FirstOfType<DocumentType>(children);
    return it == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<DocumentType>(&*it));
  }

  RefPtr<const DocumentType> Document::DocType() const noexcept
  {
    auto children = ConstChildNodeRange(*this);
    auto it = FirstOfType<DocumentType>(children);
    return it == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<DocumentType>(&*it));
  }

  RefPtr<Element> Document::DocumentElement() noexcept
  {
    return FirstElementChild();
  }

  RefPtr<const Element> Document::DocumentElement() const noexcept
  {
    return FirstElementChild();
  }

  Ref<HTMLCollection> Document::GetElementsByTagName(DOMStringAtom qualifiedName) noexcept
  {
    return HTMLCollectionAlgorithms::ElementsByTagName(*this, qualifiedName);
  }

  Ref<HTMLCollection> Document::GetElementsByTagNameNS(DOMStringAtom namespaceUri,
                                                       DOMStringAtom localName) noexcept
  {
    return HTMLCollectionAlgorithms::ElementsByTagNameNS(*this, namespaceUri, localName);
  }

  Ref<HTMLCollection> Document::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return HTMLCollectionAlgorithms::ElementsByClassName(*this, classNames);
  }

  ExceptionOr<Ref<Element>> Document::CreateElement(DOMStringAtom localName,
                                                    const ElementCreationOptionsOrString &options) noexcept
  {
    if (!NameValidation::IsValidElementLocalName(localName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (Is<HTMLDocument>(*this))
    {
      localName = ::Krys::Text::ToASCIILowercase(localName.View());
    }

    auto creationOptions = DocumentAlgorithms::FlattenElementCreationOptions(options, *this);
    if (creationOptions.HasException())
    {
      return creationOptions.ReleaseException();
    }

    DOMStringAtom namespaceURI = DOMStringAtom::Null();
    if (Is<HTMLDocument>(*this) || _contentType == u8"application/xhtml+xml")
    {
      namespaceURI = Namespace::HTML;
    }

    return ElementFactory::Create(*this, {namespaceURI, DOMStringAtom::Null(), localName},
                                  creationOptions.Value().Is, true,
                                  creationOptions.Value().CustomElementRegistry);
  }

  ExceptionOr<Ref<Element>> Document::CreateElementNS(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName,
                                                      const ElementCreationOptionsOrString &options) noexcept
  {
    return DocumentAlgorithms::InternalCreateElementNS(*this, namespaceUri, qualifiedName, options);
  }

  Ref<DocumentFragment> Document::CreateDocumentFragment() noexcept
  {
    return CreateRef<DocumentFragment>(*this);
  }

  Ref<Text> Document::CreateTextNode(DOMString &&data) noexcept
  {
    return CreateRef<Text>(*this, Krys::Move(data));
  }

  ExceptionOr<Ref<CDATASection>> Document::CreateCDATASection(DOMString &&data) noexcept
  {
    if (Is<HTMLDocument>(*this))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (data.contains(u8"]]>"))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    return CreateRef<CDATASection>(*this, Krys::Move(data));
  }

  Ref<Comment> Document::CreateComment(DOMString &&data) noexcept
  {
    return CreateRef<Comment>(*this, Krys::Move(data));
  }

  ExceptionOr<Ref<ProcessingInstruction>> Document::CreateProcessingInstruction(DOMString &&target,
                                                                                DOMString &&data) noexcept
  {
    if (!NameValidation::IsValidXMLName(target))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    if (data.contains(u8"?>"))
    {
      return ExceptionCode::InvalidCharacterError;
    }

    return CreateRef<ProcessingInstruction>(*this, Krys::Move(target), Krys::Move(data));
  }

  ExceptionOr<Ref<Node>> Document::ImportNode(Node &node, const BoolOrImportNodeOptions &options) noexcept
  {
    if (Is<Document>(node) || Is<ShadowRoot>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    bool subtree = false;
    RefPtr<HTML::CustomElementRegistry> registry = nullptr;
    if (std::holds_alternative<bool>(options))
    {
      subtree = std::get<bool>(options);
    }
    else
    {
      auto &dict = std::get<ImportNodeOptions>(options);
      subtree = !dict.SelfOnly;
      registry = dict.CustomElementRegistry;

      if (registry != nullptr && !registry->IsScoped() && registry != CustomElementRegistry())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    if (registry == nullptr)
    {
      registry = CustomElementRegistry();
    }

    return NodeAlgorithms::CloneNode(node, this, subtree, nullptr, registry.get());
  }

  ExceptionOr<Ref<Node>> Document::AdoptNode(Node &node) noexcept
  {
    if (Is<Document>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (Is<ShadowRoot>(node)) // ShadowRoot cannot disconnect itself from the host node.
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    if (Is<DocumentFragment>(node))
    {
      auto &documentFragment = Downcast<DocumentFragment>(node);
      // If node is a DocumentFragment node whose host is non-null, then return.
      if (auto host = documentFragment._host.lock())
      {
        return ShareRef(node);
      }
    }

    if (auto adoptResult = DocumentAlgorithms::AdoptNode(node, *this); adoptResult.HasException())
    {
      return adoptResult.ReleaseException();
    }

    return ShareRef(node);
  }

  // NOTE: VS says this method can be made const but it's lying, Attr constructor needs a non-const Document
  // reference and we can't make it const without breaking that.
  ExceptionOr<Ref<Attr>> Document::CreateAttribute(DOMStringAtom localName) noexcept
  {
    if (!NameValidation::IsValidAttributeLocalName(localName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    if (Is<HTMLDocument>(*this))
    {
      localName = ::Krys::Text::ToASCIILowercase(localName.View());
    }

    QualifiedName qualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName};
    return AdoptRef<Attr>(*new Attr(*this, qualifiedName));
  }

  ExceptionOr<Ref<Attr>> Document::CreateAttributeNS(DOMStringAtom namespaceUri,
                                                     DOMStringAtom qualifiedName) noexcept
  {
    auto result =
      NameValidation::ValidateAndExtract(namespaceUri, qualifiedName, ValidateAndExtractContext::Attribute);
    if (result.HasException())
    {
      return result.ReleaseException();
    }

    QualifiedName name {result.Value().NamespaceURI, result.Value().Prefix, result.Value().LocalName};
    return AdoptRef<Attr>(*new Attr(*this, name));
  }

  Ref<Event> Document::CreateEvent(DOMStringAtom interface) noexcept
  {
    // TODO(impl): EVENTS - create correct event based on 'interface'
    // TODO(impl): EVENTS - set initialized to true and isTrusted to false.
    return CreateRef<Event>(interface);
  }

  Ref<Range> Document::CreateRange() noexcept
  {
    return CreateRef<Range>(BoundaryPoint {ShareRef(*this), 0uz}, BoundaryPoint {ShareRef(*this), 0uz});
  }

  Ref<NodeIterator> Document::CreateNodeIterator(Node &root, WhatToShow whatToShow,
                                                 RefPtr<NodeFilter> &&filter) noexcept
  {
    return AdoptRef(*new NodeIterator(root, whatToShow, Krys::Move(filter)));
  }

  Ref<TreeWalker> Document::CreateTreeWalker(Node &root, WhatToShow whatToShow,
                                             RefPtr<NodeFilter> &&filter) noexcept
  {
    return AdoptRef(*new TreeWalker(root, whatToShow, Krys::Move(filter)));
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Document::Children() noexcept
  {
    return _documentRareData->Children(*this);
  }

  RefPtr<const Element> Document::FirstElementChild() const noexcept
  {
    return Mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Document::FirstElementChild() noexcept
  {
    return Mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Document::LastElementChild() const noexcept
  {
    return Mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Document::LastElementChild() noexcept
  {
    return Mixins::ParentNode::LastElementChild(*this);
  }

  size_t Document::ChildElementCount() const noexcept
  {
    return Mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Document::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Document::Append(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Document::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return Mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Document::MoveBefore(Node &node, RawPtr<Node> refChild) noexcept
  {
    return Mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Document::QuerySelector(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Document::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return Mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> Document::GetElementById(DOMStringView elementId) noexcept
  {
    return Mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

  RefPtr<const Element> Document::GetElementById(DOMStringView elementId) const noexcept
  {
    return Mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

#pragma endregion

#pragma region HTML spec extensions

  DOMString Document::Title() const noexcept
  {
    // TODO(impl): SVG - If the document element is an SVG svg element, then let value be the child text
    // content of the first SVG title element that is a child of the document element.

    auto title = GetHTMLTitleElement();
    if (title == nullptr)
    {
      return {};
    }

    auto value = TextAlgorithms::ChildTextContent(*title);

    // TODO(impl): STRINGS - Strip and collapse ASCII whitespace in value.

    return value;
  }

  ExceptionOr<void> Document::Title(DOMString &&value) noexcept
  {
    // TODO(impl: SVG - If the document element is an SVG svg element
    // If there is an SVG title element that is a child of the document element, let element be the first such
    // element. Otherwise:
    //   Let element be the result of creating an element given the document element's node document,
    //   "title", and the SVG namespace. Insert element as the first child of the document element.
    // String replace all with the given value within element.

    auto documentElement = DocumentElement();
    if (documentElement->NamespaceURI() == Namespace::HTML)
    {
      auto head = Head();
      auto title = GetHTMLTitleElement();

      if (head == nullptr && title == nullptr)
      {
        return {};
      }

      if (title == nullptr)
      {
        title = ElementFactory::Create(*this, {Namespace::HTML, DOMStringAtom::Null(), u8"title"});

        if (auto append = MutationAlgorithms::Append(*documentElement, *title); append.HasException())
        {
          return append.ReleaseException();
        }
      }

      if (auto replace = NodeAlgorithms::StringReplaceAll(Krys::Move(value), *title); replace.HasException())
      {
        return replace.ReleaseException();
      }
    }

    return {};
  }

  RefPtr<HTMLBodyElement> Document::Body() noexcept
  {
    auto html = GetHTMLHtmlElement();
    if (html == nullptr)
    {
      return nullptr;
    }

    auto children = ChildHTMLElementRange(*html);
    auto body = FirstOfType<HTMLBodyElement>(children);
    return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLBodyElement>(&*body));
  }

  RefPtr<const HTMLBodyElement> Document::Body() const noexcept
  {
    auto html = GetHTMLHtmlElement();
    if (html == nullptr)
    {
      return nullptr;
    }

    auto children = ConstChildHTMLElementRange(*html);
    auto body = FirstOfType<HTMLBodyElement>(children);
    return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLBodyElement>(&*body));
  }

  ExceptionOr<void> Document::Body(HTMLBodyElement &body) noexcept
  {
    auto currentBody = Body();
    if (currentBody == &body)
    {
      return {};
    }

    if (currentBody != nullptr)
    {
      if (auto replace = MutationAlgorithms::Replace(*currentBody, body, *body.ParentNode());
          replace.HasException())
      {
        return replace.ReleaseException();
      }
    }
    else
    {
      auto documentElement = DocumentElement();
      if (documentElement == nullptr)
      {
        return ExceptionCode::HierarchyRequestError;
      }

      if (auto append = MutationAlgorithms::Append(body, *documentElement); append.HasException())
      {
        return append.ReleaseException();
      }
    }

    return {};
  }

  RefPtr<HTMLHeadElement> Document::Head() noexcept
  {
    auto html = GetHTMLHtmlElement();
    if (html == nullptr)
    {
      return nullptr;
    }

    auto children = ChildHTMLElementRange(*html);
    auto body = FirstOfType<HTMLHeadElement>(children);
    return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLHeadElement>(&*body));
  }

  RefPtr<const HTMLHeadElement> Document::Head() const noexcept
  {
    auto html = GetHTMLHtmlElement();
    if (html == nullptr)
    {
      return nullptr;
    }

    auto children = ConstChildHTMLElementRange(*html);
    auto body = FirstOfType<HTMLHeadElement>(children);
    return body == std::ranges::end(children) ? nullptr : ShareRefPtr(Downcast<HTMLHeadElement>(&*body));
  }

  RefPtr<HTMLHtmlElement> Document::GetHTMLHtmlElement() noexcept
  {
    auto documentElement = DocumentElement();

    if (!Is<HTMLElement>(documentElement))
    {
      return nullptr;
    }

    auto *htmlElement = Downcast<HTMLElement>(documentElement.get());
    if (!Is<HTMLHtmlElement>(htmlElement))
    {
      return nullptr;
    }

    auto *html = Downcast<HTMLHtmlElement>(htmlElement);
    return ShareRefPtr(html);
  }

  RefPtr<const HTMLHtmlElement> Document::GetHTMLHtmlElement() const noexcept
  {
    return const_cast<Document *>(this)->GetHTMLHtmlElement();
  }

  RefPtr<HTMLTitleElement> Document::GetHTMLTitleElement() noexcept
  {
    auto *documentElement = DocumentElement().release();

    if (!Is<HTMLElement>(documentElement))
    {
      return nullptr;
    }

    auto *htmlElement = Downcast<HTMLElement>(documentElement);
    if (!Is<HTMLTitleElement>(htmlElement))
    {
      return nullptr;
    }

    auto *title = Downcast<HTMLTitleElement>(htmlElement);
    return AdoptRefPtr(title);
  }

  RefPtr<const HTMLTitleElement> Document::GetHTMLTitleElement() const noexcept
  {
    return const_cast<Document *>(this)->GetHTMLTitleElement();
  }

#pragma endregion
}