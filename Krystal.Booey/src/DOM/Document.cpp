#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
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
#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/VisitedLinkState.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/DOMTreeAccessors.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLBaseElement.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTitleElement.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGScriptElement.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::dom
{
  Document::Document(DocumentFlags flags) noexcept : Document()
  {
    SetDocumentFlag(flags);
  }

#pragma region Document (DOM)

  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlags::IsContainerNode),
        DocumentOrShadowRoot(static_cast<ContainerNode &>(*this)), _flags(DocumentFlags::None),
        _url(url::AboutBlankURL()), _baseURL(_url), _settings(ShareRef(Settings::Default())),
        _userActionElements(CreateUnique<UserActionElementSet>())
  {
    SetNodeFlag(NodeFlags::IsConnected);
    _styleScope = CreateUnique<style::Scope>(*this);
  }

  Document::~Document() noexcept
  {
    if (_styleSheetList)
    {
      _styleSheetList->Detach();
    }
    if (_mediaQueryMatcher)
    {
      _mediaQueryMatcher->DocumentDestroyed();
    }
    if (_fontSelector)
    {
      _fontSelector->UnregisterForInvalidationCallbacks(*this);
      static_cast<css::CSSFontSelector &>(*_fontSelector).ClearFonts();
    }
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
    return _url.String();
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
    if (!_documentRareData)
    {
      _documentRareData = CreateUnique<DocumentRareData>();
    }
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

#pragma region URLs

  void Document::SetURL(url::URL &&url) noexcept
  {
    url::URL newURL = url.IsEmpty() ? url::AboutBlankURL() : krys::move(url);
    if (newURL == _url)
    {
      return;
    }

    _url = krys::move(newURL);
    UpdateBaseURL();
  }

  const url::URL &Document::FallbackBaseURL() const noexcept
  {
    // SPEC-VIOLATION(HTML): there are no iframes or browsing contexts, so a document is never an iframe
    // srcdoc document and never has an about base URL; the fallback base URL is always the document's URL.
    return _url;
  }

  url::URL Document::CompleteURL(utf8_stringview url) const noexcept
  {
    return CompleteURL(url, _baseURL);
  }

  url::URL Document::CompleteURL(utf8_stringview url, const url::URL &baseURL) const noexcept
  {
    // NOTE: the document's encoding is always UTF-8, so the URL is parsed without one.
    return url::URL(baseURL, url);
  }

  void Document::ProcessBaseElement() noexcept
  {
    Maybe<DOMString> href;
    for (auto &node : DescendantRange(*this))
    {
      auto *element = DynamicDowncast<html::HTMLElement>(&node);
      if (element == nullptr || !Is<html::HTMLBaseElement>(*element))
      {
        continue;
      }

      href = element->GetAttribute(u8"href");
      if (href.has_value())
      {
        break;
      }
    }

    url::URL baseElementURL;
    if (href.has_value())
    {
      // https://html.spec.whatwg.org/#set-the-frozen-base-url
      // 3. If urlRecord is failure, its scheme is "data" or "javascript", or "Is base allowed for Document?"
      // returns "Blocked", then set element's frozen base URL to document's fallback base URL; otherwise to
      // urlRecord.
      // SPEC-VIOLATION(CSP): there's no content security policy, so every base is allowed.
      baseElementURL = CompleteURL(*href, FallbackBaseURL());
      if (!baseElementURL.IsValid() || baseElementURL.ProtocolIsData()
          || baseElementURL.ProtocolIsJavaScript())
      {
        baseElementURL = FallbackBaseURL();
      }
    }

    if (baseElementURL == _baseElementURL && baseElementURL.IsNull() == _baseElementURL.IsNull())
    {
      return;
    }

    _baseElementURL = krys::move(baseElementURL);
    UpdateBaseURL();
  }

  void Document::UpdateBaseURL() noexcept
  {
    // https://html.spec.whatwg.org/#document-base-url
    // 1. If there is no base element that has an href attribute in the Document, then return the Document's
    // fallback base URL.
    // 2. Otherwise, return the frozen base URL of the first base element in the Document that has an href
    // attribute, in tree order.
    _baseURL = _baseElementURL.IsNull() ? FallbackBaseURL() : _baseElementURL;

    InvalidateCachedCSSParserContext();
  }

#pragma endregion

#pragma region CSS

  const css::ParserContext &Document::CssParserContext() const noexcept
  {
    if (_cachedCSSParserContext == nullptr)
    {
      _cachedCSSParserContext = CreateUnique<css::ParserContext>(*this, url::URL {}, utf8_stringview {});
    }

    return *_cachedCSSParserContext;
  }

  void Document::InvalidateCachedCSSParserContext() noexcept
  {
    _cachedCSSParserContext = nullptr;
  }

#pragma region User Action State

  namespace
  {
    KRYS_NODISCARD Element *FindNearestCommonComposedAncestor(Element *elementA, Element *elementB) noexcept
    {
      if (!elementA || !elementB)
      {
        return nullptr;
      }

      if (elementA == elementB)
      {
        return elementA;
      }

      HashSet<Element *> ancestorChain;
      for (auto *element = elementA; element; element = element->ParentElementInComposedTree())
      {
        ancestorChain.insert(element);
      }

      for (auto *element = elementB; element; element = element->ParentElementInComposedTree())
      {
        if (ancestorChain.contains(element))
        {
          return element;
        }
      }
      return nullptr;
    }

    KRYS_NODISCARD bool HasShadowRootParent(const Element &element) noexcept
    {
      return Is<ShadowRoot>(element.ParentNode());
    }

    // FIXME: The renderer, once there's a render tree; until then an element has one when it has a render
    // style.
    KRYS_NODISCARD bool HasRenderer(const Element &element) noexcept
    {
      return element.GetRenderStyle();
    }
  }

  void Document::UpdateHoverActiveState(const HitTestRequest &request, Element *innerElement,
                                        CaptureChange captureElementChanged) noexcept
  {
    krys_debug_assert(!request.ReadOnly());

    List<RefPtr<Element>> elementsToClearActive;
    List<RefPtr<Element>> elementsToSetActive;
    List<RefPtr<Element>> elementsToClearHover;
    List<RefPtr<Element>> elementsToSetHover;

    // FIXME: Update the hover and active state of the subframe documents the element is in, once there are
    // frames.
    RefPtr<Element> innerElementInDocument = ShareRefPtr(innerElement);
    if (innerElementInDocument && &innerElementInDocument->NodeDocument() != this)
    {
      innerElementInDocument = nullptr;
    }

    RefPtr<Element> oldActiveElement = _activeElement;
    if (oldActiveElement && !request.Active())
    {
      // We are clearing the :active chain because the mouse has been released.
      for (RefPtr<Element> currentElement = oldActiveElement; currentElement;
           currentElement = ShareRefPtr(currentElement->ParentElementInComposedTree()))
      {
        elementsToClearActive.push_back(currentElement);
        _userActionElements->SetInActiveChain(*currentElement, false);
      }
      _activeElement = nullptr;
    }
    else
    {
      RefPtr<Element> newActiveElement = innerElementInDocument;
      if (!oldActiveElement && newActiveElement && request.Active() && !request.TouchMove())
      {
        // We are setting the :active chain and freezing it. If future moves happen, they
        // will need to reference this chain.
        // FIXME: The chain of renderers, once there's a render tree; until then the composed tree ancestors
        // with a renderer.
        for (auto *element = newActiveElement.get(); element;
             element = element->ParentElementInComposedTree())
        {
          if (!HasRenderer(*element))
          {
            continue;
          }
          _userActionElements->SetInActiveChain(*element, true);
        }

        _activeElement = newActiveElement;
      }
    }
    // If the mouse has just been pressed, set :active on the chain. Those (and only those)
    // nodes should remain :active until the mouse is released.
    bool allowActiveChanges = !oldActiveElement && _activeElement;

    // If the mouse is down and if this is a mouse move event, we want to restrict changes in
    // :hover/:active to only apply to elements that are in the :active chain that we froze
    // at the time the mouse went down, unless the capture element changed.
    bool mustBeInActiveChain =
      request.Active() && request.Move() && captureElementChanged == CaptureChange::No;

    RefPtr<Element> oldHoveredElement = krys::move(_hoveredElement);

    // A touch release does not set a new hover target; clearing the element we're working with
    // will clear the chain of hovered elements all the way to the top of the tree.
    if (request.TouchRelease())
    {
      innerElementInDocument = nullptr;
    }

    // Check to see if the hovered Element has changed.
    // If it hasn't, we do not need to do anything.
    RefPtr<Element> newHoveredElement = innerElementInDocument;
    while (newHoveredElement && !HasRenderer(*newHoveredElement))
    {
      newHoveredElement = ShareRefPtr(newHoveredElement->ParentElementInComposedTree());
    }

    _hoveredElement = newHoveredElement;

    auto *commonAncestor =
      FindNearestCommonComposedAncestor(oldHoveredElement.get(), newHoveredElement.get());

    if (oldHoveredElement != newHoveredElement)
    {
      for (auto *element = oldHoveredElement.get(); element; element = element->ParentElementInComposedTree())
      {
        if (element == commonAncestor)
        {
          break;
        }
        if (mustBeInActiveChain && !element->IsInActiveChain())
        {
          continue;
        }
        elementsToClearHover.push_back(ShareRefPtr(element));
      }
      // FIXME: Unset the hovered nodes in the subframe document of a frame owner, once there are frames.
    }

    bool sawCommonAncestor = false;
    for (auto *element = newHoveredElement.get(); element; element = element->ParentElementInComposedTree())
    {
      if (mustBeInActiveChain && !element->IsInActiveChain())
      {
        continue;
      }
      if (allowActiveChanges)
      {
        elementsToSetActive.push_back(ShareRefPtr(element));
      }
      if (element == commonAncestor)
      {
        sawCommonAncestor = true;
      }
      if (!sawCommonAncestor)
      {
        elementsToSetHover.push_back(ShareRefPtr(element));
      }
    }

    auto changeState =
      [](List<RefPtr<Element>> &elements, css::SelectorPseudoClass pseudoClass, bool value, auto &&setter)
    {
      if (elements.empty())
      {
        return;
      }

      style::PseudoClassChangeInvalidation styleInvalidation {*elements.back(), pseudoClass, value,
                                                              style::InvalidationScope::Descendants};

      // We need to do descendant invalidation for each shadow tree separately as the style is per-scope.
      List<UniquePtr<style::PseudoClassChangeInvalidation>> shadowDescendantStyleInvalidations;
      for (auto &element : elements)
      {
        if (HasShadowRootParent(*element))
        {
          shadowDescendantStyleInvalidations.push_back(CreateUnique<style::PseudoClassChangeInvalidation>(
            *element, pseudoClass, value, style::InvalidationScope::Descendants));
        }
      }

      for (auto &element : elements)
      {
        setter(*element);
      }
    };

    changeState(elementsToClearActive, css::SelectorPseudoClass::Active, false, [](Element &element)
                { element.SetActive(false, style::InvalidationScope::SelfChildrenAndSiblings); });
    changeState(elementsToSetActive, css::SelectorPseudoClass::Active, true, [](Element &element)
                { element.SetActive(true, style::InvalidationScope::SelfChildrenAndSiblings); });
    changeState(elementsToClearHover, css::SelectorPseudoClass::Hover, false, [&request](Element &element)
                { element.SetHovered(false, style::InvalidationScope::SelfChildrenAndSiblings, request); });
    changeState(elementsToSetHover, css::SelectorPseudoClass::Hover, true, [&request](Element &element)
                { element.SetHovered(true, style::InvalidationScope::SelfChildrenAndSiblings, request); });
  }

  void Document::HoveredElementDidDetach(Element &element) noexcept
  {
    if (!_hoveredElement || &element != _hoveredElement.get())
    {
      return;
    }

    _hoveredElement = ShareRefPtr(element.ParentElement());
    while (_hoveredElement && !HasRenderer(*_hoveredElement))
    {
      _hoveredElement = ShareRefPtr(_hoveredElement->ParentElement());
    }
    // FIXME: Schedule a hover state update (the event handler's scheduleHoverStateUpdate), once there's input
    // handling.
  }

  bool Document::HasFocus() const noexcept
  {
    return html::FocusAlgorithms::HasFocusSteps(*this);
  }

  void Document::NodeWillBeRemoved(Node &node) noexcept
  {
    // If the focused area of the document is removed, the viewport becomes the focused area. This doesn't run
    // the focusing or focus update steps, so no events are fired.
    // https://html.spec.whatwg.org/#html-element-removing-steps
    if (_focusedElement && TreeQueries::IsShadowIncludingInclusiveAncestor(node, *_focusedElement))
    {
      // Clearing the focus while the element is still in the tree also clears its ancestors' focus within.
      RefPtr<Element> focusedElement = std::exchange(_focusedElement, nullptr);
      focusedElement->SetFocus(false);
    }

    // The removed subtree's renderers go after it's removed, so the hovered element and the active chain move
    // to the nearest ancestor outside it with a renderer now (upstream: as its renderers are torn down,
    // before the removal).
    auto moveOutOfRemovedSubtree = [&](RefPtr<Element> &element)
    {
      if (!element || !TreeQueries::IsShadowIncludingInclusiveAncestor(node, *element))
      {
        return;
      }
      element = ShareRefPtr(node.ParentElement());
      while (element && !HasRenderer(*element))
      {
        element = ShareRefPtr(element->ParentElement());
      }
    };
    moveOutOfRemovedSubtree(_hoveredElement);
    moveOutOfRemovedSubtree(_activeElement);
  }

  void Document::ElementInActiveChainDidDetach(Element &element) noexcept
  {
    if (!_activeElement || &element != _activeElement.get())
    {
      return;
    }

    _activeElement = ShareRefPtr(element.ParentElement());
    while (_activeElement && !HasRenderer(*_activeElement))
    {
      _activeElement = ShareRefPtr(_activeElement->ParentElement());
    }
  }

  void Document::SetFocusedAndActive(bool flag) noexcept
  {
    if (_isFocusedAndActive == flag)
    {
      return;
    }
    _isFocusedAndActive = flag;

    // `:focus`, `:focus-visible` and `:focus-within` check whether the document is focused and active, so the
    // elements they may match restyle.
    _userActionElements->ForEachFocusedOrFocusWithinElement([](Element &element)
                                                            { element.InvalidateStyleForSubtree(); });
  }

#pragma endregion

  void Document::SetSettings(Ref<Settings> &&settings) noexcept
  {
    _settings = krys::move(settings);
    InvalidateCachedCSSParserContext();
    _exposedComputedCSSPropertyIds = null;
    if (_fontSelector)
    {
      static_cast<css::CSSFontSelector &>(*_fontSelector).SettingsChanged();
    }
    ScheduleFullStyleRebuild();
  }

  void Document::DidInsertInDocumentShadowRoot(ShadowRoot &shadowRoot) noexcept
  {
    krys_debug_assert(shadowRoot.IsConnected());
    krys_debug_assert(std::ranges::none_of(_inDocumentShadowRoots, [&](const WeakPtr<ShadowRoot> &root)
                                           { return root.get() == &shadowRoot; }));
    _inDocumentShadowRoots.push_back(CreateWeakPtr(&shadowRoot));
  }

  void Document::DidRemoveInDocumentShadowRoot(ShadowRoot &shadowRoot) noexcept
  {
    std::erase_if(_inDocumentShadowRoots,
                  [&](const WeakPtr<ShadowRoot> &root) { return !root || root.get() == &shadowRoot; });
  }

  const RenderStyle *Document::InitialContainingBlockStyle() const noexcept
  {
    if (!_initialContainingBlockStyle)
    {
      _initialContainingBlockStyle = RenderStyle::ClonePtr(style::ResolveForDocument(*this));
    }
    return _initialContainingBlockStyle.get();
  }

  const RenderStyle &Document::RenderViewStyle() const noexcept
  {
    if (_renderView)
    {
      return _renderView->Style();
    }
    if (_renderViewStyle)
    {
      return *_renderViewStyle;
    }
    return *InitialContainingBlockStyle();
  }

  LocalFrame *Document::Frame() const noexcept
  {
    return _frame.get();
  }

  LocalFrameView *Document::View() const noexcept
  {
    return _frame ? _frame->View() : nullptr;
  }

  void Document::CreateRenderTree() noexcept
  {
    krys_debug_assert(!GetRenderView());
    krys_debug_assert(View());

    // The document may have been styled without a render view; the rebuild gives the new one its style.
    _initialContainingBlockStyle = nullptr;

    // FIXME: It would be better if we could pass the resolved document style directly here.
    _renderView = CreateRenderer<boo::RenderView>(*this, RenderStyle::Create());
    Node::SetRenderer(_renderView.get());

    ResolveStyle(ResolveStyleType::Rebuild);
  }

  void Document::DestroyRenderTree() noexcept
  {
    krys_debug_assert(HasLivingRenderTree());

    SetForScope change(_renderTreeBeingDestroyed, true);

    // FIXME: The accessibility cache, the document becoming inactive and telling the view, once there are
    // any.

    _pendingRenderTreeUpdate = nullptr;
    _initialContainingBlockStyle = nullptr;

    if (auto documentElement = DocumentElement())
    {
      RenderTreeUpdater::TearDownRenderers(*documentElement);
    }

    // FIXME: RenderObject::view() uses m_renderView and we can't null it before destruction is completed
    {
      RenderTreeBuilder builder(*_renderView);
      // FIXME: This is a workaround for leftover content (see webkit.org/b/182547).
      while (_renderView->FirstChild())
      {
        builder.Destroy(*_renderView->FirstChild());
      }

      // FIXME: Delete the layout context's detached renderers, once there's layout.

      _renderView->Destroy();
    }
    (void)_renderView.release();

    Node::SetRenderer(nullptr);

    // Styles resolved against the view (viewport units, the page zoom) are stale.
    ScheduleFullStyleRebuild();
  }

  void Document::DidBecomeCurrentDocumentInFrame(LocalFrame &frame) noexcept
  {
    _frame = WeakPtr<LocalFrame>::WithRef(frame);

    if (!HasLivingRenderTree() && View())
    {
      CreateRenderTree();
    }

    // FIXME: Viewport arguments, scheduled task and animation suspension, and quirks, once there are any.
  }

  void Document::WillBeRemovedFromFrame() noexcept
  {
    // FIXME: Unload, page hide and the other steps of unloading the document, once there's navigation.
    if (HasLivingRenderTree())
    {
      DestroyRenderTree();
    }

    _frame = nullptr;
  }

  float Document::DeviceScaleFactor() const noexcept
  {
    float deviceScaleFactor = 1.0;
    if (auto *frame = Frame())
    {
      deviceScaleFactor = frame->GetMediaEnvironment().DeviceScaleFactor;
    }
    return deviceScaleFactor;
  }

  bool Document::ShouldScheduleLayout() const noexcept
  {
    if (!DocumentElement())
    {
      return false;
    }
    if (!Is<html::HTMLHtmlElement>(*DocumentElement()))
    {
      return true;
    }
    if (!Body())
    {
      return false;
    }
    if (StyleScope().HasPendingSheetsBeforeBody())
    {
      return false;
    }
    // FIXME: Wait until the view is visually non-empty, once there's painting.
    return true;
  }

  bool Document::IsLayoutPending() const noexcept
  {
    return View() && View()->LayoutContext().IsLayoutPending();
  }

  void Document::UpdateViewportUnitsOnResize() noexcept
  {
    StyleScope().DidChangeViewportSize();
  }

  Document::UpdateLayoutResult Document::UpdateLayoutIgnorePendingStylesheets() noexcept
  {
    SetForScope ignorePendingStylesheets(_ignorePendingStylesheets, true);
    // FIXME: This should just invalidate elements with missing styles.
    if (_hasNodesWithMissingStyle)
    {
      ScheduleFullStyleRebuild();
    }

    // FIXME: Update the relevancy of content-visibility elements, once there's content relevancy.

    return UpdateLayout();
  }

  Document::UpdateLayoutResult Document::UpdateLayout() noexcept
  {
    RefPtr<LocalFrameView> frameView = ShareRefPtr(View());
    if (frameView && frameView->LayoutContext().IsInRenderTreeLayout())
    {
      // View layout should not be re-entrant.
      krys_debug_assert_msg(false, "layout isn't re-entrant");
      return UpdateLayoutResult::NoChange;
    }

    UpdateLayoutResult result = UpdateLayoutResult::NoChange;

    // FIXME: Update the owner document's layout first, once documents can be nested in frames.

    if (UpdateStyleIfNeeded())
    {
      result = UpdateLayoutResult::ChangesDone;
    }

    if (frameView && GetRenderView())
    {
      auto &layoutContext = frameView->LayoutContext();
      // FIXME: Force the layout of skipped content (content-visibility) when asked, once there's layout.
      if (layoutContext.NeedsLayout())
      {
        layoutContext.Layout();
        result = UpdateLayoutResult::ChangesDone;
      }
    }

    return result;
  }

  css::CSSFontSelector &Document::GetFontSelector() const noexcept
  {
    if (!_fontSelector)
    {
      auto &document = const_cast<Document &>(*this);
      _fontSelector = css::CSSFontSelector::Create(document);
      _fontSelector->RegisterForInvalidationCallbacks(document);
    }
    return static_cast<css::CSSFontSelector &>(*_fontSelector);
  }

  void Document::FontsNeedUpdate(FontSelector &) noexcept
  {
    InvalidateMatchedPropertiesCacheAndForceStyleRecalc();
  }

  void Document::InvalidateMatchedPropertiesCacheAndForceStyleRecalc() noexcept
  {
    StyleScope().InvalidateMatchedDeclarationsCache();

    ScheduleFullStyleRebuild();
  }

  void Document::ScheduleFullStyleRebuild() noexcept
  {
    _needsFullStyleRebuild = true;
    ScheduleStyleRecalc();
  }

  void Document::ScheduleStyleRecalc() noexcept
  {
    krys_debug_assert(ChildNeedsStyleRecalc() || _needsFullStyleRebuild);

    // FIXME: Start the style recalc timer, once there's an event loop. Until then the host resolves style
    // (UpdateStyleIfNeeded) when it needs it.
  }

  void Document::UnscheduleStyleRecalc() noexcept
  {
    krys_debug_assert(!ChildNeedsStyleRecalc());

    _needsFullStyleRebuild = false;
  }

  bool Document::HasPendingStyleRecalc() const noexcept
  {
    return NeedsStyleRecalc() && !_inStyleRecalc;
  }

  bool Document::HasPendingFullStyleRebuild() const noexcept
  {
    return HasPendingStyleRecalc() && _needsFullStyleRebuild;
  }

  const FixedVector<css::PropertyId> &Document::ExposedComputedCSSPropertyIds() noexcept
  {
    if (!_exposedComputedCSSPropertyIds.has_value())
    {
      auto &settings = CssParserContext().PropertySettings;
      List<css::PropertyId> exposed;
      for (auto property : css::ComputedPropertyIds)
      {
        if (!css::IsExposed(property, settings))
        {
          continue;
        }
        // If the standard property is exposed no need to expose the alias.
        if (auto standard = css::CascadeAliasProperty(property);
            standard != property && css::IsExposed(standard, settings))
        {
          continue;
        }
        exposed.push_back(property);
      }
      auto &ids = _exposedComputedCSSPropertyIds.emplace(exposed.size());
      std::ranges::copy(exposed, ids.begin());
    }
    return *_exposedComputedCSSPropertyIds;
  }

  void Document::UpdateRenderTree(UniquePtr<style::Update> styleUpdate) noexcept
  {
    krys_debug_assert(!InRenderTreeUpdate());

    style::PostResolutionCallbackDisabler callbackDisabler(*this);
    {
      SetForScope inRenderTreeUpdate(_inRenderTreeUpdate, true);
      {
        RenderTreeUpdater updater(*this, callbackDisabler);
        updater.Commit(krys::move(styleUpdate));
      }
    }
  }

  void Document::ResolveStyle(ResolveStyleType type) noexcept
  {
    if (_inStyleRecalc)
    {
      return;
    }

    // FIXME: We should update style on our ancestor chain before proceeding, however doing so at
    // the time this comment was originally written caused several tests to crash.

    StyleScope().FlushPendingUpdate();

    {
      style::PostResolutionCallbackDisabler disabler(*this);

      _inStyleRecalc = true;

      if (_needsFullStyleRebuild)
      {
        type = ResolveStyleType::Rebuild;
      }

      if (type == ResolveStyleType::Rebuild)
      {
        // This may get set again during style resolve.
        _hasNodesWithMissingStyle = false;

        auto newStyle = style::ResolveForDocument(*this);

        auto documentChanges = _initialContainingBlockStyle
                                 ? style::DetermineChanges(newStyle, *_initialContainingBlockStyle)
                                 : EnumSet<style::Change> {style::Change::Renderer};
        if (!documentChanges.IsEmpty())
        {
          _initialContainingBlockStyle = RenderStyle::ClonePtr(newStyle);
          // The used style may end up differing from the computed style due to propagation of properties from
          // elements.
          if (_renderView)
          {
            _renderView->SetStyle(krys::move(newStyle));
          }
          else
          {
            _renderViewStyle = RenderStyle::ClonePtr(newStyle);
          }
        }

        if (auto documentElement = DocumentElement())
        {
          documentElement->InvalidateStyleForSubtree();
        }
      }

      {
        style::TreeResolver resolver(*this, krys::move(_pendingRenderTreeUpdate));
        auto styleUpdate = resolver.Resolve();

        while (resolver.NeedsInterleavedLayout())
        {
          if (styleUpdate)
          {
            UpdateRenderTree(krys::move(styleUpdate));

            // FIXME: The interleaved layout, once there's layout.
          }

          styleUpdate = resolver.Resolve();
        }

        SetHasValidStyle();
        ClearChildNeedsStyleRecalc();
        UnscheduleStyleRecalc();

        _inStyleRecalc = false;

        if (styleUpdate)
        {
          UpdateRenderTree(krys::move(styleUpdate));
        }
      }

      if (auto *frameView = View(); frameView && _renderView && _renderView->NeedsLayout())
      {
        frameView->LayoutContext().ScheduleLayout();
      }
    }
  }

  void Document::UpdateTextRenderer(Text &text, uint32 offsetOfReplacedText,
                                    uint32 lengthOfReplacedText) noexcept
  {
    if (!HasLivingRenderTree())
    {
      return;
    }

    EnsurePendingRenderTreeUpdate().AddText(text, {offsetOfReplacedText, lengthOfReplacedText, null});
  }

  style::Update &Document::EnsurePendingRenderTreeUpdate() noexcept
  {
    krys_debug_assert(HasLivingRenderTree());

    if (!_pendingRenderTreeUpdate)
    {
      _pendingRenderTreeUpdate = CreateUnique<style::Update>(*this);
    }
    return *_pendingRenderTreeUpdate;
  }

  bool Document::NeedsStyleRecalc() const noexcept
  {
    if (_needsFullStyleRebuild)
    {
      return true;
    }

    if (ChildNeedsStyleRecalc())
    {
      return true;
    }

    if (_pendingRenderTreeUpdate)
    {
      return true;
    }

    if (StyleScope().HasPendingUpdate())
    {
      return true;
    }

    return false;
  }

  bool Document::UpdateStyleIfNeeded() noexcept
  {
    StyleScope().FlushPendingUpdate();

    if (!NeedsStyleRecalc())
    {
      return false;
    }

    ResolveStyle();
    return true;
  }

  bool Document::UpdateStyleIfNeededIgnoringPendingStylesheets() noexcept
  {
    SetForScope ignorePendingStylesheets(_ignorePendingStylesheets, true);
    // FIXME: This should just invalidate elements with missing styles.
    if (_hasNodesWithMissingStyle)
    {
      ScheduleFullStyleRebuild();
    }

    // FIXME: Update the relevancy of content-visibility elements, once there's content relevancy.

    return UpdateStyleIfNeeded();
  }

  UniquePtr<RenderStyle> Document::StyleForElementIgnoringPendingStylesheets(
    Element &element, const RenderStyle *parentStyle,
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    krys_debug_assert(&element.NodeDocument() == this);
    krys_debug_assert(!pseudoElementIdentifier || parentStyle);
    krys_debug_assert(style::PostResolutionCallbacksAreSuspended());

    Maybe<RenderStyle> updatedDocumentStyle;
    if (!parentStyle && _needsFullStyleRebuild)
    {
      updatedDocumentStyle.emplace(style::ResolveForDocument(*this));
      parentStyle = &*updatedDocumentStyle;
    }

    SetForScope change(_ignorePendingStylesheets, true);
    auto resolver = ShareRef(style::Scope::ForNode(element).GetResolver());

    auto elementStyle = resolver->StyleForElement(element, {parentStyle});
    if (pseudoElementIdentifier)
    {
      auto type = pseudoElementIdentifier->Type;
      if ((type == PseudoElementType::FirstLetter || type == PseudoElementType::FirstLine)
          && elementStyle.Style && !style::SupportsFirstLineAndLetterPseudoElement(*elementStyle.Style))
      {
        return {};
      }

      auto style = resolver->StyleForPseudoElement(
        element, style::PseudoElementRequest(*pseudoElementIdentifier), {parentStyle});
      if (!style)
      {
        return nullptr;
      }
      return krys::move(style->Style);
    }

    if (elementStyle.Relations)
    {
      auto emptyUpdate = CreateUnique<style::Update>(*this);
      style::CommitRelations(krys::move(elementStyle.Relations), *emptyUpdate);
    }

    return krys::move(elementStyle.Style);
  }

  VisitedLinkState &Document::GetVisitedLinkState() const noexcept
  {
    if (!_visitedLinkState)
    {
      _visitedLinkState = CreateUnique<VisitedLinkState>(const_cast<Document &>(*this));
    }
    return *_visitedLinkState;
  }

  boo::Color Document::LinkColor(const RenderStyle &style) const noexcept
  {
    if (_linkColor.IsValid())
    {
      return _linkColor;
    }
    return css::ColorFromKeyword(css::ValueId::WebkitLink, DocumentStyleColorOptions(*this, &style));
  }

  boo::Color Document::VisitedLinkColor(const RenderStyle &style) const noexcept
  {
    if (_visitedLinkColor.IsValid())
    {
      return _visitedLinkColor;
    }
    return css::ColorFromKeyword(css::ValueId::WebkitLink, DocumentStyleColorOptions(*this, &style)
                                                             | style::ColorOptions::ForVisitedLink);
  }

  boo::Color Document::ActiveLinkColor(const RenderStyle &style) const noexcept
  {
    if (_activeLinkColor.IsValid())
    {
      return _activeLinkColor;
    }
    return css::ColorFromKeyword(css::ValueId::WebkitActivelink, DocumentStyleColorOptions(*this, &style));
  }

  void Document::ResetLinkColor() noexcept
  {
    _linkColor = {};
  }

  void Document::ResetVisitedLinkColor() noexcept
  {
    _visitedLinkColor = {};
  }

  void Document::ResetActiveLinkColor() noexcept
  {
    _activeLinkColor = {};
  }

  double Document::LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &key) const noexcept
  {
    if (!_randomCachingKeyMap)
    {
      _randomCachingKeyMap = css::calc::RandomCachingKeyMap::Create();
    }
    return _randomCachingKeyMap->LookupCSSRandomBaseValue(key);
  }

  css::MediaQueryMatcher &Document::GetMediaQueryMatcher() noexcept
  {
    if (!_mediaQueryMatcher)
    {
      _mediaQueryMatcher = css::MediaQueryMatcher::Create(*this);
    }
    return *_mediaQueryMatcher;
  }

  Ref<css::MediaQueryList> Document::MatchMedia(DOMStringView query) noexcept
  {
    // The matcher only returns null once the document is gone.
    Ref<css::MediaQueryList> list = GetMediaQueryMatcher().MatchMedia(query);
    return list;
  }

  void Document::EvaluateMediaQueriesAndReportChanges() noexcept
  {
    // FIXME: Run this from the "update the rendering" steps once there's a rendering loop.
    if (!_mediaQueryMatcher)
    {
      return;
    }

    _mediaQueryMatcher->EvaluateAll(css::MediaQueryMatcher::EventMode::DispatchNow);
  }

  ExceptionOr<SelectorQuery &> Document::SelectorQueryForString(DOMStringView selectorString) const noexcept
  {
    auto invalid = [&]
    {
      return Exception(ExceptionCode::SyntaxError,
                       u8"'" + DOMString(selectorString) + u8"' is not a valid selector.");
    };
    if (selectorString.empty())
    {
      return invalid();
    }

    auto *query = SelectorQueryCache::Singleton().Add(selectorString, *this);
    if (!query)
    {
      return invalid();
    }

    return *query;
  }

#pragma region DocumentOrShadowRoot Mixin (CSSOM)

  css::StyleSheetList &Document::StyleSheets() noexcept
  {
    if (!_styleSheetList)
    {
      _styleSheetList = css::StyleSheetList::Create(*this);
    }
    return *_styleSheetList;
  }

#pragma endregion

#pragma endregion

#pragma region Mode

  void Document::SetQuirksMode(QuirksMode mode) noexcept
  {
    if (_quirksMode == mode)
    {
      return;
    }

    _quirksMode = mode;

    if (auto *view = GetRenderView())
    {
      view->UpdateQuirksMode();
    }

    InvalidateCachedCSSParserContext();
  }

#pragma endregion
}
