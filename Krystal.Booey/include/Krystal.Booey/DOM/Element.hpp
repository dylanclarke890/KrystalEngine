#pragma once

#include "Krystal.Booey/DOM/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Dicts/ShadowRootInit.hpp"
#include "Krystal.Booey/DOM/DOMRect.hpp"
#include "Krystal.Booey/DOM/DOMRectList.hpp"
#include "Krystal.Booey/DOM/DOMTokenList.hpp"
#include "Krystal.Booey/DOM/Enums/CustomElementState.hpp"
#include "Krystal.Booey/DOM/Enums/InsertAdjacentWhere.hpp"
#include "Krystal.Booey/DOM/FocusOptions.hpp"
#include "Krystal.Booey/DOM/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/Types/NodeOrString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Graphics/FloatRect.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Booey/Page/ScrollOptions.hpp"
#include "Krystal.Booey/Platform/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Rendering/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  enum class PseudoElementType : uint8;
  class RenderElement;
  class RenderStyle;
  class RenderTreeUpdater;
}

namespace krys::boo
{
  class PlatformKeyboardEvent;
  class PlatformMouseEvent;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  struct PseudoElementIdentifier;
  class Resolver;
}

namespace krys::boo::html
{
  class CustomElementRegistry;
  class HTMLElementStack;
  class HTMLSlotElement;
  class HTMLTreeBuilder;
}

namespace krys::boo::dom
{
  class ElementRareData;
  class HTMLCollection;
  class NodeList;
  class PseudoElement;
  class ShadowRoot;

  /// @see https://dom.spec.whatwg.org/#interface-element
  class Element : public ContainerNode
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Element);

    friend class ElementFactory;
    friend class DocumentAlgorithms;
    friend class ElementAlgorithms;
    friend class ExtensibilityHooks;
    friend class SlotAlgorithms;
    friend class HTMLCollectionAlgorithms;
    friend class HTMLStackItem;
    friend class Node;
    friend class NodeAlgorithms;
    friend class NamedNodeMap;
    friend class TreeQueries;

    friend class html::HTMLElementStack;
    friend class html::HTMLTreeBuilder;

    friend class ::krys::boo::RenderTreeUpdater;

  private:
    QualifiedName _qualifiedName;
    RefPtr<ShadowRoot> _shadowRoot;
    RefPtr<html::CustomElementRegistry> _customElementRegistry;
    UniquePtr<ElementRareData> _elementRareData;
    UniquePtr<NamedNodeMap> _namedNodeMap;
    UniquePtr<DOMTokenList> _domTokenList;
    List<Ref<Attr>> _attributes;
    DOMStringAtom _is {DOMStringAtom::Null()};
    CustomElementState _customElementState {CustomElementState::Uncustomized};

    /// @see https://dom.spec.whatwg.org/#slotable-name
    DOMString _slottableName;
    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    RefPtr<html::HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slotable-manual-slot-assignment
    WeakPtr<html::HTMLSlotElement> _manuallyAssignedSlot;

  protected:
    Element(Document &document, const QualifiedName &name, NodeFlags nodeFlags) noexcept;

  public:
    ~Element() noexcept override;

#pragma region Element - https://dom.spec.whatwg.org/#element

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD DOMStringAtom NamespaceURI() const noexcept
    {
      return _qualifiedName.NamespaceURI();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD DOMStringAtom Prefix() const noexcept
    {
      return _qualifiedName.NamespacePrefix();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD DOMStringAtom LocalName() const noexcept
    {
      return _qualifiedName.LocalName();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-tagname
    KRYS_NODISCARD DOMString TagName() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-id
    void Id(DOMString &&id) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-id
    KRYS_NODISCARD DOMString Id() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classname
    void ClassName(DOMString &&className) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classname
    KRYS_NODISCARD DOMString ClassName() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classlist
    KRYS_NODISCARD DOMTokenList &ClassList() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-slot
    void Slot(DOMString &&slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-slot
    KRYS_NODISCARD DOMString Slot() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattributes
    KRYS_NODISCARD bool HasAttributes() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-attributes
    KRYS_NODISCARD NamedNodeMap &Attributes() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenames
    KRYS_NODISCARD List<DOMString> GetAttributeNames() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattribute
    KRYS_NODISCARD Maybe<DOMString> GetAttribute(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributens
    KRYS_NODISCARD Maybe<DOMString> GetAttributeNS(DOMStringAtom namespaceURI,
                                                   DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattribute
    ExceptionOr<void> SetAttribute(DOMStringAtom qualifiedName, DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributens
    ExceptionOr<void> SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                     DOMString &&value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattribute
    void RemoveAttribute(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattributens
    void RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-toggleattribute
    ExceptionOr<bool> ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattribute
    KRYS_NODISCARD bool HasAttribute(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattributens
    KRYS_NODISCARD bool HasAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenode
    KRYS_NODISCARD Attr *GetAttributeNode(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenodens
    KRYS_NODISCARD Attr *GetAttributeNodeNS(DOMStringAtom namespaceURI,
                                            DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributenode
    ExceptionOr<RefPtr<Attr>> SetAttributeNode(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributenodens
    ExceptionOr<RefPtr<Attr>> SetAttributeNodeNS(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattributenode
    ExceptionOr<Ref<Attr>> RemoveAttributeNode(Attr &attr) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-attachshadow
    ExceptionOr<Ref<ShadowRoot>> AttachShadow(const ShadowRootInit &init) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-shadowroot
    KRYS_NODISCARD RefPtr<ShadowRoot> ShadowRoot() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-customelementregistry
    KRYS_NODISCARD RefPtr<html::CustomElementRegistry> CustomElementRegistry() const noexcept
    {
      return _customElementRegistry;
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-closest
    KRYS_NODISCARD ExceptionOr<RefPtr<Element>> Closest(DOMStringView selectors) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-matches
    KRYS_NODISCARD ExceptionOr<bool> Matches(DOMStringView selectors) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-webkitmatchesselector
    KRYS_NODISCARD ExceptionOr<bool> WebkitMatchesSelector(DOMStringView selectors) const noexcept
    {
      return Matches(selectors);
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbytagname
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagName(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbytagnamens
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                              DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbyclassname
    KRYS_NODISCARD Ref<HTMLCollection> GetElementsByClassName(DOMStringAtom classNames) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-insertadjacentelement
    ExceptionOr<Element *> InsertAdjacentElement(InsertAdjacentWhere where, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-insertadjacenttext
    ExceptionOr<void> InsertAdjacentText(InsertAdjacentWhere where, DOMString &&data) noexcept;

#pragma endregion

#pragma region Element (CSSOM View) - https://drafts.csswg.org/cssom-view/#extension-to-the-element-interface

    /// @brief The bounding boxes of the element's boxes, in client coordinates.
    KRYS_NODISCARD Ref<DOMRectList> GetClientRects() noexcept;
    /// @brief The union of the element's boxes, in client coordinates.
    KRYS_NODISCARD Ref<DOMRect> GetBoundingClientRect() noexcept;
    KRYS_NODISCARD FloatRect BoundingClientRect() noexcept;
    /// @brief The element's renderer and the union of its boxes in absolute coordinates, without updating
    /// layout; null if it has no boxes.
    KRYS_NODISCARD Maybe<Pair<RenderElement *, FloatRect>> BoundingAbsoluteRectWithoutLayout() const noexcept;

    /// @brief Scrolls the element's ancestors so the element is aligned as @p options ask.
    void ScrollIntoView(const ScrollIntoViewOptions &options = {}) noexcept;
    /// @brief Scrolls the element's ancestors so its start (or, if @p alignToTop is false, its end) is
    /// aligned with theirs.
    void ScrollIntoView(bool alignToTop) noexcept;

    void Scroll(const ScrollToOptions &options = {}) noexcept;
    void Scroll(double x, double y) noexcept;
    void ScrollTo(const ScrollToOptions &options = {}) noexcept;
    void ScrollTo(double x, double y) noexcept;
    void ScrollBy(const ScrollToOptions &options = {}) noexcept;
    void ScrollBy(double x, double y) noexcept;

    KRYS_NODISCARD int32 ScrollTop() noexcept;
    void SetScrollTop(int32 newTop) noexcept;
    KRYS_NODISCARD int32 ScrollLeft() noexcept;
    void SetScrollLeft(int32 newLeft) noexcept;
    KRYS_NODISCARD int32 ScrollWidth() noexcept;
    KRYS_NODISCARD int32 ScrollHeight() noexcept;
    KRYS_NODISCARD int32 ClientTop() noexcept;
    KRYS_NODISCARD int32 ClientLeft() noexcept;
    KRYS_NODISCARD int32 ClientWidth() noexcept;
    KRYS_NODISCARD int32 ClientHeight() noexcept;

    /// @brief The nearest positioned (or table) ancestor the offset values are relative to, retargeted out of
    /// shadow trees the element isn't in.
    /// @see https://drafts.csswg.org/cssom-view/#extensions-to-the-htmlelement-interface
    KRYS_NODISCARD Element *OffsetParent() noexcept;
    /// @brief The offset parent, without retargeting it out of shadow trees.
    KRYS_NODISCARD Element *OffsetParentIgnoringShadowTrees() noexcept;
    KRYS_NODISCARD int32 OffsetTop() noexcept;
    KRYS_NODISCARD int32 OffsetLeft() noexcept;
    KRYS_NODISCARD int32 OffsetWidth() noexcept;
    KRYS_NODISCARD int32 OffsetHeight() noexcept;

#pragma endregion

#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-nodename
    KRYS_NODISCARD DOMString NodeName() const noexcept final;

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    KRYS_NODISCARD Maybe<DOMString> TextContent() const noexcept final;

    /// @see https://dom.spec.whatwg.org/#dom-node-textcontent
    ExceptionOr<void> TextContent(DOMString &&value) noexcept final;

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

#pragma region NonDocumentTypeChildNode Mixin - https://dom.spec.whatwg.org/#interface-nondocumenttypechildnode

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<const Element> PreviousElementSibling() const noexcept;

    /// @brief Returns the first preceding sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-previouselementsibling
    KRYS_NODISCARD RefPtr<Element> PreviousElementSibling() noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<const Element> NextElementSibling() const noexcept;

    /// @brief Returns the first following sibling that is an element; otherwise null.
    /// @see https://dom.spec.whatwg.org/#dom-nondocumenttypechildnode-nextelementsibling
    KRYS_NODISCARD RefPtr<Element> NextElementSibling() noexcept;

#pragma endregion

#pragma region ChildNode Mixin - https://dom.spec.whatwg.org/#childnode

    /// @brief Inserts `nodes` just before `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-before
    ExceptionOr<void> Before(const List<NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    ExceptionOr<void> After(const List<NodeOrString> &nodes) noexcept;

    /// @brief Replaces `this` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    ExceptionOr<void> ReplaceWith(const List<NodeOrString> &nodes) noexcept;

    /// @brief Removes `this`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region Slottable Mixin - https://dom.spec.whatwg.org/#slotable

    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD RefPtr<html::HTMLSlotElement> AssignedSlot() noexcept;

#pragma endregion

    /// @brief Whether the parser has finished inserting this element's children. Elements not created by the
    /// parser have always finished.
    KRYS_NODISCARD bool IsFinishedParsingChildren() const noexcept
    {
      return !HasNodeFlag(NodeFlags::IsParsingChildren);
    }

    KRYS_NODISCARD const QualifiedName &TagQualifiedName() const noexcept
    {
      return _qualifiedName;
    }

    /// @brief The attribute list, synchronised first.
    KRYS_NODISCARD const List<Ref<Attr>> &AttributeList() const noexcept
    {
      SynchronizeAllAttributes();
      return _attributes;
    }

    /// @brief Whether @p className is one of the element's classes (ASCII case-insensitively in quirks mode,
    /// where @p className must be lowercase).
    KRYS_NODISCARD bool HasClassName(DOMStringView className) const noexcept;

    /// @brief The element's classes in order, without duplicates, ASCII-lowercased in quirks mode.
    KRYS_NODISCARD List<DOMStringAtom> ClassNames() const noexcept;

    /// @brief The classes a `class` attribute with @p value would give the element (none for null), as
    /// `ClassNames` gives them.
    KRYS_NODISCARD List<DOMStringAtom> ClassNamesForValue(Maybe<DOMStringView> value) const noexcept;

    /// @brief The element's ID, ASCII-lowercased in quirks mode; empty if it has none.
    KRYS_NODISCARD DOMString IdForStyleResolution() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-state
    KRYS_NODISCARD CustomElementState GetCustomElementState() const noexcept
    {
      return _customElementState;
    }

    /// @brief Writes attributes that are updated lazily (the `style` attribute after a change to the inline
    /// style) back to the attribute list. Everything that reads the attribute list calls this first.
    void SynchronizeAllAttributes() const noexcept
    {
      if (HasNodeFlag(NodeFlags::StyleAttributeIsDirty)) KRYS_UNLIKELY
      {
        SynchronizeStyleAttribute();
      }
    }

    /// @brief Whether the element is a replaced element (its content is outside the scope of CSS), given its
    /// style if there is one.
    /// @see https://html.spec.whatwg.org/#replaced-elements
    KRYS_NODISCARD virtual bool IsReplaced(const RenderStyle * = nullptr) const noexcept
    {
      return false;
    }

    /// @brief Whether the element is relevant to the user, for `content-visibility: auto`.
    /// @see https://drafts.csswg.org/css-contain/#relevant-to-the-user
    KRYS_NODISCARD bool IsRelevantToUser() const noexcept
    {
      // FIXME: Content relevancy (on screen, focused, selected, in the top layer) is never computed, so no
      // element is relevant to the user.
      return false;
    }

    /// @brief The base value of a `random()` of the element (or of one of its pseudo-elements) that isn't
    /// shared between elements.
    KRYS_NODISCARD double
      LookupCSSRandomBaseValue(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                               const css::calc::RandomCachingKey &key) const noexcept;

    KRYS_NODISCARD bool HasRandomCachingKeyMap() const noexcept;

#pragma region Style

    enum class ResolveComputedStyleMode : uint8
    {
      Normal,
      RenderedOnly,
      Editability
    };

    using Node::ComputedStyle;

    /// @brief The computed style of the element or one of its pseudo-elements, resolved if it isn't known.
    /// Null if the element isn't connected.
    KRYS_NODISCARD const RenderStyle *
      ComputedStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept override;

    /// @brief Resolves the computed style of the element and of its composed tree ancestors whose styles are
    /// unknown or out of date, outside style resolution.
    /// @return Null in `RenderedOnly` mode if the element or an ancestor has `display: none`.
    KRYS_NODISCARD const RenderStyle *
      ResolveComputedStyle(ResolveComputedStyleMode mode = ResolveComputedStyleMode::Normal) noexcept;

    /// @brief The element's computed style if it's known: the one resolved outside style resolution, or else
    /// the one it's rendered with.
    KRYS_NODISCARD const RenderStyle *ExistingComputedStyle() const noexcept;

    /// @brief The style the element is rendered with, including with `display: contents`.
    KRYS_NODISCARD const RenderStyle *RenderOrDisplayContentsStyle() const noexcept;
    KRYS_NODISCARD const RenderStyle *RenderOrDisplayContentsStyle(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    /// @brief Drops the computed styles of the element and its descendants.
    void ResetComputedStyle() noexcept;
    void ResetStyleRelations() noexcept;
    void ResetChildStyleRelations() noexcept;
    void ResetAllDescendantStyleRelations() noexcept;

    KRYS_NODISCARD bool HasDisplayContents() const noexcept;
    KRYS_NODISCARD bool HasDisplayNone() const noexcept;
    void StoreDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept;
    void ClearDisplayContentsOrNoneStyle() noexcept;

    /// @brief The renderer the element gets with @p style, or null if it gets none.
    // FIXME: The insertion position in the render tree, and the overrides that need renderer classes (images,
    // line breaks, form controls, media, frames, MathML, SVG, ...).
    KRYS_NODISCARD virtual RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept;

    /// @brief Whether the element would have a renderer with @p style.
    // FIXME: The overrides (form controls, frame sets, SVG elements, ...) come with the render tree.
    KRYS_NODISCARD virtual bool RendererIsNeeded(const RenderStyle &style) noexcept;

    /// @brief Whether @p child, a child of the element in the composed tree, gets a renderer when the element
    /// has one.
    // FIXME: The overrides (details, media, SVG, MathML, ...) come with their renderers.
    KRYS_NODISCARD virtual bool ChildShouldCreateRenderer(const Node &) const noexcept
    {
      return true;
    }

    /// @brief Called by the render tree updater after the element's renderer (and its descendants') was
    /// created or replaced.
    virtual void DidAttachRenderers() noexcept
    {
    }

    /// @brief The URL of the image the element shows: its `src` attribute, or null if it has none.
    KRYS_NODISCARD virtual Maybe<DOMString> ImageSourceURL() const noexcept;

    /// @brief The element's renderer, or null if it isn't rendered.
    KRYS_NODISCARD RenderElement *Renderer() const noexcept;

    /// @brief The element's `::before` pseudo-element, or null if it has none.
    KRYS_NODISCARD PseudoElement *BeforePseudoElement() const noexcept;

    /// @brief The scroll position of the element's layer, kept while the element has no renderer.
    KRYS_NODISCARD IntPoint SavedLayerScrollPosition() const noexcept;
    void SetSavedLayerScrollPosition(const IntPoint &position) noexcept;

    /// @brief The element's `::after` pseudo-element, or null if it has none.
    KRYS_NODISCARD PseudoElement *AfterPseudoElement() const noexcept;

    /// @brief The element's `::before` or `::after` pseudo-element (@p type), created if it doesn't exist.
    PseudoElement &EnsurePseudoElement(PseudoElementType type) noexcept;

    /// @brief The pseudo-element @p pseudoElementIdentifier if it's a `::before` or `::after` that exists.
    KRYS_NODISCARD RefPtr<PseudoElement>
      PseudoElementIfExists(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept;
    KRYS_NODISCARD RefPtr<const PseudoElement>
      PseudoElementIfExists(const style::PseudoElementIdentifier &pseudoElementIdentifier) const noexcept;

    void ClearBeforePseudoElement() noexcept;
    void ClearAfterPseudoElement() noexcept;

    void InvalidateStyle() noexcept;
    void InvalidateStyleAndLayerComposition() noexcept;
    void InvalidateStyleForSubtree() noexcept;
    void InvalidateStyleAndRenderersForSubtree() noexcept;
    void InvalidateRenderer() noexcept;
    void InvalidateStyleInternal() noexcept;
    void InvalidateStyleForSubtreeInternal() noexcept;
    void InvalidateForQueryContainerSizeChange() noexcept;
    void InvalidateForResumingQueryContainerResolution() noexcept;

    /// @brief Whether a change to the element needs to invalidate style: false when the element isn't
    /// connected or its style (or the whole document's) will be resolved again anyway.
    KRYS_NODISCARD bool NeedsStyleInvalidation() const noexcept;

    /// @brief The style resolver of the element's tree: its shadow root's, or its document's.
    KRYS_NODISCARD style::Resolver &StyleResolver() noexcept;

    KRYS_NODISCARD bool NeedsUpdateQueryContainerDependentStyle() const noexcept
    {
      return HasNodeFlag(NodeFlags::NeedsUpdateQueryContainerDependentStyle);
    }

    void ClearNeedsUpdateQueryContainerDependentStyle() noexcept
    {
      ClearNodeFlag(NodeFlags::NeedsUpdateQueryContainerDependentStyle);
    }

    KRYS_NODISCARD bool StyleAffectedByEmpty() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::StyleAffectedByEmpty);
    }

    KRYS_NODISCARD bool StyleIsAffectedByPreviousSibling() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::StyleIsAffectedByPreviousSibling);
    }

    KRYS_NODISCARD bool DescendantsAffectedByPreviousSibling() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::DescendantsAffectedByPreviousSibling);
    }

    KRYS_NODISCARD bool AffectsNextSiblingElementStyle() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::AffectsNextSiblingElementStyle);
    }

    KRYS_NODISCARD bool ChildrenAffectedByFirstChildRules() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::ChildrenAffectedByFirstChildRules);
    }

    KRYS_NODISCARD bool ChildrenAffectedByLastChildRules() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::ChildrenAffectedByLastChildRules);
    }

    KRYS_NODISCARD bool ChildrenAffectedByForwardPositionalRules() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::ChildrenAffectedByForwardPositionalRules);
    }

    KRYS_NODISCARD bool DescendantsAffectedByForwardPositionalRules() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::DescendantsAffectedByForwardPositionalRules);
    }

    KRYS_NODISCARD bool ChildrenAffectedByBackwardPositionalRules() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::ChildrenAffectedByBackwardPositionalRules);
    }

    KRYS_NODISCARD bool DescendantsAffectedByBackwardPositionalRules() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::DescendantsAffectedByBackwardPositionalRules);
    }

    KRYS_NODISCARD bool AffectedByHasWithPositionalPseudoClass() const noexcept
    {
      return HasStyleFlag(NodeStyleFlags::AffectedByHasWithPositionalPseudoClass);
    }

    /// @brief Whether selector matching set a flag on the element while matching its children.
    KRYS_NODISCARD bool HasFlagsSetDuringStylingOfChildren() const noexcept;

    /// @brief The element's index among its parent's element children, plus one, as cached by selector
    /// matching; zero if it isn't known.
    KRYS_NODISCARD uint32 ChildIndex() const noexcept;

    void SetStyleAffectedByEmpty() noexcept
    {
      SetStyleFlag(NodeStyleFlags::StyleAffectedByEmpty);
    }

    void SetStyleIsAffectedByPreviousSibling() noexcept
    {
      SetStyleFlag(NodeStyleFlags::StyleIsAffectedByPreviousSibling);
    }

    void SetDescendantsAffectedByPreviousSibling() noexcept
    {
      SetStyleFlag(NodeStyleFlags::DescendantsAffectedByPreviousSibling);
    }

    void SetAffectsNextSiblingElementStyle() noexcept
    {
      SetStyleFlag(NodeStyleFlags::AffectsNextSiblingElementStyle);
    }

    void SetChildrenAffectedByFirstChildRules() noexcept
    {
      SetStyleFlag(NodeStyleFlags::ChildrenAffectedByFirstChildRules);
    }

    void SetChildrenAffectedByLastChildRules() noexcept
    {
      SetStyleFlag(NodeStyleFlags::ChildrenAffectedByLastChildRules);
    }

    void SetChildrenAffectedByForwardPositionalRules() noexcept
    {
      SetStyleFlag(NodeStyleFlags::ChildrenAffectedByForwardPositionalRules);
    }

    void SetDescendantsAffectedByForwardPositionalRules() noexcept
    {
      SetStyleFlag(NodeStyleFlags::DescendantsAffectedByForwardPositionalRules);
    }

    void SetChildrenAffectedByBackwardPositionalRules() noexcept
    {
      SetStyleFlag(NodeStyleFlags::ChildrenAffectedByBackwardPositionalRules);
    }

    void SetDescendantsAffectedByBackwardPositionalRules() noexcept
    {
      SetStyleFlag(NodeStyleFlags::DescendantsAffectedByBackwardPositionalRules);
    }

    void SetAffectedByHasWithPositionalPseudoClass() noexcept
    {
      SetStyleFlag(NodeStyleFlags::AffectedByHasWithPositionalPseudoClass);
    }

    void SetChildIndex(uint32 index) noexcept;

#pragma endregion

#pragma region Mouse Events

    enum class EventIsDispatched : bool
    {
      No,
      Yes
    };

    enum class EventIsDefaultPrevented : bool
    {
      No,
      Yes
    };

    struct DispatchMouseEventResult
    {
      EventIsDispatched Dispatched;
      EventIsDefaultPrevented DefaultPrevented;
    };

    /// @brief Dispatches a trusted mouse event of type @p eventType from @p platformEvent at this element; a
    /// `click` with a @p detail of 2 is followed by a `dblclick`.
    /// @return Whether the event was dispatched without being cancelled, and whether it was cancelled.
    DispatchMouseEventResult DispatchMouseEvent(const PlatformMouseEvent &platformEvent,
                                                DOMStringAtom eventType, int32 detail = 0,
                                                Element *relatedTarget = nullptr) noexcept;

#pragma endregion

#pragma region Wheel Events

    /// @brief Dispatches a trusted `wheel` event from @p platformEvent at this element, then, unless it's
    /// cancelled, scrolls (the event's default action).
    /// @param processing Gains whether the event reached the DOM, and whether it was cancelled or handled.
    /// @return Whether the event was neither cancelled nor handled.
    bool DispatchWheelEvent(const PlatformWheelEvent &platformEvent, EnumSet<EventHandling> &processing,
                            bool isCancelable = true) noexcept;

#pragma endregion

#pragma region Keyboard Events

    /// @brief Dispatches a trusted keyboard event from @p platformEvent (a char or key up) at this element.
    /// @return Whether the event was neither cancelled nor handled.
    bool DispatchKeyEvent(const PlatformKeyboardEvent &platformEvent) noexcept;

#pragma endregion

#pragma region User Action State

    /// @brief Whether the element has user action state in its document (`Document::UserActionElements`).
    KRYS_NODISCARD bool IsUserActionElement() const noexcept
    {
      return HasNodeFlag(NodeFlags::IsUserActionElement);
    }

    void SetUserActionElement(bool flag) noexcept
    {
      flag ? SetNodeFlag(NodeFlags::IsUserActionElement) : ClearNodeFlag(NodeFlags::IsUserActionElement);
    }

    /// @brief Whether the element is in the chain of elements that became `:active` when the pointer was
    /// pressed.
    KRYS_NODISCARD bool IsInActiveChain() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementInActiveChain();
    }

    KRYS_NODISCARD bool Active() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementActive();
    }

    KRYS_NODISCARD bool Hovered() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementHovered();
    }

    KRYS_NODISCARD bool Focused() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementFocused();
    }

    KRYS_NODISCARD bool HasFocusVisible() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementHasFocusVisible();
    }

    KRYS_NODISCARD bool HasFocusWithin() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementHasFocusWithin();
    }

    void SetActive(bool value,
                   style::InvalidationScope invalidationScope = style::InvalidationScope::All) noexcept;
    void SetHovered(bool value, style::InvalidationScope invalidationScope = style::InvalidationScope::All,
                    HitTestRequest request = {}) noexcept;
    /// @brief Sets the element's focus, and the focus within of its ancestors in the composed tree.
    void SetFocus(bool value, FocusVisibility visibility = FocusVisibility::Invisible) noexcept;
    void SetHasFocusVisible(bool value) noexcept;
    void SetHasFocusWithin(bool value) noexcept;
    /// @brief Gives the element's composed tree ancestors focus within while its focusability is checked.
    void SetHasTentativeFocus(bool value) noexcept;

    /// @brief Clears the element's hover and active state, moving the document's hovered element and active
    /// chain to its ancestors, before its renderer goes.
    void ClearHoverAndActiveStatusBeforeDetachingRenderer() noexcept;

#pragma endregion

  private:
    KRYS_NODISCARD ElementRareData &EnsureElementRareData() noexcept;

    KRYS_NODISCARD bool IsUserActionElementInActiveChain() const noexcept;
    KRYS_NODISCARD bool IsUserActionElementActive() const noexcept;
    KRYS_NODISCARD bool IsUserActionElementFocused() const noexcept;
    KRYS_NODISCARD bool IsUserActionElementHovered() const noexcept;
    KRYS_NODISCARD bool IsUserActionElementHasFocusVisible() const noexcept;
    KRYS_NODISCARD bool IsUserActionElementHasFocusWithin() const noexcept;

    KRYS_NODISCARD const RenderStyle &
      ResolvePseudoElementStyle(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept;

  protected:
    /// @brief Sets an attribute in the null namespace to `value` without running the attribute change steps
    /// or queueing a mutation record, for lazily synchronised attributes.
    void SetSynchronizedLazyAttribute(DOMStringAtom localName, DOMString &&value) noexcept;

#pragma region Parser Hooks

    /// @brief Called by the parser when it pushes this element onto the stack of open elements.
    void BeginParsingChildren() noexcept
    {
      SetNodeFlag(NodeFlags::IsParsingChildren);
    }

    /// @brief Called by the parser when it pops this element off the stack of open elements. Overrides must
    /// call the base implementation.
    /// @see https://html.spec.whatwg.org/#stack-of-open-elements
    virtual void FinishParsingChildren() noexcept;

#pragma endregion

#pragma region Extension Hooks

    /// @brief The attribute change steps, run whenever one of the element's attributes is set, changed or
    /// removed. Overrides must call the base implementation.
    /// @param oldValue The previous value, or null if the attribute was added.
    /// @param value The new value, or null if the attribute was removed.
    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change-ext
    virtual void OnAttributeChanged(DOMStringAtom localName, Maybe<DOMStringView> oldValue,
                                    Maybe<DOMStringView> value, DOMStringAtom namespaceURI) noexcept;

#pragma endregion

  private:
    void SynchronizeStyleAttribute() const noexcept;
  };

  /// @brief Invalidates the style of @p sibling and its following siblings that sibling combinators make
  /// depend on their previous siblings, up to the first one that doesn't affect the next.
  void InvalidateForSiblingCombinators(Element *sibling) noexcept;
}

namespace krys::boo::dom
{
  /// @brief Whether an element with @p style is in the top layer, or is a `::backdrop`.
  KRYS_NODISCARD bool IsInTopLayerOrBackdrop(const RenderStyle &style, const Element *element) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::Element)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
