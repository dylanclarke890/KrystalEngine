#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/Timelines/AnimationTimelinesController.hpp"
#include "Krystal.Booey/Animation/Timelines/DocumentTimeline.hpp"
#include "Krystal.Booey/Animation/Timelines/StyleOriginatedTimelinesController.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/CSS/Calc/RandomCachingKeyMap.hpp"
#include "Krystal.Booey/CSS/Font/CSSFontSelector.hpp"
#include "Krystal.Booey/CSS/Media/MediaQueryList.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorAdjust.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Queries/MediaQueryMatcher.hpp"
#include "Krystal.Booey/CSS/StyleSheets/StyleSheetList.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/CSS/Values/SerialisationContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/DocumentAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/EventDispatcher.hpp"
#include "Krystal.Booey/DOM/Algorithms/MutationAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.Booey/DOM/Algorithms/NodeAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/SelectorQuery.hpp"
#include "Krystal.Booey/DOM/Algorithms/TextAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Collections/HTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveHTMLCollection.hpp"
#include "Krystal.Booey/DOM/Collections/LiveNodeList.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"
#include "Krystal.Booey/DOM/ContentVisibility/ContentVisibilityDocumentState.hpp"
#include "Krystal.Booey/DOM/Events/CompositionEvent.hpp"
#include "Krystal.Booey/DOM/Events/CustomEvent.hpp"
#include "Krystal.Booey/DOM/Events/DragEvent.hpp"
#include "Krystal.Booey/DOM/Events/Event.hpp"
#include "Krystal.Booey/DOM/Events/EventNames.hpp"
#include "Krystal.Booey/DOM/Events/FocusEvent.hpp"
#include "Krystal.Booey/DOM/Events/KeyboardEvent.hpp"
#include "Krystal.Booey/DOM/Events/MouseEvent.hpp"
#include "Krystal.Booey/DOM/Events/PointerEvent.hpp"
#include "Krystal.Booey/DOM/Events/TextEvent.hpp"
#include "Krystal.Booey/DOM/Events/TouchEvent.hpp"
#include "Krystal.Booey/DOM/Events/UIEvent.hpp"
#include "Krystal.Booey/DOM/Highlights/HighlightRegistry.hpp"
#include "Krystal.Booey/DOM/Internals/DocumentFontLoader.hpp"
#include "Krystal.Booey/DOM/Internals/ElementFactory.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Internals/UserActionElementSet.hpp"
#include "Krystal.Booey/DOM/Internals/VisitedLinkState.hpp"
#include "Krystal.Booey/DOM/Iterators/SubtreeRanges.hpp"
#include "Krystal.Booey/DOM/Mixins/NonElementParentNode.hpp"
#include "Krystal.Booey/DOM/Mixins/ParentNode.hpp"
#include "Krystal.Booey/DOM/Nodes/Attr.hpp"
#include "Krystal.Booey/DOM/Nodes/CDATASection.hpp"
#include "Krystal.Booey/DOM/Nodes/Comment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentFragment.hpp"
#include "Krystal.Booey/DOM/Nodes/DocumentType.hpp"
#include "Krystal.Booey/DOM/Nodes/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Nodes/HTMLDocument.hpp"
#include "Krystal.Booey/DOM/Nodes/ProcessingInstruction.hpp"
#include "Krystal.Booey/DOM/Nodes/Text.hpp"
#include "Krystal.Booey/DOM/Ranges/Range.hpp"
#include "Krystal.Booey/DOM/Traversal/TreeWalker.hpp"
#include "Krystal.Booey/Editing/Editor.hpp"
#include "Krystal.Booey/Editing/Selection/FrameSelection.hpp"
#include "Krystal.Booey/Editing/Selection/VisibleSelection.hpp"
#include "Krystal.Booey/HTML/Algorithms/CustomElementAlgorithms.hpp"
#include "Krystal.Booey/HTML/Algorithms/DOMTreeAccessors.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/ReflectAttribute.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasRenderingContext.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcher.hpp"
#include "Krystal.Booey/HTML/CloseWatcher/CloseWatcherManager.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAnchorElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLAreaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBaseElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLButtonElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLCanvasElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLDialogElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLEmbedElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormControlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFrameSetElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHeadElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLHtmlElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLImageElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMetaElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLScriptElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLTitleElement.hpp"
#include "Krystal.Booey/HTML/EventLoop/EventLoop.hpp"
#include "Krystal.Booey/HTML/Events/PageTransitionEvent.hpp"
#include "Krystal.Booey/HTML/Images/LazyLoadImageObserver.hpp"
#include "Krystal.Booey/HTML/Parser/DocumentParser.hpp"
#include "Krystal.Booey/HTML/Sanitizer/SanitizerAlgorithms.hpp"
#include "Krystal.Booey/Infra/Namespaces.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Loader/Cache/CachedFontLoadRequest.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceLoader.hpp"
#include "Krystal.Booey/Page/ChromeClient.hpp"
#include "Krystal.Booey/Page/DOMSelection.hpp"
#include "Krystal.Booey/Page/EditorClient.hpp"
#include "Krystal.Booey/Page/IntersectionObserver/IntersectionObserver.hpp"
#include "Krystal.Booey/Page/LocalDOMWindow.hpp"
#include "Krystal.Booey/Page/LocalFrame.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/MouseEventWithHitTestResults.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Page/PointerCaptureController.hpp"
#include "Krystal.Booey/Page/PointerLockController.hpp"
#include "Krystal.Booey/Page/ResizeObserver/ResizeObserver.hpp"
#include "Krystal.Booey/Page/ResizeObserver/ResizeObserverEntry.hpp"
#include "Krystal.Booey/Page/Settings.hpp"
#include "Krystal.Booey/Page/VisualViewport.hpp"
#include "Krystal.Booey/Platform/Locale/PlatformLocale.hpp"
#include "Krystal.Booey/Platform/Scrolling/Scrollbar.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollbarTheme.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/Iterators/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/Style/Change.hpp"
#include "Krystal.Booey/Style/Invalidation/PseudoClassChangeInvalidation.hpp"
#include "Krystal.Booey/Style/Relations.hpp"
#include "Krystal.Booey/Style/ResolvedStyle.hpp"
#include "Krystal.Booey/Style/ResolveForDocument.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope/Scope.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/Style/Update.hpp"
#include "Krystal.Booey/Style/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/SVG/Elements/SVGElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGScriptElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGSvgElement.hpp"
#include "Krystal.Booey/SVG/Elements/SVGTitleElement.hpp"
#include "Krystal.Booey/SVG/SVGDocumentExtensions.hpp"
#include "Krystal.Booey/Text/EncodingRegistry.hpp"
#include "Krystal.Booey/XML/XMLDocumentParser.hpp"
#include "Krystal.Core/Text/ASCII.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Utils/ScopeExit.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"
#include "Krystal.Core/Utils/Tags.hpp"
#include <algorithm>
#include <chrono>
#include <format>
#include <random>

namespace krys::boo::dom
{
  namespace
  {
    constexpr auto IntersectionObserversInitialUpdateDelay = std::chrono::milliseconds(2'000);

    /// @brief A random (version 4) UUID, in its string form.
    KRYS_NODISCARD DOMString CreateVersion4UUIDString() noexcept
    {
      static std::mt19937_64 generator {std::random_device {}()};
      uint64 high = generator();
      uint64 low = generator();
      // The version (4) and the variant (10xx).
      high = (high & 0xFF'FF'FF'FF'FF'FF'0F'FFull) | 0x00'00'00'00'00'00'40'00ull;
      low = (low & 0x3F'FF'FF'FF'FF'FF'FF'FFull) | 0x80'00'00'00'00'00'00'00ull;
      auto string = std::format("{:08x}-{:04x}-{:04x}-{:04x}-{:012x}", high >> 32, (high >> 16) & 0xFFFF,
                                high & 0xFFFF, low >> 48, low & 0xFF'FF'FF'FF'FF'FFull);
      return DOMString(string.begin(), string.end());
    }

    void CallbackForContainIntrinsicSize(const List<Ref<ResizeObserverEntry>> &entries,
                                         ResizeObserver &observer) noexcept
    {
      for (auto &entry : entries)
      {
        Ref<Element> target = ShareRef(entry->Target());
        if (!target->IsConnected())
        {
          observer.Unobserve(*target);
          target->ClearLastRememberedLogicalWidth();
          target->ClearLastRememberedLogicalHeight();
          continue;
        }

        auto *box = target->GetRenderBox();
        if (!box)
        {
          observer.Unobserve(*target);
          continue;
        }
        krys_debug_assert(!IsSkippedContentRoot(*box));
        krys_debug_assert(box->Style().HasAutoLengthContainIntrinsicSize());

        auto &contentBoxSize = entry->ContentBoxSize().front();
        if (box->Style().LogicalContainIntrinsicWidth().HasAuto())
        {
          auto adjustedWidth =
            LayoutUnit {ApplyZoom(static_cast<float>(contentBoxSize->InlineSize()), box->Style())};
          target->SetLastRememberedLogicalWidth(adjustedWidth);
        }

        if (box->Style().LogicalContainIntrinsicHeight().HasAuto())
        {
          auto adjustedHeight =
            LayoutUnit {ApplyZoom(static_cast<float>(contentBoxSize->BlockSize()), box->Style())};
          target->SetLastRememberedLogicalHeight(adjustedHeight);
        }
      }
    }
  }

  Document::Document(DocumentFlags flags) noexcept : Document()
  {
    SetDocumentFlag(flags);

    // An HTML document is a text/html one, however it's made.
    if (HasDocumentFlag(DocumentFlags::IsHTMLDocument))
    {
      _contentType = u8"text/html";
    }
  }

#pragma region Document (DOM)

  Document::Document() noexcept
      : ContainerNode(*this, NodeType::DOCUMENT_NODE, NodeFlags::IsContainerNode),
        DocumentOrShadowRoot(static_cast<ContainerNode &>(*this)), _flags(DocumentFlags::None),
        _url(url::AboutBlankURL()), _baseURL(_url), _settings(ShareRef(Settings::Default())),
        _userActionElements(CreateUnique<UserActionElementSet>()),
        _styleRecalcTimer([this] { (void)ShareRef(*this)->UpdateStyleIfNeeded(); }),
        _intersectionObserversInitialUpdateTimer(
          [this] { ScheduleRenderingUpdate({RenderingUpdateStep::IntersectionObservations}); })
  {
    // Set this's origin to the origin of current global object's associated Document. (Skipped: documents
    // have no origin.)
    SetNodeFlag(NodeFlags::IsConnected);
    _styleScope = CreateUnique<style::Scope>(*this);
    _selection = CreateUnique<FrameSelection>(this);
    _editor = CreateUnique<boo::Editor>(*this);
  }

  Document::~Document() noexcept
  {
    if (_intersectionObserverData)
    {
      for (const auto &observer : _intersectionObserverData->Observers)
      {
        if (observer)
        {
          observer->RootDestroyed();
        }
      }
      _intersectionObserverData->Observers.clear();
      // Document cannot be a target.
      krys_debug_assert(_intersectionObserverData->Registrations.empty());
    }

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

    CommonTeardown();

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

  const krys::text::ICodec &Document::Encoding() const noexcept
  {
    if (_encoding)
    {
      return *_encoding;
    }
    static const krys::text::ICodec *utf8 = EncodingRegistry().Find(u8"utf-8");
    krys_debug_assert(utf8);
    return *utf8;
  }

  void Document::SetEncoding(const krys::text::ICodec &encoding) noexcept
  {
    _encoding = &encoding;
  }

  DOMString Document::CharacterSet() const noexcept
  {
    return DOMString(Encoding().Name());
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
    // 1. If localName is not a valid element local name, then throw an "InvalidCharacterError" DOMException.
    if (!NameValidation::IsValidElementLocalName(localName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // 2. If this is an HTML document, then set localName to localName in ASCII lowercase.
    if (Is<HTMLDocument>(*this))
    {
      localName = ::krys::text::ToASCIILower(localName.View());
    }

    // 3. Let registry and is be the result of flattening element creation options given options and this.
    auto creationOptions = DocumentAlgorithms::FlattenElementCreationOptions(options, *this);
    if (creationOptions.HasException())
    {
      return creationOptions.ReleaseException();
    }

    // 4. Let namespace be the HTML namespace, if this is an HTML document or this's content type is
    //    "application/xhtml+xml"; otherwise null.
    DOMStringAtom namespaceURI = DOMStringAtom::Null();
    if (Is<HTMLDocument>(*this) || _contentType == u8"application/xhtml+xml")
    {
      namespaceURI = infra::Namespaces::HTML;
    }

    // 5. Return the result of creating an element given this, localName, namespace, null, is, true, and
    // registry.
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

  Ref<Text> Document::CreateTextNode(DOMString data) noexcept
  {
    return CreateRef<Text>(*this, krys::move(data));
  }

  ExceptionOr<Ref<CDATASection>> Document::CreateCDATASection(DOMString data) noexcept
  {
    // 1. If this is an HTML document, then throw a "NotSupportedError" DOMException.
    if (Is<HTMLDocument>(*this))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 2. If data contains the string "]]>", then throw an "InvalidCharacterError" DOMException.
    if (data.contains(u8"]]>"))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // 3. Return the result of creating a CDATA section node given this and data.
    return CreateRef<CDATASection>(*this, krys::move(data));
  }

  Ref<Comment> Document::CreateComment(DOMString data) noexcept
  {
    return CreateRef<Comment>(*this, krys::move(data));
  }

  ExceptionOr<Ref<ProcessingInstruction>> Document::CreateProcessingInstruction(DOMString target,
                                                                                DOMString data) noexcept
  {
    return ProcessingInstruction::Create(*this, krys::move(target), krys::move(data));
  }

  ExceptionOr<Ref<Node>> Document::ImportNode(Node &node, const BoolOrImportNodeOptions &options) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If node is a document or shadow root, then throw a "NotSupportedError" DOMException.
    if (Is<Document>(node) || Is<ShadowRoot>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 2. Let subtree be false.
    bool subtree = false;

    // 3. Let registry be null.
    RefPtr<html::CustomElementRegistry> registry = nullptr;

    // 4. If options is a boolean, then set subtree to options.
    if (std::holds_alternative<bool>(options))
    {
      subtree = std::get<bool>(options);
    }
    // 5. Otherwise:
    else
    {
      // 5a. Set subtree to the negation of options["selfOnly"].
      auto &dict = std::get<ImportNodeOptions>(options);
      subtree = !dict.SelfOnly;

      // 5b. If options["customElementRegistry"] exists, then set registry to it.
      registry = dict.CustomElementRegistry;

      // 5c. If registry's is scoped is false and registry is not this's custom element registry, then throw a
      //     "NotSupportedError" DOMException.
      if (registry != nullptr && !registry->IsScoped() && registry != CustomElementRegistry())
      {
        return Exception {ExceptionCode::NotSupportedError};
      }
    }

    // 6. If registry is null, then set registry to the result of looking up a custom element registry given
    // this.
    if (registry == nullptr)
    {
      registry = CustomElementRegistry();
    }

    // 7. Return the result of cloning a node given node with document set to this, subtree set to subtree,
    // and
    //    fallbackRegistry set to registry.
    return NodeAlgorithms::CloneNode(node, this, subtree, nullptr, registry.get());
  }

  ExceptionOr<Ref<Node>> Document::AdoptNode(Node &node) noexcept
  {
    html::CEReactionsScope ceReactions;
    // 1. If node is a document, then throw a "NotSupportedError" DOMException.
    if (Is<Document>(node))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 2. If node is a shadow root, then throw a "HierarchyRequestError" DOMException.
    if (Is<ShadowRoot>(node))
    {
      return Exception {ExceptionCode::HierarchyRequestError};
    }

    // 3. Adopt node into this.
    if (auto adoptResult = DocumentAlgorithms::AdoptNode(node, *this); adoptResult.HasException())
    {
      return adoptResult.ReleaseException();
    }

    // 4. Return node.
    return ShareRef(node);
  }

  // NOTE: VS says this method can be made const but it's lying, Attr constructor needs a non-const Document
  // reference and we can't make it const without breaking that.
  ExceptionOr<Ref<Attr>> Document::CreateAttribute(DOMStringAtom localName) noexcept
  {
    // 1. If localName is not a valid attribute local name, then throw an "InvalidCharacterError"
    // DOMException.
    if (!NameValidation::IsValidAttributeLocalName(localName.View()))
    {
      return Exception {ExceptionCode::InvalidCharacterError};
    }

    // 2. If this is an HTML document, then set localName to localName in ASCII lowercase.
    if (Is<HTMLDocument>(*this))
    {
      localName = krys::text::ToASCIILower(localName.View());
    }

    // 3. Return the result of creating an attribute given this and localName.
    QualifiedName qualifiedName {DOMStringAtom::Null(), DOMStringAtom::Null(), localName};
    return AdoptRef<Attr>(*new Attr(*this, qualifiedName));
  }

  ExceptionOr<Ref<Attr>> Document::CreateAttributeNS(DOMStringAtom namespaceUri,
                                                     DOMStringAtom qualifiedName) noexcept
  {
    // 1. Let (namespace, prefix, localName) be the result of validating and extracting namespace and
    //    qualifiedName given "attribute".
    auto nameResult =
      NameValidation::ValidateAndExtract(namespaceUri, qualifiedName, ValidateAndExtractContext::Attribute);
    if (nameResult.HasException())
    {
      return nameResult.ReleaseException();
    }

    // 2. Return the result of creating an attribute given this, localName, namespace, and prefix.
    return AdoptRef<Attr>(*new Attr(*this, *nameResult));
  }

  namespace
  {
    /// @brief A new event of the interface that createEvent's table gives @p interface; null if none does.
    KRYS_NODISCARD RefPtr<Event> CreateEventOfInterface(DOMStringView interface) noexcept
    {
      auto matches = [interface](DOMStringView name)
      {
        return infra::StringAlgorithms::ASCIICaseInsensitiveMatch(interface, name);
      };
      DOMStringAtom empty(u8"");

      if (matches(u8"compositionevent"))
      {
        return CreateRef<CompositionEvent>(empty);
      }
      if (matches(u8"customevent"))
      {
        // (Without script, there's no detail type to give it.)
        return CreateRef<CustomEvent<EmptyTag>>(empty);
      }
      if (matches(u8"dragevent"))
      {
        return CreateRef<DragEvent>(empty);
      }
      if (matches(u8"event") || matches(u8"events") || matches(u8"htmlevents") || matches(u8"svgevents"))
      {
        return CreateRef<Event>(empty);
      }
      if (matches(u8"focusevent"))
      {
        return CreateRef<FocusEvent>(empty);
      }
      if (matches(u8"keyboardevent"))
      {
        return CreateRef<KeyboardEvent>(empty);
      }
      if (matches(u8"mouseevent") || matches(u8"mouseevents"))
      {
        return CreateRef<MouseEvent>(empty);
      }
      if (matches(u8"textevent"))
      {
        return CreateRef<TextEvent>();
      }
      if (matches(u8"uievent") || matches(u8"uievents"))
      {
        return CreateRef<UIEvent>(empty);
      }
      if (matches(u8"touchevent"))
      {
        return CreateRef<TouchEvent>(empty);
      }
      // `devicemotionevent` and `deviceorientationevent` name interfaces that aren't exposed.
      return nullptr;
    }
  }

  ExceptionOr<Ref<Event>> Document::CreateEvent(DOMStringView interface) noexcept
  {
    // 1. Let constructor be null.
    // 2. If interface is an ASCII case-insensitive match for any of the strings in the first column in the
    //    following table, then set constructor to the interface in the second column on the same row as the
    //    matching string.
    // 4. If the interface indicated by constructor is not exposed on the relevant global object of this, then
    //    throw a "NotSupportedError" DOMException. (The table leaves out the interfaces that aren't.)
    // 5. Let event be the result of creating an event given constructor.
    RefPtr<Event> event = CreateEventOfInterface(interface);

    // 3. If constructor is null, then throw a "NotSupportedError" DOMException.
    if (!event)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // 6. Initialize event's type attribute to the empty string.
    event->_type = DOMStringAtom(u8"");
    event->_bubbles = false;
    event->_cancellable = false;
    event->_composed = false;

    // 7. Initialize event's timeStamp attribute to the result of calling current high resolution time with
    // this's
    //    relevant global object. (The event's constructor records its creation time.)

    // 8. Initialize event's isTrusted attribute to false.
    event->_isTrusted = false;

    // 9. Unset event's initialized flag.
    event->_initialized = false;

    // 10. Return event.
    return ShareRef(*event);
  }

  Ref<Range> Document::CreateRange() noexcept
  {
    return CreateRef<Range>(BoundaryPoint {ShareRef(*this), 0uz}, BoundaryPoint {ShareRef(*this), 0uz});
  }

  Ref<NodeIterator> Document::CreateNodeIterator(Node &root, WhatToShow whatToShow,
                                                 RefPtr<NodeFilter> &&filter) noexcept
  {
    // 1. Let iterator be a new NodeIterator object.
    // 2. Set iterator's root to root.
    // 3. Set iterator's reference to (root, true).
    // 4. Set iterator's whatToShow to whatToShow.
    // 5. Set iterator's filter to filter.
    // 6. Return iterator.
    return AdoptRef(*new NodeIterator(root, whatToShow, krys::move(filter)));
  }

  Ref<TreeWalker> Document::CreateTreeWalker(Node &root, WhatToShow whatToShow,
                                             RefPtr<NodeFilter> &&filter) noexcept
  {
    // 1. Let walker be a new TreeWalker object.
    // 2. Set walker's root and walker's current to root.
    // 3. Set walker's whatToShow to whatToShow.
    // 4. Set walker's filter to filter.
    // 5. Return walker.
    return AdoptRef(*new TreeWalker(root, whatToShow, krys::move(filter)));
  }

#pragma endregion

#pragma region Document (HTML)

  namespace
  {
    // The steps parseHTML() and parseHTMLUnsafe() share: a new document with declarative shadow roots and
    // @p context's settings, into which @p html is parsed.
    KRYS_NODISCARD Ref<Document>
      ParseHTMLIntoNewDocument(Document &context, DOMStringView html,
                               Maybe<html::SanitizerConfig> &&configuration) noexcept
    {
      // Let document be a new Document, whose content type is "text/html" and origin is this's relevant
      // global object's associated Document's origin.
      // (There are no origins; the document takes the context's settings.)
      auto document = CreateRef<HTMLDocument>();
      document->SetSettings(ShareRef(context.GetSettings()));

      // Set document's allow declarative shadow roots to true.
      document->SetAllowDeclarativeShadowRoots(true);

      // Parse HTML from a string given document, html, and sanitizerConfig.
      // https://html.spec.whatwg.org/#parse-html-from-a-string
      // 1. Set document's type to "html".
      // 2. Let parser be a new HTML parser whose allow declarative shadow roots is document's allow
      //    declarative shadow roots and parser sanitizer configuration is configuration, associated with
      //    document.
      auto parser = CreateRef<html::DocumentParser>(*document);
      parser->SanitizerConfiguration(krys::move(configuration));
      // 3. Place html into the input stream for parser.
      parser->InputStream().Append(DOMString(html), IsEOF(true));
      // 4. Start parser and let it run until it has consumed all the characters just inserted into the input
      //    stream.
      parser->PumpTokenizer();
      return document;
    }
  }

  // https://html.spec.whatwg.org/#dom-parsehtmlunsafe
  ExceptionOr<Ref<Document>> Document::ParseHTMLUnsafe(Document &context, DOMStringView markup,
                                                       html::ParseHTMLUnsafeOptions &&options) noexcept
  {
    // 1. Let (compliantHTML, compliantOptions) be the result of invoking the get trusted type compliant input
    //    algorithm with the current global object, html, options, and "Document parseHTMLUnsafe".
    // (There are no Trusted Types.)
    // 2. Let document be a new Document, whose content type is "text/html" and origin is this's relevant
    //    global object's associated Document's origin.
    // 3. Set document's allow declarative shadow roots to true.
    // (ParseHTMLIntoNewDocument, below.)
    // 4. Let sanitizerConfig be the result of getting a sanitizer config from options given compliantOptions
    //    and false.
    auto sanitizerConfig = html::SanitizerAlgorithms::GetSanitizerConfigFromOptions(
      options.Sanitizer ? &*options.Sanitizer : nullptr, false);
    if (sanitizerConfig.HasException())
    {
      return sanitizerConfig.ReleaseException();
    }

    // 5. Parse HTML from a string given document, compliantHTML, and sanitizerConfig.
    // 6. Return document.
    return ParseHTMLIntoNewDocument(context, markup, sanitizerConfig.ReleaseValue());
  }

  // https://html.spec.whatwg.org/#dom-parsehtml
  ExceptionOr<Ref<Document>> Document::ParseHTML(Document &context, DOMStringView html,
                                                 html::SetHTMLOptions &&options) noexcept
  {
    // 1. Let document be a new Document, whose content type is "text/html" and origin is this's relevant
    //    global object's associated Document's origin.
    // 2. Set document's allow declarative shadow roots to true.
    // (ParseHTMLIntoNewDocument, below.)
    // 3. Let sanitizerConfig be the result of getting a sanitizer config from options given options and true.
    auto sanitizerConfig = html::SanitizerAlgorithms::GetSanitizerConfigFromOptions(&options.Sanitizer, true);
    if (sanitizerConfig.HasException())
    {
      return sanitizerConfig.ReleaseException();
    }

    // 4. Assert: sanitizerConfig is non-null.
    krys_debug_assert(sanitizerConfig.Value().has_value());

    // 5. Parse HTML from a string given document, html, and sanitizerConfig.
    // 6. Return document.
    return ParseHTMLIntoNewDocument(context, html, sanitizerConfig.ReleaseValue());
  }

  namespace
  {
    // The first SVG title element that is a child of the document element.
    template <typename TElement>
    KRYS_NODISCARD auto *FirstSVGTitleChild(TElement &documentElement) noexcept
    {
      using Title = match_constness_t<TElement, svg::SVGTitleElement>;
      for (auto *child = TreeQueries::FirstElementChild(documentElement); child;
           child = TreeQueries::NextElementSibling(*child))
      {
        if (auto *svgChild = DynamicDowncast<svg::SVGElement>(*child))
        {
          if (auto *title = DynamicDowncast<svg::SVGTitleElement>(*svgChild))
          {
            return static_cast<Title *>(title);
          }
        }
      }
      return static_cast<Title *>(nullptr);
    }

    KRYS_NODISCARD bool IsSVGSvgElement(const Element *element) noexcept
    {
      auto *svgElement = element ? DynamicDowncast<svg::SVGElement>(*element) : nullptr;
      return svgElement && Is<svg::SVGSvgElement>(*svgElement);
    }
  }

  DOMString Document::Title() const noexcept
  {
    auto documentElement = DocumentElement();
    if (IsSVGSvgElement(documentElement.get()))
    {
      // If the document element is an SVG svg element, then let value be the child text content of the first
      // SVG title element that is a child of the document element.
      auto *title = FirstSVGTitleChild(*documentElement);
      if (!title)
      {
        return {};
      }

      return infra::StringAlgorithms::StripAndCollapseASCIIWhitespace(
        TextAlgorithms::ChildTextContent(*title));
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
  }

  ExceptionOr<void> Document::Title(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    auto documentElement = DocumentElement();
    if (IsSVGSvgElement(documentElement.get()))
    {
      // If there is an SVG title element that is a child of the document element, let element be the first
      // such element. Otherwise:
      RefPtr<Element> title = ShareRefPtr(FirstSVGTitleChild(*documentElement));
      if (!title)
      {
        // Let element be the result of creating an element given the document element's node document,
        // "title", and the SVG namespace. Insert element as the first child of the document element.
        title = ElementFactory::Create(*this, {infra::Namespaces::SVG, DOMStringAtom::Null(), u8"title"});

        if (auto insert =
              MutationAlgorithms::PreInsert(*title, *documentElement, documentElement->FirstChild());
            insert.HasException())
        {
          return insert.ReleaseException();
        }
      }

      // String replace all with the given value within element.
      if (auto replace = NodeAlgorithms::StringReplaceAll(krys::move(value), *title); replace.HasException())
      {
        return replace.ReleaseException();
      }
    }
    // If the document element is in the HTML namespace:
    else if (documentElement && documentElement->NamespaceURI() == infra::Namespaces::HTML)
    {
      RefPtr<html::HTMLHeadElement> head = Head();
      RefPtr<Element> element = ShareRefPtr(html::DOMTreeAccessors::GetTitleElement(*this));

      // 1. If the title element is null and the head element is null, then return.
      if (head == nullptr && element == nullptr)
      {
        return {};
      }

      // 2. If the title element is non-null, let element be the title element.
      // 3. Otherwise:
      if (element == nullptr)
      {
        // 3a. Let element be the result of creating an element given the document element's node document,
        //     "title", and the HTML namespace.
        element = ElementFactory::Create(*this, {infra::Namespaces::HTML, DOMStringAtom::Null(), u8"title"});

        // 3b. Append element to the head element.
        if (auto append = MutationAlgorithms::Append(*element, *head); append.HasException())
        {
          return append.ReleaseException();
        }
      }

      // 4. String replace all with the given value within element.
      if (auto replace = NodeAlgorithms::StringReplaceAll(krys::move(value), *element);
          replace.HasException())
      {
        return replace.ReleaseException();
      }
    }
    // Otherwise: Do nothing.

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

  void Document::Dir(DOMString value) noexcept
  {
    html::CEReactionsScope ceReactions;
    auto html = html::DOMTreeAccessors::GetHtmlElement(*this);
    if (html == nullptr)
    {
      return;
    }

    html->Dir(krys::move(value));
  }

  namespace
  {
    KRYS_NODISCARD html::HTMLBodyElement *FirstBodyElementChild(const Document &document) noexcept
    {
      auto *html = html::DOMTreeAccessors::GetHtmlElement(document);
      if (html == nullptr)
      {
        return nullptr;
      }

      auto children = ChildNodeRange(*html);
      auto body = FirstOfType<html::HTMLBodyElement>(children);
      return body == std::ranges::end(children) ? nullptr : &Downcast<html::HTMLBodyElement>(*body);
    }

    template <DerivedFrom<html::HTMLElement> T>
    KRYS_NODISCARD bool IsHTMLElementOfType(const Element &element) noexcept
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(element);
      return htmlElement && Is<T>(*htmlElement);
    }

    /// @brief The document's collection that @p cache holds, or a new one that it holds from now on.
    KRYS_NODISCARD Ref<HTMLCollection>
      CachedDocumentCollection(WeakPtr<HTMLCollection> &cache, Document &document,
                               LiveHTMLCollectionFilterFunc &&filter) noexcept
    {
      if (auto collection = cache.lock())
      {
        return ShareRef(*collection.get());
      }

      auto collection = CreateRef<LiveHTMLCollection>(document, krys::move(filter));
      cache = CreateWeakPtr(collection.get());
      return collection;
    }
  }

  RefPtr<html::HTMLBodyElement> Document::Body() noexcept
  {
    return ShareRefPtr(FirstBodyElementChild(*this));
  }

  RefPtr<const html::HTMLBodyElement> Document::Body() const noexcept
  {
    return ShareRefPtr(FirstBodyElementChild(*this));
  }

  ExceptionOr<void> Document::SetBodyOrFrameset(html::HTMLElement &body) noexcept
  {
    html::CEReactionsScope ceReactions;

    // 1. If the new value is not a body or frameset element, then throw a "HierarchyRequestError"
    //    DOMException.
    if (!Is<html::HTMLBodyElement>(body) && !Is<html::HTMLFrameSetElement>(body))
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 2. Otherwise, if the new value is the same as the body element, return.
    RefPtr<html::HTMLElement> currentBody = ShareRefPtr(BodyOrFrameset());
    if (currentBody == &body)
    {
      return {};
    }

    // 3. Otherwise, if the body element is not null, then replace the body element with the new value within
    //    the body element's parent and return.
    if (currentBody != nullptr)
    {
      if (auto replace = MutationAlgorithms::Replace(*currentBody, body, *currentBody->ParentNode());
          replace.HasException())
      {
        return replace.ReleaseException();
      }
      return {};
    }

    // 4. Otherwise, if there is no document element, throw a "HierarchyRequestError" DOMException.
    auto documentElement = DocumentElement();
    if (documentElement == nullptr)
    {
      return ExceptionCode::HierarchyRequestError;
    }

    // 5. Otherwise, the body element is null, but there's a document element. Append the new value to the
    //    document element.
    if (auto append = MutationAlgorithms::Append(body, *documentElement); append.HasException())
    {
      return append.ReleaseException();
    }
    return {};
  }

  RefPtr<html::HTMLHeadElement> Document::Head() noexcept
  {
    return ShareRefPtr(html::DOMTreeAccessors::GetHeadElement(*this));
  }

  RefPtr<const html::HTMLHeadElement> Document::Head() const noexcept
  {
    return ShareRefPtr(html::DOMTreeAccessors::GetHeadElement(*this));
  }

  Ref<HTMLCollection> Document::Images() noexcept
  {
    return CachedDocumentCollection(_images, *this, [](const Element &element)
                                    { return IsHTMLElementOfType<html::HTMLImageElement>(element); });
  }

  Ref<HTMLCollection> Document::Embeds() noexcept
  {
    return CachedDocumentCollection(_embeds, *this, [](const Element &element)
                                    { return IsHTMLElementOfType<html::HTMLEmbedElement>(element); });
  }

  Ref<HTMLCollection> Document::Plugins() noexcept
  {
    return Embeds();
  }

  Ref<HTMLCollection> Document::Links() noexcept
  {
    return CachedDocumentCollection(_links, *this,
                                    [](const Element &element)
                                    {
                                      return (IsHTMLElementOfType<html::HTMLAnchorElement>(element)
                                              || IsHTMLElementOfType<html::HTMLAreaElement>(element))
                                             && element.HasAttribute(u8"href");
                                    });
  }

  Ref<HTMLCollection> Document::Forms() noexcept
  {
    return CachedDocumentCollection(_forms, *this, [](const Element &element)
                                    { return IsHTMLElementOfType<html::HTMLFormElement>(element); });
  }

  Ref<HTMLCollection> Document::Scripts() noexcept
  {
    return CachedDocumentCollection(_scripts, *this, [](const Element &element)
                                    { return IsHTMLElementOfType<html::HTMLScriptElement>(element); });
  }

#pragma region Obsolete members

  namespace
  {
    // The obsolete colour attributes reflect the body element's attributes when it's a body, and are empty
    // and can't be set when there's no body element or it's a frameset.
    KRYS_NODISCARD DOMString BodyColorAttribute(const Document &document, DOMStringView name) noexcept
    {
      auto *body = DynamicDowncast<html::HTMLBodyElement>(document.BodyOrFrameset());
      if (!body)
      {
        return {};
      }
      return body->GetAttribute(DOMStringAtom(name)).value_or(DOMString());
    }

    void SetBodyColorAttribute(Document &document, DOMStringView name, DOMString value) noexcept
    {
      if (auto *body = DynamicDowncast<html::HTMLBodyElement>(document.BodyOrFrameset()))
      {
        (void)body->SetAttribute(DOMStringAtom(name), krys::move(value));
      }
    }
  }

  DOMString Document::FgColor() const noexcept
  {
    return BodyColorAttribute(*this, u8"text");
  }

  void Document::FgColor(DOMString value) noexcept
  {
    SetBodyColorAttribute(*this, u8"text", krys::move(value));
  }

  DOMString Document::LinkColor() const noexcept
  {
    return BodyColorAttribute(*this, u8"link");
  }

  void Document::LinkColor(DOMString value) noexcept
  {
    SetBodyColorAttribute(*this, u8"link", krys::move(value));
  }

  DOMString Document::VlinkColor() const noexcept
  {
    return BodyColorAttribute(*this, u8"vlink");
  }

  void Document::VlinkColor(DOMString value) noexcept
  {
    SetBodyColorAttribute(*this, u8"vlink", krys::move(value));
  }

  DOMString Document::AlinkColor() const noexcept
  {
    return BodyColorAttribute(*this, u8"alink");
  }

  void Document::AlinkColor(DOMString value) noexcept
  {
    SetBodyColorAttribute(*this, u8"alink", krys::move(value));
  }

  DOMString Document::BgColor() const noexcept
  {
    return BodyColorAttribute(*this, u8"bgcolor");
  }

  void Document::BgColor(DOMString value) noexcept
  {
    SetBodyColorAttribute(*this, u8"bgcolor", krys::move(value));
  }

  Ref<HTMLCollection> Document::Anchors() noexcept
  {
    return CachedDocumentCollection(
      _anchors, *this, [](const Element &element)
      { return IsHTMLElementOfType<html::HTMLAnchorElement>(element) && element.HasAttribute(u8"name"); });
  }

  Ref<HTMLCollection> Document::Applets() noexcept
  {
    return CachedDocumentCollection(_applets, *this, [](const Element &) { return false; });
  }

#pragma endregion

  Ref<NodeList> Document::GetElementsByName(DOMStringView elementName) noexcept
  {
    return CreateRef<LiveNodeList>(*this,
                                   [name = DOMString(elementName)](const Node &node)
                                   {
                                     auto *element = DynamicDowncast<html::HTMLElement>(node);
                                     return element && element->GetAttribute(u8"name") == name;
                                   });
  }

  DOMString Document::LastModified() const noexcept
  {
    // FIXME: The response's Last-Modified header, or the file's modification date for a file, once responses
    // have headers.
    // When the last modification date and time aren't known, the attribute returns the current date and time.
    auto now = std::chrono::floor<std::chrono::seconds>(std::chrono::system_clock::now());
    auto text = std::format("{:%m/%d/%Y %H:%M:%S}", std::chrono::current_zone()->to_local(now));
    return DOMString(DOMStringView(reinterpret_cast<const char8 *>(text.data()), text.size()));
  }

#pragma endregion

#pragma region ParentNode

  Ref<HTMLCollection> Document::Children() noexcept
  {
    return EnsureNodeRareData().Children(*this);
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

  void Document::SetURL(url::URL url) noexcept
  {
    url::URL newURL = url.IsEmpty() ? url::AboutBlankURL() : krys::move(url);

    // https://wicg.github.io/scroll-to-text-fragment/#process-and-consume-fragment-directive
    _fragmentDirective = newURL.ConsumeFragmentDirective();

    if (newURL == _url)
    {
      return;
    }

    // 1. Set document's URL to url.
    _url = krys::move(newURL);

    // 2. Respond to base URL changes given document.
    UpdateBaseURL();
  }

  const url::URL &Document::FallbackBaseURL() const noexcept
  {
    // 1. If document's URL matches about:blank or matches about:srcdoc, and document's about base URL is
    //    non-null, then return document's about base URL. (Skipped: documents have no about base URL.)
    // 2. Return document's URL.
    return _url;
  }

  url::URL Document::CompleteURL(utf8_stringview url) const noexcept
  {
    // 1. Let encoding be UTF-8.
    // 2. If environment is a Document object, then set encoding to environment's character encoding.
    // 3. Otherwise, if environment's relevant global object is a Window object, set encoding to environment's
    //    relevant global object's associated Document's character encoding.
    // Documents are always UTF-8, which the URL parser always uses.
    // 4. Let baseURL be environment's base URL, if environment is a Document object; otherwise environment's
    // API
    //    base URL.
    // 5. Return the result of applying the URL parser to url, with baseURL and encoding.
    return CompleteURL(url, _baseURL);
  }

  url::URL Document::CompleteURL(utf8_stringview url, const url::URL &baseURL) const noexcept
  {
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

    // The frozen base URL of the first base element with an href; only that one's matters.
    url::URL baseElementURL;
    if (href.has_value())
    {
      // To set the frozen base URL for an element element:
      // https://html.spec.whatwg.org/#set-the-frozen-base-url
      // 1. Let document be element's node document.
      // 2. Let urlRecord be the result of parsing the value of element's href content attribute with
      //    document's fallback base URL, and document's character encoding. (Thus, the base element isn't
      //    affected by itself.)
      baseElementURL = CompleteURL(*href, FallbackBaseURL());

      // 3. If any of the following are true: urlRecord is failure; urlRecord's scheme is "data" or
      //    "javascript"; or running Is base allowed for Document? on urlRecord and document returns
      //    "Blocked", then set element's frozen base URL to document's fallback base URL and return.
      // SPEC-VIOLATION(CSP): there's no content security policy, so every base is allowed.
      // 4. Set element's frozen base URL to urlRecord.
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

    // 5. Respond to base URL changes given document.
    // (Also when step 3 returns, which only matters while the fallback base URL is the base URL already.)
    _baseElementURL = krys::move(baseElementURL);
    UpdateBaseURL();
  }

  void Document::UpdateBaseURL() noexcept
  {
    // The document base URL, kept up to date:
    // https://html.spec.whatwg.org/#document-base-url
    // 1. If document has a descendant base element that has an href attribute, then return the frozen base
    // URL
    //    of the first such base element in tree order.
    // 2. If document's base URL override is non-null, then return document's base URL override. (Skipped:
    //    documents have no base URL override.)
    // 3. Return document's fallback base URL.
    _baseURL = _baseElementURL.IsNull() ? FallbackBaseURL() : _baseElementURL;

    // Then respond to base URL changes given document:
    // https://html.spec.whatwg.org/#respond-to-base-url-changes
    // 1. The user agent should update any user interface elements which are displaying affected URLs, or data
    //    derived from such URLs, to the user. (Skipped: Booey has no such user interface.)
    // 2. Ensure that the CSS :link/:visited/etc. pseudo-classes are updated appropriately. (Nothing to do:
    //    without a visited link store, whether a link is visited doesn't depend on its URL.)
    // 3. For each descendant of document's shadow-including descendants: (Skipped: no speculation rules.)
    // 4. Consider speculative loads given document. (Skipped: no speculative loading.)
    // Style sheets parsed from now on resolve against the new base URL.
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

    auto captureElementChanged = CaptureChange::No;
    if (!request.ReadOnly())
    {
      RefPtr<Element> targetElement = ShareRefPtr(result.TargetElement());
      if (auto *page = GetPage())
      {
        // Before we dispatch a new mouse event, we must run the Process Pending Capture Element steps as
        // defined in https://w3c.github.io/pointerevents/#process-pending-pointer-capture.
        auto &pointerCaptureController = page->GetPointerCaptureController();
        RefPtr<Element> previousCaptureElement =
          ShareRefPtr(pointerCaptureController.PointerCaptureElement(this, event.PointerId()));
        pointerCaptureController.ProcessPendingPointerCapture(event.PointerId());
        RefPtr<Element> captureElement =
          ShareRefPtr(pointerCaptureController.PointerCaptureElement(this, event.PointerId()));
        // If the capture element has changed while running the Process Pending Capture Element steps then
        // we need to indicate that when calling UpdateHoverActiveState to be sure that the :active and :hover
        // element chains are updated.
        if (previousCaptureElement != captureElement)
        {
          captureElementChanged = CaptureChange::Yes;
        }
        // If we have a capture element, we must target it instead of what would normally hit-test for this
        // event.
        if (captureElement)
        {
          targetElement = krys::move(captureElement);
        }
      }
      UpdateHoverActiveState(request, targetElement.get(), captureElementChanged);
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
    // FIXME: Schedule a hover state update (the event handler's scheduleHoverStateUpdate).
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

      ScheduleContentRelevancyUpdate(ContentRelevancy::Focused);

      if (auto *editorClient = GetEditor().Client())
      {
        editorClient->SetInputMethodState(nullptr);
      }
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
      if (auto *page = frame->GetPage())
      {
        page->GetDragCaretController().NodeWillBeRemoved(node);
      }
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

  const DOMString &Document::OriginIdentifierForPasteboard() const noexcept
  {
    if (_uniqueIdentifier.empty())
    {
      _uniqueIdentifier = u8"null:" + CreateVersion4UUIDString();
    }
    return _uniqueIdentifier;
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

  GraphicsClient *Document::GetGraphicsClient() const noexcept
  {
    auto *page = GetPage();
    return page ? &page->GetChromeClient() : nullptr;
  }

  html::DocumentVisibilityState Document::VisibilityState() const noexcept
  {
    // The visibility of the document is inherited from the visibility of the page. If there is no page
    // associated with the document, we will assume that the page is hidden, as specified by the spec.
    auto *page = GetPage();
    if (!page)
    {
      return html::DocumentVisibilityState::Hidden;
    }
    return page->VisibilityState();
  }

  void Document::VisibilityStateChanged() noexcept
  {
    bool pageIsVisible = GetPage() && GetPage()->IsVisible();

    // https://w3c.github.io/page-visibility/#reacting-to-visibilitychange-changes
    if (!pageIsVisible)
    {
      _deferResizeEventForVisibilityChange = true;
    }

    html::EventLoop::Current().QueueTask(
      html::TaskSource::UserInteraction, *this,
      [protectedThis = ShareRef(*this)]
      {
        (void)EventDispatcher::DispatchToTarget(
          *Event::Create(DOMStringAtom(EventNames::VisibilityChange), true, false), protectedThis.get());

        bool pageIsVisible = protectedThis->GetPage() && protectedThis->GetPage()->IsVisible();
        if (!pageIsVisible)
        {
          return;
        }

        protectedThis->_deferResizeEventForVisibilityChange = false;
        if (protectedThis->_needsDOMWindowResizeEvent || protectedThis->_needsVisualViewportResizeEvent)
        {
          protectedThis->ScheduleRenderingUpdate(RenderingUpdateStep::Resize);
        }
      });

    auto clients = _visibilityStateCallbackClients;
    for (auto &client : clients)
    {
      if (auto *protectedClient = client.get())
      {
        protectedClient->VisibilityStateChanged();
      }
    }
  }

  void Document::RegisterForVisibilityStateChangedCallbacks(VisibilityChangeClient &client) noexcept
  {
    std::erase_if(_visibilityStateCallbackClients, [](const auto &weakClient) { return !weakClient; });
    _visibilityStateCallbackClients.push_back(CreateWeakPtr(&client));
  }

  void Document::UnregisterForVisibilityStateChangedCallbacks(VisibilityChangeClient &client) noexcept
  {
    std::erase_if(_visibilityStateCallbackClients,
                  [&](const auto &weakClient) { return !weakClient || weakClient.get() == &client; });
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
    if (_intersectionObserversInitialUpdateTimer.IsActive())
    {
      _intersectionObserversInitialUpdateTimer.Stop();
      requestedSteps.Add(RenderingUpdateStep::IntersectionObservations);
    }
    if (auto *page = GetPage())
    {
      page->ScheduleRenderingUpdate(requestedSteps);
    }
  }

  void Document::AddIntersectionObserver(IntersectionObserver &observer) noexcept
  {
    krys_debug_assert(
      std::ranges::find(_intersectionObservers, &observer, [](auto &weak) { return weak.get(); })
      == _intersectionObservers.end());
    _intersectionObservers.push_back(CreateWeakPtr(&observer));
  }

  void Document::RemoveIntersectionObserver(IntersectionObserver &observer) noexcept
  {
    auto it = std::ranges::find(_intersectionObservers, &observer, [](auto &weak) { return weak.get(); });
    if (it != _intersectionObservers.end())
    {
      _intersectionObservers.erase(it);
    }
  }

  void Document::PrepareCanvasesForDisplayOrFlushIfNeeded() noexcept
  {
    auto contexts = std::exchange(_canvasContextsToPrepare, {});
    for (auto &weakContext : contexts)
    {
      auto *context = weakContext.get();
      if (!context)
      {
        continue;
      }
      Ref<html::CanvasBase> canvas = ShareRef(context->GetCanvasBase());

      context->SetIsInPreparationForDisplayOrFlush(false);

      // Some canvas contexts hold memory that should be periodically freed.
      if (context->HasDeferredOperations())
      {
        context->FlushDeferredOperations();
      }

      // Some canvases need to do work when rendering has finished but before their content is composited.
      if (auto *htmlCanvas = DynamicDowncast<html::HTMLCanvasElement>(*canvas))
      {
        if (htmlCanvas->NeedsPreparationForDisplay())
        {
          htmlCanvas->PrepareForDisplay();
        }
      }
    }
  }

  void Document::AddCanvasNeedingPreparationForDisplayOrFlush(html::CanvasRenderingContext &context) noexcept
  {
    if (context.HasDeferredOperations() || context.NeedsPreparationForDisplay())
    {
      const bool shouldSchedule = std::ranges::none_of(_canvasContextsToPrepare, [](const auto &weakContext)
                                                       { return weakContext.get() != nullptr; });
      _canvasContextsToPrepare.push_back(CreateWeakPtr(&context));
      context.SetIsInPreparationForDisplayOrFlush(true);
      if (shouldSchedule)
      {
        ScheduleRenderingUpdate({RenderingUpdateStep::PrepareCanvasesForDisplayOrFlush});
      }
    }
  }

  void
    Document::RemoveCanvasNeedingPreparationForDisplayOrFlush(html::CanvasRenderingContext &context) noexcept
  {
    std::erase_if(_canvasContextsToPrepare,
                  [&](const auto &weakContext) { return weakContext.get() == &context; });
    context.SetIsInPreparationForDisplayOrFlush(false);
  }

  void Document::UpdateIntersectionObservations() noexcept
  {
    UpdateIntersectionObservations(_intersectionObservers);
  }

  void Document::UpdateIntersectionObservations(
    const List<WeakPtr<IntersectionObserver>> &intersectionObservers) noexcept
  {
    RefPtr<LocalFrame> frame = ShareRefPtr(Frame());
    if (!frame)
    {
      return;
    }

    RefPtr<LocalFrameView> frameView = ShareRefPtr(View());
    if (!frameView)
    {
      return;
    }

    // FIXME: The page deferring intersection observations (shouldDeferIntersectionObservations), once it can.
    if (!GetPage())
    {
      return;
    }

    bool needsLayout =
      frameView->LayoutContext().IsLayoutPending() || (GetRenderView() && GetRenderView()->NeedsLayout());
    if (needsLayout || HasPendingStyleRecalc())
    {
      if (!intersectionObservers.empty())
      {
        ScheduleRenderingUpdate({RenderingUpdateStep::IntersectionObservations});
      }
      return;
    }

    List<WeakPtr<IntersectionObserver>> intersectionObserversWithPendingNotifications;

    // Copied, as observing and unobserving change the list.
    for (auto &weakObserver : List<WeakPtr<IntersectionObserver>>(intersectionObservers))
    {
      RefPtr<IntersectionObserver> observer = ShareRefPtr(weakObserver.get());
      if (!observer)
      {
        continue;
      }

      auto needNotify = observer->UpdateObservations(*frame);
      if (needNotify == IntersectionObserver::NeedNotify::Yes)
      {
        intersectionObserversWithPendingNotifications.push_back(CreateWeakPtr(observer.get()));
      }
    }

    for (auto &weakObserver : intersectionObserversWithPendingNotifications)
    {
      if (RefPtr<IntersectionObserver> observer = ShareRefPtr(weakObserver.get()))
      {
        observer->Notify();
      }
    }
  }

  void Document::ScheduleInitialIntersectionObservationUpdate() noexcept
  {
    if (ReadyState() == html::DocumentReadyState::Complete)
    {
      ScheduleRenderingUpdate({RenderingUpdateStep::IntersectionObservations});
    }
    else if (!_intersectionObserversInitialUpdateTimer.IsActive())
    {
      _intersectionObserversInitialUpdateTimer.StartOneShot(IntersectionObserversInitialUpdateDelay);
    }
  }

  svg::SVGDocumentExtensions &Document::SVGExtensions() noexcept
  {
    if (!_svgExtensions)
    {
      _svgExtensions = CreateUnique<svg::SVGDocumentExtensions>(*this);
    }
    return *_svgExtensions;
  }

  IntersectionObserverData &Document::EnsureIntersectionObserverData() noexcept
  {
    if (!_intersectionObserverData)
    {
      _intersectionObserverData = CreateUnique<IntersectionObserverData>();
    }
    return *_intersectionObserverData;
  }

  html::LazyLoadImageObserver &Document::GetLazyLoadImageObserver() noexcept
  {
    if (!_lazyLoadImageObserver)
    {
      _lazyLoadImageObserver = CreateUnique<html::LazyLoadImageObserver>();
    }
    return *_lazyLoadImageObserver;
  }

  ContentVisibilityDocumentState &Document::GetContentVisibilityDocumentState() noexcept
  {
    if (!_contentVisibilityDocumentState)
    {
      _contentVisibilityDocumentState = CreateUnique<ContentVisibilityDocumentState>();
    }
    return *_contentVisibilityDocumentState;
  }

  bool Document::IsObservingContentVisibilityTargets() const noexcept
  {
    return _contentVisibilityDocumentState && _contentVisibilityDocumentState->HasObservationTargets();
  }

  DidUpdateAnyContentRelevancy Document::UpdateRelevancyOfContentVisibilityElements(
    UpdateLayoutIfContentVisibilityChanged updateLayout) noexcept
  {
    if (_contentRelevancyUpdate.IsEmpty() || !IsObservingContentVisibilityTargets())
    {
      return DidUpdateAnyContentRelevancy::No;
    }
    const DidUpdateAnyContentRelevancy result =
      _contentVisibilityDocumentState->UpdateRelevancyOfContentVisibilityElements(_contentRelevancyUpdate);
    if (result == DidUpdateAnyContentRelevancy::Yes
        && updateLayout == UpdateLayoutIfContentVisibilityChanged::Yes)
    {
      (void)UpdateLayoutIgnorePendingStylesheets();
    }
    _contentRelevancyUpdate = {};
    return result;
  }

  void Document::ScheduleContentRelevancyUpdate(ContentRelevancy contentRelevancy) noexcept
  {
    if (!IsObservingContentVisibilityTargets())
    {
      return;
    }
    _contentRelevancyUpdate.Add(contentRelevancy);
    ScheduleRenderingUpdate({RenderingUpdateStep::UpdateContentRelevancy});
  }

  void Document::UpdateContentRelevancyForScrollIfNeeded(const Element &scrollAnchor) noexcept
  {
    if (!_contentVisibilityDocumentState)
    {
      return;
    }
    _contentVisibilityDocumentState->UpdateContentRelevancyForScrollIfNeeded(scrollAnchor);
  }

  void Document::AddResizeObserver(ResizeObserver &observer) noexcept
  {
    if (std::ranges::find(_resizeObservers, &observer, [](auto &weak) { return weak.get(); })
        == _resizeObservers.end())
    {
      _resizeObservers.push_back(CreateWeakPtr(&observer));
    }
  }

  void Document::RemoveResizeObserver(ResizeObserver &observer) noexcept
  {
    auto it = std::ranges::find(_resizeObservers, &observer, [](auto &weak) { return weak.get(); });
    if (it != _resizeObservers.end())
    {
      _resizeObservers.erase(it);
    }
  }

  bool Document::HasResizeObservers() const noexcept
  {
    return !_resizeObservers.empty();
  }

  size_t Document::GatherResizeObservations(size_t deeperThan) noexcept
  {
    size_t minDepth = ResizeObserver::MaxElementDepth();
    for (auto &weakObserver : _resizeObservers)
    {
      RefPtr<ResizeObserver> observer = ShareRefPtr(weakObserver.get());
      if (!observer || !observer->HasObservations())
      {
        continue;
      }

      auto depth = observer->GatherObservations(deeperThan);
      minDepth = std::min(minDepth, depth);
    }
    return minDepth;
  }

  size_t Document::GatherResizeObservationsForContainIntrinsicSize() noexcept
  {
    if (!_resizeObserverForContainIntrinsicSize)
    {
      return ResizeObserver::MaxElementDepth();
    }

    return _resizeObserverForContainIntrinsicSize->GatherObservations(0);
  }

  void Document::DeliverResizeObservations() noexcept
  {
    if (_resizeObserverForContainIntrinsicSize)
    {
      Ref<ResizeObserver> observer = _resizeObserverForContainIntrinsicSize;
      observer->DeliverObservations();
    }

    List<RefPtr<ResizeObserver>> observersToNotify;
    for (auto &weakObserver : _resizeObservers)
    {
      RefPtr<ResizeObserver> observer = ShareRefPtr(weakObserver.get());
      if (observer && observer->HasActiveObservations())
      {
        observersToNotify.push_back(krys::move(observer));
      }
    }
    for (auto &observer : observersToNotify)
    {
      observer->DeliverObservations();
    }
  }

  bool Document::HasSkippedResizeObservations() const noexcept
  {
    return std::ranges::any_of(_resizeObservers,
                               [](auto &observer) { return observer && observer->HasSkippedObservations(); });
  }

  void Document::SetHasSkippedResizeObservations(bool skipped) noexcept
  {
    for (auto &observer : _resizeObservers)
    {
      if (observer)
      {
        observer->SetHasSkippedObservations(skipped);
      }
    }
  }

  void Document::UpdateResizeObservations(Page &page) noexcept
  {
    if (_renderView)
    {
      // Per spec, this is recorded when ResizeObserver events are determined and delivered.
      // See https://drafts.csswg.org/css-anchor-position-1/#last-successful-position-option.
      auto lastSuccessfulPositionOptionMap =
        style::AnchorPositionEvaluator::RecordLastSuccessfulPositionOptions(_renderView->PositionTryBoxes());
      StyleScope().SetLastSuccessfulPositionOptionIndexMap(krys::move(lastSuccessfulPositionOptionMap));
    }

    if (!HasResizeObservers() && !_resizeObserverForContainIntrinsicSize && !_contentVisibilityDocumentState)
    {
      return;
    }

    size_t resizeObserverDepth = 0;
    while (true)
    {
      page.LayoutIfNeeded();
      // If we have determined a change because of visibility we need to get up-to-date layout before
      // reporting any values to resize observers.
      if (_contentVisibilityDocumentState
          && _contentVisibilityDocumentState->DetermineInitialVisibleContentVisibility()
               == HadInitialVisibleContentVisibilityDetermination::Yes)
      {
        continue;
      }

      // Start check resize observers;
      if (!resizeObserverDepth
          && GatherResizeObservationsForContainIntrinsicSize() != ResizeObserver::MaxElementDepth())
      {
        DeliverResizeObservations();
      }

      resizeObserverDepth = GatherResizeObservations(resizeObserverDepth);
      if (resizeObserverDepth == ResizeObserver::MaxElementDepth())
      {
        break;
      }
      DeliverResizeObservations();
    }

    if (HasSkippedResizeObservations())
    {
      SetHasSkippedResizeObservations(false);
      // (The loop error isn't reported, as there's no script error reporting.)
      // Starting a new schedule the next round of notify.
      ScheduleRenderingUpdate({RenderingUpdateStep::ResizeObservations});
    }
  }

  void Document::ObserveForContainIntrinsicSize(Element &element) noexcept
  {
    if (!_resizeObserverForContainIntrinsicSize)
    {
      _resizeObserverForContainIntrinsicSize =
        ResizeObserver::CreateNativeObserver(*this, CallbackForContainIntrinsicSize);
    }
    _resizeObserverForContainIntrinsicSize->Observe(element, {ResizeObserverBoxOptions::ContentBox});
  }

  void Document::UnobserveForContainIntrinsicSize(Element &element) noexcept
  {
    if (RefPtr<ResizeObserver> observer = _resizeObserverForContainIntrinsicSize)
    {
      observer->Unobserve(element);
    }
  }

  void Document::ResetObservationSizeForContainIntrinsicSize(Element &element) noexcept
  {
    if (RefPtr<ResizeObserver> observer = _resizeObserverForContainIntrinsicSize)
    {
      observer->ResetObservationSize(element);
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
      auto eventName = type == ScrollEventType::Scroll ? EventNames::Scroll : EventNames::ScrollEnd;
      (void)EventDispatcher::FireEvent<Event>(DOMStringAtom(eventName), *target,
                                              EventInit {.Bubbles = Is<Document>(*target)});
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
    // FIXME: Deferring the resize events while the page asks (shouldDeferResizeEvents), once it can.
    if (_deferResizeEventForVisibilityChange)
    {
      return;
    }

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
  }

  void Document::RemoveTopLayerElement(Element &element) noexcept
  {
    krys_assert(&element.NodeDocument() == this && element.IsInTopLayer());
    Ref<Element> protectedElement = ShareRef(element);
    _topLayerPendingRemovals.remove(protectedElement);
    KRYS_MAYBE_UNUSED bool didRemove = _topLayerElements.remove(protectedElement);
    krys_assert(didRemove);
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

  // https://html.spec.whatwg.org/#run-light-dismiss-activities
  void Document::HandleLightDismiss(const PointerEvent &event, Node &target) noexcept
  {
    krys_debug_assert(event.IsTrusted());

    // 1. Run light dismiss open popovers with event.
    // https://html.spec.whatwg.org/#light-dismiss-open-popovers
    // 1. Assert: event's isTrusted attribute is true.
    // 2. Let target be event's target.
    // 3. Let document be target's node document.
    // 4. If the result of running topmost auto or hint popover given document is null, then return.
    if (TopmostAutoOrHintPopover())
    {
      RefPtr<html::HTMLElement> ancestor = TopmostClickedPopover(target);
      // 5. If event's type is "pointerdown": set document's popover pointerdown target to the result of
      //    running topmost clicked popover given target.
      if (event.Type() == EventNames::PointerDown)
      {
        _popoverPointerDownTarget = CreateWeakPtr(ancestor.get());
      }
      // 6. If event's type is "pointerup":
      else if (event.Type() == EventNames::PointerUp)
      {
        // 6a. Let ancestor be the result of running topmost clicked popover given target.
        // 6b. Let sameTarget be true if ancestor is document's popover pointerdown target.
        bool sameTarget = ancestor.get() == _popoverPointerDownTarget.get();
        // 6c. Set document's popover pointerdown target to null.
        _popoverPointerDownTarget = nullptr;
        // 6d. If sameTarget is false, then return.
        // 6e. Run hide popovers until given document, ancestor, false, and true.
        if (sameTarget)
        {
          HideAllPopoversUntil(ancestor.get(), FocusPreviousElement::No, FireEvents::Yes);
        }
      }
    }

    // 2. Run light dismiss open dialogs with event.
    // https://html.spec.whatwg.org/#light-dismiss-open-dialogs
    // 1. Assert: event's isTrusted attribute is true.
    // 2. Let document be event's target's node document.
    // 3. If document's open dialogs list is empty, then return.
    if (_openDialogs.empty())
    {
      return;
    }

    // https://html.spec.whatwg.org/#nearest-clicked-dialog
    auto nearestClickedDialog = [&]() -> html::HTMLDialogElement *
    {
      // 1. Let target be event's target.
      // 2. If target is a dialog element, target has an open attribute, target's is modal is true, and
      // event's
      //    clientX and clientY are outside the bounds of target, then return null.
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
      // 3. Let currentNode be target.
      // 4. While currentNode is not null:
      // 4a. If currentNode is a dialog element and currentNode has an open attribute, then return
      // currentNode. 4b. Set currentNode to currentNode's parent in the flat tree. (Only elements can be
      // dialogs, so a non-element target starts from its parent.)
      auto *element = DynamicDowncast<Element>(target);
      for (element = element ? element : target.ParentElementInComposedTree(); element;
           element = element->ParentElementInComposedTree())
      {
        if (auto *dialog = DynamicDowncast<html::HTMLDialogElement>(*element); dialog && dialog->Open())
        {
          return dialog;
        }
      }

      // 5. Return null.
      return nullptr;
    };

    // 4. Let ancestor be the result of running nearest clicked dialog given event.
    RefPtr<html::HTMLDialogElement> ancestor = ShareRefPtr(nearestClickedDialog());

    // 5. If event's type is "pointerdown", then set document's dialog pointerdown target to ancestor.
    if (event.Type() == EventNames::PointerDown)
    {
      _dialogPointerDownTarget = CreateWeakPtr(ancestor.get());
      return;
    }

    // 6. If event's type is "pointerup":
    if (event.Type() == EventNames::PointerUp)
    {
      // 6a. Let sameTarget be true if ancestor is document's dialog pointerdown target.
      bool sameTarget = ancestor.get() == _dialogPointerDownTarget.get();

      // 6b. Set document's dialog pointerdown target to null.
      _dialogPointerDownTarget = nullptr;

      // 6c. If sameTarget is false, then return.
      if (!sameTarget)
      {
        return;
      }

      // 6d. Let topmostDialog be the last element of document's open dialogs list.
      Ref<html::HTMLDialogElement> topmostDialog = _openDialogs.last();

      // 6e. If ancestor is topmostDialog, then return.
      if (ancestor.get() == topmostDialog.get())
      {
        return;
      }

      // 6f. If topmostDialog's computed closed-by state is not Any, then return.
      if (topmostDialog->ComputedClosedByState() != html::HTMLDialogElement::ClosedByState::Any)
      {
        return;
      }

      // 6g. Assert: topmostDialog's close watcher is not null.
      // (Null in a document without a window.)
      // 6h. Request to close topmostDialog's close watcher with false.
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

  // https://html.spec.whatwg.org/#topmost-auto-or-hint-popover
  html::HTMLElement *Document::TopmostAutoOrHintPopover() const noexcept
  {
    // 1. If document's showing hint popover list is not empty, then return document's showing hint popover
    //    list's last element.
    // 2. If document's showing auto popover list is not empty, then return document's showing auto popover
    //    list's last element.
    // 3. Return null.
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

  // https://html.spec.whatwg.org/#hide-popovers-until
  void Document::HideAllPopoversUntil(html::HTMLElement *endpoint, FocusPreviousElement focusPreviousElement,
                                      FireEvents fireEvents) noexcept
  {
    // 1. Let endpointIsHint be true if document's showing hint popover list contains endpoint; otherwise
    //    false.
    auto hintPopovers = ShowingHintPopoverList();
    bool endpointIsHint =
      endpoint
      && std::ranges::find(hintPopovers, endpoint, [](auto &popover) { return popover.get(); })
           != hintPopovers.end();

    // 2. Run hide popover stack until given document, endpoint, Hint, focusPreviousElement, and fireEvents.
    HidePopoverStackUntil(endpoint, PopoverState::Hint, focusPreviousElement, fireEvents);

    // 3. Let autoEndpoint be endpoint.
    RefPtr<html::HTMLElement> autoEndpoint = ShareRefPtr(endpoint);
    // 4. If endpointIsHint is true, then set autoEndpoint to document's hint stack parent.
    if (endpointIsHint)
    {
      autoEndpoint = ShareRefPtr(HintStackParent());
    }
    // 5. Run hide popover stack until given document, autoEndpoint, Auto, focusPreviousElement, and
    //    fireEvents.
    HidePopoverStackUntil(autoEndpoint.get(), PopoverState::Auto, focusPreviousElement, fireEvents);
  }

  // https://html.spec.whatwg.org/#hide-popover-stack-until
  void Document::HidePopoverStackUntil(html::HTMLElement *endpoint, PopoverState stackType,
                                       FocusPreviousElement focusPreviousElement,
                                       FireEvents fireEvents) noexcept
  {
    krys_debug_assert(stackType == PopoverState::Auto || stackType == PopoverState::Hint);
    // 1. Let popoverList be document's showing auto popover list if stackType is Auto; otherwise document's
    //    showing hint popover list.
    auto popoverList = stackType == PopoverState::Auto ? ShowingAutoPopoverList() : ShowingHintPopoverList();

    // 2. Let lastHideIndex be 0 if popoverList does not contain endpoint; otherwise the index of endpoint in
    //    popoverList plus 1.
    // 3. Let toHide be a slice of popoverList from lastHideIndex, in reverse order.
    // 4. Let toRemain be a slice of popoverList from 0 to lastHideIndex.
    auto endpointIt = std::ranges::find(popoverList, endpoint, [](auto &popover) { return popover.get(); });
    auto lastHideIndex =
      endpointIt == popoverList.end() ? 0uz : static_cast<size_t>(endpointIt - popoverList.begin()) + 1;
    List<Ref<html::HTMLElement>> toHide {popoverList.rbegin(), popoverList.rend() - lastHideIndex};
    List<Ref<html::HTMLElement>> toRemain {popoverList.begin(), popoverList.begin() + lastHideIndex};

    // 5. For each popover of toHide: run the hide popover algorithm given popover, focusPreviousElement,
    //    fireEvents, false, and null.
    for (auto &popover : toHide)
    {
      (void)popover->HidePopoverInternal(focusPreviousElement, fireEvents, false, nullptr);
    }

    // 6. Let newPopoverList be document's showing auto popover list if stackType is Auto; otherwise
    // document's
    //    showing hint popover list.
    // 7. Let toCheck be newPopoverList in reverse order.
    auto newPopoverList =
      stackType == PopoverState::Auto ? ShowingAutoPopoverList() : ShowingHintPopoverList();
    // 8. For each popover of toCheck:
    for (auto popover = newPopoverList.rbegin(); popover != newPopoverList.rend(); ++popover)
    {
      // 8a. If toRemain contains popover, then continue.
      if (std::ranges::find(toRemain, *popover) != toRemain.end())
      {
        continue;
      }
      // 8b. Run the hide popover algorithm given popover, focusPreviousElement, false, false, and null.
      (void)(*popover)->HidePopoverInternal(focusPreviousElement, FireEvents::No, false, nullptr);
    }
  }

  void Document::SetHintStackParent(html::HTMLElement *element) noexcept
  {
    _hintStackParent = CreateWeakPtr(element);
  }

  namespace
  {
    /// @see https://html.spec.whatwg.org/#target-popover
    KRYS_NODISCARD RefPtr<html::HTMLElement> TargetPopover(html::HTMLElement &element) noexcept
    {
      auto *control = DynamicDowncast<html::HTMLFormControlElement>(element);
      // 1. If node is not a button element, then return node's popover target element.
      auto *button = DynamicDowncast<html::HTMLButtonElement>(element);
      if (!button)
      {
        return control ? control->PopoverTargetElement() : nullptr;
      }
      // 2. If node is disabled, then return null.
      if (button->IsDisabledFormControl())
      {
        return nullptr;
      }
      // 3. Let target be the result of running node's get the commandfor-associated element.
      // 4. If target is null, then return node's popover target element.
      RefPtr<Element> target = button->CommandForElement();
      if (!target)
      {
        return button->PopoverTargetElement();
      }
      // 5. If node has a form owner:
      if (button->Form())
      {
        // 5a. If element is a submit button, then return null.
        // 5b. If element's type attribute is in the Reset Button state, then return null.
        // 5c. If element's type attribute is in the Auto state, then return null.
        // (A button with a command or commandfor attribute and no valid type is in the Auto state, which
        // behaves as the Button state.)
        if (button->IsSubmitButton() || button->Type() == u8"reset"
            || !text::IsASCIICaselessEqual(
              DOMStringView(button->GetAttribute(u8"type").value_or(DOMString())), DOMStringView(u8"button")))
        {
          return nullptr;
        }
      }
      // 6. Let command be node's command attribute.
      // 7. If command is not in the Toggle Popover, Hide Popover, or Show Popover states, then return null.
      auto command = button->GetCommandType();
      if (command != CommandType::TogglePopover && command != CommandType::HidePopover
          && command != CommandType::ShowPopover)
      {
        return nullptr;
      }
      // 8. If target's popover attribute is in the No Popover state, then return null.
      auto *popover = DynamicDowncast<html::HTMLElement>(*target);
      if (!popover || popover->GetPopoverState() == PopoverState::None)
      {
        return nullptr;
      }
      // 9. Return target.
      return ShareRefPtr(popover);
    }
  }

  // https://html.spec.whatwg.org/#topmost-clicked-popover
  RefPtr<html::HTMLElement> Document::TopmostClickedPopover(Node &node) const noexcept
  {
    auto *startElement = DynamicDowncast<Element>(node);
    if (!startElement)
    {
      startElement = node.ParentElementInComposedTree();
    }

    // 1. Let clickedPopover be the result of running nearest inclusive open popover given node.
    // 2. Let targetPopover be the result of running nearest inclusive target popover given node.
    // https://html.spec.whatwg.org/#nearest-inclusive-open-popover
    // https://html.spec.whatwg.org/#nearest-inclusive-target-popover
    // (Both walk the flat tree up from node, so they're found together.)
    html::HTMLElement *clickedPopover = nullptr;
    RefPtr<html::HTMLElement> targetPopover;
    for (auto *element = startElement; element && (!clickedPopover || !targetPopover);
         element = element->ParentElementInComposedTree())
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(*element);
      if (!htmlElement)
      {
        continue;
      }
      // If currentNode's opened in popover mode is "auto" or "hint", and currentNode's popover visibility
      // state is showing, then return currentNode.
      if (!clickedPopover && htmlElement->IsPopoverShowing()
          && htmlElement->GetPopoverData()->OpenedInPopoverMode() != PopoverState::None)
      {
        clickedPopover = htmlElement;
      }
      // Let targetPopover be currentNode's target popover. If targetPopover is not null and targetPopover's
      // popover attribute is in the Auto state or the Hint state, and targetPopover's popover visibility
      // state is showing, then return targetPopover.
      if (!targetPopover)
      {
        auto popover = TargetPopover(*htmlElement);
        if (popover && popover->IsPopoverShowing()
            && (popover->GetPopoverState() == PopoverState::Auto
                || popover->GetPopoverState() == PopoverState::Hint))
        {
          targetPopover = krys::move(popover);
        }
      }
    }

    // https://html.spec.whatwg.org/#get-the-popover-stack-position
    auto autoPopovers = ShowingAutoPopoverList();
    auto hintPopovers = ShowingHintPopoverList();
    auto stackPosition = [&](html::HTMLElement *popover) -> size_t
    {
      // 1. Let hintList be popover's node document's showing hint popover list.
      // 2. Let autoList be popover's node document's showing auto popover list.
      if (!popover)
      {
        return 0;
      }
      auto byPointer = [](auto &element)
      {
        return element.get();
      };
      // 3. If popover is in hintList, then return the index of popover in hintList + the size of autoList +
      //    1.
      if (auto it = std::ranges::find(hintPopovers, popover, byPointer); it != hintPopovers.end())
      {
        return static_cast<size_t>(it - hintPopovers.begin()) + autoPopovers.size() + 1;
      }
      // 4. If popover is in autoList, then return the index of popover in autoList + 1.
      if (auto it = std::ranges::find(autoPopovers, popover, byPointer); it != autoPopovers.end())
      {
        return static_cast<size_t>(it - autoPopovers.begin()) + 1;
      }
      // 5. Return 0.
      return 0;
    };
    // 3. If the result of getting the popover stack position given clickedPopover is greater than the result
    //    of getting the popover stack position given targetPopover, then return clickedPopover.
    if (stackPosition(clickedPopover) > stackPosition(targetPopover.get()))
    {
      return ShareRefPtr(clickedPopover);
    }
    // 4. Return targetPopover.
    return targetPopover;
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

      // The page may have suspended its animation frame callbacks before the controller was made.
      if (!GetPage() || GetPage()->ScriptedAnimationsSuspended())
      {
        _scriptedAnimationController->Suspend();
      }
    }
    return _scriptedAnimationController->RegisterCallback(krys::move(callback));
  }

  void Document::SuspendScriptedAnimationControllerCallbacks() noexcept
  {
    if (_scriptedAnimationController)
    {
      _scriptedAnimationController->Suspend();
    }
  }

  void Document::ResumeScriptedAnimationControllerCallbacks() noexcept
  {
    if (_scriptedAnimationController)
    {
      _scriptedAnimationController->Resume();
    }
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

    for (auto &weakElement : List<WeakPtr<Element>>(_documentSuspensionCallbackElements))
    {
      if (RefPtr element = ShareRefPtr(weakElement.get()))
      {
        element->PrepareForDocumentSuspension();
      }
    }

    // FIXME: View transitions, the page's overlay scrollbars, the composited layers' pending update and font
    // loading, once there are those.
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

    for (auto &weakElement : List<WeakPtr<Element>>(_documentSuspensionCallbackElements))
    {
      if (RefPtr element = ShareRefPtr(weakElement.get()))
      {
        element->ResumeFromDocumentSuspension();
      }
    }

    if (_timelinesController)
    {
      _timelinesController->ResumeAnimations();
    }

    ResumeScheduledTasks(reason);

    _isSuspended = false;
  }

  void Document::RegisterForDocumentSuspensionCallbacks(Element &element) noexcept
  {
    std::erase_if(_documentSuspensionCallbackElements, [](const auto &weakElement) { return !weakElement; });
    if (std::ranges::none_of(_documentSuspensionCallbackElements,
                             [&](const auto &weakElement) { return weakElement.get() == &element; }))
    {
      _documentSuspensionCallbackElements.push_back(CreateWeakPtr(&element));
    }
  }

  void Document::UnregisterForDocumentSuspensionCallbacks(Element &element) noexcept
  {
    std::erase_if(_documentSuspensionCallbackElements,
                  [&](const auto &weakElement) { return !weakElement || weakElement.get() == &element; });
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

  void Document::QueueTaskToDispatchEvent(html::TaskSource source, Ref<Event> &&event) noexcept
  {
    html::EventLoop::Current().QueueTask(
      source, *this, [protectedThis = ShareRef(*this), event = krys::move(event)]
      { (void)EventDispatcher::DispatchToTarget(*event, protectedThis.get()); });
  }

  void Document::ExitPointerLock() noexcept
  {
    RefPtr<Page> page = ShareRefPtr(GetPage());
    if (!page)
    {
      return;
    }
    if (auto *target = page->GetPointerLockController().GetElement())
    {
      if (&target->NodeDocument() != this)
      {
        return;
      }
    }
    page->GetPointerLockController().RequestPointerUnlock();
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

  ExceptionOr<void> Document::SetXMLVersion(DOMString version) noexcept
  {
    if (!xml::XMLDocumentParser::SupportsXMLVersion(version))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    _xmlVersion = krys::move(version);
    return {};
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

  void Document::ImplicitClose() noexcept
  {
    Ref<Document> protectedThis = ShareRef(*this);
    if (Frame())
    {
      if (auto *svgExtensions = SVGExtensionsIfExists())
      {
        svgExtensions->DispatchLoadEventToOutermostSVGElements();
      }
    }

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

    if (auto *svgExtensions = SVGExtensionsIfExists())
    {
      svgExtensions->StartAnimations();
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
    (void)EventDispatcher::FireEvent<Event>(DOMStringAtom(EventNames::Load), *window, {}, true);
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
      // https://html.spec.whatwg.org/#pragma-set-default-language
      // 1. If the meta element has no content attribute, then return.
      // 2. If the element's content attribute contains a U+002C COMMA character (,), then return.
      if (content.find(u8',') != DOMStringView::npos)
      {
        return;
      }

      // 3. Let input be the value of the element's content attribute.
      // 4. Let position point at the first character of input.
      // 5. Skip ASCII whitespace within input given position.
      size_t start = 0;
      while (start < content.size() && text::IsASCIIWhitespace(content[start]))
      {
        ++start;
      }

      // 6. Collect a sequence of code points that are not ASCII whitespace from input given position.
      // 7. Let candidate be the string that resulted from the previous step.
      size_t end = start;
      while (end < content.size() && !text::IsASCIIWhitespace(content[end]))
      {
        ++end;
      }

      // 8. If candidate is the empty string, return.
      // 9. Set the pragma-set default language to candidate.
      if (end > start)
      {
        SetContentLanguage(DOMString(content.substr(start, end - start)));
      }
      return;
    }

    if (text::IsASCIICaselessEqual(equiv, utf8_stringview(u8"default-style")))
    {
      // https://html.spec.whatwg.org/#attr-meta-http-equiv-default-style
      // 1. If the meta element has no content attribute, or if that attribute's value is the empty string,
      //    then return.
      // 2. Change the preferred CSS style sheet set name with the name being the value of the element's
      //    content attribute.
      if (!content.empty())
      {
        StyleScope().SetPreferredStylesheetSetName(DOMString(content));
      }
      return;
    }

    // Refresh is ignored, as Booey doesn't navigate by itself; so are Content-Security-Policy (there's no
    // security policy), Set-Cookie (obsolete), X-UA-Compatible and Content-Type (the parser reads the
    // charset).
  }

  void Document::SetContentLanguage(DOMString language) noexcept
  {
    if (_contentLanguage == language)
    {
      return;
    }
    _contentLanguage = krys::move(language);

    // Recalculate style so language is used when selecting the initial font, and :lang() matches.
    StyleScope().DidChangeStyleSheetEnvironment();
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

      // FIXME: Delete the layout context's detached renderers.

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
    // empty document's (takeDOMWindowFrom).
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
    // FIXME: Unload, page hide and the other steps of unloading the document.
    StopActiveDOMObjects();
    _selection->WillBeRemovedFromFrame();

    if (HasLivingRenderTree())
    {
      DestroyRenderTree();
    }

    if (auto *page = GetPage())
    {
      page->GetPointerLockController().DocumentDetached(*this);
    }

    _styleRecalcTimer.Stop();

    if (_timelinesController)
    {
      _timelinesController->DetachFromDocument();
    }
    _timeline = nullptr;

    CommonTeardown();

    _frame = nullptr;
  }

  void Document::CommonTeardown() noexcept
  {
    if (auto *svgExtensions = SVGExtensionsIfExists())
    {
      svgExtensions->PauseAnimations();
    }

    // FIXME: The rest of the teardown shared by destruction and the last reference going, once there's any.
    auto intersectionObservers = _intersectionObservers;
    for (auto &weakIntersectionObserver : intersectionObservers)
    {
      if (RefPtr<IntersectionObserver> intersectionObserver = ShareRefPtr(weakIntersectionObserver.get()))
      {
        intersectionObserver->Disconnect();
      }
    }

    auto resizeObservers = _resizeObservers;
    for (auto &weakResizeObserver : resizeObservers)
    {
      if (RefPtr<ResizeObserver> resizeObserver = ShareRefPtr(weakResizeObserver.get()))
      {
        resizeObserver->Disconnect();
      }
    }
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

  List<RefPtr<WebAnimation>> Document::GetAnimations() noexcept
  {
    return MatchingAnimations([](Element &target) -> bool
                              { return !TreeQueries::ContainingShadowRoot(target); });
  }

  List<RefPtr<WebAnimation>>
    Document::MatchingAnimations(KRYS_NOESCAPE const Func<bool(Element &)> &function) noexcept
  {
    // For the list of animations to be current, we need to account for any pending CSS changes, such as
    // updates to CSS Animations and CSS Transitions. This requires updating layout as well since resolving
    // layout-dependent media queries could yield animations.
    (void)UpdateStyleIfNeeded();

    List<RefPtr<WebAnimation>> animations;

    auto effectCanBeListed = [&](AnimationEffect *effect)
    {
      if (auto *keyframeEffect = DynamicDowncast<KeyframeEffect>(effect))
      {
        RefPtr target = ShareRefPtr(keyframeEffect->Target());
        return target && target->IsConnected() && &target->NodeDocument() == this && function(*target);
      }

      return false;
    };

    for (auto *animation : WebAnimation::Instances())
    {
      if (animation->IsRelevant() && effectCanBeListed(animation->Effect()))
      {
        animations.push_back(ShareRefPtr(animation));
      }
    }

    std::ranges::stable_sort(animations, [](const auto &lhs, const auto &rhs)
                             { return CompareAnimationsByCompositeOrder(*lhs, *rhs); });

    return animations;
  }

  AnimationTimelinesController &Document::EnsureTimelinesController() noexcept
  {
    if (!_timelinesController)
    {
      _timelinesController = CreateUnique<AnimationTimelinesController>(*this);
    }
    return *_timelinesController;
  }

  StyleOriginatedTimelinesController &Document::EnsureStyleOriginatedTimelinesController() noexcept
  {
    if (!_styleOriginatedTimelinesController)
    {
      _styleOriginatedTimelinesController = CreateUnique<StyleOriginatedTimelinesController>();
    }
    return *_styleOriginatedTimelinesController;
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
    auto *window = Window();
    if (!window)
    {
      return;
    }

    if (auto *timelinesController = TimelinesController())
    {
      timelinesController->UpdateAnimationsAndSendEvents(
        Seconds {std::chrono::duration<double, std::milli>(window->FrozenNowTimestamp())});
    }
  }

  void Document::RunPostRenderingUpdateAnimationTasks() noexcept
  {
    if (auto *timelinesController = TimelinesController())
    {
      timelinesController->RunPostRenderingUpdateTasks();
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
    // FIXME: Wait until the view is visually non-empty.
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

  Document::UpdateLayoutResult
    Document::UpdateLayoutIgnorePendingStylesheets(EnumSet<LayoutOptions> layoutOptions,
                                                   const Element *context) noexcept
  {
    layoutOptions.Add(LayoutOptions::IgnorePendingStylesheets);
    return UpdateLayout(layoutOptions, context);
  }

  namespace
  {
    KRYS_NODISCARD RenderElement *RootForSkippedLayout(RenderElement &rootCandidate) noexcept
    {
      auto skippedSubtreeRoot = [&](auto &skippedRootOrSkippedContent) -> RenderElement *
      {
        if (auto *renderBox = DynamicDowncast<RenderBox>(skippedRootOrSkippedContent);
            renderBox && IsSkippedContentRoot(*renderBox))
        {
          return renderBox;
        }
        for (auto *ancestor = skippedRootOrSkippedContent.ContainingBlock(); ancestor;
             ancestor = ancestor->ContainingBlock())
        {
          if (IsSkippedContentRoot(*ancestor))
          {
            return ancestor;
          }
        }
        return &skippedRootOrSkippedContent.View();
      };
      return skippedSubtreeRoot(rootCandidate);
    }
  }

  Document::UpdateLayoutResult Document::UpdateLayout(EnumSet<LayoutOptions> layoutOptions,
                                                      const Element *context) noexcept
  {
    bool oldIgnore = _ignorePendingStylesheets;
    auto restoreIgnorePendingStylesheets = MakeScopeExit([&] { _ignorePendingStylesheets = oldIgnore; });

    if (layoutOptions.Contains(LayoutOptions::IgnorePendingStylesheets))
    {
      if (!HaveStylesheetsLoaded())
      {
        _ignorePendingStylesheets = true;
        // FIXME: This should just invalidate elements with missing styles.
        if (_hasNodesWithMissingStyle)
        {
          ScheduleFullStyleRebuild();
        }
      }
      if (UpdateRelevancyOfContentVisibilityElements(UpdateLayoutIfContentVisibilityChanged::No)
          == DidUpdateAnyContentRelevancy::Yes)
      {
        _ignorePendingStylesheets = oldIgnore;
        return UpdateLayout(layoutOptions, context);
      }
    }

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
      auto runForcedLayoutOnSkippedContentIfNeeded = [&]
      {
        if (!layoutOptions.ContainsAny({LayoutOptions::TreatContentVisibilityHiddenAsVisible,
                                        LayoutOptions::TreatContentVisibilityAutoAsVisible,
                                        LayoutOptions::TreatRevealedWhenFoundAsVisible}))
        {
          return false;
        }

        if (context
            && (!context->Renderer() || !context->Renderer()->Style().IsSkippedRootOrSkippedContent()))
        {
          return false;
        }

        auto *rootForLayout = RootForSkippedLayout(context ? *context->Renderer() : *GetRenderView());
        if (!rootForLayout)
        {
          krys_debug_assert_msg(false, "a skipped subtree always has a root");
          return false;
        }

        auto markRendererDirtyIfNeeded = [&](auto &renderer)
        {
          auto everhadLayoutAndWasSkippedDuringLast =
            renderer.WasSkippedDuringLastLayoutDueToContentVisibility();
          // Never had layout or was skipped at the last one (or marked dirty since the last layout, but not
          // self needs layout which is required to "refresh" stale content).
          if (!everhadLayoutAndWasSkippedDuringLast || *everhadLayoutAndWasSkippedDuringLast
              || (renderer.NeedsLayout() && !renderer.SelfNeedsLayout()))
          {
            renderer.SetNeedsLayout();
          }
          return renderer.NeedsLayout();
        };

        auto isSkippedContentStale = markRendererDirtyIfNeeded(*rootForLayout);
        if (layoutOptions.Contains(LayoutOptions::TreatContentVisibilityHiddenAsVisible))
        {
          for (auto &descendant : DescendantsOfType<RenderObject>(*rootForLayout))
          {
            isSkippedContentStale |= markRendererDirtyIfNeeded(descendant);
          }
        }
        else if (layoutOptions.Contains(LayoutOptions::TreatContentVisibilityAutoAsVisible)
                 || layoutOptions.Contains(LayoutOptions::TreatRevealedWhenFoundAsVisible))
        {
          for (auto &descendant : DescendantsOfType<RenderObject>(*rootForLayout))
          {
            // FIXME: While 'c-v: auto' is used 'hidden' inside 'c-v: hidden' we could entirly skip hidden
            // subtrees here.
            auto shouldLayoutSkippedContent =
              (layoutOptions.Contains(LayoutOptions::TreatContentVisibilityAutoAsVisible)
               && descendant.Style().UsedContentVisibility() == ContentVisibility::Auto)
              || (layoutOptions.Contains(LayoutOptions::TreatRevealedWhenFoundAsVisible)
                  && descendant.Style().AutoRevealsWhenFound());

            if (shouldLayoutSkippedContent)
            {
              isSkippedContentStale |= markRendererDirtyIfNeeded(descendant);
            }
          }
        }

        if (!isSkippedContentStale)
        {
          return false;
        }

        auto overrideTypes = [&]
        {
          auto types = EnumSet<ContentVisibilityOverrideScope::OverrideType> {};

          if (layoutOptions.Contains(LayoutOptions::TreatContentVisibilityHiddenAsVisible))
          {
            types.Add(ContentVisibilityOverrideScope::OverrideType::Hidden);
          }
          if (layoutOptions.Contains(LayoutOptions::TreatContentVisibilityAutoAsVisible))
          {
            types.Add(ContentVisibilityOverrideScope::OverrideType::Auto);
          }
          if (layoutOptions.Contains(LayoutOptions::TreatRevealedWhenFoundAsVisible))
          {
            types.Add(ContentVisibilityOverrideScope::OverrideType::RevealedWhenFound);
          }
          return types;
        };
        auto contentVisibilityOverrideScope = ContentVisibilityOverrideScope {layoutContext, overrideTypes()};
        layoutContext.Layout();
        return true;
      };

      auto didRunLayout = runForcedLayoutOnSkippedContentIfNeeded();
      if (!didRunLayout && layoutContext.NeedsLayout())
      {
        layoutContext.Layout();
        didRunLayout = true;
      }

      result = didRunLayout ? UpdateLayoutResult::ChangesDone : result;

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

  void Document::AddElementWithPendingUserAgentShadowTreeUpdate(Element &element) noexcept
  {
    krys_debug_assert(&element.NodeDocument() == this);
    krys_debug_assert(!HasElementWithPendingUserAgentShadowTreeUpdate(element));
    _elementsWithPendingUserAgentShadowTreeUpdates.push_back(WeakPtr<Element>::WithRef(element));
  }

  void Document::RemoveElementWithPendingUserAgentShadowTreeUpdate(Element &element) noexcept
  {
    krys_debug_assert(&element.NodeDocument() == this);
    std::erase_if(_elementsWithPendingUserAgentShadowTreeUpdates,
                  [&](const auto &entry) { return !entry || entry.get() == &element; });
  }

  bool Document::HasElementWithPendingUserAgentShadowTreeUpdate(Element &element) const noexcept
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
      if (RefPtr<Element> element = ShareRefPtr(weakElement.get()))
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

    if (auto *styleOriginatedTimelinesController = GetStyleOriginatedTimelinesController())
    {
      styleOriginatedTimelinesController->DocumentDidResolveStyle();
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

  void Document::UpdateSVGRenderer(svg::SVGElement &element) noexcept
  {
    if (!HasLivingRenderTree())
    {
      return;
    }

    EnsurePendingRenderTreeUpdate().AddSVGRendererUpdate(element);
  }

  style::Update &Document::EnsurePendingRenderTreeUpdate() noexcept
  {
    krys_debug_assert(HasLivingRenderTree());

    if (!_pendingRenderTreeUpdate)
    {
      _pendingRenderTreeUpdate = CreateUnique<style::Update>(*this);
    }

    ScheduleRenderingUpdate({});

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

    (void)UpdateRelevancyOfContentVisibilityElements();

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
    return css::ColorFromKeyword(css::ValueId::WebkitLink, StyleColorOptions(&style));
  }

  boo::Color Document::VisitedLinkColor(const RenderStyle &style) const noexcept
  {
    if (_visitedLinkColor.IsValid())
    {
      return _visitedLinkColor;
    }
    return css::ColorFromKeyword(css::ValueId::WebkitLink,
                                 StyleColorOptions(&style) | style::ColorOptions::ForVisitedLink);
  }

  boo::Color Document::ActiveLinkColor(const RenderStyle &style) const noexcept
  {
    if (_activeLinkColor.IsValid())
    {
      return _activeLinkColor;
    }
    return css::ColorFromKeyword(css::ValueId::WebkitActivelink, StyleColorOptions(&style));
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

  EnumSet<boo::ColorScheme> Document::ResolvedColorScheme(const RenderStyle *style) const noexcept
  {
    bool isNormal = !style || style->GetColorScheme().IsNormal();
    return isNormal ? _colorScheme : style->GetColorScheme().GetColorScheme();
  }

  bool Document::UseDarkAppearance(const RenderStyle *style) const noexcept
  {
    auto colorScheme = ResolvedColorScheme(style);

    if (colorScheme.Contains(boo::ColorScheme::Dark) && !colorScheme.Contains(boo::ColorScheme::Light))
    {
      return true;
    }

    bool pageUsesDarkAppearance = false;
    if (auto *documentPage = GetPage())
    {
      pageUsesDarkAppearance = documentPage->UseDarkAppearance();
    }

    if (GetSettings().UseSystemAppearance)
    {
      return pageUsesDarkAppearance;
    }

    if (colorScheme.Contains(boo::ColorScheme::Dark))
    {
      return pageUsesDarkAppearance;
    }

    return false;
  }

  style::ColorOptions Document::StyleColorOptions(const RenderStyle *style) const noexcept
  {
    auto options = style::ColorOptions::None;
    if (GetSettings().UseSystemAppearance)
    {
      options |= style::ColorOptions::UseSystemAppearance;
    }
    if (UseDarkAppearance(style))
    {
      options |= style::ColorOptions::UseDarkAppearance;
    }
    return options;
  }

  static void ProcessColorSchemeString(DOMStringView colorScheme,
                                       const Func<void(DOMStringView key)> &callback) noexcept
  {
    size_t length = colorScheme.size();
    for (size_t i = 0; i < length;)
    {
      // Skip to first non-separator.
      while (i < length && text::IsASCIIWhitespace(colorScheme[i]))
      {
        ++i;
      }
      size_t keyBegin = i;

      // Skip to first separator.
      while (i < length && !text::IsASCIIWhitespace(colorScheme[i]))
      {
        ++i;
      }
      size_t keyEnd = i;

      if (keyBegin == keyEnd)
      {
        continue;
      }

      callback(colorScheme.substr(keyBegin, keyEnd - keyBegin));
    }
  }

  void Document::ProcessColorScheme(DOMStringView colorSchemeString) noexcept
  {
    EnumSet<boo::ColorScheme> colorScheme;
    bool allowsTransformations = true;
    bool autoEncountered = false;

    ProcessColorSchemeString(colorSchemeString,
                             [&](DOMStringView key)
                             {
                               if (text::IsASCIICaselessEqual(key, utf8_stringview(u8"auto")))
                               {
                                 colorScheme = {};
                                 allowsTransformations = true;
                                 autoEncountered = true;
                                 return;
                               }

                               if (autoEncountered)
                               {
                                 return;
                               }

                               if (text::IsASCIICaselessEqual(key, utf8_stringview(u8"light")))
                               {
                                 colorScheme.Add(boo::ColorScheme::Light);
                               }
                               else if (text::IsASCIICaselessEqual(key, utf8_stringview(u8"dark")))
                               {
                                 colorScheme.Add(boo::ColorScheme::Dark);
                               }
                               else if (text::IsASCIICaselessEqual(key, utf8_stringview(u8"only")))
                               {
                                 allowsTransformations = false;
                               }
                             });

    // If the value was just "only", that is synonymous for "only light".
    if (colorScheme.IsEmpty() && !allowsTransformations)
    {
      colorScheme.Add(boo::ColorScheme::Light);
    }

    _colorScheme = colorScheme;

    if (auto *page = GetPage())
    {
      page->UpdateStyleAfterChangeInEnvironment();
    }
  }

  void Document::MetaElementColorSchemeChanged() noexcept
  {
    // To obtain a page's supported color-schemes:
    // https://html.spec.whatwg.org/#meta-color-scheme
    auto &context = CssParserContext();

    // 1. Let candidate elements be the list of all meta elements that meet the following criteria, in tree
    //    order: the element is in a document tree; the element has a name attribute, whose value is an ASCII
    //    case-insensitive match for color-scheme; and the element has a content attribute.
    // 2. For each element in candidate elements:
    // 2a. Let parsed be the result of parsing a list of component values given the value of element's
    //     content attribute.
    // 2b. If parsed is a valid CSS 'color-scheme' property value, then return parsed.
    auto parseColorScheme = [&](const html::HTMLMetaElement &metaElement) -> Maybe<css::ColorScheme>
    {
      auto nameValue = metaElement.GetAttribute(u8"name");
      auto content = metaElement.GetAttribute(u8"content");
      if (!nameValue || !content
          || !text::IsASCIICaselessEqual(*nameValue, utf8_stringview(u8"color-scheme")))
      {
        return null;
      }
      return css::PropertyParserHelpers::ParseUnresolvedColorScheme(*content, context);
    };

    // 3. Return null.
    // (The empty string, which leaves the page's color scheme unset.)
    DOMString colorSchemeString;
    for (auto &node : DescendantRange(*this))
    {
      auto *htmlElement = DynamicDowncast<html::HTMLElement>(node);
      auto *metaElement = htmlElement ? DynamicDowncast<html::HTMLMetaElement>(*htmlElement) : nullptr;
      if (!metaElement)
      {
        continue;
      }
      if (auto colorScheme = parseColorScheme(*metaElement))
      {
        StringBuilder builder;
        css::Serialise(builder, css::DefaultSerialisationContext(), *colorScheme);
        colorSchemeString = krys::move(builder).ToString();
        break;
      }
    }
    ProcessColorScheme(colorSchemeString);
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

    // 1. Let selector be the result of parse a selector selectors.
    auto *query = SelectorQueryCache::Singleton().Add(selectorString, *this);

    // 2. If selector is failure, then throw a "SyntaxError" DOMException.
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
    return html::DOMTreeAccessors::GetBodyElement(*this);
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
