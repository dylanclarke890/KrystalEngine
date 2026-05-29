#pragma once

#include "Krystal.HTML/DOM/ContainerNode.hpp"
#include "Krystal.HTML/DOM/Dicts/ElementCreationOptions.hpp"
#include "Krystal.HTML/DOM/Dicts/ImportNodeOptions.hpp"
#include "Krystal.HTML/DOM/DOMImplementation.hpp"
#include "Krystal.HTML/DOM/Enums/DocumentFlags.hpp"
#include "Krystal.HTML/DOM/Enums/DocumentReadyState.hpp"
#include "Krystal.HTML/DOM/Enums/DocumentVisibilityState.hpp"
#include "Krystal.HTML/DOM/Enums/QuirksMode.hpp"
#include "Krystal.HTML/DOM/Internals/RareData/DocumentRareData.hpp"
#include "Krystal.HTML/DOM/NodeIterator.hpp"
#include "Krystal.HTML/DOM/Range.hpp"
#include "Krystal.HTML/Types/ExceptionOr.hpp"
#include "Krystal.HTML/Types/NodeOrString.hpp"
#include "Krystal.Lib/Types/List.hpp"

namespace Krys::HTML
{
  class Attr;
  class CDATASection;
  class Comment;
  class CustomElementRegistry;
  class DocumentType;
  class Element;
  class HTMLCollection;
  class HTMLBodyElement;
  class HTMLHeadElement;
  class HTMLHtmlElement;
  class HTMLTitleElement;
  class ProcessingInstruction;
  class Text;
  class TreeWalker;

  /// @see https://dom.spec.whatwg.org/#interface-document
  class Document : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Document);
    KRYS_TYPE_CAST_TRAITS_ACCESS();

    friend class CustomElementAlgorithms;
    friend class DocumentAlgorithms;
    friend class DocumentRareData;
    friend class DOMImplementation;
    friend class HTMLCollectionAlgorithms;
    friend class LiveRangeUpdater;
    friend class MutationAlgorithms;
    friend class NodeAlgorithms;
    friend class RenderBlocking;

  private:
    UniquePtr<DocumentRareData> _documentRareData;
    UniquePtr<DOMImplementation> _implementation;
    DOMString _contentType {u8"application/xml"};
    List<RawPtr<Range>> _liveRanges;
    List<RawPtr<NodeIterator>> _nodeIterators;
    RefPtr<CustomElementRegistry> _customElementRegistry;
    bool _allowDeclarativeShadowRoots : 1 {false};
    QuirksMode _quirksMode : BitCount<QuirksMode>() {QuirksMode::NoQuirks};
    DocumentFlags _flags : BitCount<DocumentFlags>() {DocumentFlags::None};

#pragma region HTML spec

    DocumentReadyState _currentDocumentReadiness
        : BitCount<DocumentReadyState>() {DocumentReadyState::Complete};
    DocumentVisibilityState _visibilityState
        : BitCount<DocumentVisibilityState>() {DocumentVisibilityState::Hidden};

    List<WeakRef<Element>> _renderBlockingElements;

#pragma endregion

  protected:
    Document(DocumentFlags flags) noexcept;

  public:
#pragma region Document - https://dom.spec.whatwg.org/#interface-document

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

#pragma region Node - https://dom.spec.whatwg.org/#interface-node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final
    {
      return u8"#document";
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
    ExceptionOr<void> MoveBefore(Node &node, RawPtr<Node> child) noexcept;

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

#pragma region DocumentOrShadowRoot Mixin - https://dom.spec.whatwg.org/#mixin-documentorshadowroot

    /// @brief Returns node's CustomElementRegistry object, if any; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-documentorshadowroot-customelementregistry
    RefPtr<CustomElementRegistry> CustomElementRegistry() const noexcept;

#pragma endregion

#pragma region HTML spec extensions - https://html.spec.whatwg.org/multipage/dom.html#the-document-object

    KRYS_NODISCARD static Ref<Document> ParseHTMLUnsafe(const DOMString &html) noexcept;

    // TODO(check): do we need to implement these?
    // resource metadata management
    // [ PutForwards = href, LegacyUnforgeable ] readonly attribute Location? location;
    // attribute USVString domain;
    // readonly attribute USVString referrer;
    // attribute USVString cookie;
    // readonly attribute DOMString lastModified;

    KRYS_NODISCARD DocumentReadyState ReadyState() const noexcept
    {
      return _currentDocumentReadiness;
    }

    // DOM tree accessors
    // getter object (DOMString name);

    KRYS_NODISCARD DOMString Title() const noexcept;
    ExceptionOr<void> Title(DOMString &&value) noexcept;

    KRYS_NODISCARD DOMString Dir() const noexcept;
    void Dir(DOMString &&value) noexcept;

    // NOTE: the HTML spec says that body returns a HTMLElement but that's because it can also return a
    // HTMLFrameSetElement which is a legacy element that we don't support. In our case, body will always
    // return an HTMLBodyElement if it exists.

    KRYS_NODISCARD RefPtr<HTMLBodyElement> Body() noexcept;
    KRYS_NODISCARD RefPtr<const HTMLBodyElement> Body() const noexcept;
    ExceptionOr<void> Body(HTMLBodyElement &body) noexcept;

    KRYS_NODISCARD RefPtr<HTMLHeadElement> Head() noexcept;
    KRYS_NODISCARD RefPtr<const HTMLHeadElement> Head() const noexcept;

    // [SameObject] readonly attribute HTMLCollection images;
    // [SameObject] readonly attribute HTMLCollection embeds;
    // [SameObject] readonly attribute HTMLCollection plugins;
    // [SameObject] readonly attribute HTMLCollection links;
    // [SameObject] readonly attribute HTMLCollection forms;
    // [SameObject] readonly attribute HTMLCollection scripts;
    // NodeList getElementsByName(DOMString elementName);
    // readonly attribute HTMLOrSVGScriptElement? currentScript; // classic scripts in a document tree only

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
      return _visibilityState == DocumentVisibilityState::Hidden;
    }

    KRYS_NODISCARD DocumentVisibilityState VisibilityState() const noexcept
    {
      return _visibilityState;
    }

    // special event handler IDL attributes that only apply to Document objects
    // [LegacyLenientThis] attribute EventHandler onreadystatechange;
    // attribute EventHandler onvisibilitychange;

  protected:
    KRYS_NODISCARD RefPtr<HTMLHtmlElement> GetHTMLHtmlElement() noexcept;
    KRYS_NODISCARD RefPtr<const HTMLHtmlElement> GetHTMLHtmlElement() const noexcept;

    KRYS_NODISCARD RefPtr<HTMLTitleElement> GetHTMLTitleElement() noexcept;
    KRYS_NODISCARD RefPtr<const HTMLTitleElement> GetHTMLTitleElement() const noexcept;

  public:
#pragma endregion

  protected:
    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD RawPtr<EventTarget> GetParent(Event &event) const noexcept override
    {
      // SPEC-VIOLATION(HTML, DOM): We don't have global objects so we'll always return nullptr here.
      // A document’s get the parent algorithm, given an event, returns null if event’s type
      // attribute value is "load" or document does not have a browsing context; otherwise the document’s
      // relevant global object.

      return nullptr;
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

    KRYS_NODISCARD List<RawPtr<Range>> &LiveRanges() noexcept
    {
      return _liveRanges;
    }

    KRYS_NODISCARD List<RawPtr<NodeIterator>> &NodeIterators() noexcept
    {
      return _nodeIterators;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Document)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsDocumentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
