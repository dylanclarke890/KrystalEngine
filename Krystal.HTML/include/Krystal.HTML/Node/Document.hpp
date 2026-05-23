#pragma once

#include "Krystal.HTML/Iterator/NodeFilter.hpp"
#include "Krystal.HTML/Iterator/NodeIterator.hpp"
#include "Krystal.HTML/Iterator/TreeWalker.hpp"
#include "Krystal.HTML/Iterator/WhatToShow.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/DOMImplementation.hpp"
#include "Krystal.HTML/Node/ElementCreationOptions.hpp"
#include "Krystal.HTML/Node/Enums/QuirksMode.hpp"
#include "Krystal.HTML/Node/ImportNodeOptions.hpp"
#include "Krystal.HTML/Node/RareData/DocumentRareData.hpp"
#include "Krystal.HTML/Ranges/Range.hpp"
#include "Krystal.HTML/URL.hpp"
#include "Krystal.HTML/Utils/ExceptionOr.hpp"
#include "Krystal.Lib/Core/TypeCast.hpp"
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
  class ProcessingInstruction;

  class Document : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Document);
    KRYS_TYPE_CAST_TRAITS_ACCESS();

    friend class DocumentRareData;
    friend class DOMImplementation;
    friend class MutationAlgorithms;
    friend class LiveRangeUpdater;
    friend class NodeAlgorithms;
    friend class HTMLCollectionAlgorithms;
    friend class CustomElementAlgorithms;
    friend class TreeMutationDispatcher;

  protected:
    enum class Type
    {
      XML,
      HTML
    };

  private:
    UniquePtr<DocumentRareData> _documentRareData;
    UniquePtr<DOMImplementation> _implementation;
    DOMString _contentType {u8"application/xml"};
    QuirksMode _quirksMode {QuirksMode::NoQuirks};
    Type _documentType {Type::XML};
    List<RawPtr<Range>> _liveRanges;
    List<RawPtr<NodeIterator>> _nodeIterators;
    RefPtr<CustomElementRegistry> _customElementRegistry;
    bool _allowDeclarativeShadowRoots : 1 {false};

  protected:
    Document(Type documentType) noexcept;

  public:
    Document() noexcept;

#pragma region Document

    KRYS_NODISCARD DOMImplementation &Implementation() noexcept;
    KRYS_NODISCARD DOMString URL() const noexcept;
    KRYS_NODISCARD DOMString DocumentURI() const noexcept;
    KRYS_NODISCARD DOMString CompatMode() const noexcept;
    KRYS_NODISCARD DOMString CharacterSet() const noexcept;
    KRYS_NODISCARD DOMString Charset() const noexcept;       // legacy alias of .characterSet
    KRYS_NODISCARD DOMString InputEncoding() const noexcept; // legacy alias of .characterSet
    KRYS_NODISCARD DOMString ContentType() const noexcept;

    KRYS_NODISCARD RefPtr<DocumentType> DocType() noexcept;
    KRYS_NODISCARD RefPtr<const DocumentType> DocType() const noexcept;
    KRYS_NODISCARD RefPtr<Element> DocumentElement() noexcept;
    KRYS_NODISCARD RefPtr<const Element> DocumentElement() const noexcept;

    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagName(DOMStringAtom qualifiedName) noexcept;
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagNameNS(DOMStringAtom namespaceUri,
                                                              DOMStringAtom localName) noexcept;
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByClassName(DOMStringAtom classNames) noexcept;

    KRYS_NODISCARD ExceptionOr<Ref<Element>>
      CreateElement(DOMStringAtom localName, const ElementCreationOptionsOrString &options = {}) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<Element>>
      CreateElementNS(DOMStringAtom namespaceUri, DOMStringAtom qualifiedName,
                      const ElementCreationOptionsOrString &options = {}) noexcept;
    KRYS_NODISCARD Ref<DocumentFragment> CreateDocumentFragment() noexcept;
    KRYS_NODISCARD Ref<Text> CreateTextNode(DOMString &&data) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<CDATASection>> CreateCDATASection(DOMString &&data) noexcept;
    KRYS_NODISCARD Ref<Comment> CreateComment(DOMString &&data) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<ProcessingInstruction>>
      CreateProcessingInstruction(DOMString &&target, DOMString &&data) noexcept;

    KRYS_NODISCARD ExceptionOr<Ref<Node>> ImportNode(Node &node,
                                                     const BoolOrImportNodeOptions &options = false) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<Node>> AdoptNode(Node &node) noexcept;

    KRYS_NODISCARD ExceptionOr<Ref<Attr>> CreateAttribute(DOMStringAtom localName) noexcept;
    KRYS_NODISCARD ExceptionOr<Ref<Attr>> CreateAttributeNS(DOMStringAtom namespaceUri,
                                                            DOMStringAtom qualifiedName) noexcept;

    KRYS_NODISCARD Ref<Event> CreateEvent(DOMStringAtom interface) noexcept;

    KRYS_NODISCARD Ref<Range> CreateRange() noexcept;

    KRYS_NODISCARD Ref<NodeIterator> CreateNodeIterator(Node &root,
                                                        WhatToShow whatToShow = WhatToShow::SHOW_ALL,
                                                        RefPtr<NodeFilter> &&filter = nullptr) noexcept;
    KRYS_NODISCARD Ref<TreeWalker> CreateTreeWalker(Node &root, WhatToShow whatToShow = WhatToShow::SHOW_ALL,
                                                    RefPtr<NodeFilter> &&filter = nullptr) noexcept;

#pragma endregion

#pragma region Node

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

    RefPtr<CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      return _customElementRegistry;
    }

#pragma endregion

  protected:
    KRYS_NODISCARD List<RawPtr<Range>> &LiveRanges() noexcept
    {
      return _liveRanges;
    }

    KRYS_NODISCARD const List<RawPtr<Range>> &LiveRanges() const noexcept
    {
      return _liveRanges;
    }

    KRYS_NODISCARD List<RawPtr<NodeIterator>> &NodeIterators() noexcept
    {
      return _nodeIterators;
    }

    KRYS_NODISCARD const List<RawPtr<NodeIterator>> &NodeIterators() const noexcept
    {
      return _nodeIterators;
    }

    /// @see https://dom.spec.whatwg.org/#get-the-parent
    KRYS_NODISCARD RawPtr<EventTarget> GetParent(Event &event) const noexcept override
    {
      // SPEC-VIOLATION(HTML, DOM): We don't have global objects so we'll always return nullptr here.
      // A document’s get the parent algorithm, given an event, returns null if event’s type
      // attribute value is "load" or document does not have a browsing context; otherwise the document’s
      // relevant global object.

      return nullptr;
    }

    KRYS_NODISCARD bool IsHTMLDocument() const noexcept
    {
      return _documentType == Type::HTML;
    }

    KRYS_NODISCARD bool IsXMLDocument() const noexcept
    {
      return _documentType == Type::HTML;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(Krys::HTML::Document)
  static bool IsType(const Krys::HTML::Node &node) noexcept
  {
    return node.IsDocumentNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
