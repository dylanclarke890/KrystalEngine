#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Animation/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/DocumentTimeline.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/CSS/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/StyleSheetList.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/CDATASection.hpp"
#include "Krystal.Booey/DOM/Comment.hpp"
#include "Krystal.Booey/DOM/DocumentFontLoader.hpp"
#include "Krystal.Booey/DOM/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Event.hpp"
#include "Krystal.Booey/DOM/HighlightRegistry.hpp"
#include "Krystal.Booey/DOM/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/MouseEvent.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"
#include "Krystal.Booey/DOM/PopoverData.hpp"
#include "Krystal.Booey/DOM/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/DOM/TreeWalker.hpp"
#include "Krystal.Booey/DOM/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/VisitedLinkState.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Editing/VisibleSelection.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/DOMTreeAccessors.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EnumeratedAttributes.hpp"
#include "Krystal.Booey/HTML/Attributes/Reflection.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/HTML/FormController.hpp"
#include "Krystal.Booey/HTML/HashChangeEvent.hpp"
#include "Krystal.Booey/HTML/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/HTMLBaseElement.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/HTMLTitleElement.hpp"
#include "Krystal.Booey/HTML/MicroParsers/Refresh.hpp"
#include "Krystal.Booey/HTML/PageTransitionEvent.hpp"
#include "Krystal.Booey/HTML/PopStateEvent.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFontLoadRequest.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Loader/NavigationScheduler.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/History.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/Location.hpp"
#include "Krystal.Booey/Page/MouseEventWithHitTestResults.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/Scrollbar.hpp"
#include "Krystal.Booey/Platform/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
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
        _userActionElements(CreateUnique<UserActionElementSet>()),
        _styleRecalcTimer([this] { (void)ShareRef(*this)->UpdateStyleIfNeeded(); })
  {
    SetNodeFlag(NodeFlags::IsConnected);
    _styleScope = CreateUnique<style::Scope>(*this);
    _selection = CreateUnique<FrameSelection>(this);
    _editor = CreateUnique<boo::Editor>(*this);
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
    if (_fontLoader)
    {
      _fontLoader->StopLoadingAndClearFonts();
    }
    if (_fontSelector)
    {
      _fontSelector->UnregisterForInvalidationCallbacks(*this);
      static_cast<css::CSSFontSelector &>(*_fontSelector).ClearFonts();
    }
    if (_timelinesController)
    {
      _timelinesController->DetachFromDocument();
    }
  }

  void Document::DecrementReferencingNodeCount() noexcept
  {
    krys_debug_assert(_referencingNodeCount > 0);
    --_referencingNodeCount;
    if (!_referencingNodeCount && !GetRefCount())
    {
      PrepareToDeleteWithNoRefs();
      delete this;
    }
  }

  void Document::RemovedLastRef() noexcept
  {
    if (!_referencingNodeCount)
    {
      delete this;
      return;
    }

    // The nodes keep the document from here; its count resumes from zero.
    ResumeFromNoRefs();

    // If removing a child removes the last node reference, we don't want the document to be destroyed until
    // RemoveDetachedChildren returns, so we protect ourselves.
    IncrementReferencingNodeCount();

    krys_assert_msg(!HasLivingRenderTree(), "a document's render tree goes before its last reference");

    // We must make sure not to be retaining any of our children through these extra pointers or we will
    // create a reference cycle.
    _focusedElement = nullptr;
    _hoveredElement = nullptr;
    _activeElement = nullptr;
    _sequentialFocusNavigationStartingPoint = nullptr;
    _pendingScrollEventTargets.clear();
    _userActionElements->Clear();
    _pendingRenderTreeUpdate = nullptr;
    if (_fontLoader)
    {
      _fontLoader->StopLoadingAndClearFonts();
    }
    // FIXME: The title element, the parser, the selection, form controller, markers and CSS canvas elements,
    // once there are any.

    // The children are detached without their removing steps, so the elements in the top layer leave it
    // first.
    while (!_topLayerElements.empty())
    {
      Ref<Element> element = _topLayerElements.last();
      element->RemoveFromTopLayer();
    }
    _openDialogs.clear();
    _hintStackParent = nullptr;

    RemoveDetachedChildren();

    // Last, since it may delete the document.
    DecrementReferencingNodeCount();
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
    html::CEReactionsScope ceReactions;
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
    html::CEReactionsScope ceReactions;
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
    html::CEReactionsScope ceReactions;
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
    html::CEReactionsScope ceReactions;
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
    html::CEReactionsScope ceReactions;
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
    html::CEReactionsScope ceReactions;
    return mixins::ParentNode::Prepend(*this, nodes);
  }

  ExceptionOr<void> Document::Append(const List<NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return mixins::ParentNode::Append(*this, nodes);
  }

  ExceptionOr<void> Document::ReplaceChildren(const List<NodeOrString> &nodes) noexcept
  {
    html::CEReactionsScope ceReactions;
    return mixins::ParentNode::ReplaceChildren(*this, nodes);
  }

  ExceptionOr<void> Document::MoveBefore(Node &node, Node *refChild) noexcept
  {
    html::CEReactionsScope ceReactions;
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

    // https://wicg.github.io/scroll-to-text-fragment/#process-and-consume-fragment-directive
    _fragmentDirective = newURL.ConsumeFragmentDirective();

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
    Maybe<DOMString> target;
    for (auto &node : DescendantRange(*this))
    {
      auto *element = DynamicDowncast<html::HTMLElement>(&node);
      if (element == nullptr || !Is<html::HTMLBaseElement>(*element))
      {
        continue;
      }

      if (!href)
      {
        href = element->GetAttribute(u8"href");
        if (href && target)
        {
          break;
        }
      }
      if (!target)
      {
        target = element->GetAttribute(u8"target");
        if (target && href)
        {
          break;
        }
      }
    }

    _baseTarget = target.value_or(DOMString());

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

    KRYS_NODISCARD bool HasRenderer(const Element &element) noexcept
    {
      return element.Renderer();
    }
  }

  bool Document::HitTest(const HitTestRequest &request, HitTestResult &result) noexcept
  {
    return HitTest(request, result.GetHitTestLocation(), result);
  }

  bool Document::HitTest(const HitTestRequest &request, const HitTestLocation &location,
                         HitTestResult &result) noexcept
  {
    Ref protectedThis = ShareRef(*this);

    if (!GetRenderView())
    {
      return false;
    }

    // FIXME: Child frames (allowsAnyFrameContent: updating their layout too) come with frames.
    (void)UpdateLayout();
    if (!GetRenderView())
    {
      return false;
    }

    bool resultLayer = GetRenderView()->Layer()->HitTest(request, location, result);

    // The frame view's scrollbars are not the layer scrollbars tested by
    // RenderLayerScrollableArea::HitTestOverflowControls, so we need to test them separately here. In case of
    // using overlay scrollbars, the layer hit test will always work so we need to check the frame view's
    // scrollbars in that case too.
    if (!resultLayer || ScrollbarTheme::Theme().UsesOverlayScrollbars())
    {
      if (request.AllowsFrameScrollbars())
      {
        RefPtr<LocalFrameView> frameView = ShareRefPtr(View());
        IntPoint windowPoint = frameView->ContentsToWindow(location.RoundedPoint());
        if (RefPtr<Scrollbar> frameScrollbar = ShareRefPtr(frameView->ScrollbarAtPoint(windowPoint)))
        {
          result.SetScrollbar(krys::move(frameScrollbar));
          return true;
        }
      }
    }
    return resultLayer;
  }

  MouseEventWithHitTestResults Document::PrepareMouseEvent(const HitTestRequest &request,
                                                           const DoublePoint &documentPoint,
                                                           const PlatformMouseEvent &event) noexcept
  {
    if (!HasLivingRenderTree())
    {
      return MouseEventWithHitTestResults(event, HitTestResult(LayoutPoint()));
    }

    HitTestResult result {LayoutPoint(documentPoint)};
    HitTest(request, result);

    if (!request.ReadOnly())
    {
      // FIXME: Pointer capture (processPendingPointerCapture, targeting the capture element and reporting its
      // changes) comes with pointer events.
      RefPtr<Element> targetElement = ShareRefPtr(result.TargetElement());
      UpdateHoverActiveState(request, targetElement.get());
    }

    return MouseEventWithHitTestResults(event, result);
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
        for (RenderElement *current = newActiveElement->Renderer(); current; current = current->Parent())
        {
          auto *element = current->GetElement();
          if (!element || current->IsRenderTextOrLineBreak())
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

    // A removed sequential focus navigation starting point moves to where the node was, so navigation
    // continues from there: after its previous sibling, or at the start of its parent.
    // The specification doesn't say what happens; this follows other engines.
    if (_sequentialFocusNavigationStartingPoint
        && TreeQueries::IsShadowIncludingInclusiveAncestor(node, *_sequentialFocusNavigationStartingPoint))
    {
      if (auto *previousSibling = node.PreviousSibling())
      {
        _sequentialFocusNavigationStartingPoint = ShareRefPtr(previousSibling);
        _sequentialFocusNavigationStartingPointPosition =
          SequentialFocusNavigationStartingPointPosition::After;
      }
      else if (auto *parent = node.ParentNode(); parent && parent != this)
      {
        _sequentialFocusNavigationStartingPoint = ShareRefPtr<Node>(parent);
        _sequentialFocusNavigationStartingPointPosition =
          SequentialFocusNavigationStartingPointPosition::Inside;
      }
      else
      {
        _sequentialFocusNavigationStartingPoint = nullptr;
      }
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

    if (auto *frame = Frame())
    {
      frame->GetEventHandler().NodeWillBeRemoved(node);
      _selection->NodeWillBeRemoved(node);
    }
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

  void Document::AddPendingScrollEventTarget(ContainerNode &originalTarget,
                                             ScrollEventType eventType) noexcept
  {
    Ref<ContainerNode> target = [&]() -> Ref<ContainerNode>
    {
      if (auto *element = DynamicDowncast<html::HTMLElement>(originalTarget);
          element && element->IsTextControlInnerTextElement())
      {
        if (auto *shadowHost = TreeQueries::ShadowHost(*element))
        {
          return ShareRef<ContainerNode>(*shadowHost);
        }
      }
      return ShareRef(originalTarget);
    }();

    auto found =
      std::ranges::find_if(_pendingScrollEventTargets, [&](const auto &pair)
                           { return pair.first.get() == target.get() && pair.second == eventType; });
    if (found != _pendingScrollEventTargets.end())
    {
      return;
    }

    if (_pendingScrollEventTargets.empty())
    {
      ScheduleRenderingUpdate({RenderingUpdateStep::Scroll});
    }

    _pendingScrollEventTargets.emplace_back(krys::move(target), eventType);
  }

  void Document::ServiceCaretAnimation() noexcept
  {
    _selection->GetCaretAnimator().ServiceCaretAnimation();
  }

  DOMSelection *Document::GetSelection() noexcept
  {
    if (!Frame())
    {
      return nullptr;
    }
    if (!_domSelection)
    {
      _domSelection = CreateRef<DOMSelection>(*this);
    }
    return _domSelection.get();
  }

  boo::Page *Document::GetPage() const noexcept
  {
    auto *frame = Frame();
    return frame ? frame->GetPage() : nullptr;
  }

  CachedResourceLoader &Document::GetCachedResourceLoader() const noexcept
  {
    if (!_cachedResourceLoader)
    {
      _cachedResourceLoader = CachedResourceLoader::Create(const_cast<Document *>(this));
    }
    return *_cachedResourceLoader;
  }

  void Document::ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps) noexcept
  {
    // FIXME: The initial update of intersection observers, once there are any.
    if (auto *page = GetPage())
    {
      page->ScheduleRenderingUpdate(requestedSteps);
    }
  }

  void Document::RunScrollSteps() noexcept
  {
    // FIXME: Deferring the scroll events (shouldDeferScrollEvents), servicing scroll animations and scroll
    // anchoring, once there are those.

    // FIXME: The order of dispatching is not specified: https://github.com/WICG/visual-viewport/issues/66.
    auto currentTargets = std::exchange(_pendingScrollEventTargets, {});
    for (auto &[target, type] : currentTargets)
    {
      bool bubbles = Is<Document>(*target);
      auto eventName = type == ScrollEventType::Scroll ? EventNames::Scroll : EventNames::ScrollEnd;
      EventIDLInitializer<Event> initialize = [bubbles](Event &event)
      {
        event._bubbles = bubbles;
      };
      (void)EventDispatcher::FireEvent<Event>(DOMStringAtom(eventName), *target, krys::move(initialize));
    }

    if (_needsVisualViewportScrollEvent)
    {
      _needsVisualViewportScrollEvent = false;
      if (RefPtr<LocalDOMWindow> window = _domWindow)
      {
        (void)EventDispatcher::FireEvent(DOMStringAtom(EventNames::Scroll), window->GetVisualViewport());
      }
    }
  }

  void Document::SetNeedsVisualViewportResize() noexcept
  {
    _needsVisualViewportResizeEvent = true;
    ScheduleRenderingUpdate(RenderingUpdateStep::Resize);
  }

  void Document::SetNeedsVisualViewportScrollEvent() noexcept
  {
    if (!_needsVisualViewportScrollEvent)
    {
      ScheduleRenderingUpdate(RenderingUpdateStep::Scroll);
    }
    _needsVisualViewportScrollEvent = true;
  }

  void Document::SetNeedsDOMWindowResizeEvent() noexcept
  {
    _needsDOMWindowResizeEvent = true;
    ScheduleRenderingUpdate(RenderingUpdateStep::Resize);
  }

  void Document::RunResizeSteps() noexcept
  {
    // FIXME: Deferring the resize events (shouldDeferResizeEvents, and while the document is hidden), once
    // there are those.
    // FIXME: The order of dispatching is not specified: https://github.com/WICG/visual-viewport/issues/65.
    if (_needsDOMWindowResizeEvent)
    {
      _needsDOMWindowResizeEvent = false;
      if (RefPtr<LocalDOMWindow> window = _domWindow)
      {
        (void)EventDispatcher::FireEvent(DOMStringAtom(EventNames::Resize), *window);
      }
    }
    if (_needsVisualViewportResizeEvent)
    {
      _needsVisualViewportResizeEvent = false;
      if (RefPtr<LocalDOMWindow> window = _domWindow)
      {
        (void)EventDispatcher::FireEvent(DOMStringAtom(EventNames::Resize), window->GetVisualViewport());
      }
    }
  }

  void Document::SetCSSTarget(Element *newTarget) noexcept
  {
    if (_cssTarget.get() == newTarget)
    {
      return;
    }

    Maybe<style::PseudoClassChangeInvalidation> oldInvalidation;
    if (auto *oldTarget = _cssTarget.get())
    {
      oldInvalidation.emplace(*oldTarget, css::SelectorPseudoClass::Target, false);
    }

    Maybe<style::PseudoClassChangeInvalidation> newInvalidation;
    if (newTarget)
    {
      newInvalidation.emplace(*newTarget, css::SelectorPseudoClass::Target, true);
    }
    _cssTarget = CreateWeakPtr(newTarget);
  }

  void Document::AddTopLayerElement(Element &element) noexcept
  {
    krys_assert(&element.NodeDocument() == this && !element.IsInTopLayer());
    KRYS_MAYBE_UNUSED auto result = _topLayerElements.add(ShareRef(element));
    krys_assert(result.second);
    // FIXME: The auto popover list, with popovers (B4d).
  }

  void Document::RemoveTopLayerElement(Element &element) noexcept
  {
    krys_assert(&element.NodeDocument() == this && element.IsInTopLayer());
    Ref<Element> protectedElement = ShareRef(element);
    _topLayerPendingRemovals.remove(protectedElement);
    KRYS_MAYBE_UNUSED bool didRemove = _topLayerElements.remove(protectedElement);
    krys_assert(didRemove);
    // FIXME: The auto popover list, with popovers (B4d).
  }

  bool Document::IsPendingTopLayerRemoval(const Element &element) const noexcept
  {
    return element.IsInTopLayer()
           && _topLayerPendingRemovals.contains(ShareRef(const_cast<Element &>(element)));
  }

  void Document::AddPendingTopLayerRemoval(Element &element) noexcept
  {
    krys_assert(&element.NodeDocument() == this && element.IsInTopLayer());
    (void)_topLayerPendingRemovals.add(ShareRef(element));
  }

  void Document::ProcessTopLayerRemovals() noexcept
  {
    List<Ref<Element>> pendingRemovals {_topLayerPendingRemovals.begin(), _topLayerPendingRemovals.end()};
    for (auto &element : pendingRemovals)
    {
      if (!element->IsInTopLayer() || !IsPendingTopLayerRemoval(*element))
      {
        continue;
      }

      auto *renderer = element->Renderer();
      if (!renderer || renderer->Style().GetOverlay() == Overlay::None)
      {
        element->RemoveFromTopLayer();
      }
    }
  }

  html::HTMLDialogElement *Document::ActiveModalDialog() const noexcept
  {
    for (auto element = _topLayerElements.rbegin(); element != _topLayerElements.rend(); ++element)
    {
      if (auto *dialog = DynamicDowncast<html::HTMLDialogElement>(element->get());
          dialog && dialog->IsModal())
      {
        return dialog;
      }
    }
    return nullptr;
  }

  void Document::AddOpenDialog(html::HTMLDialogElement &dialog) noexcept
  {
    krys_debug_assert(!_openDialogs.contains(ShareRef(dialog)));
    _openDialogs.add(ShareRef(dialog));
  }

  void Document::RemoveOpenDialog(html::HTMLDialogElement &dialog) noexcept
  {
    _openDialogs.remove(ShareRef(dialog));
  }

  void Document::HandleLightDismiss(const MouseEvent &event, Node &target) noexcept
  {
    krys_debug_assert(event.IsTrusted());

    // Light dismiss open popovers: a release on the popover (or on an invoker of a popover) the press was on
    // hides the popovers above it.
    if (TopmostAutoOrHintPopover())
    {
      RefPtr<html::HTMLElement> ancestor = TopmostClickedPopover(target);
      if (event.Type() == EventNames::MouseDown)
      {
        _popoverPointerDownTarget = CreateWeakPtr(ancestor.get());
      }
      else if (event.Type() == EventNames::MouseUp)
      {
        bool sameTarget = ancestor.get() == _popoverPointerDownTarget.get();
        _popoverPointerDownTarget = nullptr;
        if (sameTarget)
        {
          HideAllPopoversUntil(ancestor.get(), FocusPreviousElement::No, FireEvents::Yes);
        }
      }
    }

    // Light dismiss open dialogs.
    if (_openDialogs.empty())
    {
      return;
    }

    // The nearest clicked dialog: none for a press on a modal dialog's backdrop (outside its bounds).
    auto nearestClickedDialog = [&]() -> html::HTMLDialogElement *
    {
      if (auto *dialog = DynamicDowncast<html::HTMLDialogElement>(target);
          dialog && dialog->Open() && dialog->IsModal())
      {
        auto bounds = dialog->BoundingClientRect();
        auto x = static_cast<float>(event.ClientX());
        auto y = static_cast<float>(event.ClientY());
        if (x < bounds.X() || x > bounds.MaxX() || y < bounds.Y() || y > bounds.MaxY())
        {
          return nullptr;
        }
      }
      auto *element = DynamicDowncast<Element>(target);
      for (element = element ? element : target.ParentElementInComposedTree(); element;
           element = element->ParentElementInComposedTree())
      {
        if (auto *dialog = DynamicDowncast<html::HTMLDialogElement>(*element); dialog && dialog->Open())
        {
          return dialog;
        }
      }
      return nullptr;
    };
    RefPtr<html::HTMLDialogElement> ancestor = ShareRefPtr(nearestClickedDialog());

    if (event.Type() == EventNames::MouseDown)
    {
      _dialogPointerDownTarget = CreateWeakPtr(ancestor.get());
      return;
    }

    if (event.Type() == EventNames::MouseUp)
    {
      bool sameTarget = ancestor.get() == _dialogPointerDownTarget.get();
      _dialogPointerDownTarget = nullptr;
      if (!sameTarget)
      {
        return;
      }
      Ref<html::HTMLDialogElement> topmostDialog = _openDialogs.last();
      if (ancestor.get() == topmostDialog.get())
      {
        return;
      }
      if (topmostDialog->ComputedClosedByState() != html::HTMLDialogElement::ClosedByState::Any)
      {
        return;
      }
      if (auto *closeWatcher = topmostDialog->GetCloseWatcher())
      {
        (void)closeWatcher->RequestToClose(false);
      }
    }
  }

  List<Ref<html::HTMLElement>> Document::ShowingAutoPopoverList() const noexcept
  {
    List<Ref<html::HTMLElement>> popovers;
    for (auto &element : _topLayerElements)
    {
      if (auto *popover = DynamicDowncast<html::HTMLElement>(element.get());
          popover && popover->IsPopoverShowing()
          && popover->GetPopoverData()->OpenedInPopoverMode() == PopoverState::Auto)
      {
        popovers.push_back(ShareRef(*popover));
      }
    }
    return popovers;
  }

  List<Ref<html::HTMLElement>> Document::ShowingHintPopoverList() const noexcept
  {
    List<Ref<html::HTMLElement>> popovers;
    for (auto &element : _topLayerElements)
    {
      if (auto *popover = DynamicDowncast<html::HTMLElement>(element.get());
          popover && popover->IsPopoverShowing()
          && popover->GetPopoverData()->OpenedInPopoverMode() == PopoverState::Hint)
      {
        popovers.push_back(ShareRef(*popover));
      }
    }
    return popovers;
  }

  html::HTMLElement *Document::TopmostAutoOrHintPopover() const noexcept
  {
    for (auto element = _topLayerElements.rbegin(); element != _topLayerElements.rend(); ++element)
    {
      if (auto *popover = DynamicDowncast<html::HTMLElement>(element->get());
          popover && popover->IsPopoverShowing()
          && popover->GetPopoverData()->OpenedInPopoverMode() == PopoverState::Hint)
      {
        return popover;
      }
    }
    auto autoPopovers = ShowingAutoPopoverList();
    return autoPopovers.empty() ? nullptr : autoPopovers.back().get();
  }

  void Document::HideAllPopoversUntil(html::HTMLElement *endpoint, FocusPreviousElement focusPreviousElement,
                                      FireEvents fireEvents) noexcept
  {
    auto hintPopovers = ShowingHintPopoverList();
    bool endpointIsHint =
      endpoint
      && std::ranges::find(hintPopovers, endpoint, [](auto &popover) { return popover.get(); })
           != hintPopovers.end();

    HidePopoverStackUntil(endpoint, PopoverState::Hint, focusPreviousElement, fireEvents);

    // If endpoint is a hint popover, the auto popovers are hidden but for those it opened from.
    RefPtr<html::HTMLElement> autoEndpoint = ShareRefPtr(endpoint);
    if (endpointIsHint)
    {
      autoEndpoint = ShareRefPtr(HintStackParent());
    }
    HidePopoverStackUntil(autoEndpoint.get(), PopoverState::Auto, focusPreviousElement, fireEvents);
  }

  void Document::HidePopoverStackUntil(html::HTMLElement *endpoint, PopoverState stackType,
                                       FocusPreviousElement focusPreviousElement,
                                       FireEvents fireEvents) noexcept
  {
    krys_debug_assert(stackType == PopoverState::Auto || stackType == PopoverState::Hint);
    auto popoverList = stackType == PopoverState::Auto ? ShowingAutoPopoverList() : ShowingHintPopoverList();

    auto endpointIt = std::ranges::find(popoverList, endpoint, [](auto &popover) { return popover.get(); });
    auto lastHideIndex =
      endpointIt == popoverList.end() ? 0uz : static_cast<size_t>(endpointIt - popoverList.begin()) + 1;
    List<Ref<html::HTMLElement>> toHide {popoverList.rbegin(), popoverList.rend() - lastHideIndex};
    List<Ref<html::HTMLElement>> toRemain {popoverList.begin(), popoverList.begin() + lastHideIndex};

    for (auto &popover : toHide)
    {
      (void)popover->HidePopoverInternal(focusPreviousElement, fireEvents, false, nullptr);
    }

    // Popovers shown while hiding (by beforetoggle listeners) are hidden too, without events.
    auto newPopoverList =
      stackType == PopoverState::Auto ? ShowingAutoPopoverList() : ShowingHintPopoverList();
    for (auto popover = newPopoverList.rbegin(); popover != newPopoverList.rend(); ++popover)
    {
      if (std::ranges::find(toRemain, *popover) != toRemain.end())
      {
        continue;
      }
      (void)(*popover)->HidePopoverInternal(focusPreviousElement, FireEvents::No, false, nullptr);
    }
  }

  void Document::SetHintStackParent(html::HTMLElement *element) noexcept
  {
    _hintStackParent = CreateWeakPtr(element);
  }

  RefPtr<html::HTMLElement> Document::TopmostClickedPopover(Node &node) const noexcept
  {
    auto *startElement = DynamicDowncast<Element>(node);
    if (!startElement)
    {
      startElement = node.ParentElementInComposedTree();
    }

    // The nearest inclusive open popover, and the nearest inclusive target popover: the showing auto or
    // hint popover an element is or is in, or the one it (a button) shows.
    html::HTMLElement *clickedPopover = nullptr;
    html::HTMLElement *targetPopover = nullptr;
    for (auto *element = startElement; element && (!clickedPopover || !targetPopover);
         element = element->ParentElementInComposedTree())
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element);
      if (!htmlElement)
      {
        continue;
      }
      if (!clickedPopover && htmlElement->IsPopoverShowing()
          && htmlElement->GetPopoverData()->OpenedInPopoverMode() != PopoverState::None)
      {
        clickedPopover = htmlElement;
      }
      if (!targetPopover)
      {
        auto isShowingAutoOrHintPopover = [](html::HTMLElement *popover)
        {
          return popover && popover->IsPopoverShowing()
                 && (popover->GetPopoverState() == PopoverState::Auto
                     || popover->GetPopoverState() == PopoverState::Hint);
        };
        RefPtr<html::HTMLElement> popover;
        if (auto *button = DynamicDowncast<html::HTMLButtonElement>(*htmlElement))
        {
          RefPtr<Element> commandForElement = button->CommandForElement();
          popover =
            ShareRefPtr(commandForElement ? DynamicDowncast<html::HTMLElement>(*commandForElement) : nullptr);
        }
        if (!isShowingAutoOrHintPopover(popover.get()))
        {
          auto *control = DynamicDowncast<html::HTMLFormControlElement>(*htmlElement);
          popover = control ? control->PopoverTargetElement() : nullptr;
        }
        if (isShowingAutoOrHintPopover(popover.get()))
        {
          targetPopover = popover.get();
        }
      }
    }

    // The popover stack position: hint popovers are above auto popovers, and 0 is neither.
    auto autoPopovers = ShowingAutoPopoverList();
    auto hintPopovers = ShowingHintPopoverList();
    auto stackPosition = [&](html::HTMLElement *popover) -> size_t
    {
      if (!popover)
      {
        return 0;
      }
      auto byPointer = [](auto &element)
      {
        return element.get();
      };
      if (auto it = std::ranges::find(hintPopovers, popover, byPointer); it != hintPopovers.end())
      {
        return static_cast<size_t>(it - hintPopovers.begin()) + autoPopovers.size() + 1;
      }
      if (auto it = std::ranges::find(autoPopovers, popover, byPointer); it != autoPopovers.end())
      {
        return static_cast<size_t>(it - autoPopovers.begin()) + 1;
      }
      return 0;
    };
    return ShareRefPtr(stackPosition(clickedPopover) > stackPosition(targetPopover) ? clickedPopover
                                                                                    : targetPopover);
  }

  Element *Document::FindAnchor(DOMStringView name) const noexcept
  {
    if (name.empty())
    {
      return nullptr;
    }
    if (auto element = const_cast<Document *>(this)->GetElementById(name))
    {
      return element.get();
    }
    for (auto &node : DescendantRange(const_cast<Document &>(*this)))
    {
      auto *anchor = DynamicDowncast<html::HTMLAnchorElement>(node);
      if (!anchor)
      {
        continue;
      }
      auto anchorName = anchor->GetAttribute(u8"name");
      if (!anchorName)
      {
        continue;
      }
      // Quirks mode, ASCII case-insensitive comparison of names.
      // FIXME: This behavior is not mentioned in the HTML specification. We should either remove this or get
      // this into the specification.
      if (InQuirksMode() ? text::IsASCIICaselessEqual(DOMStringView(*anchorName), name) : *anchorName == name)
      {
        return anchor;
      }
    }
    return nullptr;
  }

  bool Document::HaveStylesheetsLoaded() const noexcept
  {
    return !const_cast<Document *>(this)->StyleScope().HasPendingSheets() || _ignorePendingStylesheets;
  }

  void Document::DidRemoveAllPendingStylesheet() noexcept
  {
    // FIXME: Running the scripts waiting for style sheets, once there are scripts.
    if (!_gotoAnchorNeededAfterStylesheetsLoad || !View())
    {
      return;
    }

    // https://html.spec.whatwg.org/multipage/browsing-the-web.html#try-to-scroll-to-the-fragment
    html::EventLoop::Current().QueueTask(html::TaskSource::Networking, *this,
                                         [protectedThis = ShareRef(*this), this]
                                         {
                                           auto *frameView = View();
                                           if (!frameView)
                                           {
                                             return;
                                           }
                                           if (!HaveStylesheetsLoaded())
                                           {
                                             _gotoAnchorNeededAfterStylesheetsLoad = true;
                                             return;
                                           }
                                           _gotoAnchorNeededAfterStylesheetsLoad = false;
                                           (void)frameView->ScrollToFragment(_url);
                                         });
  }

  void Document::EnqueueHashchangeEvent(USVString &&oldURL, USVString &&newURL) noexcept
  {
    // FIXME: popstate event and hashchange event are supposed to fire in a single task.
    QueueTaskToDispatchEventOnWindow(html::TaskSource::DOMManipulation,
                                     html::HashChangeEvent::Create(krys::move(oldURL), krys::move(newURL)));
  }

  LocalDOMWindow *Document::Window() const noexcept
  {
    return _domWindow.get();
  }

  bool Document::IsFullyActive() const noexcept
  {
    // A Document d is fully active if d is the active document of a navigable navigable, and either navigable
    // is a top-level traversable or navigable's container document is fully active.
    return _frame && _frame->Document() == this;
  }

  void Document::StatePopped(RefPtr<html::SerializedScriptValue> &&stateObject) noexcept
  {
    if (!Frame())
    {
      return;
    }

    RefPtr<LocalDOMWindow> window = _domWindow;
    auto event =
      html::PopStateEvent::Create(krys::move(stateObject), window ? &window->GetHistory() : nullptr);
    DispatchWindowEvent(*event);
  }

  void Document::UpdateURLForPushOrReplaceState(const url::URL &url) noexcept
  {
    if (!Frame())
    {
      return;
    }

    SetURL(url::URL(url));
    // FIXME: The outgoing referrer and the document loader's request URL, once there are those.
  }

  int32 Document::CircularSequentialID() noexcept
  {
    ++_circularSequentialID;
    if (_circularSequentialID <= 0)
    {
      _circularSequentialID = 1;
    }
    return _circularSequentialID;
  }

  bool Document::AddTimeout(int32 timeoutId, DOMTimer &timer) noexcept
  {
    return _timeouts.emplace(timeoutId, ShareRef(timer)).second;
  }

  RefPtr<DOMTimer> Document::TakeTimeout(int32 timeoutId) noexcept
  {
    auto it = _timeouts.find(timeoutId);
    if (it == _timeouts.end())
    {
      return nullptr;
    }
    RefPtr<DOMTimer> timer = it->second;
    _timeouts.erase(it);
    return timer;
  }

  int32 Document::RequestAnimationFrame(FrameRequestCallback &&callback) noexcept
  {
    if (!_scriptedAnimationController)
    {
      _scriptedAnimationController = CreateUnique<ScriptedAnimationController>(*this);
    }
    return _scriptedAnimationController->RegisterCallback(krys::move(callback));
  }

  void Document::CancelAnimationFrame(int32 id) noexcept
  {
    if (_scriptedAnimationController)
    {
      _scriptedAnimationController->CancelCallback(id);
    }
  }

  void Document::ServiceRequestAnimationFrameCallbacks() noexcept
  {
    if (_scriptedAnimationController && _domWindow)
    {
      _scriptedAnimationController->ServiceRequestAnimationFrameCallbacks(_domWindow->FrozenNowTimestamp());
    }
  }

  int32 Document::RequestIdleCallback(IdleRequestCallback &&callback,
                                      MonotonicTime::duration timeout) noexcept
  {
    if (!_idleCallbackController)
    {
      _idleCallbackController = CreateUnique<IdleCallbackController>(*this);
    }
    return _idleCallbackController->QueueIdleCallback(krys::move(callback), timeout);
  }

  void Document::CancelIdleCallback(int32 id) noexcept
  {
    if (_idleCallbackController)
    {
      _idleCallbackController->RemoveIdleCallback(id);
    }
  }

  bool Document::HasPendingIdleCallback() const noexcept
  {
    return _idleCallbackController && _idleCallbackController->HasPendingIdleCallback();
  }

  void Document::Suspend(ReasonForSuspension reason) noexcept
  {
    if (_isSuspended)
    {
      return;
    }

    // FIXME: Elements' document suspension callbacks (media), view transitions, the page's overlay
    // scrollbars, the composited layers' pending update and font loading, once there are those.
    SuspendScheduledTasks(reason);

    // The frame's timers: the layout, the animations and autoscrolling stop.
    if (RefPtr<LocalFrameView> view = ShareRefPtr(View()))
    {
      view->LayoutContext().UnscheduleLayout();
      if (_timelinesController)
      {
        _timelinesController->SuspendAnimations();
      }
      Frame()->GetEventHandler().StopAutoscrollTimer();
    }

    _isSuspended = true;
  }

  void Document::Resume(ReasonForSuspension reason) noexcept
  {
    if (!_isSuspended)
    {
      return;
    }

    if (_timelinesController)
    {
      _timelinesController->ResumeAnimations();
    }

    ResumeScheduledTasks(reason);

    _isSuspended = false;
  }

  void Document::SuspendScheduledTasks(ReasonForSuspension reason) noexcept
  {
    if (_scheduledTasksAreSuspended)
    {
      return;
    }

    if (_scriptedAnimationController)
    {
      _scriptedAnimationController->Suspend();
    }
    SuspendActiveDOMObjects(reason);

    _scheduledTasksAreSuspended = true;
  }

  void Document::ResumeScheduledTasks(ReasonForSuspension reason) noexcept
  {
    if (!_scheduledTasksAreSuspended || _reasonForSuspendingActiveDOMObjects != reason)
    {
      return;
    }

    ResumeActiveDOMObjects(reason);
    if (_scriptedAnimationController)
    {
      _scriptedAnimationController->Resume();
    }

    _scheduledTasksAreSuspended = false;
  }

  void Document::SuspendActiveDOMObjects(ReasonForSuspension reason) noexcept
  {
    if (_activeDOMObjectsAreSuspended)
    {
      return;
    }

    _activeDOMObjectsAreSuspended = true;
    _reasonForSuspendingActiveDOMObjects = reason;
    for (auto &[id, timer] : _timeouts)
    {
      timer->Suspend();
    }
  }

  void Document::ResumeActiveDOMObjects(ReasonForSuspension reason) noexcept
  {
    if (_reasonForSuspendingActiveDOMObjects != reason)
    {
      return;
    }

    _activeDOMObjectsAreSuspended = false;
    for (auto &[id, timer] : _timeouts)
    {
      timer->Resume();
    }
  }

  void Document::SetBackForwardCacheState(BackForwardCacheState state) noexcept
  {
    if (_backForwardCacheState == state)
    {
      return;
    }

    _backForwardCacheState = state;

    switch (state)
    {
      case BackForwardCacheState::InBackForwardCache:
        if (RefPtr<LocalFrameView> frameView = ShareRefPtr(View()))
        {
          frameView->CacheCurrentScrollState();
        }
        // FIXME: The pointer lock and the shared object pool, once there are those.
        StyleScope().ClearResolver();
        _styleRecalcTimer.Stop();
        break;
      case BackForwardCacheState::NotInBackForwardCache:
        if (ChildNeedsStyleRecalc())
        {
          ScheduleStyleRecalc();
        }
        break;
      case BackForwardCacheState::AboutToEnterBackForwardCache: break;
    }
  }

  void Document::DetachFromCachedFrame() noexcept
  {
    krys_debug_assert(_backForwardCacheState == BackForwardCacheState::InBackForwardCache);
    _frame = nullptr;
  }

  void Document::StopActiveDOMObjects() noexcept
  {
    _activeDOMObjectsAreStopped = true;

    // Stopping a timer releases its action, which may hold the document's nodes.
    auto timeouts = std::exchange(_timeouts, {});
    for (auto &[id, timer] : timeouts)
    {
      timer->Stop();
    }
    _scriptedAnimationController = nullptr;
    _idleCallbackController = nullptr;

    // Its close watchers are destroyed.
    if (auto *closeWatcherManager = _domWindow ? _domWindow->CloseWatcherManagerIfExists() : nullptr)
    {
      closeWatcherManager->DestroyAll();
    }
  }

  LocalDOMWindow *Document::DefaultView() const noexcept
  {
    // If this's browsing context is null, then return null; otherwise its WindowProxy.
    return Frame() ? _domWindow.get() : nullptr;
  }

  void Document::DispatchWindowEvent(Event &event) noexcept
  {
    if (RefPtr<LocalDOMWindow> window = _domWindow)
    {
      (void)EventDispatcher::DispatchToTarget(event, window.get());
    }
  }

  void Document::QueueTaskToDispatchEventOnWindow(html::TaskSource source, Ref<Event> &&event) noexcept
  {
    html::EventLoop::Current().QueueTask(source, *this,
                                         [protectedThis = ShareRef(*this), event = krys::move(event)]
                                         { protectedThis->DispatchWindowEvent(*event); });
  }

  EventTarget *Document::GetParent(Event &event) const noexcept
  {
    // A Document's get the parent algorithm, given an event, returns null if event's type attribute value is
    // "load" or the Document does not have a browsing context; otherwise the Document's relevant global
    // object.
    if (event.Type() == EventNames::Load || !Frame())
    {
      return nullptr;
    }
    return _domWindow.get();
  }

  HighlightRegistry &Document::GetHighlightRegistry() noexcept
  {
    if (!_highlightRegistry)
    {
      _highlightRegistry = HighlightRegistry::Create();
    }
    return *_highlightRegistry;
  }

  HighlightRegistry &Document::FragmentHighlightRegistry() noexcept
  {
    if (!_fragmentHighlightRegistry)
    {
      _fragmentHighlightRegistry = HighlightRegistry::Create();
    }
    return *_fragmentHighlightRegistry;
  }

  HighlightRegistry *Document::HighlightRegistryIfExists() const noexcept
  {
    return _highlightRegistry.get();
  }

  HighlightRegistry *Document::FragmentHighlightRegistryIfExists() const noexcept
  {
    return _fragmentHighlightRegistry.get();
  }

  bool Document::HasHighlight() const noexcept
  {
    return (_highlightRegistry && !_highlightRegistry->IsEmpty())
           || (_fragmentHighlightRegistry && !_fragmentHighlightRegistry->IsEmpty());
  }

  namespace
  {
    void CollectHighlightRangesFromRegister(List<WeakPtr<HighlightRange>> &highlightRanges,
                                            const HighlightRegistry &highlightRegistry) noexcept
    {
      for (auto &[name, highlight] : highlightRegistry.Map())
      {
        for (auto &highlightRange : highlight->HighlightRanges())
        {
          // FIXME: Only the live ranges that changed (Range::didChangeForHighlight), once ranges track it.
          if (!highlightRange->StartPosition().IsNull() && !highlightRange->EndPosition().IsNull()
              && !highlightRange->Range().IsLiveRange())
          {
            continue;
          }

          auto simpleRange = MakeSimpleRange(highlightRange->Range());
          if (&simpleRange.StartContainer().TreeScope() != &simpleRange.EndContainer().TreeScope())
          {
            continue;
          }
          highlightRanges.push_back(CreateWeakPtr(highlightRange.get()));
        }
      }
    }
  }

  void Document::UpdateHighlightPositions() noexcept
  {
    List<WeakPtr<HighlightRange>> highlightRanges;
    if (_highlightRegistry)
    {
      CollectHighlightRangesFromRegister(highlightRanges, *_highlightRegistry);
    }
    if (_fragmentHighlightRegistry)
    {
      CollectHighlightRangesFromRegister(highlightRanges, *_fragmentHighlightRegistry);
    }

    for (auto &weakRangeData : highlightRanges)
    {
      RefPtr<HighlightRange> highlightRange = ShareRefPtr(weakRangeData.get());
      if (!highlightRange)
      {
        continue;
      }

      VisibleSelection visibleSelection(MakeSimpleRange(highlightRange->Range()));
      auto startPosition = visibleSelection.VisibleStart().DeepEquivalent();
      auto endPosition = visibleSelection.VisibleEnd().DeepEquivalent();
      if (!weakRangeData.get())
      {
        continue;
      }

      if (auto simpleRange = MakeSimpleRange(highlightRange->StartPosition(), highlightRange->EndPosition()))
      {
        if (auto init = StaticRangeInit::Create(simpleRange->Start, simpleRange->End); !init.HasException())
        {
          Highlight::RepaintRange(*CreateRef<StaticRange>(init.ReleaseValue()));
        }
      }

      if (!startPosition.IsNull())
      {
        highlightRange->SetStartPosition(krys::move(startPosition));
      }
      if (!endPosition.IsNull())
      {
        highlightRange->SetEndPosition(krys::move(endPosition));
      }

      Highlight::RepaintRange(highlightRange->Range());
    }
  }

  void Document::FinishedParsing() noexcept
  {
    // https://html.spec.whatwg.org/#the-end
    Ref<Document> protectedThis = ShareRef(*this);
    bool wasLoading = _parsing || _currentDocumentReadiness == html::DocumentReadyState::Loading;
    SetParsing(false);

    if (wasLoading)
    {
      // Update the current document readiness to "interactive", then fire DOMContentLoaded at the Document,
      // with its bubbles attribute initialized to true.
      DocumentAlgorithms::UpdateCurrentDocumentReadiness(*this, html::DocumentReadyState::Interactive);
      if (!_loadTiming.DOMContentLoadedEventStartTime)
      {
        _loadTiming.DOMContentLoadedEventStartTime = _domWindow ? _domWindow->NowTimestamp() : 0;
        auto event = Event::Create(DOMStringAtom(EventNames::DOMContentLoaded), true, false);
        (void)EventDispatcher::DispatchToTarget(*event, this);
        _loadTiming.DOMContentLoadedEventEndTime = _domWindow ? _domWindow->NowTimestamp() : 0;
      }
    }

    if (auto *frame = Frame())
    {
      frame->Loader().FinishedParsing();
    }
  }

  void Document::DecrementLoadEventDelayCount() noexcept
  {
    krys_debug_assert(_loadEventDelayCount);
    if (--_loadEventDelayCount)
    {
      return;
    }
    // The frame checks whether the document has loaded on a later task.
    html::EventLoop::Current().QueueTask(html::TaskSource::DOMManipulation, *this,
                                         [protectedThis = ShareRef(*this)]
                                         {
                                           if (auto *frame = protectedThis->Frame())
                                           {
                                             frame->Loader().CheckCompleted();
                                           }
                                         });
  }

  html::FormController &Document::GetFormController() noexcept
  {
    if (!_formController)
    {
      _formController = CreateUnique<html::FormController>();
    }
    return *_formController;
  }

  List<DOMString> Document::FormElementsState() const noexcept
  {
    if (!_formController)
    {
      return {};
    }
    return _formController->FormElementsState(*this);
  }

  void Document::SetStateForNewFormElements(const List<DOMString> &stateVector) noexcept
  {
    if (stateVector.empty() && !_formController)
    {
      return;
    }
    GetFormController().SetStateForNewFormElements(stateVector);
  }

  void Document::ImplicitClose() noexcept
  {
    Ref<Document> protectedThis = ShareRef(*this);
    DispatchWindowLoadEvent();
    DispatchPageshowEvent(false);

    // An event handler may have removed the frame.
    if (!Frame())
    {
      return;
    }

    UpdateStyleIfNeeded();

    // Always do a layout after loading if needed.
    RefPtr<LocalFrameView> view = ShareRefPtr(View());
    if (view && GetRenderView() && (!GetRenderView()->FirstChild() || GetRenderView()->NeedsLayout()))
    {
      view->LayoutContext().Layout();
      (void)view->LayoutContext().UpdateCompositingLayersAfterLayoutIfNeeded();
    }
  }

  void Document::DispatchWindowLoadEvent() noexcept
  {
    RefPtr<LocalDOMWindow> window = _domWindow;
    if (!window)
    {
      return;
    }

    // Fire an event named load at window, with legacy target override flag set.
    if (!_loadTiming.LoadEventStartTime)
    {
      _loadTiming.LoadEventStartTime = window->NowTimestamp();
    }
    (void)EventDispatcher::FireEvent<Event>(DOMStringAtom(EventNames::Load), *window, nullptr, true);
    if (!_loadTiming.LoadEventEndTime)
    {
      _loadTiming.LoadEventEndTime = window->NowTimestamp();
    }
    _loadEventFinished = true;
    GetCachedResourceLoader().DocumentDidFinishLoadEvent();
  }

  void Document::ProcessMetaHttpEquiv(DOMStringView equiv, DOMStringView content) noexcept
  {
    if (text::IsASCIICaselessEqual(equiv, utf8_stringview(u8"content-language")))
    {
      // https://html.spec.whatwg.org/#attr-meta-http-equiv-content-language
      // If the element's content attribute contains a U+002C COMMA character (,) then return. Collect a
      // sequence of code points that are not ASCII whitespace, after skipping ASCII whitespace; if it's
      // empty, return; otherwise it's the pragma-set default language.
      if (content.find(u8',') != DOMStringView::npos)
      {
        return;
      }
      size_t start = 0;
      while (start < content.size() && text::IsASCIIWhitespace(content[start]))
      {
        ++start;
      }
      size_t end = start;
      while (end < content.size() && !text::IsASCIIWhitespace(content[end]))
      {
        ++end;
      }
      if (end > start)
      {
        SetContentLanguage(DOMString(content.substr(start, end - start)));
      }
      return;
    }

    if (text::IsASCIICaselessEqual(equiv, utf8_stringview(u8"default-style")))
    {
      // https://html.spec.whatwg.org/#attr-meta-http-equiv-default-style
      if (!content.empty())
      {
        StyleScope().SetPreferredStylesheetSetName(DOMString(content));
      }
      return;
    }

    if (text::IsASCIICaselessEqual(equiv, utf8_stringview(u8"refresh")))
    {
      // https://html.spec.whatwg.org/#shared-declarative-refresh-steps
      // If document's will declaratively refresh is true, then return.
      auto *frame = Frame();
      if (_willDeclarativelyRefresh || !frame)
      {
        return;
      }
      auto directive = html::ParseRefresh(content);
      if (!directive)
      {
        return;
      }
      auto url = directive->URL.empty() ? GetURL() : CompleteURL(directive->URL);
      if (!url.IsValid())
      {
        return;
      }
      _willDeclarativelyRefresh = true;
      frame->GetNavigationScheduler().ScheduleRedirect(*this, directive->Delay, url, IsMetaRefresh::Yes);
      return;
    }

    // FIXME: Content-Security-Policy and the other pragmas, once there's a security policy; Set-Cookie is
    // obsolete and ignored, as are X-UA-Compatible and Content-Type (the parser reads the charset).
  }

  void Document::SetContentLanguage(DOMString &&language) noexcept
  {
    if (_contentLanguage == language)
    {
      return;
    }
    _contentLanguage = krys::move(language);

    // Recalculate style so language is used when selecting the initial font, and :lang() matches.
    StyleScope().DidChangeStyleSheetEnvironment();
  }

  boo::Location *Document::GetLocation() const noexcept
  {
    // Return this's relevant global object's Location object, if this is fully active, and null otherwise.
    return Frame() && _domWindow ? &_domWindow->GetLocation() : nullptr;
  }

  void Document::DispatchPagehideEvent(bool persisted) noexcept
  {
    // If the document's page showing is true: set it to false, and fire a page transition event named
    // pagehide at its relevant global object with persisted.
    if (!_pageShowing)
    {
      return;
    }
    _pageShowing = false;
    if (RefPtr<LocalDOMWindow> window = _domWindow)
    {
      auto event = html::PageTransitionEvent::Create(DOMStringAtom(EventNames::PageHide), persisted);
      (void)EventDispatcher::DispatchToTarget(*event, window.get(), true);
    }
  }

  void Document::DispatchPageshowEvent(bool persisted) noexcept
  {
    // If the Document's page showing is false, then set it to true and fire a page transition event named
    // pageshow at the window with false.
    if (_pageShowing)
    {
      return;
    }
    _pageShowing = true;
    if (RefPtr<LocalDOMWindow> window = _domWindow)
    {
      auto event = html::PageTransitionEvent::Create(DOMStringAtom(EventNames::PageShow), persisted);
      (void)EventDispatcher::DispatchToTarget(*event, window.get(), true);
    }
  }

  void Document::SetSequentialFocusNavigationStartingPoint(Node *node) noexcept
  {
    _sequentialFocusNavigationStartingPointPosition = SequentialFocusNavigationStartingPointPosition::At;
    // Setting the starting point to the document or its document element means navigation starts from the
    // start of the document.
    if (!node || node == this || node == TreeQueries::DocumentElement(*this))
    {
      _sequentialFocusNavigationStartingPoint = nullptr;
      return;
    }
    _sequentialFocusNavigationStartingPoint = ShareRefPtr(node);
  }

  void Document::SetFocusedAndActive(bool flag) noexcept
  {
    if (_isFocusedAndActive == flag)
    {
      return;
    }
    _isFocusedAndActive = flag;

    _selection->SetFocused(flag);

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

  boo::Locale &Document::GetCachedLocale(utf8_stringview locale) const noexcept
  {
    utf8_string localeKey(locale);
    if (locale.empty() || !GetSettings().LangAttributeAwareFormControlUIEnabled)
    {
      localeKey = DefaultLanguage();
    }
    auto [it, isNewEntry] = _localeCache.try_emplace(localeKey);
    if (isNewEntry)
    {
      it->second = boo::Locale::Create(localeKey);
    }
    return *it->second;
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

    _renderView->SetIsInWindow(true);

    ResolveStyle(ResolveStyleType::Rebuild);
  }

  void Document::DestroyRenderTree() noexcept
  {
    krys_debug_assert(HasLivingRenderTree());

    SetForScope change(_renderTreeBeingDestroyed, true);

    // FIXME: The accessibility cache and telling the view, once there are any.

    // documentWillBecomeInactive
    if (auto *frameView = View())
    {
      frameView->SetIsInWindow(false);
    }

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

  void Document::CreateDOMWindow() noexcept
  {
    krys_assert(Frame());
    krys_assert(!_domWindow);

    _domWindow = LocalDOMWindow::Create(*this);
    // A document in a browsing context has its window's custom element registry, which is created with it.
    (void)_domWindow->EnsureCustomElementRegistry();
  }

  void Document::DidBecomeCurrentDocumentInFrame(LocalFrame &frame) noexcept
  {
    _frame = WeakPtr<LocalFrame>::WithRef(frame);

    // FIXME: The document loader's writer makes the window as it makes the document, reusing the initial
    // empty document's (takeDOMWindowFrom), once there's document loading (B3b).
    if (!_domWindow)
    {
      CreateDOMWindow();
    }

    if (!HasLivingRenderTree() && View())
    {
      CreateRenderTree();
    }

    // FIXME: Viewport arguments, scheduled task and animation suspension, and quirks, once there are any.
  }

  void Document::WillBeRemovedFromFrame() noexcept
  {
    // FIXME: Unload, page hide and the other steps of unloading the document, once there's navigation; the
    // back/forward cache suspends the document's active objects rather than stopping them (B3d).
    StopActiveDOMObjects();
    _selection->WillBeRemovedFromFrame();

    if (HasLivingRenderTree())
    {
      DestroyRenderTree();
    }

    _styleRecalcTimer.Stop();

    if (_timelinesController)
    {
      _timelinesController->DetachFromDocument();
    }
    _timeline = nullptr;

    _frame = nullptr;
  }

  DocumentTimeline &Document::Timeline() noexcept
  {
    if (!_timeline)
    {
      _timeline = DocumentTimeline::Create(*this);
    }
    return *_timeline;
  }

  DocumentTimeline *Document::ExistingTimeline() const noexcept
  {
    return _timeline.get();
  }

  AnimationTimelinesController &Document::EnsureTimelinesController() noexcept
  {
    if (!_timelinesController)
    {
      _timelinesController = CreateUnique<AnimationTimelinesController>(*this);
    }
    return *_timelinesController;
  }

  void Document::KeyframesRuleDidChange(const css::CSSOMString &name) noexcept
  {
    for (auto *animation : WebAnimation::Instances())
    {
      auto *cssAnimation = DynamicDowncast<CSSAnimation>(animation);
      if (!cssAnimation || !cssAnimation->IsRelevant())
      {
        continue;
      }

      if (cssAnimation->AnimationName().View() != name)
      {
        continue;
      }

      auto owningElement = cssAnimation->OwningElement();
      if (!owningElement || !owningElement->Element.IsConnected()
          || &owningElement->Element.NodeDocument() != this)
      {
        continue;
      }

      cssAnimation->KeyframesRuleDidChange();
    }
  }

  void Document::UpdateAnimationsAndSendEvents() noexcept
  {
    // FIXME: The window's frozen timestamp, once there's a window; the rendering update's timestamp stands
    // in.
    auto *page = GetPage();
    if (!page)
    {
      return;
    }

    if (auto *timelinesController = TimelinesController())
    {
      timelinesController->UpdateAnimationsAndSendEvents(Seconds {page->LastRenderingUpdateTimestamp()});
    }
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

  Document::UpdateLayoutResult Document::UpdateLayout(EnumSet<LayoutOptions> layoutOptions) noexcept
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

      if (layoutOptions.Contains(LayoutOptions::UpdateCompositingLayers)
          && layoutContext.UpdateCompositingLayersAfterLayoutIfNeeded())
      {
        result = UpdateLayoutResult::ChangesDone;
      }
    }

    return result;
  }

  css::CSSFontSelector *Document::FontSelectorIfExists() const noexcept
  {
    return static_cast<css::CSSFontSelector *>(_fontSelector.get());
  }

  DocumentFontLoader &Document::GetFontLoader() const noexcept
  {
    if (!_fontLoader)
    {
      _fontLoader = UniquePtr<DocumentFontLoader>(new DocumentFontLoader(const_cast<Document &>(*this)));
    }
    return *_fontLoader;
  }

  RefPtr<FontLoadRequest> Document::GetFontLoadRequest(utf8_stringview url, bool isSVG,
                                                       bool isInitiatingElementInUserAgentShadowTree,
                                                       LoadedFromOpaqueSource loadedFromOpaqueSource) noexcept
  {
    CachedResourceHandle<CachedFont> cachedFont = GetFontLoader().GetCachedFont(
      CompleteURL(url), isSVG, isInitiatingElementInUserAgentShadowTree, loadedFromOpaqueSource);
    if (!cachedFont)
    {
      return nullptr;
    }
    return CachedFontLoadRequest::Create(*cachedFont);
  }

  void Document::BeginLoadingFontSoon(FontLoadRequest &request) noexcept
  {
    CachedResourceHandle<CachedFont> font = &Downcast<CachedFontLoadRequest>(request).GetCachedFont();
    GetFontLoader().BeginLoadingFontSoon(*font);
  }

  void Document::SuspendFontLoading() noexcept
  {
    if (_fontLoader)
    {
      _fontLoader->SuspendFontLoading();
    }
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
    // FIXME: Not in the back/forward cache, once there's navigation.
    if (_styleRecalcTimer.IsActive())
    {
      return;
    }

    krys_debug_assert(ChildNeedsStyleRecalc() || _needsFullStyleRebuild);

    _styleRecalcTimer.StartOneShot({});

    // Any change repaints the whole page, so a style change needs a rendering update to paint it.
    // FIXME: Scheduled by the repaints instead (the layer flush), once there's repaint tracking.
    ScheduleRenderingUpdate({});
  }

  void Document::UnscheduleStyleRecalc() noexcept
  {
    krys_debug_assert(!ChildNeedsStyleRecalc());

    _styleRecalcTimer.Stop();
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

  void Document::AddElementWithPendingUserAgentShadowTreeUpdate(html::HTMLInputElement &element) noexcept
  {
    krys_debug_assert(&element.NodeDocument() == this);
    krys_debug_assert(!HasElementWithPendingUserAgentShadowTreeUpdate(element));
    _elementsWithPendingUserAgentShadowTreeUpdates.push_back(
      WeakPtr<html::HTMLInputElement>::WithRef(element));
  }

  void Document::RemoveElementWithPendingUserAgentShadowTreeUpdate(html::HTMLInputElement &element) noexcept
  {
    krys_debug_assert(&element.NodeDocument() == this);
    std::erase_if(_elementsWithPendingUserAgentShadowTreeUpdates,
                  [&](const auto &entry) { return !entry || entry.get() == &element; });
  }

  bool
    Document::HasElementWithPendingUserAgentShadowTreeUpdate(html::HTMLInputElement &element) const noexcept
  {
    return std::ranges::any_of(_elementsWithPendingUserAgentShadowTreeUpdates,
                               [&](const auto &entry) { return entry.get() == &element; });
  }

  void Document::ResolveStyle(ResolveStyleType type) noexcept
  {
    if (_inStyleRecalc)
    {
      return;
    }

    // FIXME: We should update style on our ancestor chain before proceeding, however doing so at
    // the time this comment was originally written caused several tests to crash.

    // FIXME: Do this user agent shadow tree update per tree scope.
    auto pendingUserAgentShadowTreeUpdates = _elementsWithPendingUserAgentShadowTreeUpdates;
    for (auto &weakElement : pendingUserAgentShadowTreeUpdates)
    {
      if (RefPtr<html::HTMLInputElement> element = ShareRefPtr(weakElement.get()))
      {
        element->UpdateUserAgentShadowTree();
      }
    }

    StyleScope().FlushPendingUpdate();
    if (auto *frameView = View())
    {
      frameView->WillRecalcStyle();
    }

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
            SetForScope interleavedLayoutScope(_isInStyleInterleavedLayout, true);

            UpdateRenderTree(krys::move(styleUpdate));

            if (auto *frameView = View(); frameView && frameView->LayoutContext().NeedsLayout())
            {
              frameView->LayoutContext().InterleavedLayout();
            }
          }

          styleUpdate = resolver.Resolve();
        }

        SetHasValidStyle();
        ClearChildNeedsStyleRecalc();
        UnscheduleStyleRecalc();

        _inStyleRecalc = false;

        if (_fontLoader)
        {
          _fontLoader->LoadPendingFonts();
        }

        if (styleUpdate)
        {
          UpdateRenderTree(krys::move(styleUpdate));
        }
      }

      if (auto *frameView = View())
      {
        (void)frameView->LayoutContext().UpdateCompositingLayersAfterStyleChange();

        if (_renderView && _renderView->NeedsLayout())
        {
          frameView->LayoutContext().ScheduleLayout();
        }
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

  void Document::EvaluateMediaQueriesAndReportChanges() noexcept
  {
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

  namespace
  {
    // See https://www.w3.org/TR/cssom-view-1/#potentially-scrollable.
    KRYS_NODISCARD bool IsBodyPotentiallyScrollable(Document &document, html::HTMLBodyElement &body) noexcept
    {
      // An element is potentially scrollable if all of the following conditions are true:
      // - The element has an associated CSS layout box.
      // - The element is not the HTML body element, or it is and the root element's used value of the
      //   overflow-x or overflow-y properties is not visible.
      // - The element's used value of the overflow-x or overflow-y properties is not visible.
      //
      // FIXME: We should use RenderObject::hasNonVisibleOverflow() instead of Element::computedStyle() but
      // the used values are currently not correctly updated. See https://webkit.org/b/182292.
      auto documentElement = document.DocumentElement();
      if (!body.Renderer() || !documentElement)
      {
        return false;
      }
      auto *documentElementStyle = documentElement->ComputedStyle(null);
      if (!documentElementStyle || documentElementStyle->IsOverflowVisible())
      {
        return false;
      }
      auto *bodyStyle = body.ComputedStyle(null);
      return bodyStyle && !bodyStyle->IsOverflowVisible();
    }

    KRYS_NODISCARD float ZoomForClient(const RenderStyle &style) noexcept
    {
      return style.UsedZoom();
    }
  }

  RefPtr<Element> Document::ScrollingElementForAPI() noexcept
  {
    if (InQuirksMode())
    {
      UpdateLayoutIgnorePendingStylesheets();
    }
    return ScrollingElement();
  }

  RefPtr<Element> Document::ScrollingElement() noexcept
  {
    // See https://drafts.csswg.org/cssom-view/#dom-document-scrollingelement.
    // The scrollingElement attribute, on getting, must run these steps:
    // 1. If the Document is in quirks mode, follow these substeps:
    if (InQuirksMode())
    {
      // 1. If the HTML body element exists, and it is not potentially scrollable, return the
      // HTML body element and abort these steps.
      if (auto firstBody = Body(); firstBody && !IsBodyPotentiallyScrollable(*this, *firstBody))
      {
        return firstBody;
      }

      // 2. Return null and abort these steps.
      return nullptr;
    }

    // 2. If there is a root element, return the root element and abort these steps.
    // 3. Return null.
    return DocumentElement();
  }

  html::HTMLElement *Document::BodyOrFrameset() const noexcept
  {
    // Return the first body or frameset child of the html element.
    auto element = DocumentElement();
    if (!element || !Is<html::HTMLHtmlElement>(*element))
    {
      return nullptr;
    }
    for (auto *child = element->FirstChild(); child; child = child->NextSibling())
    {
      auto *childElement = DynamicDowncast<html::HTMLElement>(child);
      if (childElement
          && (Is<html::HTMLBodyElement>(*childElement) || Is<html::HTMLFrameSetElement>(*childElement)))
      {
        return const_cast<html::HTMLElement *>(childElement);
      }
    }
    return nullptr;
  }

  void Document::ConvertAbsoluteToClientQuads(List<FloatQuad> &quads, const RenderStyle &style) const noexcept
  {
    auto *frameView = View();
    if (!frameView)
    {
      return;
    }

    float inverseFrameScale = frameView->AbsoluteToDocumentScaleFactor(ZoomForClient(style));
    auto documentToClientOffset = frameView->DocumentToClientOffset();

    for (auto &quad : quads)
    {
      if (inverseFrameScale != 1)
      {
        quad.Scale(inverseFrameScale);
      }

      quad.Move(documentToClientOffset);
    }
  }

  void Document::ConvertAbsoluteToClientRects(List<FloatRect> &rects, const RenderStyle &style) const noexcept
  {
    auto *frameView = View();
    if (!frameView)
    {
      return;
    }

    float inverseFrameScale = frameView->AbsoluteToDocumentScaleFactor(ZoomForClient(style));
    auto documentToClientOffset = frameView->DocumentToClientOffset();

    for (auto &rect : rects)
    {
      if (inverseFrameScale != 1)
      {
        rect.Scale(inverseFrameScale);
      }

      rect.Move(documentToClientOffset);
    }
  }

  void Document::ConvertAbsoluteToClientRect(FloatRect &rect, const RenderStyle &style) const noexcept
  {
    auto *frameView = View();
    if (!frameView)
    {
      return;
    }

    rect = frameView->AbsoluteToDocumentRect(rect, ZoomForClient(style));
    rect = frameView->DocumentToClientRect(rect);
  }
}
