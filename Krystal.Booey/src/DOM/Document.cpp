#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/DOMTreeAccessors.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTitleElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGScriptElement.hpp"

namespace krys::boo::dom
{
  Document::Document(DocumentFlags flags) noexcept : Document()
  {
    SetDocumentFlag(flags);
  }

#pragma region Document (DOM)

  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlags::IsContainerNode), _flags(DocumentFlags::None)
  {
    SetNodeFlag(NodeFlags::IsConnected);
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
    // TODO(DOCUMENT, URL): return the url, serialized.
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
    // TODO(DOCUMENT, ENCODING): pass encoding to document on creation
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
    return NodeAlgorithms::GetElementsByTagName(*this, qualifiedName);
  }

  Ref<HTMLCollection> Document::GetElementsByTagNameNS(DOMStringAtom namespaceUri,
                                                       DOMStringAtom localName) noexcept
  {
    return NodeAlgorithms::GetElementsByTagNameNS(*this, namespaceUri, localName);
  }

  Ref<HTMLCollection> Document::GetElementsByClassName(DOMStringAtom classNames) noexcept
  {
    return NodeAlgorithms::GetElementsByClassName(*this, classNames);
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
      localName = ::krys::text::ToASCIILower(localName.View());
    }

    auto creationOptions = DocumentAlgorithms::FlattenElementCreationOptions(options, *this);
    if (creationOptions.HasException())
    {
      return creationOptions.ReleaseException();
    }

    DOMStringAtom namespaceURI = DOMStringAtom::Null();
    if (Is<HTMLDocument>(*this) || _contentType == u8"application/xhtml+xml")
    {
      namespaceURI = infra::Namespaces::HTML;
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
    return CreateRef<Text>(*this, krys::move(data));
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

    return CreateRef<CDATASection>(*this, krys::move(data));
  }

  Ref<Comment> Document::CreateComment(DOMString &&data) noexcept
  {
    return CreateRef<Comment>(*this, krys::move(data));
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

    return AdoptRef<ProcessingInstruction>(
      *new ProcessingInstruction(*this, krys::move(target), krys::move(data)));
  }

  ExceptionOr<Ref<Node>> Document::ImportNode(Node &node, const BoolOrImportNodeOptions &options) noexcept
  {
    if (Is<Document>(node) || Is<ShadowRoot>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    bool subtree = false;
    RefPtr<html::CustomElementRegistry> registry = nullptr;
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
      localName = krys::text::ToASCIILower(localName.View());
    }

    QualifiedName qualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName};
    return AdoptRef<Attr>(*new Attr(*this, qualifiedName));
  }

  ExceptionOr<Ref<Attr>> Document::CreateAttributeNS(DOMStringAtom namespaceUri,
                                                     DOMStringAtom qualifiedName) noexcept
  {
    auto nameResult =
      NameValidation::ValidateAndExtract(namespaceUri, qualifiedName, ValidateAndExtractContext::Attribute);
    if (nameResult.HasException())
    {
      return nameResult.ReleaseException();
    }

    return AdoptRef<Attr>(*new Attr(*this, *nameResult));
  }

  Ref<Event> Document::CreateEvent(DOMStringAtom interface) noexcept
  {
    // TODO(DOCUMENT, EVENTS): create correct event based on 'interface'
    // TODO(DOCUMENT, EVENTS): set initialized to true and isTrusted to false.
    return CreateRef<Event>(interface);
  }

  Ref<Range> Document::CreateRange() noexcept
  {
    return CreateRef<Range>(BoundaryPoint {ShareRef(*this), 0uz}, BoundaryPoint {ShareRef(*this), 0uz});
  }

  Ref<NodeIterator> Document::CreateNodeIterator(Node &root, WhatToShow whatToShow,
                                                 RefPtr<NodeFilter> &&filter) noexcept
  {
    return AdoptRef(*new NodeIterator(root, whatToShow, krys::move(filter)));
  }

  Ref<TreeWalker> Document::CreateTreeWalker(Node &root, WhatToShow whatToShow,
                                             RefPtr<NodeFilter> &&filter) noexcept
  {
    return AdoptRef(*new TreeWalker(root, whatToShow, krys::move(filter)));
  }

#pragma endregion

#pragma region Document (HTML)

  ExceptionOr<Ref<Document>> Document::ParseHTMLUnsafe(DOMStringView html) noexcept
  {
    (void)html;
    krys_not_implemented();
  }

  ExceptionOr<Ref<Document>> Document::ParseHTML(DOMStringView &html) noexcept
  {
    (void)html;
    krys_not_implemented();
  }

  DOMString Document::Title() const noexcept
  {
    auto documentElement = DocumentElement();
    if (Is<svg::SVGElement>(*documentElement))
    {
      // TODO(DOCUMENT, SVG): If the document element is an SVG svg element, then let value be the child text
      // content of the first SVG title element that is a child of the document element.
      // return StringAlgorithms::StripAndCollapseASCIIWhitespace(TextAlgorithms::ChildTextContent(*title));
    }
    else
    {
      auto title = html::DOMTreeAccessors::GetTitleElement(*this);
      if (title == nullptr)
      {
        return {};
      }

      return infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(
        TextAlgorithms::ChildTextContent(*title));
    }

    return {};
  }

  ExceptionOr<void> Document::Title(DOMString &&value) noexcept
  {
    auto documentElement = DocumentElement();
    if (Is<svg::SVGElement>(*documentElement))
    {
      // TODO(DOCUMENT, SVG) - If the document element is an SVG svg element
      // If there is an SVG title element that is a child of the document element, let element be the first
      // such element. Otherwise:
      //   Let element be the result of creating an element given the document element's node document,
      //   "title", and the SVG namespace. Insert element as the first child of the document element.
      // String replace all with the given value within element.
    }
    else if (documentElement->NamespaceURI() == infra::Namespaces::HTML)
    {
      auto head = Head();
      auto title = html::DOMTreeAccessors::GetTitleElement(*this);

      if (head == nullptr && title == nullptr)
      {
        return {};
      }

      if (title == nullptr)
      {
        title = ElementFactory::Create(*this, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"title"});

        if (auto append = MutationAlgorithms::Append(*documentElement, *title); append.HasException())
        {
          return append.ReleaseException();
        }
      }

      if (auto replace = NodeAlgorithms::StringReplaceAll(krys::move(value), *title); replace.HasException())
      {
        return replace.ReleaseException();
      }
    }

    return {};
  }

  DOMString Document::Dir() const noexcept
  {
    auto html = html::DOMTreeAccessors::GetHtmlElement(*this);
    if (html == nullptr)
    {
      return {};
    }

    return html->Dir();
  }

  void Document::Dir(DOMString &&value) noexcept
  {
    auto html = html::DOMTreeAccessors::GetHeadElement(*this);
    if (html == nullptr)
    {
      return;
    }

    html->Dir(krys::move(value));
  }

  RefPtr<html::HTMLBodyElement> Document::Body() noexcept
  {
    return html::DOMTreeAccessors::GetBodyElement(*this);
  }

  RefPtr<const html::HTMLBodyElement> Document::Body() const noexcept
  {
    return html::DOMTreeAccessors::GetBodyElement(*this);
  }

  ExceptionOr<void> Document::Body(html::HTMLBodyElement &body) noexcept
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

  RefPtr<html::HTMLHeadElement> Document::Head() noexcept
  {
    return html::DOMTreeAccessors::GetHeadElement(*this);
  }

  RefPtr<const html::HTMLHeadElement> Document::Head() const noexcept
  {
    return html::DOMTreeAccessors::GetHeadElement(*this);
  }

  // TODO(DOCUMENT, HTML): Document::Images
  // Ref<HTMLCollection> Document::Images() noexcept
  // {
  // }

  // TODO(DOCUMENT, HTML): Document::Embeds
  // Ref<HTMLCollection> Document::Embeds() noexcept
  // {
  // }

  // TODO(DOCUMENT, HTML): Document::Plugins
  // Ref<HTMLCollection> Document::Plugins() noexcept
  // {
  // }

  // TODO(DOCUMENT, HTML): Document::Links
  // Ref<HTMLCollection> Document::Links() noexcept
  // {
  // }

  // TODO(DOCUMENT, HTML): Document::Forms
  // Ref<HTMLCollection> Document::Forms() noexcept
  // {
  // }

  // TODO(DOCUMENT, HTML): Document::Scripts
  // Ref<HTMLCollection> Document::Scripts() noexcept
  // {
  // }

  // TODO(DOCUMENT, HTML): GetElementsByName
  // Ref<NodeList> Document::GetElementsByName(DOMStringView elementName) noexcept
  //{
  //}

  html::HTMLOrSVGScriptElement Document::CurrentScript() noexcept
  {
    // TODO(DOCUMENT, HTML): Current running script (will we even support this?)
    return {};
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Document::Children() noexcept
  {
    return _documentRareData->Children(*this);
  }

  RefPtr<const Element> Document::FirstElementChild() const noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<Element> Document::FirstElementChild() noexcept
  {
    return mixins::ParentNode::FirstElementChild(*this);
  }

  RefPtr<const Element> Document::LastElementChild() const noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  RefPtr<Element> Document::LastElementChild() noexcept
  {
    return mixins::ParentNode::LastElementChild(*this);
  }

  size_t Document::ChildElementCount() const noexcept
  {
    return mixins::ParentNode::ChildElementCount(*this);
  }

  ExceptionOr<void> Document::Prepend(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Document::Append(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Document::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    return mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Document::MoveBefore(Node &node, Node *refChild) noexcept
  {
    return mixins::ParentNode::MoveBefore(*this, node, refChild);
  }

  ExceptionOr<RefPtr<Element>> Document::QuerySelector(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelector(*this, selectors);
  }

  ExceptionOr<Ref<NodeList>> Document::QuerySelectorAll(DOMStringView selectors) noexcept
  {
    return mixins::ParentNode::QuerySelectorAll(*this, selectors);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> Document::GetElementById(DOMStringView elementId) noexcept
  {
    return mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

  RefPtr<const Element> Document::GetElementById(DOMStringView elementId) const noexcept
  {
    return mixins::NonElementParentNode::GetElementById(*this, elementId);
  }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (DOM)

  RefPtr<html::CustomElementRegistry> Document::CustomElementRegistry() const noexcept
  {
    return _customElementRegistry;
  }

#pragma endregion

#pragma region DocumentOrShadowRoot Mixin (HTML)

  RefPtr<Element> Document::ActiveElement() const noexcept
  {
    // TODO(DOCUMENTORSHADOWROOT, HTML): return the active element.
    return nullptr;
  }

#pragma endregion

}