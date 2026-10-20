#pragma once

#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Dicts/ElementCreationOptions.hpp"
#include "Krystal.Booey/DOM/Dicts/ImportNodeOptions.hpp"
#include "Krystal.Booey/DOM/DocumentOrShadowRoot.hpp"
#include "Krystal.Booey/DOM/DOMImplementation.hpp"
#include "Krystal.Booey/DOM/Enums/DocumentFlags.hpp"
#include "Krystal.Booey/DOM/Enums/QuirksMode.hpp"
#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Booey/DOM/Internals/RareData/DocumentRareData.hpp"
#include "Krystal.Booey/DOM/NodeIterator.hpp"
#include "Krystal.Booey/DOM/Range.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/DOM/Types/NodeOrString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/DoublePoint.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/FontSelector.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Booey/HTML/Enums/DocumentReadyState.hpp"
#include "Krystal.Booey/HTML/Enums/DocumentVisibilityState.hpp"
#include "Krystal.Booey/HTML/Types/HTMLOrSVGScriptElement.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/FixedVector.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class HitTestLocation;
  class HitTestRequest;
  class HitTestResult;
  class MouseEventWithHitTestResults;
  class PlatformMouseEvent;
  class LocalFrame;
  class LocalFrameView;
  class RenderElement;
  class RenderStyle;
  class RenderTreeUpdater;
  class RenderView;
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
  class HTMLBaseElement;
  class HTMLBodyElement;
  class HTMLDocumentParser;
  class HTMLHeadElement;
  class HTMLHtmlElement;
  class HTMLTitleElement;
  class HTMLTreeBuilder;
  class RenderBlocking;
}

namespace krys::boo::dom
{
  class Attr;
  class CDATASection;
  class Comment;
  class DocumentType;
  class Element;
  class HTMLCollection;
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

    mutable UniquePtr<css::ParserContext> _cachedCSSParserContext;
    Ref<Settings> _settings;

    UniquePtr<UserActionElementSet> _userActionElements;
    RefPtr<Element> _hoveredElement;
    // The element the pointer was pressed on: the chain of its ancestors stays `:active` until it's released.
    RefPtr<Element> _activeElement;
    bool _isFocusedAndActive {true};
    // The focused area of the document: an element, or the viewport if null.
    RefPtr<Element> _focusedElement;
    FocusTrigger _latestFocusTrigger {FocusTrigger::Other};
    // The scroll events to fire at the next scroll steps, in the order they were queued.
    List<std::pair<Ref<ContainerNode>, ScrollEventType>> _pendingScrollEventTargets;
    // Where sequential focus navigation starts from when it's inside the focused area: unset if null.
    RefPtr<Node> _sequentialFocusNavigationStartingPoint;
    SequentialFocusNavigationStartingPointPosition _sequentialFocusNavigationStartingPointPosition {
      SequentialFocusNavigationStartingPointPosition::At};

    List<WeakPtr<ShadowRoot>> _inDocumentShadowRoots;
    RefPtr<css::StyleSheetList> _styleSheetList;
    RefPtr<css::MediaQueryMatcher> _mediaQueryMatcher;
    mutable RefPtr<FontSelector> _fontSelector;

    WeakPtr<LocalFrame> _frame;
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

  protected:
    Document(DocumentFlags flags) noexcept;

  public:
    ~Document() noexcept override;

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
    /// the viewport) for the next scroll steps, once per target and type.
    // FIXME: A text control's inner text element's events go to its shadow host, once there are text
    // controls; scheduling a rendering update (scheduleRenderingUpdate), once there's a rendering loop: the
    // host runs the scroll steps.
    void AddPendingScrollEventTarget(ContainerNode &originalTarget, ScrollEventType eventType) noexcept;

    /// @brief Fires the pending scroll events, in the order they were queued: at elements without bubbling,
    /// and at the document bubbling.
    /// @see https://drafts.csswg.org/cssom-view/#run-the-scroll-steps
    void RunScrollSteps() noexcept;

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
    UpdateLayoutResult UpdateLayout() noexcept;
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

    /// @brief Restyles the document after the fonts its font selector provides changed.
    void FontsNeedUpdate(FontSelector &selector) noexcept override;

    void InvalidateMatchedPropertiesCacheAndForceStyleRecalc() noexcept;

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

#pragma region Media queries - https://drafts.csswg.org/cssom-view/#the-mediaquerylist-interface

    /// @brief Evaluates the document's `MediaQueryList`s and dispatches their `change` events.
    KRYS_NODISCARD css::MediaQueryMatcher &GetMediaQueryMatcher() noexcept;

    /// @brief A new `MediaQueryList` for @p query, evaluated against the document.
    /// @note `matchMedia()` is a `Window` member; there's no window, so it's on the document.
    /// @see https://drafts.csswg.org/cssom-view/#dom-window-matchmedia
    KRYS_NODISCARD Ref<css::MediaQueryList> MatchMedia(DOMStringView query) noexcept;

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
    void SetQuirksMode(QuirksMode mode) noexcept;

    void UpdateRenderTree(UniquePtr<style::Update> styleUpdate) noexcept;
    KRYS_NODISCARD style::Update &EnsurePendingRenderTreeUpdate() noexcept;

    /// @brief Finds the first `base` element with an `href` in the document tree and updates the document
    /// base URL from it. Called whenever a `base` element is inserted, removed or moved, or its `href`
    /// changes.
    void ProcessBaseElement() noexcept;

    void UpdateBaseURL() noexcept;

    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD EventTarget *GetParent(Event &event) const noexcept override
    {
      (void)event;
      // SPEC-VIOLATION(HTML, DOM): We don't have global objects so we'll always return nullptr here.
      // A document’s get the parent algorithm, given an event, returns null if event’s type
      // attribute value is "load" or document does not have a browsing context; otherwise the document’s
      // relevant global object.

      return nullptr;
    }

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
