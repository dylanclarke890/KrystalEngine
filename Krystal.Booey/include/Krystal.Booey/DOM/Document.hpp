#pragma once

#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Dicts/ElementCreationOptions.hpp"
#include "Krystal.Booey/DOM/Dicts/ImportNodeOptions.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Enums/BackForwardCacheState.hpp"
#include "Krystal.Booey/DOM/Enums/DocumentFlags.hpp"
#include "Krystal.Booey/DOM/Enums/LayoutOptions.hpp"
#include "Krystal.Booey/DOM/Enums/PopoverState.hpp"
#include "Krystal.Booey/DOM/Enums/QuirksMode.hpp"
#include "Krystal.Booey/DOM/Enums/ReasonForSuspension.hpp"
#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Booey/DOM/IdleCallbackController.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/DocumentRareData.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/ScriptedAnimationController.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/NodeOrString.hpp"
#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/HTML/Enums/DocumentReadyState.hpp"
#include "Krystal.Booey/HTML/Enums/DocumentVisibilityState.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Types/HTMLOrSVGScriptElement.hpp"
#include "Krystal.Booey/Loader/LoadedFromOpaqueSource.hpp"
#include "Krystal.Booey/Page/DOMTimer.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/ListHashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class AnimationTimelinesController;
  class CachedResourceLoader;
  class DocumentTimeline;
  class FontLoadRequest;
  class DOMSelection;
  class Editor;
  class FrameSelection;
  class HitTestLocation;
  class HitTestRequest;
  class HitTestResult;
  class MouseEventWithHitTestResults;
  class PlatformMouseEvent;
  class LocalDOMWindow;
  class LocalFrame;
  class Location;
  class LocalFrameView;
  class Locale;
  class RenderElement;
  class RenderStyle;
  class RenderTreeUpdater;
  class RenderView;
  class ResizeObserver;
  struct Settings;
}

namespace krys::boo::css
{
  class CSSFontSelector;
  class MediaQueryList;
  class MediaQueryMatcher;
  class StyleSheetList;
  struct ParserContext;

  enum class PropertyId : uint16;
}

namespace krys::boo::style
{
  class Update;
  struct PseudoElementIdentifier;
}

namespace krys::boo::css::calc
{
  class RandomCachingKeyMap;
  struct RandomCachingKey;
}

namespace krys::boo::html
{
  class CustomElementAlgorithms;
  class CustomElementRegistry;
  class FocusAlgorithms;
  class FormController;
  class HTMLBaseElement;
  class HTMLDialogElement;
  class HTMLBodyElement;
  class HTMLDocumentParser;
  class HTMLElement;
  class HTMLHeadElement;
  class HTMLHtmlElement;
  class HTMLInputElement;
  class HTMLTitleElement;
  class HTMLTreeBuilder;
  class RenderBlocking;
  class SerializedScriptValue;

  enum class TaskSource : uint8;
}

namespace krys::boo::dom
{
  class Attr;
  class CDATASection;
  class Comment;
  class DocumentType;
  class Element;
  class HighlightRegistry;
  class HTMLCollection;
  class MouseEvent;
  class SelectorQuery;
  class ProcessingInstruction;
  class ShadowRoot;
  class Text;
  class TreeWalker;
  class UserActionElementSet;
  class VisitedLinkState;

  /// @see https://dom.spec.whatwg.org/#interface-document
  /// @brief The event a scroll fires at what scrolled.
  enum class ScrollEventType : bool
  {
    Scroll,
    Scrollend
  };

  class DocumentFontLoader;

  /// @brief When a document reached each point of its loading, in milliseconds since its window's time
  /// origin; 0 for those it hasn't reached.
  /// @see https://html.spec.whatwg.org/#load-timing-info
  struct DocumentLoadTimingInfo
  {
    double DOMInteractiveTime {0};
    double DOMContentLoadedEventStartTime {0};
    double DOMContentLoadedEventEndTime {0};
    double DOMCompleteTime {0};
    double LoadEventStartTime {0};
    double LoadEventEndTime {0};
  };

  class Document : public ContainerNode, public DocumentOrShadowRoot, public FontSelectorClient
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Document);
    KRYS_TYPE_CAST_TRAITS_ACCESS();

    friend class DocumentAlgorithms;
    friend class DocumentRareData;
    friend class DOMImplementation;
    friend class HTMLCollectionAlgorithms;
    friend class LiveRangeUpdater;
    friend class MutationAlgorithms;
    friend class NodeAlgorithms;

    friend class html::CustomElementAlgorithms;
    friend class html::FocusAlgorithms;
    friend class html::HTMLBaseElement;
    friend class html::HTMLDocumentParser;
    friend class html::HTMLTreeBuilder;
    friend class html::RenderBlocking;

    friend class ::krys::boo::RenderTreeUpdater;

  private:
    UniquePtr<DocumentRareData> _documentRareData;
    UniquePtr<DOMImplementation> _implementation;
    DOMString _contentType {u8"application/xml"};
    List<Range *> _liveRanges;
    List<NodeIterator *> _nodeIterators;
    bool _allowDeclarativeShadowRoots : 1 {false};
    QuirksMode _quirksMode : BitCount<QuirksMode>() {QuirksMode::NoQuirks};
    DocumentFlags _flags : BitCount<DocumentFlags>() {DocumentFlags::None};
    html::DocumentReadyState _currentDocumentReadiness
      : BitCount<html::DocumentReadyState>() {html::DocumentReadyState::Complete};
    html::DocumentVisibilityState _visibilityState
      : BitCount<html::DocumentVisibilityState>() {html::DocumentVisibilityState::Hidden};

    /// @see https://html.spec.whatwg.org/#parser-cannot-change-the-mode-flag
    bool _parserCannotChangeTheMode : 1 {false};

    List<WeakRef<Element>> _renderBlockingElements;

    /// @see https://dom.spec.whatwg.org/#concept-document-url
    url::URL _url;
    /// @see https://html.spec.whatwg.org/#document-base-url
    url::URL _baseURL;
    /// @brief The frozen base URL of the first `base` element with an `href` in the document tree, or null.
    /// @see https://html.spec.whatwg.org/#frozen-base-url
    url::URL _baseElementURL;
    /// @brief The target of the first `base` element with a `target` in the document tree.
    DOMString _baseTarget;

    mutable UniquePtr<css::ParserContext> _cachedCSSParserContext;
    Ref<Settings> _settings;

    UniquePtr<UserActionElementSet> _userActionElements;
    RefPtr<Element> _hoveredElement;
    // The element the pointer was pressed on: the chain of its ancestors stays `:active` until it's released.
    RefPtr<Element> _activeElement;
    bool _isFocusedAndActive {true};
    BackForwardCacheState _backForwardCacheState {BackForwardCacheState::NotInBackForwardCache};
    Maybe<ReasonForSuspension> _reasonForSuspendingActiveDOMObjects;
    bool _isSuspended {false};
    bool _scheduledTasksAreSuspended {false};
    bool _activeDOMObjectsAreSuspended {false};
    bool _activeDOMObjectsAreStopped {false};
    UniquePtr<FrameSelection> _selection;
    UniquePtr<boo::Editor> _editor;
    UniquePtr<html::FormController> _formController;
    RefPtr<DOMSelection> _domSelection;
    // The focused area of the document: an element, or the viewport if null.
    RefPtr<Element> _focusedElement;
    FocusTrigger _latestFocusTrigger {FocusTrigger::Other};
    // The scroll events to fire at the next scroll steps, in the order they were queued.
    List<std::pair<Ref<ContainerNode>, ScrollEventType>> _pendingScrollEventTargets;
    // Where sequential focus navigation starts from when it's inside the focused area: unset if null.
    RefPtr<Node> _sequentialFocusNavigationStartingPoint;
    // The element the URL's fragment indicates, which `:target` matches.
    WeakPtr<Element> _cssTarget;
    // The top layer, bottom to top, and the elements in it waiting to be removed.
    ListHashSet<Ref<Element>> _topLayerElements;
    ListHashSet<Ref<Element>> _topLayerPendingRemovals;
    // The open dialogs, in the order they opened, and the dialog the last press was in (light dismiss).
    ListHashSet<Ref<html::HTMLDialogElement>> _openDialogs;
    WeakPtr<html::HTMLDialogElement> _dialogPointerDownTarget;
    // The popover the last press was in (light dismiss), and the showing auto popover the first showing hint
    // popover opened from.
    WeakPtr<html::HTMLElement> _popoverPointerDownTarget;
    WeakPtr<html::HTMLElement> _hintStackParent;
    // How many popovers are being hidden, and whether one is being shown: no popover shows meanwhile.
    uint32 _hidingPopoverNestingCount {0};
    bool _isShowingPopover {false};
    // The resize observers observing the document's elements, and the document's own, which keeps the last
    // remembered sizes.
    List<WeakPtr<ResizeObserver>> _resizeObservers;
    RefPtr<ResizeObserver> _resizeObserverForContainIntrinsicSize;
    // Whether the fragment is scrolled to once the pending style sheets have loaded.
    bool _gotoAnchorNeededAfterStylesheetsLoad {false};
    // The custom highlights (`CSS.highlights`), and the text fragments' highlights.
    RefPtr<HighlightRegistry> _highlightRegistry;
    RefPtr<HighlightRegistry> _fragmentHighlightRegistry;
    // The text directives taken from the URL's fragment (after `:~:`).
    utf8_string _fragmentDirective;
    SequentialFocusNavigationStartingPointPosition _sequentialFocusNavigationStartingPointPosition {
      SequentialFocusNavigationStartingPointPosition::At};

    List<WeakPtr<ShadowRoot>> _inDocumentShadowRoots;
    RefPtr<css::StyleSheetList> _styleSheetList;
    RefPtr<css::MediaQueryMatcher> _mediaQueryMatcher;
    mutable RefPtr<FontSelector> _fontSelector;
    mutable RefPtr<CachedResourceLoader> _cachedResourceLoader;
    mutable UniquePtr<DocumentFontLoader> _fontLoader;
    UniquePtr<AnimationTimelinesController> _timelinesController;
    RefPtr<DocumentTimeline> _timeline;

    WeakPtr<LocalFrame> _frame;
    // The document's window, made as the document becomes a frame's.
    RefPtr<LocalDOMWindow> _domWindow;
    bool _needsDOMWindowResizeEvent {false};
    bool _needsVisualViewportResizeEvent {false};
    bool _needsVisualViewportScrollEvent {false};
    /// @brief Whether the document's parser is running (a document a frame loads).
    bool _parsing {false};
    /// @see https://html.spec.whatwg.org/#page-showing
    bool _pageShowing {false};
    bool _loadEventFinished {false};
    /// @brief Things other than loads that delay the document's load event.
    uint32 _loadEventDelayCount {0};
    /// @see https://html.spec.whatwg.org/#unload-counter
    uint32 _unloadCounter {0};
    /// @see https://html.spec.whatwg.org/#will-declaratively-refresh
    bool _willDeclarativelyRefresh {false};
    /// @see https://html.spec.whatwg.org/#pragma-set-default-language
    DOMString _contentLanguage;
    /// @brief The document's timers (`setTimeout()`, `setInterval()`), by ID.
    /// @see https://html.spec.whatwg.org/#map-of-settimeout-and-setinterval-ids
    HashMap<int32, Ref<DOMTimer>> _timeouts;
    int32 _circularSequentialID {0};
    /// @see https://html.spec.whatwg.org/#timer-nesting-level
    int32 _timerNestingLevel {0};
    UniquePtr<ScriptedAnimationController> _scriptedAnimationController;
    UniquePtr<IdleCallbackController> _idleCallbackController;
    DocumentLoadTimingInfo _loadTiming;
    // The root of the render tree: a document has one while it's shown in a frame with a view.
    RenderPtr<boo::RenderView> _renderView;
    bool _renderTreeBeingDestroyed {false};
    // Renderer updates (of text) that the next style resolution applies.
    UniquePtr<style::Update> _pendingRenderTreeUpdate;

    mutable UniquePtr<VisitedLinkState> _visitedLinkState;
    mutable UniquePtr<RenderStyle> _initialContainingBlockStyle;
    // The style the render view would have, for a document without one; `RenderTreeUpdater` keeps it here.
    UniquePtr<RenderStyle> _renderViewStyle;
    mutable RefPtr<css::calc::RandomCachingKeyMap> _randomCachingKeyMap;
    bool _hasStyleWithViewportUnits {false};

    Timer _styleRecalcTimer;
    List<WeakPtr<Element>> _autofocusCandidates;
    mutable HashMap<utf8_string, UniquePtr<Locale>> _localeCache;
    // The inputs inserted without their user agent shadow tree, which the next style resolution creates.
    List<WeakPtr<html::HTMLInputElement>> _elementsWithPendingUserAgentShadowTreeUpdates;
    bool _autofocusProcessed {false};
    // FIXME: Documents create their render trees on their first style resolution, which is a rebuild.
    bool _needsFullStyleRebuild {true};
    bool _inStyleRecalc {false};
    bool _inRenderTreeUpdate {false};
    bool _isInStyleInterleavedLayout {false};
    bool _isResolvingTreeStyle {false};
    bool _ignorePendingStylesheets {false};
    bool _hasNodesWithMissingStyle {false};
    Maybe<FixedVector<css::PropertyId>> _exposedComputedCSSPropertyIds;

    boo::Color _textColor {boo::Color::black};
    boo::Color _linkColor;
    boo::Color _visitedLinkColor;
    boo::Color _activeLinkColor;
    /// @brief The nodes that refer to this document (all of them but the document itself): it lives while any
    /// do, even with no references left.
    uint32 _referencingNodeCount {0};

  protected:
    Document(DocumentFlags flags) noexcept;

  public:
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept final
    {
      return true;
    }

    ~Document() noexcept override;

    void IncrementReferencingNodeCount() noexcept
    {
      ++_referencingNodeCount;
    }
    /// @brief A node stopped referring to the document: the last one, once the document has no references,
    /// deletes it.
    void DecrementReferencingNodeCount() noexcept;

#pragma region Document (DOM) - https://dom.spec.whatwg.org/#interface-document

    /// @see https://dom.spec.whatwg.org/#dom-document-document
    Document() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-implementation
    KRYS_NODISCARD DOMImplementation &Implementation() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-url
    KRYS_NODISCARD DOMString URL() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-documenturi
    KRYS_NODISCARD DOMString DocumentURI() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-compatmode
    KRYS_NODISCARD DOMString CompatMode() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-characterset
    KRYS_NODISCARD DOMString CharacterSet() const noexcept;

    /// @note legacy alias of .characterSet
    /// @see https://dom.spec.whatwg.org/#dom-document-charset
    KRYS_NODISCARD DOMString Charset() const noexcept;

    /// @note legacy alias of .characterSet
    /// @see https://dom.spec.whatwg.org/#dom-document-inputencoding
    KRYS_NODISCARD DOMString InputEncoding() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-contenttype
    KRYS_NODISCARD DOMString ContentType() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-doctype
    KRYS_NODISCARD RefPtr<DocumentType> DocType() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-doctype
    KRYS_NODISCARD RefPtr<const DocumentType> DocType() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-documentelement
    KRYS_NODISCARD RefPtr<Element> DocumentElement() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-documentelement
    KRYS_NODISCARD RefPtr<const Element> DocumentElement() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-getelementsbytagname
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagName(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-getelementsbytagnamens
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagNameNS(DOMStringAtom namespaceUri,
                                                              DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-getelementsbyclassname
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByClassName(DOMStringAtom classNames) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createelement
    KRYS_NODISCARD ExceptionOr<Ref<Element>>
      CreateElement(DOMStringAtom localName, const ElementCreationOptionsOrString &options = {}) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createelementns
    KRYS_NODISCARD ExceptionOr<Ref<Element>>
      CreateElementNS(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName,
                      const ElementCreationOptionsOrString &options = {}) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createdocumentfragment
    KRYS_NODISCARD Ref<DocumentFragment> CreateDocumentFragment() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createtextnode
    KRYS_NODISCARD Ref<Text> CreateTextNode(DOMString &&data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createcdatasection
    KRYS_NODISCARD ExceptionOr<Ref<CDATASection>> CreateCDATASection(DOMString &&data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createcomment
    KRYS_NODISCARD Ref<Comment> CreateComment(DOMString &&data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createprocessinginstruction
    KRYS_NODISCARD ExceptionOr<Ref<ProcessingInstruction>>
      CreateProcessingInstruction(DOMString &&target, DOMString &&data) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-importnode
    KRYS_NODISCARD ExceptionOr<Ref<Node>> ImportNode(Node &node,
                                                     const BoolOrImportNodeOptions &options = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-adoptnode
    KRYS_NODISCARD ExceptionOr<Ref<Node>> AdoptNode(Node &node) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createattribute
    KRYS_NODISCARD ExceptionOr<Ref<Attr>> CreateAttribute(DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createattributens
    KRYS_NODISCARD ExceptionOr<Ref<Attr>> CreateAttributeNS(DOMStringAtom namespaceUri,
                                                            DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createevent
    KRYS_NODISCARD Ref<Event> CreateEvent(DOMStringAtom interface) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createrange
    KRYS_NODISCARD Ref<Range> CreateRange() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createnodeiterator
    KRYS_NODISCARD Ref<NodeIterator> CreateNodeIterator(Node &root,
                                                        WhatToShow whatToShow = WhatToShow::SHOW_ALL,
                                                        RefPtr<NodeFilter> &&filter = nullptr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-document-createtreewalker
    KRYS_NODISCARD Ref<TreeWalker> CreateTreeWalker(Node &root, WhatToShow whatToShow = WhatToShow::SHOW_ALL,
                                                    RefPtr<NodeFilter> &&filter = nullptr) noexcept;

#pragma endregion

#pragma region Document (HTML) - https://html.spec.whatwg.org/multipage/dom.html#the-document-object

    /// @see https://html.spec.whatwg.org/#dom-parsehtmlunsafe
    KRYS_NODISCARD static ExceptionOr<Ref<Document>> ParseHTMLUnsafe(DOMStringView html) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-parsehtml
    KRYS_NODISCARD static ExceptionOr<Ref<Document>> ParseHTML(DOMStringView &html) noexcept;

    // TODO(DOCUMENT, HTML): check - do we need to implement these?
    // resource metadata management - https://html.spec.whatwg.org/#resource-metadata-management

    /// @see https://html.spec.whatwg.org/#dom-document-location
    // [ PutForwards = href, LegacyUnforgeable ] readonly attribute Location? location;

    /// @see https://html.spec.whatwg.org/#dom-document-domain
    // attribute USVString domain;

    /// @see https://html.spec.whatwg.org/#dom-document-referrer
    // readonly attribute USVString referrer;

    /// @see https://html.spec.whatwg.org/#dom-document-cookie
    // attribute USVString cookie;

    /// @see https://html.spec.whatwg.org/#dom-document-lastmodified
    // readonly attribute DOMString lastModified;

    /// @see https://html.spec.whatwg.org/#dom-document-readystate
    KRYS_NODISCARD html::DocumentReadyState ReadyState() const noexcept
    {
      return _currentDocumentReadiness;
    }

    // DOM tree accessors - https://html.spec.whatwg.org/#dom-tree-accessors

    // TODO(DOCUMENT, HTML): implement this getter:
    // getter object (DOMString name);

    /// @see https://html.spec.whatwg.org/#document.title
    KRYS_NODISCARD DOMString Title() const noexcept;

    /// @see https://html.spec.whatwg.org/#document.title
    ExceptionOr<void> Title(DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-dir
    KRYS_NODISCARD DOMString Dir() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-dir
    void Dir(DOMString &&value) noexcept;

    // NOTE: the HTML spec says that body returns a HTMLElement but that's because it can also return a
    // HTMLFrameSetElement which is a legacy element that we don't support. In our case, body will always
    // return an HTMLBodyElement if it exists.

    /// @see https://html.spec.whatwg.org/#dom-document-body
    KRYS_NODISCARD RefPtr<html::HTMLBodyElement> Body() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-body
    KRYS_NODISCARD RefPtr<const html::HTMLBodyElement> Body() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-body
    ExceptionOr<void> Body(html::HTMLBodyElement &body) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-head
    KRYS_NODISCARD RefPtr<html::HTMLHeadElement> Head() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-head
    KRYS_NODISCARD RefPtr<const html::HTMLHeadElement> Head() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-images
    KRYS_NODISCARD Ref<HTMLCollection> Images() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-embeds
    KRYS_NODISCARD Ref<HTMLCollection> Embeds() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-plugins
    KRYS_NODISCARD Ref<HTMLCollection> Plugins() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-links
    KRYS_NODISCARD Ref<HTMLCollection> Links() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-forms
    KRYS_NODISCARD Ref<HTMLCollection> Forms() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-scripts
    KRYS_NODISCARD Ref<HTMLCollection> Scripts() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-getelementsbyname
    KRYS_NODISCARD Ref<NodeList> GetElementsByName(DOMStringView elementName) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-document-currentscript
    KRYS_NODISCARD html::HTMLOrSVGScriptElement CurrentScript() noexcept;

    // dynamic markup insertion
    // [CEReactions] Document open(optional DOMString unused1, optional DOMString unused2);
    // WindowProxy? open(USVString url, DOMString name, DOMString features);
    // [CEReactions] undefined close();
    // [CEReactions] undefined write((TrustedHTML or DOMString)... text);
    // [CEReactions] undefined writeln((TrustedHTML or DOMString)... text);

    // user interaction
    // readonly attribute WindowProxy? defaultView;
    // boolean hasFocus();
    // [CEReactions] attribute DOMString designMode;
    // [CEReactions] boolean execCommand(DOMString commandId, optional boolean showUI = false, optional
    // DOMString value = "");
    // boolean queryCommandEnabled(DOMString commandId);
    // boolean queryCommandIndeterm(DOMString commandId);
    // boolean queryCommandState(DOMString commandId);
    // boolean queryCommandSupported(DOMString commandId);
    // DOMString queryCommandValue(DOMString commandId);

    KRYS_NODISCARD bool Hidden() const noexcept
    {
      return _visibilityState == html::DocumentVisibilityState::Hidden;
    }

    KRYS_NODISCARD html::DocumentVisibilityState VisibilityState() const noexcept
    {
      return _visibilityState;
    }

    // special event handler IDL attributes that only apply to Document objects
    // [LegacyLenientThis] attribute EventHandler onreadystatechange;
    // attribute EventHandler onvisibilitychange;

#pragma endregion

#pragma region Node - https://dom.spec.whatwg.org/#interface-node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#document";
    }

#pragma endregion

#pragma region URLs - https://html.spec.whatwg.org/#document-base-url

    /// @brief The document's URL; `about:blank` until the host sets one.
    /// @see https://dom.spec.whatwg.org/#concept-document-url
    KRYS_NODISCARD const url::URL &GetURL() const noexcept
    {
      return _url;
    }

    /// @brief Sets the document's URL. An empty URL becomes `about:blank`.
    void SetURL(url::URL &&url) noexcept;

    /// @brief The target of the first `base` element with a `target` attribute, or the empty string.
    /// @see https://html.spec.whatwg.org/#get-an-element's-target
    KRYS_NODISCARD const DOMString &BaseTarget() const noexcept
    {
      return _baseTarget;
    }

    /// @see https://html.spec.whatwg.org/#fallback-base-url
    KRYS_NODISCARD const url::URL &FallbackBaseURL() const noexcept;

    /// @see https://html.spec.whatwg.org/#document-base-url
    KRYS_NODISCARD const url::URL &BaseURL() const noexcept
    {
      return _baseURL;
    }

    /// @brief Parses @p url relative to the document base URL.
    /// @see https://html.spec.whatwg.org/#encoding-parsing-a-url
    KRYS_NODISCARD url::URL CompleteURL(utf8_stringview url) const noexcept;

    /// @brief Parses @p url relative to @p baseURL.
    /// @see https://html.spec.whatwg.org/#encoding-parsing-a-url
    KRYS_NODISCARD url::URL CompleteURL(utf8_stringview url, const url::URL &baseURL) const noexcept;

#pragma endregion

#pragma region CSS

    /// @brief The context the document's style sheets are parsed with: its base URL, mode and settings.
    /// Cached until one of them changes.
    KRYS_NODISCARD const css::ParserContext &CssParserContext() const noexcept;

    /// @brief The document's settings: the defaults (`Settings::Default()`) unless the host gave it its own,
    /// or those of the document that created it.
    KRYS_NODISCARD Settings &GetSettings() const noexcept
    {
      return *_settings;
    }

    /// @brief Gives the document @p settings, and resolves its style again with them. Style sheets that were
    /// already parsed aren't parsed again.
    void SetSettings(Ref<Settings> &&settings) noexcept;

    /// @brief The locale form controls in @p locale (a language tag, from a `lang` attribute) use: the user
    /// interface's, unless @p locale is given and `Settings::LangAttributeAwareFormControlUIEnabled` is on.
    KRYS_NODISCARD boo::Locale &GetCachedLocale(utf8_stringview locale) const noexcept;

    void InvalidateCachedCSSParserContext() noexcept;

    /// @brief The connected shadow roots among the document's shadow-including descendants.
    KRYS_NODISCARD const List<WeakPtr<ShadowRoot>> &InDocumentShadowRoots() const noexcept
    {
      return _inDocumentShadowRoots;
    }

    void DidInsertInDocumentShadowRoot(ShadowRoot &shadowRoot) noexcept;
    void DidRemoveInDocumentShadowRoot(ShadowRoot &shadowRoot) noexcept;

#pragma endregion

#pragma region User Action State

    KRYS_NODISCARD UserActionElementSet &UserActionElements() const noexcept
    {
      return *_userActionElements;
    }

    /// @brief The document's selection (upstream, its frame's).
    KRYS_NODISCARD FrameSelection &Selection() const noexcept
    {
      return *_selection;
    }

    KRYS_NODISCARD boo::Editor &GetEditor() const noexcept
    {
      return *_editor;
    }

    /// @brief The script's view of the selection; null unless the document is fully active (shown in a
    /// frame).
    /// @see https://w3c.github.io/selection-api/#dom-document-getselection
    KRYS_NODISCARD DOMSelection *GetSelection() noexcept;

    /// @brief The innermost element under the pointer that has a renderer, if any.
    KRYS_NODISCARD Element *HoveredElement() const noexcept
    {
      return _hoveredElement.get();
    }

    enum class CaptureChange : bool
    {
      No,
      Yes
    };

    /// @brief Updates the hover and active state for a pointer over @p innerElement (or outside the document,
    /// if it's null), as hit testing finds it: hover follows the pointer, and a press makes the element and
    /// its ancestors `:active` until the release.
    void UpdateHoverActiveState(const HitTestRequest &request, Element *innerElement,
                                CaptureChange captureElementChanged = CaptureChange::No) noexcept;

    /// @brief Hit tests @p documentPoint for @p event and, unless the request is read-only, updates the hover
    /// and active state to match.
    KRYS_NODISCARD MouseEventWithHitTestResults PrepareMouseEvent(const HitTestRequest &request,
                                                                  const DoublePoint &documentPoint,
                                                                  const PlatformMouseEvent &event) noexcept;

    void HoveredElementDidDetach(Element &element) noexcept;
    /// @brief Moves the hovered element and the active chain out of @p node's subtree before it's removed.
    void NodeWillBeRemoved(Node &node) noexcept;
    void ElementInActiveChainDidDetach(Element &element) noexcept;

    /// @brief Whether the document's window has system focus: `:focus` and its relatives only match then.
    /// Hosts set it as their window gains and loses focus; it's true by default.
    KRYS_NODISCARD bool IsFocusedAndActive() const noexcept
    {
      return _isFocusedAndActive;
    }

    void SetFocusedAndActive(bool flag) noexcept;

#pragma endregion

#pragma region Scrolling

    /// @brief Queues a `scroll` or `scrollend` event at @p originalTarget (an element, or the document for
    /// the viewport) for the next scroll steps, once per target and type. A text control's inner text
    /// element's events go to the text control.
    void AddPendingScrollEventTarget(ContainerNode &originalTarget, ScrollEventType eventType) noexcept;

    /// @brief Fires the pending scroll events, in the order they were queued: at elements without bubbling,
    /// and at the document bubbling.
    /// @see https://drafts.csswg.org/cssom-view/#run-the-scroll-steps
    void RunScrollSteps() noexcept;

    /// @brief The rendering update's resize observations step: records the last successful position options,
    /// then gathers and delivers the resize observations, deeper each round, laying out the page between
    /// them. Observations that were skipped (no deeper than the last round's) are reported as an error at the
    /// window, and left for the next update.
    /// @see https://html.spec.whatwg.org/#update-the-rendering
    void UpdateResizeObservations(Page &page) noexcept;

    void AddResizeObserver(ResizeObserver &observer) noexcept;
    void RemoveResizeObserver(ResizeObserver &observer) noexcept;
    KRYS_NODISCARD bool HasResizeObservers() const noexcept;
    /// @brief Gathers the observations of the document's resize observers deeper than @p deeperThan.
    /// @return The depth of the shallowest one, or `ResizeObserver::MaxElementDepth()` if there's none.
    KRYS_NODISCARD size_t GatherResizeObservations(size_t deeperThan) noexcept;
    void DeliverResizeObservations() noexcept;
    KRYS_NODISCARD bool HasSkippedResizeObservations() const noexcept;
    void SetHasSkippedResizeObservations(bool skipped) noexcept;

    /// @brief Gathers the observations of the elements whose last remembered sizes are kept.
    KRYS_NODISCARD size_t GatherResizeObservationsForContainIntrinsicSize() noexcept;
    /// @brief Keeps @p element's last remembered size (`contain-intrinsic-size: auto`) up to date.
    void ObserveForContainIntrinsicSize(Element &element) noexcept;
    void UnobserveForContainIntrinsicSize(Element &element) noexcept;
    /// @brief Has @p element's size reported again, so its last remembered size is updated (or cleared, if it
    /// was disconnected).
    void ResetObservationSizeForContainIntrinsicSize(Element &element) noexcept;

    /// @brief The view's size or zoom changed: the next resize steps fire `resize` at the window.
    void SetNeedsDOMWindowResizeEvent() noexcept;

    /// @brief The visual viewport's size changed: the next resize steps fire `resize` at it.
    void SetNeedsVisualViewportResize() noexcept;

    /// @brief The visual viewport moved within the layout viewport: the next scroll steps fire `scroll` at
    /// it.
    void SetNeedsVisualViewportScrollEvent() noexcept;

    /// @brief Fires `resize` at the window if the view's size or zoom changed since the last time.
    /// @see https://drafts.csswg.org/cssom-view/#run-the-resize-steps
    void RunResizeSteps() noexcept;

#pragma endregion

#pragma region Focus - https://html.spec.whatwg.org/#focus

    /// @brief The focused area of the document if it's an element; null if it's the viewport.
    /// @see https://html.spec.whatwg.org/#focused-area-of-the-document
    KRYS_NODISCARD Element *FocusedElement() const noexcept
    {
      return _focusedElement.get();
    }

    /// @see https://html.spec.whatwg.org/#dom-document-hasfocus
    KRYS_NODISCARD bool HasFocus() const noexcept;

    /// @brief Sets the sequential focus navigation starting point to @p node (as when the user clicks it), or
    /// unsets it if @p node is null, the document or its document element (navigation then starts from the
    /// start of the document).
    /// @see https://html.spec.whatwg.org/#sequential-focus-navigation-starting-point
    void SetSequentialFocusNavigationStartingPoint(Node *node) noexcept;

    KRYS_NODISCARD Node *SequentialFocusNavigationStartingPoint() const noexcept
    {
      return _sequentialFocusNavigationStartingPoint.get();
    }

    /// @brief The document's target element, which `:target` matches; null if there's none.
    /// @see https://html.spec.whatwg.org/#target-element
    KRYS_NODISCARD Element *CSSTarget() const noexcept
    {
      return _cssTarget.get();
    }

    /// @brief Makes @p newTarget (or nothing, if it's null) the document's target element, invalidating the
    /// style of the old and new targets.
    void SetCSSTarget(Element *newTarget) noexcept;

    /// @brief The element the fragment @p name indicates: the element with the ID @p name, or else the first
    /// `<a>` whose `name` is @p name (ASCII case-insensitively in quirks mode); null for an empty name.
    /// @see https://html.spec.whatwg.org/#find-a-potential-indicated-element
    KRYS_NODISCARD Element *FindAnchor(DOMStringView name) const noexcept;

    /// @brief Adds @p element, which mustn't be in it, to the top of the document's top layer. Use
    /// `Element::AddToTopLayer`, which calls this.
    /// @see https://drafts.csswg.org/css-position-4/#add-an-element-to-the-top-layer
    void AddTopLayerElement(Element &element) noexcept;

    /// @brief Removes @p element, which must be in it, from the document's top layer. Use
    /// `Element::RemoveFromTopLayer`, which calls this.
    /// @see https://drafts.csswg.org/css-position-4/#remove-an-element-from-the-top-layer-immediately
    void RemoveTopLayerElement(Element &element) noexcept;

    /// @brief The document's top layer, bottom to top.
    /// @see https://drafts.csswg.org/css-position-4/#document-top-layer
    KRYS_NODISCARD const ListHashSet<Ref<Element>> &TopLayerElements() const noexcept
    {
      return _topLayerElements;
    }

    /// @brief Whether the document's top layer has any elements.
    KRYS_NODISCARD bool HasTopLayerElement() const noexcept
    {
      return !_topLayerElements.empty();
    }

    /// @brief Whether @p element is in the top layer's pending removals: it leaves the top layer once its
    /// `overlay` is `none` (after any transition of it) or it isn't rendered.
    /// @see https://drafts.csswg.org/css-position-4/#document-pending-top-layer-removals
    KRYS_NODISCARD bool IsPendingTopLayerRemoval(const Element &element) const noexcept;

    /// @brief Adds @p element, which must be in the top layer, to its pending removals. Use
    /// `Element::RequestRemoveFromTopLayer`, which calls this.
    void AddPendingTopLayerRemoval(Element &element) noexcept;

    /// @brief Removes the elements pending removal whose `overlay` is `none` or that aren't rendered from the
    /// top layer, in the rendering update.
    /// @see https://drafts.csswg.org/css-position-4/#process-top-layer-removals
    void ProcessTopLayerRemovals() noexcept;

    /// @brief The topmost modal dialog in the top layer, which blocks the document: everything else is inert.
    /// @see https://html.spec.whatwg.org/#blocked-by-a-modal-dialog
    KRYS_NODISCARD html::HTMLDialogElement *ActiveModalDialog() const noexcept;

    /// @brief The open dialogs, in the order they opened.
    /// @see https://html.spec.whatwg.org/#open-dialogs-list
    KRYS_NODISCARD const ListHashSet<Ref<html::HTMLDialogElement>> &OpenDialogs() const noexcept
    {
      return _openDialogs;
    }
    void AddOpenDialog(html::HTMLDialogElement &dialog) noexcept;
    void RemoveOpenDialog(html::HTMLDialogElement &dialog) noexcept;

    /// @brief Closes what a press and release outside it light dismisses: the auto and hint popovers it
    /// wasn't in or on an invoker of, and the topmost open dialog if its `closedby` is `any`. Run as @p event
    /// (a trusted `mousedown` or `mouseup`, standing in for `pointerdown` and `pointerup`) is about to be
    /// dispatched at @p target.
    /// @see https://html.spec.whatwg.org/#light-dismiss-activities
    void HandleLightDismiss(const MouseEvent &event, Node &target) noexcept;

    /// @brief The showing popovers opened as auto popovers, in top layer order.
    /// @see https://html.spec.whatwg.org/#showing-auto-popover-list
    KRYS_NODISCARD List<Ref<html::HTMLElement>> ShowingAutoPopoverList() const noexcept;
    /// @brief The showing popovers opened as hint popovers, in top layer order.
    /// @see https://html.spec.whatwg.org/#showing-hint-popover-list
    KRYS_NODISCARD List<Ref<html::HTMLElement>> ShowingHintPopoverList() const noexcept;
    /// @brief The last showing hint popover, else the last showing auto popover, if any.
    /// @see https://html.spec.whatwg.org/#topmost-auto-popover
    KRYS_NODISCARD html::HTMLElement *TopmostAutoOrHintPopover() const noexcept;

    /// @brief Hides the showing hint popovers above @p endpoint (all of them if it isn't one), then the
    /// showing auto popovers above it (or above the hint stack parent if it's a hint popover).
    /// @see https://html.spec.whatwg.org/#hide-all-popovers-until
    void HideAllPopoversUntil(html::HTMLElement *endpoint, FocusPreviousElement focusPreviousElement,
                              FireEvents fireEvents) noexcept;
    /// @brief Hides the showing popovers of @p stackType (`Auto` or `Hint`) above @p endpoint, newest first,
    /// or all of them if it isn't one.
    /// @see https://html.spec.whatwg.org/#hide-popover-stack-until
    void HidePopoverStackUntil(html::HTMLElement *endpoint, PopoverState stackType,
                               FocusPreviousElement focusPreviousElement, FireEvents fireEvents) noexcept;

    /// @brief The popover a press on @p node is in, or on an invoker of, if it's a showing auto or hint
    /// popover; the higher in the stacks of the two.
    /// @see https://html.spec.whatwg.org/#topmost-clicked-popover
    KRYS_NODISCARD RefPtr<html::HTMLElement> TopmostClickedPopover(Node &node) const noexcept;

    /// @brief The showing auto popover the first showing hint popover opened from, if any.
    /// @see https://html.spec.whatwg.org/#hint-stack-parent
    KRYS_NODISCARD html::HTMLElement *HintStackParent() const noexcept
    {
      return _hintStackParent.get();
    }
    void SetHintStackParent(html::HTMLElement *element) noexcept;

    /// @see https://html.spec.whatwg.org/#hiding-popover-nesting-count
    KRYS_NODISCARD uint32 HidingPopoverNestingCount() const noexcept
    {
      return _hidingPopoverNestingCount;
    }
    void IncrementHidingPopoverNestingCount() noexcept
    {
      ++_hidingPopoverNestingCount;
    }
    void DecrementHidingPopoverNestingCount() noexcept
    {
      krys_debug_assert(_hidingPopoverNestingCount);
      --_hidingPopoverNestingCount;
    }

    /// @see https://html.spec.whatwg.org/#showing-popover
    KRYS_NODISCARD bool IsShowingPopover() const noexcept
    {
      return _isShowingPopover;
    }
    void SetIsShowingPopover(bool value) noexcept
    {
      _isShowingPopover = value;
    }

    /// @brief Whether the style sheets that block rendering have loaded (or are being ignored).
    KRYS_NODISCARD bool HaveStylesheetsLoaded() const noexcept;

    /// @brief Whether the fragment is scrolled to once the pending style sheets have loaded.
    KRYS_NODISCARD bool GotoAnchorNeededAfterStylesheetsLoad() const noexcept
    {
      return _gotoAnchorNeededAfterStylesheetsLoad;
    }
    void SetGotoAnchorNeededAfterStylesheetsLoad(bool value) noexcept
    {
      _gotoAnchorNeededAfterStylesheetsLoad = value;
    }

    /// @brief Run when the last pending style sheet has loaded: scrolls to the fragment if that waited for
    /// it.
    void DidRemoveAllPendingStylesheet() noexcept;

    /// @brief Queues a task to fire `hashchange` from @p oldURL to @p newURL.
    void EnqueueHashchangeEvent(dom::USVString &&oldURL, dom::USVString &&newURL) noexcept;

    /// @brief Whether the document is its frame's (its navigable's active document).
    /// @see https://html.spec.whatwg.org/#fully-active
    KRYS_NODISCARD bool IsFullyActive() const noexcept;

    /// @brief The frame went to a history entry of the document with @p stateObject: fires `popstate`.
    void StatePopped(RefPtr<html::SerializedScriptValue> &&stateObject) noexcept;

    /// @brief Gives the document @p url, which `pushState()` or `replaceState()` set.
    void UpdateURLForPushOrReplaceState(const url::URL &url) noexcept;

    /// @brief The document's custom highlights, which `::highlight()` styles.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-css-highlights
    KRYS_NODISCARD HighlightRegistry &GetHighlightRegistry() noexcept;
    KRYS_NODISCARD HighlightRegistry *HighlightRegistryIfExists() const noexcept;

    /// @brief The highlights of the text the URL's text directives indicate, which `::target-text` styles.
    KRYS_NODISCARD HighlightRegistry &FragmentHighlightRegistry() noexcept;
    KRYS_NODISCARD HighlightRegistry *FragmentHighlightRegistryIfExists() const noexcept;

    /// @brief Whether any highlight is registered, so text may need painting with highlights.
    KRYS_NODISCARD bool HasHighlight() const noexcept;

    /// @brief Works out where each highlight range's painting starts and ends, repainting what moved; run at
    /// each rendering update.
    void UpdateHighlightPositions() noexcept;

    /// @brief The fragment directive of the document's URL (what followed `:~:`), which `SetURL` takes from
    /// the URL.
    /// @see https://wicg.github.io/scroll-to-text-fragment/#fragment-directive
    KRYS_NODISCARD const utf8_string &FragmentDirective() const noexcept
    {
      return _fragmentDirective;
    }

    /// @brief Run when the parser stops parsing the document: its frame's loader scrolls to the fragment.
    // FIXME: The rest of the end (https://html.spec.whatwg.org/#the-end): readiness, DOMContentLoaded and the
    // load event, with document loading (B3).
    void FinishedParsing() noexcept;

    /// @brief Whether the latest focus change the user made was by clicking.
    KRYS_NODISCARD bool WasLastFocusByClick() const noexcept
    {
      return _latestFocusTrigger == FocusTrigger::Click;
    }

    /// @brief The style of the initial containing block, which the document element inherits from.
    /// @note This may end up differing from the used style of the initial containing block due to
    /// adjustments.
    // FIXME: Created on first use if the document hasn't resolved its style (style resolution keeps it up to
    // date), as resolvers are used outside style resolution.
    KRYS_NODISCARD const RenderStyle *InitialContainingBlockStyle() const noexcept;

    /// @brief The used style of the initial containing block: its computed style, with the writing mode and
    /// direction propagated from the document element or `<body>`.
    // FIXME: The style of the render view, once there is one.
    KRYS_NODISCARD const RenderStyle &RenderViewStyle() const noexcept;

    enum class ResolveStyleType : bool
    {
      Normal,
      Rebuild
    };

    /// @brief Resolves the styles of the elements that need resolving and applies them.
    void ResolveStyle(ResolveStyleType type = ResolveStyleType::Normal) noexcept;

    /// @brief Resolves style if anything needs resolving.
    /// @return Whether style was resolved.
    bool UpdateStyleIfNeeded() noexcept;
    bool UpdateStyleIfNeededIgnoringPendingStylesheets() noexcept;

    /// @brief Whether an element's style, or the style sheets, need updating.
    KRYS_NODISCARD bool NeedsStyleRecalc() const noexcept;
    KRYS_NODISCARD bool HasPendingStyleRecalc() const noexcept;
    KRYS_NODISCARD bool HasPendingFullStyleRebuild() const noexcept;

    void ScheduleFullStyleRebuild() noexcept;
    void ScheduleStyleRecalc() noexcept;
    void UnscheduleStyleRecalc() noexcept;

    KRYS_NODISCARD bool InStyleRecalc() const noexcept
    {
      return _inStyleRecalc;
    }

    void AddElementWithPendingUserAgentShadowTreeUpdate(html::HTMLInputElement &element) noexcept;
    void RemoveElementWithPendingUserAgentShadowTreeUpdate(html::HTMLInputElement &element) noexcept;
    KRYS_NODISCARD bool
      HasElementWithPendingUserAgentShadowTreeUpdate(html::HTMLInputElement &element) const noexcept;

    /// @brief Whether the document is laid out in the middle of a style update, for container queries.
    KRYS_NODISCARD bool IsInStyleInterleavedLayout() const noexcept
    {
      return _isInStyleInterleavedLayout;
    }

    KRYS_NODISCARD bool InRenderTreeUpdate() const noexcept
    {
      return _inRenderTreeUpdate;
    }

    KRYS_NODISCARD bool IsResolvingTreeStyle() const noexcept
    {
      return _isResolvingTreeStyle;
    }

    void SetIsResolvingTreeStyle(bool value) noexcept
    {
      _isResolvingTreeStyle = value;
    }

    KRYS_NODISCARD bool IsIgnoringPendingStylesheets() const noexcept
    {
      return _ignorePendingStylesheets;
    }

    void SetHasNodesWithMissingStyle() noexcept
    {
      _hasNodesWithMissingStyle = true;
    }

    KRYS_NODISCARD bool HasNodesWithMissingStyle() const noexcept
    {
      return _hasNodesWithMissingStyle;
    }

    /// @brief The style of @p element (or of one of its pseudo-elements) inheriting from @p parentStyle,
    /// resolved outside style resolution.
    KRYS_NODISCARD UniquePtr<RenderStyle> StyleForElementIgnoringPendingStylesheets(
      Element &element, const RenderStyle *parentStyle,
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier = null) noexcept;

#pragma region Frame and layout

    /// @brief The frame the document is shown in, if any.
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;
    /// @brief The page of the document's frame, if any.
    KRYS_NODISCARD boo::Page *GetPage() const noexcept;
    /// @brief The document's window (its relevant global object), if it has been in a frame; it keeps it
    /// after leaving the frame.
    KRYS_NODISCARD LocalDOMWindow *Window() const noexcept;
    /// @brief The document's window while it's in a frame (its browsing context's `WindowProxy`); null
    /// otherwise.
    /// @see https://html.spec.whatwg.org/#dom-document-defaultview
    KRYS_NODISCARD LocalDOMWindow *DefaultView() const noexcept;
    /// @brief Fires @p event at the document's window, if it has one.
    void DispatchWindowEvent(Event &event) noexcept;

    /// @see https://html.spec.whatwg.org/#load-timing-info
    KRYS_NODISCARD const DocumentLoadTimingInfo &LoadTiming() const noexcept
    {
      return _loadTiming;
    }

    /// @brief Whether the document's parser is running: it's being loaded into a frame.
    KRYS_NODISCARD bool Parsing() const noexcept
    {
      return _parsing;
    }
    void SetParsing(bool parsing) noexcept
    {
      _parsing = parsing;
    }

    /// @brief Whether something other than a load delays the document's load event.
    KRYS_NODISCARD bool IsDelayingLoadEvent() const noexcept
    {
      return _loadEventDelayCount != 0;
    }
    void IncrementLoadEventDelayCount() noexcept
    {
      ++_loadEventDelayCount;
    }
    /// @brief Ends a delay of the load event; the last one has the frame check whether the document loaded.
    void DecrementLoadEventDelayCount() noexcept;

    /// @brief Whether the window's load event has fired.
    KRYS_NODISCARD bool LoadEventFinished() const noexcept
    {
      return _loadEventFinished;
    }

    /// @see https://html.spec.whatwg.org/#page-showing
    KRYS_NODISCARD bool PageShowing() const noexcept
    {
      return _pageShowing;
    }

    /// @brief The document finished loading: fires `load` at the window, then `pageshow`.
    void ImplicitClose() noexcept;

    /// @brief Fires `load` at the window, with the document as its target.
    void DispatchWindowLoadEvent() noexcept;

    /// @brief Fires `pageshow` at the window if the document isn't showing yet.
    /// @see https://html.spec.whatwg.org/#fire-a-page-transition-event
    void DispatchPageshowEvent(bool persisted) noexcept;

    /// @brief Runs the pragma directive @p equiv (a `<meta http-equiv>`'s) with @p content: `refresh`,
    /// `content-language` and `default-style`; the others are ignored.
    /// @see https://html.spec.whatwg.org/#pragma-directives
    void ProcessMetaHttpEquiv(DOMStringView equiv, DOMStringView content) noexcept;

    /// @brief Saves and restores the state of the document's form controls, made on first use.
    KRYS_NODISCARD html::FormController &GetFormController() noexcept;

    KRYS_NODISCARD html::FormController *FormControllerIfExists() const noexcept
    {
      return _formController.get();
    }

    /// @brief The state of the document's form controls, saved to its history item as it's left.
    KRYS_NODISCARD List<DOMString> FormElementsState() const noexcept;

    /// @brief The form state the document's controls are given as they're parsed: their history item's.
    void SetStateForNewFormElements(const List<DOMString> &stateVector) noexcept;

    /// @brief The document's language from its `content-language` pragma, for elements without a language of
    /// their own; empty if it has none.
    /// @see https://html.spec.whatwg.org/#pragma-set-default-language
    KRYS_NODISCARD const DOMString &ContentLanguage() const noexcept
    {
      return _contentLanguage;
    }
    void SetContentLanguage(DOMString &&language) noexcept;

    /// @brief The window's `Location`, while the document is fully active; null otherwise.
    /// @see https://html.spec.whatwg.org/#dom-document-location
    KRYS_NODISCARD boo::Location *GetLocation() const noexcept;

    /// @brief Fires `pagehide` at the window if the document is showing.
    /// @see https://html.spec.whatwg.org/#unload-a-document
    void DispatchPagehideEvent(bool persisted) noexcept;

    /// @brief How many `beforeunload`, `pagehide` and `unload` events of the document are being dispatched:
    /// while there's one, it can't show dialogs.
    /// @see https://html.spec.whatwg.org/#unload-counter
    KRYS_NODISCARD uint32 UnloadCounter() const noexcept
    {
      return _unloadCounter;
    }
    void IncrementUnloadCounter() noexcept
    {
      ++_unloadCounter;
    }
    void DecrementUnloadCounter() noexcept
    {
      krys_debug_assert(_unloadCounter);
      --_unloadCounter;
    }

    /// @brief The next ID for a timer: positive, wrapping around.
    KRYS_NODISCARD int32 CircularSequentialID() noexcept;
    /// @brief Adds @p timer as the timer @p timeoutId.
    /// @return False, adding nothing, if there's a timer @p timeoutId already.
    bool AddTimeout(int32 timeoutId, DOMTimer &timer) noexcept;
    /// @brief Removes the timer @p timeoutId, if there is one, and returns it.
    RefPtr<DOMTimer> TakeTimeout(int32 timeoutId) noexcept;
    /// @brief How deeply nested the timer whose action is running is; 0 outside a timer.
    KRYS_NODISCARD int32 TimerNestingLevel() const noexcept
    {
      return _timerNestingLevel;
    }
    void SetTimerNestingLevel(int32 level) noexcept
    {
      _timerNestingLevel = level;
    }

    /// @see https://html.spec.whatwg.org/#dom-animationframeprovider-requestanimationframe
    int32 RequestAnimationFrame(FrameRequestCallback &&callback) noexcept;
    /// @see https://html.spec.whatwg.org/#animationframeprovider-cancelanimationframe
    void CancelAnimationFrame(int32 id) noexcept;
    /// @brief Runs the animation frame callbacks with the window's frozen timestamp.
    /// @see https://html.spec.whatwg.org/#run-the-animation-frame-callbacks
    void ServiceRequestAnimationFrameCallbacks() noexcept;

    /// @see https://w3c.github.io/requestidlecallback/#the-requestidlecallback-method
    int32 RequestIdleCallback(IdleRequestCallback &&callback, MonotonicTime::duration timeout) noexcept;
    /// @see https://w3c.github.io/requestidlecallback/#the-cancelidlecallback-method
    void CancelIdleCallback(int32 id) noexcept;
    KRYS_NODISCARD bool HasPendingIdleCallback() const noexcept;
    KRYS_NODISCARD IdleCallbackController *GetIdleCallbackController() const noexcept
    {
      return _idleCallbackController.get();
    }

    /// @brief Stops the document's timers and drops its animation frame and idle callbacks and its tasks:
    /// it's leaving its frame.
    void StopActiveDOMObjects() noexcept;

    KRYS_NODISCARD bool ActiveDOMObjectsAreStopped() const noexcept
    {
      return _activeDOMObjectsAreStopped;
    }

    /// @brief Suspends the document (as it enters the back/forward cache): its tasks wait, and its timers,
    /// animation frame callbacks, animations and layout stop, until it's resumed for the same @p reason.
    void Suspend(ReasonForSuspension reason) noexcept;
    void Resume(ReasonForSuspension reason) noexcept;

    KRYS_NODISCARD bool IsSuspended() const noexcept
    {
      return _isSuspended;
    }

    /// @brief Suspends the document's tasks, timers and animation frame callbacks.
    void SuspendScheduledTasks(ReasonForSuspension reason) noexcept;
    void ResumeScheduledTasks(ReasonForSuspension reason) noexcept;

    KRYS_NODISCARD bool ScheduledTasksAreSuspended() const noexcept
    {
      return _scheduledTasksAreSuspended;
    }

    /// @brief Suspends the document's timers.
    void SuspendActiveDOMObjects(ReasonForSuspension reason) noexcept;
    void ResumeActiveDOMObjects(ReasonForSuspension reason) noexcept;

    KRYS_NODISCARD bool ActiveDOMObjectsAreSuspended() const noexcept
    {
      return _activeDOMObjectsAreSuspended;
    }

    KRYS_NODISCARD Maybe<ReasonForSuspension> ReasonForSuspendingActiveDOMObjects() const noexcept
    {
      return _reasonForSuspendingActiveDOMObjects;
    }

    KRYS_NODISCARD BackForwardCacheState GetBackForwardCacheState() const noexcept
    {
      return _backForwardCacheState;
    }

    /// @brief Notes where the document is in entering the back/forward cache: as it's cached, its view's
    /// scroll position is kept, and its style resolver and pending style update go.
    void SetBackForwardCacheState(BackForwardCacheState state) noexcept;

    /// @brief Detaches the document, now in the back/forward cache, from its frame.
    void DetachFromCachedFrame() noexcept;
    /// @brief Queues a task on @p source that fires @p event at the document's window, if it has one then.
    void QueueTaskToDispatchEventOnWindow(html::TaskSource source, Ref<Event> &&event) noexcept;
    /// @brief The loader of the document's subresources (images), made on first use.
    KRYS_NODISCARD CachedResourceLoader &GetCachedResourceLoader() const noexcept;
    /// @brief Asks the page for a rendering update that runs @p requestedSteps, if the document is in one.
    void ScheduleRenderingUpdate(EnumSet<RenderingUpdateStep> requestedSteps) noexcept;

    /// @see https://html.spec.whatwg.org/#autofocus-candidates
    KRYS_NODISCARD List<WeakPtr<Element>> &AutofocusCandidates() noexcept
    {
      return _autofocusCandidates;
    }
    /// @see https://html.spec.whatwg.org/#autofocus-processed-flag
    KRYS_NODISCARD bool AutofocusProcessed() const noexcept
    {
      return _autofocusProcessed;
    }
    void SetAutofocusProcessed() noexcept
    {
      _autofocusProcessed = true;
    }

    /// @brief The view the document is laid out in: its frame's, if any.
    KRYS_NODISCARD LocalFrameView *View() const noexcept;

    /// @brief The root of the document's render tree, if it has one.
    KRYS_NODISCARD const boo::RenderView *GetRenderView() const noexcept
    {
      return _renderView.get();
    }

    KRYS_NODISCARD boo::RenderView *GetRenderView() noexcept
    {
      return _renderView.get();
    }

    KRYS_NODISCARD bool RenderTreeBeingDestroyed() const noexcept
    {
      return _renderTreeBeingDestroyed;
    }

    KRYS_NODISCARD bool HasLivingRenderTree() const noexcept
    {
      return _renderView && !_renderTreeBeingDestroyed;
    }

    /// @brief Creates the document's render tree, for the view of its frame.
    void CreateRenderTree() noexcept;
    void DestroyRenderTree() noexcept;

    /// @brief Schedules an update of @p text's renderer after @p lengthOfReplacedText bytes from
    /// @p offsetOfReplacedText were replaced.
    void UpdateTextRenderer(Text &text, uint32 offsetOfReplacedText, uint32 lengthOfReplacedText) noexcept;

    /// @brief Makes the document's window: the document must be in a frame, and not have one yet.
    void CreateDOMWindow() noexcept;

    /// @brief Called when @p frame starts showing the document.
    void DidBecomeCurrentDocumentInFrame(LocalFrame &frame) noexcept;
    /// @brief Called when the document's frame stops showing it.
    void WillBeRemovedFromFrame() noexcept;

    /// @brief Whether the document is being printed.
    // FIXME: Always false: frames don't print.
    KRYS_NODISCARD bool Printing() const noexcept
    {
      return false;
    }

    /// @brief Device pixels per CSS pixel: the frame's media environment's, or 1.
    KRYS_NODISCARD float DeviceScaleFactor() const noexcept;

    /// @brief How a background colour is composited: over what's beneath it.
    /// FIXME: Punching out white backgrounds in dark mode (`punchOutWhiteBackgroundsInDarkMode`, a setting
    /// off by default) isn't ported.
    KRYS_NODISCARD CompositeOperator CompositeOperatorForBackgroundColor(const Color &,
                                                                         const RenderElement &) const noexcept
    {
      return CompositeOperator::SourceOver;
    }

    KRYS_NODISCARD bool ShouldScheduleLayout() const noexcept;
    KRYS_NODISCARD bool IsLayoutPending() const noexcept;

    /// @brief Updates the styles that use viewport units after the view's size changed.
    void UpdateViewportUnitsOnResize() noexcept;

    enum class UpdateLayoutResult : bool
    {
      NoChange,
      ChangesDone
    };

    /// @brief Lays the document out, then hit tests its layers at the result's location.
    /// @return Whether a layer was hit.
    bool HitTest(const HitTestRequest &request, HitTestResult &result) noexcept;
    bool HitTest(const HitTestRequest &request, const HitTestLocation &location,
                 HitTestResult &result) noexcept;

    /// @brief Updates style, then lays the document out if it needs it.
    UpdateLayoutResult UpdateLayout(EnumSet<LayoutOptions> layoutOptions = {}) noexcept;
    UpdateLayoutResult UpdateLayoutIgnorePendingStylesheets() noexcept;

    /// @brief The element that scrolls the viewport: the root element, or in quirks mode the body unless it's
    /// potentially scrollable.
    /// @see https://drafts.csswg.org/cssom-view/#dom-document-scrollingelement
    KRYS_NODISCARD RefPtr<Element> ScrollingElement() noexcept;
    /// @brief The scrolling element, after updating layout in quirks mode.
    KRYS_NODISCARD RefPtr<Element> ScrollingElementForAPI() noexcept;
    /// @brief The first body or frameset child of the html element.
    KRYS_NODISCARD html::HTMLElement *BodyOrFrameset() const noexcept;

    /// @brief Converts absolute quads (or rects) to the client coordinates of the CSSOM View APIs.
    void ConvertAbsoluteToClientQuads(List<FloatQuad> &quads, const RenderStyle &style) const noexcept;
    void ConvertAbsoluteToClientRects(List<FloatRect> &rects, const RenderStyle &style) const noexcept;
    void ConvertAbsoluteToClientRect(FloatRect &rect, const RenderStyle &style) const noexcept;

#pragma endregion

    /// @brief The document's font selector, which resolves the families its styles name.
    KRYS_NODISCARD css::CSSFontSelector &GetFontSelector() const noexcept;
    KRYS_NODISCARD css::CSSFontSelector *FontSelectorIfExists() const noexcept;

    /// @brief The loader of the document's web fonts, made on first use.
    KRYS_NODISCARD DocumentFontLoader &GetFontLoader() const noexcept;
    /// @brief A request for the web font file at @p url (resolved against the document), or null if it can't
    /// be requested. The file doesn't load until `BeginLoadingFontSoon`.
    KRYS_NODISCARD RefPtr<FontLoadRequest>
      GetFontLoadRequest(utf8_stringview url, bool isSVG, bool isInitiatingElementInUserAgentShadowTree,
                         LoadedFromOpaqueSource loadedFromOpaqueSource) noexcept;
    void BeginLoadingFontSoon(FontLoadRequest &request) noexcept;
    void SuspendFontLoading() noexcept;

    /// @brief Restyles the document after the fonts its font selector provides changed.
    void FontsNeedUpdate(FontSelector &selector) noexcept override;

    void InvalidateMatchedPropertiesCacheAndForceStyleRecalc() noexcept;

    /// @brief The document's default timeline, made on first use.
    /// @see https://drafts.csswg.org/web-animations-1/#dom-document-timeline
    KRYS_NODISCARD DocumentTimeline &Timeline() noexcept;
    KRYS_NODISCARD DocumentTimeline *ExistingTimeline() const noexcept;
    KRYS_NODISCARD AnimationTimelinesController *TimelinesController() const noexcept
    {
      return _timelinesController.get();
    }
    KRYS_NODISCARD AnimationTimelinesController &EnsureTimelinesController() noexcept;
    /// @see https://drafts.csswg.org/web-animations-1/#update-animations-and-send-events
    void UpdateAnimationsAndSendEvents() noexcept;
    /// @brief Moves the selection's caret on through its blinking.
    void ServiceCaretAnimation() noexcept;
    /// @brief Recomputes the keyframes of the document's CSS animations of the `@keyframes` rule @p name.
    void KeyframesRuleDidChange(const css::CSSOMString &name) noexcept;

    /// @brief Determines whether links are visited, for `:visited` and the link styles.
    KRYS_NODISCARD VisitedLinkState &GetVisitedLinkState() const noexcept;

    void SetHasStyleWithViewportUnits() noexcept
    {
      _hasStyleWithViewportUnits = true;
    }

    KRYS_NODISCARD bool HasStyleWithViewportUnits() const noexcept
    {
      return _hasStyleWithViewportUnits;
    }

    /// @brief The properties a computed style declaration lists, in order.
    KRYS_NODISCARD const FixedVector<css::PropertyId> &ExposedComputedCSSPropertyIds() noexcept;

    /// @brief The colour of `-internal-document-text-color`: the `<body>` element's colour, as of its last
    /// style resolution, or black.
    KRYS_NODISCARD const boo::Color &TextColor() const noexcept
    {
      return _textColor;
    }

    void SetTextColor(const boo::Color &color) noexcept
    {
      _textColor = color;
    }

    /// @brief The colour of `-webkit-link`: the `<body link>` colour, or the keyword's.
    KRYS_NODISCARD boo::Color LinkColor(const RenderStyle &style) const noexcept;
    /// @brief The colour of visited `-webkit-link`: the `<body vlink>` colour, or the keyword's.
    KRYS_NODISCARD boo::Color VisitedLinkColor(const RenderStyle &style) const noexcept;
    /// @brief The colour of `-webkit-activelink`: the `<body alink>` colour, or the keyword's.
    KRYS_NODISCARD boo::Color ActiveLinkColor(const RenderStyle &style) const noexcept;

    void SetLinkColor(const boo::Color &color) noexcept
    {
      _linkColor = color;
    }

    void SetVisitedLinkColor(const boo::Color &color) noexcept
    {
      _visitedLinkColor = color;
    }

    void SetActiveLinkColor(const boo::Color &color) noexcept
    {
      _activeLinkColor = color;
    }

    void ResetLinkColor() noexcept;
    void ResetVisitedLinkColor() noexcept;
    void ResetActiveLinkColor() noexcept;

    /// @brief The base value of a `random()` shared between the elements of the document (`element-shared`).
    KRYS_NODISCARD double LookupCSSRandomBaseValue(const css::calc::RandomCachingKey &key) const noexcept;

#pragma region DocumentOrShadowRoot Mixin (CSSOM) - https://drafts.csswg.org/cssom/#extensions-to-the-document-or-shadow-root-interface

    /// @brief The style sheets of the document, in order.
    /// @see https://drafts.csswg.org/cssom/#dom-documentorshadowroot-stylesheets
    KRYS_NODISCARD css::StyleSheetList &StyleSheets() noexcept;

    /// @brief The (cached) query for @p selectorString, as used by `querySelector()`, `matches()` and
    /// `closest()`.
    /// @return A `SyntaxError` if @p selectorString isn't a valid selector list.
    KRYS_NODISCARD ExceptionOr<SelectorQuery &>
      SelectorQueryForString(DOMStringView selectorString) const noexcept;

#pragma endregion

#pragma region GlobalEventHandlers, DocumentAndElementEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

#pragma region Media queries - https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface

    /// @brief Evaluates the document's `MediaQueryList`s and dispatches their `change` events.
    KRYS_NODISCARD css::MediaQueryMatcher &GetMediaQueryMatcher() noexcept;

    /// @brief Re-evaluates the document's `MediaQueryList`s and fires `change` at each one whose result
    /// changed. The host calls this after changing the media environment.
    /// @see https://drafts.csswg.org/cssom-view/#evaluate-media-queries-and-report-changes
    void EvaluateMediaQueriesAndReportChanges() noexcept;

#pragma endregion

#pragma endregion

#pragma region Mode - https://dom.spec.whatwg.org/#concept-document-mode

    /// @see https://dom.spec.whatwg.org/#concept-document-quirks
    KRYS_NODISCARD bool InQuirksMode() const noexcept
    {
      return _quirksMode == QuirksMode::Quirks;
    }

    /// @see https://dom.spec.whatwg.org/#concept-document-limited-quirks
    KRYS_NODISCARD bool InLimitedQuirksMode() const noexcept
    {
      return _quirksMode == QuirksMode::LimitedQuirks;
    }

    /// @see https://dom.spec.whatwg.org/#concept-document-no-quirks
    KRYS_NODISCARD bool InNoQuirksMode() const noexcept
    {
      return _quirksMode == QuirksMode::NoQuirks;
    }

#pragma endregion

#pragma region ParentNode Mixin - https://dom.spec.whatwg.org/#parentnode

    /// @brief Returns the child elements.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-children
    KRYS_NODISCARD Ref<HTMLCollection> Children() noexcept;

    /// @brief Returns the first child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild
    KRYS_NODISCARD RefPtr<const Element> FirstElementChild() const noexcept;

    /// @brief Returns the first child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-firstelementchild
    KRYS_NODISCARD RefPtr<Element> FirstElementChild() noexcept;

    /// @brief Returns the last child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild
    KRYS_NODISCARD RefPtr<const Element> LastElementChild() const noexcept;

    /// @brief Returns the last child that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-lastelementchild
    KRYS_NODISCARD RefPtr<Element> LastElementChild() noexcept;

    /// @brief Returns the number of child elements.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-childelementcount
    KRYS_NODISCARD size_t ChildElementCount() const noexcept;

    /// @brief Inserts `nodes` before the first child of `this`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-prepend
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> Prepend(const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` after the last child of `this`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-append
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> Append(const List<NodeOrString> &nodes) noexcept;

    /// @brief Replace all children of `this` with `nodes`, while replacing strings in `nodes` with equivalent
    /// Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-replacechildren
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> ReplaceChildren(const List<NodeOrString> &nodes) noexcept;

    /// @brief Moves, without first removing, `movedNode` into `this` after `child` if `child` is non-null;
    /// otherwise after the last child of node. This method preserves state associated with `movedNode`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-movebefore
    /// @throws HierarchyRequestError if the constraints of the node tree are violated, or the state
    /// associated with the moved node cannot be preserved.
    ExceptionOr<void> MoveBefore(Node &node, Node *child) noexcept;

    /// @brief Returns the first element that is a descendant of `this` that matches `selectors`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-queryselector
    KRYS_NODISCARD ExceptionOr<RefPtr<Element>> QuerySelector(DOMStringView selectors) noexcept;

    /// @brief Returns all element descendants of `this` that match `selectors`.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-queryselectorall
    KRYS_NODISCARD ExceptionOr<Ref<NodeList>> QuerySelectorAll(DOMStringView selectors) noexcept;

#pragma endregion

#pragma region NonElementParentNode Mixin - https://dom.spec.whatwg.org/#interface-nonelementparentnode

    /// @brief Returns the first element within node’s descendants whose ID is `elementId`.
    /// @see https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid
    RefPtr<Element> GetElementById(DOMStringView elementId) noexcept;

    /// @brief Returns the first element within node’s descendants whose ID is `elementId`.
    /// @see https://dom.spec.whatwg.org/#dom-nonelementparentnode-getelementbyid
    RefPtr<const Element> GetElementById(DOMStringView elementId) const noexcept;

#pragma endregion

  protected:
    /// @brief With nodes still referring to it, the document stops retaining its nodes (the elements it
    /// tracks, its children) and waits for them to go; without, it's deleted.
    void RemovedLastRef() noexcept override;

    /// @brief The teardown shared by the last reference going and leaving the frame: the observers
    /// disconnect.
    void CommonTeardown() noexcept;

    void SetQuirksMode(QuirksMode mode) noexcept;

    void UpdateRenderTree(UniquePtr<style::Update> styleUpdate) noexcept;
    KRYS_NODISCARD style::Update &EnsurePendingRenderTreeUpdate() noexcept;

    /// @brief Finds the first `base` elements with an `href` and with a `target` in the document tree, and
    /// updates the document base URL and base target from them. Called whenever a `base` element is inserted,
    /// removed or moved, or its `href` or `target` changes.
    void ProcessBaseElement() noexcept;

    void UpdateBaseURL() noexcept;

    /// @see https://html.spec.whatwg.org/#the-document-object:get-the-parent
    KRYS_NODISCARD EventTarget *GetParent(Event &event) const noexcept override;

    KRYS_NODISCARD bool IsScriptingDisabled() const noexcept
    {
      // SPEC-VIOLATION(HTML, DOM): We don't have global objects or JavaScript so we'll always return true
      // here. A document is scripting-disabled if the document does not have a browsing context.

      return true;
    }

#pragma region Type Checks

    KRYS_NODISCARD bool IsHTMLDocument() const noexcept
    {
      return HasFlag(_flags, DocumentFlags::IsHTMLDocument);
    }

    KRYS_NODISCARD bool IsXMLDocument() const noexcept
    {
      return HasFlag(_flags, DocumentFlags::IsXMLDocument);
    }

    KRYS_NODISCARD bool IsXHTMLDocument() const noexcept
    {
      return HasFlag(_flags, DocumentFlags::IsXHTMLDocument);
    }

    KRYS_NODISCARD bool IsSVGDocument() const noexcept
    {
      return HasFlag(_flags, DocumentFlags::IsSVGDocument);
    }

#pragma endregion

#pragma region Document Flags

    KRYS_NODISCARD bool HasDocumentFlag(DocumentFlags flags) const noexcept
    {
      return HasFlag(_flags, flags);
    }

    void SetDocumentFlag(DocumentFlags flags) noexcept
    {
      _flags = _flags | flags;
    }

    void ClearDocumentFlag(DocumentFlags flags) noexcept
    {
      _flags = _flags & ~flags;
    }

#pragma endregion

    KRYS_NODISCARD List<Range *> &LiveRanges() noexcept
    {
      return _liveRanges;
    }

    KRYS_NODISCARD List<NodeIterator *> &NodeIterators() noexcept
    {
      return _nodeIterators;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Document)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsDocumentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
