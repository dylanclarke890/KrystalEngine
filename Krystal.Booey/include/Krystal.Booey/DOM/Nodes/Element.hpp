#pragma once

#include "Krystal.Booey/Animation/ElementAnimationRareData.hpp"
#include "Krystal.Booey/Animation/GetAnimationsOptions.hpp"
#include "Krystal.Booey/Animation/Keyframes/KeyframeInput.hpp"
#include "Krystal.Booey/DOM/ContentVisibility/ContentRelevancy.hpp"
#include "Krystal.Booey/DOM/Events/SimulatedClick.hpp"
#include "Krystal.Booey/DOM/Geometry/DOMRect.hpp"
#include "Krystal.Booey/DOM/Mixins/ARIAMixin.hpp"
#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Nodes/FocusOptions.hpp"
#include "Krystal.Booey/DOM/Nodes/NamedNodeMap.hpp"
#include "Krystal.Booey/DOM/Nodes/PointerLockOptions.hpp"
#include "Krystal.Booey/DOM/Types/NodeOrString.hpp"
#include "Krystal.Booey/DOM/Types/QualifiedName.hpp"
#include "Krystal.Booey/Input/PlatformWheelEvent.hpp"
#include "Krystal.Booey/Page/Scrolling/ScrollOptions.hpp"
#include "Krystal.Booey/Platform/Scrolling/ScrollTypes.hpp"
#include "Krystal.Booey/Rendering/HitTesting/HitTestRequest.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Geometry/Point.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Pair.hpp"

namespace krys::i18n
{
  class Locale;
}

namespace krys::net
{
  class Url;
}

namespace krys::boo::css::calc
{
  struct RandomCachingKey;
}

namespace krys::boo::style
{
  struct PseudoElementIdentifier;
  struct ResolutionContext;
  class Resolver;
  struct UnadjustedStyle;
  enum class Change : uint8;
}

namespace krys::boo::html
{
  class CustomElement;
  class CustomElementInterface;
  class CustomElementReactionQueue;
  class CustomElementRegistry;
  class CustomStateSet;
  class OpenElementStack;
  class TreeBuilder;

  struct SetHTMLOptions;
  struct SetHTMLUnsafeOptions;
}

namespace krys::boo::xml
{
  class XMLDocumentParser;
}

namespace krys::boo::dom
{
  class DOMRectList;
  class DOMTokenList;
  class DatasetDOMStringMap;
  class DocumentAlgorithms;
  class ElementAlgorithms;
  class ElementFactory;
  class ElementRareData;
  class ExtensibilityHooks;
  class HTMLCollection;
  class NamedNodeMap;
  class NodeAlgorithms;
  class NodeList;
  class PopoverData;
  class SlotAlgorithms;
  class TreeQueries;

  struct GetHTMLOptions;
  struct ShadowRootInit;

  enum class ParserContentPolicy : uint8;
  enum class PopoverState : uint8;

  /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-state
  enum class CustomElementState : uint8
  {
    Undefined,
    Failed,
    Uncustomized,
    Precustomized,
    Custom,
  };

  consteval CustomElementState MaxEnumValue(CustomElementState) noexcept
  {
    return CustomElementState::Custom;
  }

  /// @brief The state of a button's `command` attribute: the command it sends its `commandfor` element.
  /// @see https://html.spec.whatwg.org/#attr-button-command
  enum class CommandType : uint8
  {
    Invalid,

    /// @brief A command starting with `--`, which only the element's `command` listeners carry out.
    Custom,

    TogglePopover,
    HidePopover,
    ShowPopover,

    ShowModal,
    Close,
    RequestClose,
  };

  consteval CommandType MaxEnumValue(CommandType) noexcept
  {
    return CommandType::RequestClose;
  }

  /// @see https://dom.spec.whatwg.org/#insert-adjacent
  enum class InsertAdjacentWhere : uint8
  {
    BeforeBegin,
    AfterBegin,
    BeforeEnd,
    AfterEnd,
  };

  consteval InsertAdjacentWhere MaxEnumValue(InsertAdjacentWhere) noexcept
  {
    return InsertAdjacentWhere::AfterEnd;
  }
}

namespace krys::boo
{
  class HTMLButtonElement;
  class HTMLElement;
  class HTMLSlotElement;
  class LayoutUnit;
  class PlatformKeyboardEvent;
  class PlatformMouseEvent;
  class PseudoElement;
  class RenderElement;
  class RenderStyle;
  class RenderTreeUpdater;
  class ShadowRoot;
  class StylePropertyMap;
  class StylePropertyMapReadOnly;

  struct IntersectionObserverData;
  struct KeyframeAnimationOptions;
  struct ResizeObserverData;

  enum class PseudoElementType : uint8;

  /// @brief A node with a qualified name and attributes.
  /// @see https://dom.spec.whatwg.org/#interface-element
  /// @see https://dom.spec.whatwg.org/#concept-element
  class KRYS_EMPTY_BASE_CLASS Element : public ContainerNode, public dom::mixins::ARIAMixin<Element>
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(Element);

    friend class dom::ElementFactory;
    friend class dom::DocumentAlgorithms;
    friend class dom::ElementAlgorithms;
    friend class dom::ExtensibilityHooks;
    friend class dom::SlotAlgorithms;
    friend class HTMLCollectionAlgorithms;
    friend class StackItem;
    friend class Node;
    friend class dom::NodeAlgorithms;
    friend class dom::NamedNodeMap;
    friend class dom::TreeQueries;

    friend class html::OpenElementStack;
    friend class html::TreeBuilder;
    friend class xml::XMLDocumentParser;

    friend class ::krys::boo::RenderTreeUpdater;

  private:
    /// @brief The element's namespace, namespace prefix and local name.
    /// @see https://dom.spec.whatwg.org/#concept-element-qualified-name
    /// @see https://dom.spec.whatwg.org/#concept-element-namespace
    /// @see https://dom.spec.whatwg.org/#concept-element-namespace-prefix
    /// @see https://dom.spec.whatwg.org/#concept-element-local-name
    QualifiedName _qualifiedName;
    /// @see https://dom.spec.whatwg.org/#concept-element-shadow-root
    RefPtr<ShadowRoot> _shadowRoot;
    /// @see https://dom.spec.whatwg.org/#element-custom-element-registry
    RefPtr<html::CustomElementRegistry> _customElementRegistry;
    /// @brief The associated `NamedNodeMap`, created on first use.
    UniquePtr<dom::NamedNodeMap> _namedNodeMap;
    /// @brief The `classList`, created on first use.
    UniquePtr<dom::DOMTokenList> _domTokenList;
    /// @see https://dom.spec.whatwg.org/#concept-element-attribute
    List<Ref<Attr>> _attributes;
    /// @brief Kept as given, but ignored: customized built-in elements aren't supported.
    /// @see https://dom.spec.whatwg.org/#concept-element-is-value
    DOMStringAtom _is {null};
    /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-state
    dom::CustomElementState _customElementState {dom::CustomElementState::Uncustomized};

    /// @see https://dom.spec.whatwg.org/#slotable-name
    DOMString _slottableName;
    /// @see https://dom.spec.whatwg.org/#slotable-assigned-slot
    WeakPtr<HTMLSlotElement> _assignedSlot;
    /// @see https://dom.spec.whatwg.org/#slottable-manual-slot-assignment
    WeakPtr<HTMLSlotElement> _manuallyAssignedSlot;

  protected:
    Element(Document &document, const QualifiedName &name, dom::NodeFlags nodeFlags) noexcept;

  public:
    /// @brief False for elements with `role="img"`, whose content editing ignores.
    KRYS_NODISCARD bool CanContainRangeEndPoint() const noexcept override;

    ~Element() noexcept override;

    void OnAdopt(Document &oldDocument) noexcept override;

#pragma region Element - https://dom.spec.whatwg.org/#element

    /// @see https://dom.spec.whatwg.org/#dom-element-namespaceuri
    KRYS_NODISCARD const DOMStringAtom &NamespaceURI() const noexcept KRYS_LIFETIME_BOUND
    {
      return _qualifiedName.NamespaceURI();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-prefix
    KRYS_NODISCARD const DOMStringAtom &Prefix() const noexcept KRYS_LIFETIME_BOUND
    {
      return _qualifiedName.NamespacePrefix();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-localname
    KRYS_NODISCARD const DOMStringAtom &LocalName() const noexcept KRYS_LIFETIME_BOUND
    {
      return _qualifiedName.LocalName();
    }

    /// @brief The element's HTML-uppercased qualified name.
    /// @see https://dom.spec.whatwg.org/#dom-element-tagname
    /// @see https://dom.spec.whatwg.org/#element-html-uppercased-qualified-name
    KRYS_NODISCARD DOMString TagName() const noexcept;

    /// @brief Reflects the `id` attribute, as `className` and `slot` reflect theirs.
    /// @see https://dom.spec.whatwg.org/#dom-element-id
    /// @see https://dom.spec.whatwg.org/#concept-reflect
    void Id(DOMString id) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-id
    KRYS_NODISCARD DOMString Id() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classname
    void ClassName(DOMString className) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classname
    KRYS_NODISCARD DOMString ClassName() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-classlist
    KRYS_NODISCARD dom::DOMTokenList &ClassList() noexcept;

    /// @see https://drafts.csswg.org/css-shadow-parts/#dom-element-part
    KRYS_NODISCARD dom::DOMTokenList &Part() noexcept;

    /// @brief The element's `data-*` attributes, by property name.
    /// @see https://html.spec.whatwg.org/#dom-dataset
    KRYS_NODISCARD dom::DatasetDOMStringMap &Dataset() noexcept;

    /// @brief The element's computed style, as a read-only property map.
    /// @see https://drafts.css-houdini.org/css-typed-om/#dom-element-computedstylemap
    KRYS_NODISCARD StylePropertyMapReadOnly &ComputedStyleMap() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-slot
    void Slot(DOMString slot) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-slot
    KRYS_NODISCARD DOMString Slot() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattributes
    KRYS_NODISCARD bool HasAttributes() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-attributes
    KRYS_NODISCARD dom::NamedNodeMap &Attributes() noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributenames
    KRYS_NODISCARD List<DOMString> GetAttributeNames() const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattribute
    KRYS_NODISCARD Maybe<DOMString> GetAttribute(DOMStringAtom qualifiedName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getattributens
    KRYS_NODISCARD Maybe<DOMString> GetAttributeNS(DOMStringAtom namespaceURI,
                                                   DOMStringAtom localName) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattribute
    ExceptionOr<void> SetAttribute(DOMStringAtom qualifiedName, DOMString value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-setattributens
    ExceptionOr<void> SetAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom qualifiedName,
                                     DOMString value) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattribute
    void RemoveAttribute(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-removeattributens
    void RemoveAttributeNS(DOMStringAtom namespaceURI, DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-toggleattribute
    ExceptionOr<bool> ToggleAttribute(DOMStringAtom qualifiedName, const Maybe<bool> &force) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-hasattribute
    /// @see https://dom.spec.whatwg.org/#concept-element-attribute-has
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
    ExceptionOr<Ref<ShadowRoot>> AttachShadow(const dom::ShadowRootInit &init) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-shadowroot
    KRYS_NODISCARD RefPtr<ShadowRoot> ShadowRoot() const noexcept;

    /// @brief The element's user agent shadow root, or null if it has none.
    KRYS_NODISCARD boo::ShadowRoot *UserAgentShadowRoot() const noexcept;

    /// @brief The element's user agent shadow root, attached if it has none.
    boo::ShadowRoot &EnsureUserAgentShadowRoot() noexcept;

    /// @brief Attaches a user agent shadow root to the element, which must have no shadow root.
    boo::ShadowRoot &CreateUserAgentShadowRoot() noexcept;

    /// @brief Builds the element's user agent shadow tree, if the document has it pending
    /// (`Document::AddElementWithPendingUserAgentShadowTreeUpdate`); called before the document's style is
    /// resolved.
    virtual void UpdateUserAgentShadowTree() noexcept
    {
    }

    /// @brief The name of the user agent part the element is (its `useragentpart` attribute), which user
    /// agent style sheets select it by as a pseudo-element of its host; null if it isn't one.
    KRYS_NODISCARD DOMStringAtom UserAgentPart() const noexcept;

    /// @brief Sets the `useragentpart` attribute, which names the part a user agent shadow tree element
    /// plays.
    void SetUserAgentPart(const DOMStringAtom &part) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-customelementregistry
    KRYS_NODISCARD html::CustomElementRegistry *CustomElementRegistry() const noexcept
    {
      return _customElementRegistry.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-element-closest
    KRYS_NODISCARD ExceptionOr<RefPtr<Element>> Closest(DOMStringView selectors) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-matches
    KRYS_NODISCARD ExceptionOr<bool> Matches(DOMStringView selectors) const noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbytagname
    KRYS_NODISCARD Ref<dom::HTMLCollection> GetElementsByTagName(DOMStringAtom qualifiedName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbytagnamens
    KRYS_NODISCARD Ref<dom::HTMLCollection> GetElementsByTagNameNS(DOMStringAtom namespaceURI,
                                                                   DOMStringAtom localName) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-getelementsbyclassname
    KRYS_NODISCARD Ref<dom::HTMLCollection> GetElementsByClassName(DOMStringAtom classNames) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-insertadjacentelement
    ExceptionOr<Element *> InsertAdjacentElement(dom::InsertAdjacentWhere where, Element &element) noexcept;

    /// @see https://dom.spec.whatwg.org/#dom-element-insertadjacenttext
    ExceptionOr<void> InsertAdjacentText(dom::InsertAdjacentWhere where, DOMString data) noexcept;

#pragma endregion

#pragma region Element (DOM Parsing) - https://html.spec.whatwg.org/multipage/dynamic-markup-insertion.html#dom-parsing-and-serialization

    /// @brief Replaces the element's children (a template's contents) with @p html, parsed as HTML in its
    /// context, sanitized by @p options (the default configuration if it gives none) with unsafe content
    /// removed.
    /// @see https://html.spec.whatwg.org/#dom-element-sethtml
    ExceptionOr<void> SetHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> SetHTML(DOMStringView html) noexcept;

    /// @brief Appends @p html, parsed as HTML in the element's context, sanitized by @p options (the default
    /// configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-element-appendhtml
    ExceptionOr<void> AppendHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> AppendHTML(DOMStringView html) noexcept;

    /// @brief Prepends @p html, parsed as HTML in the element's context, sanitized by @p options (the default
    /// configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-element-prependhtml
    ExceptionOr<void> PrependHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> PrependHTML(DOMStringView html) noexcept;

    /// @brief Replaces the element's children (a template's contents) with @p html, parsed as HTML in its
    /// context; sanitized only if @p options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-element-sethtmlunsafe
    ExceptionOr<void> SetHTMLUnsafe(DOMStringView html, const html::SetHTMLUnsafeOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> SetHTMLUnsafe(DOMStringView html) noexcept;

    /// @brief Appends @p html, parsed as HTML in the element's context; sanitized only if @p options gives a
    /// sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-element-appendhtmlunsafe
    ExceptionOr<void> AppendHTMLUnsafe(DOMStringView html,
                                       const html::SetHTMLUnsafeOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> AppendHTMLUnsafe(DOMStringView html) noexcept;

    /// @brief Prepends @p html, parsed as HTML in the element's context; sanitized only if @p options gives a
    /// sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-element-prependhtmlunsafe
    ExceptionOr<void> PrependHTMLUnsafe(DOMStringView html,
                                        const html::SetHTMLUnsafeOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> PrependHTMLUnsafe(DOMStringView html) noexcept;

    /// @brief Inserts before the element @p html, parsed as HTML in its parent's context, sanitized by @p
    /// options (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtml
    ExceptionOr<void> BeforeHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> BeforeHTML(DOMStringView html) noexcept;

    /// @brief Inserts after the element @p html, parsed as HTML in its parent's context, sanitized by @p
    /// options (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtml
    ExceptionOr<void> AfterHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> AfterHTML(DOMStringView html) noexcept;

    /// @brief Replaces the element with @p html, parsed as HTML in its parent's context, sanitized by @p
    /// options (the default configuration if it gives none) with unsafe content removed.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtml
    ExceptionOr<void> ReplaceWithHTML(DOMStringView html, const html::SetHTMLOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> ReplaceWithHTML(DOMStringView html) noexcept;

    /// @brief Inserts before the element @p html, parsed as HTML in its parent's context; sanitized only if
    /// @p options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-beforehtmlunsafe
    ExceptionOr<void> BeforeHTMLUnsafe(DOMStringView html,
                                       const html::SetHTMLUnsafeOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> BeforeHTMLUnsafe(DOMStringView html) noexcept;

    /// @brief Inserts after the element @p html, parsed as HTML in its parent's context; sanitized only if @p
    /// options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-afterhtmlunsafe
    ExceptionOr<void> AfterHTMLUnsafe(DOMStringView html, const html::SetHTMLUnsafeOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> AfterHTMLUnsafe(DOMStringView html) noexcept;

    /// @brief Replaces the element with @p html, parsed as HTML in its parent's context; sanitized only if @p
    /// options gives a sanitizer.
    /// @see https://html.spec.whatwg.org/#dom-nondocumenttypechildnode-replacewithhtmlunsafe
    ExceptionOr<void> ReplaceWithHTMLUnsafe(DOMStringView html,
                                            const html::SetHTMLUnsafeOptions &options) noexcept;

    /// @brief The same, with default options.
    ExceptionOr<void> ReplaceWithHTMLUnsafe(DOMStringView html) noexcept;

    /// @brief The element's children serialised as HTML, with the shadow roots @p options asks for.
    /// @see https://html.spec.whatwg.org/#dom-element-gethtml
    KRYS_NODISCARD DOMString GetHTML(dom::GetHTMLOptions options) const noexcept;

    /// @brief The same, with no shadow roots.
    /// @note An overload rather than a default argument, which would need ShadowRoot's definition here.
    KRYS_NODISCARD DOMString GetHTML() const noexcept;

    /// @brief The children serialized; in an XML document, an `InvalidStateError` if they can't be serialized
    /// as well-formed XML.
    /// @see https://html.spec.whatwg.org/#dom-element-innerhtml
    KRYS_NODISCARD ExceptionOr<DOMString> InnerHTML() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-element-innerhtml
    ExceptionOr<void> InnerHTML(DOMStringView markup) noexcept;

    /// @brief The element serialized; in an XML document, an `InvalidStateError` if it can't be serialized as
    /// well-formed XML.
    /// @see https://html.spec.whatwg.org/#dom-element-outerhtml
    KRYS_NODISCARD ExceptionOr<DOMString> OuterHTML() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-element-outerhtml
    ExceptionOr<void> OuterHTML(DOMStringView markup) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-element-insertadjacenthtml
    ExceptionOr<void> InsertAdjacentHTML(dom::InsertAdjacentWhere where, DOMStringView markup) noexcept;

#pragma endregion

#pragma region Element (Pointer Events) - https://w3c.github.io/pointerevents/#extensions-to-the-element-interface

    /// @see https://w3c.github.io/pointerevents/#dom-element-setpointercapture
    ExceptionOr<void> SetPointerCapture(int32 pointerId) noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-element-releasepointercapture
    ExceptionOr<void> ReleasePointerCapture(int32 pointerId) noexcept;

    /// @see https://w3c.github.io/pointerevents/#dom-element-haspointercapture
    KRYS_NODISCARD bool HasPointerCapture(int32 pointerId) noexcept;

#pragma endregion

#pragma region Element (Pointer Lock) - https://w3c.github.io/pointerlock/#extensions-to-the-element-interface

    /// @brief Asks to lock the pointer to this element: while it's locked, the mouse's events come here with
    /// the cursor hidden, and its moves carry the mouse's movement. `pointerlockchange` fires at the document
    /// once it's locked, or `pointerlockerror` if it can't be.
    /// @param promise Called once, as a microtask, when the lock is applied or the request fails.
    /// @see https://w3c.github.io/pointerlock/#dom-element-requestpointerlock
    void RequestPointerLock(dom::PointerLockOptions options = {},
                            Func<void(ExceptionOr<void> &&)> &&promise = {}) noexcept;

#pragma endregion

#pragma region Element (CSSOM View) - https://drafts.csswg.org/cssom-view/#extension-to-the-element-interface

    /// @brief The bounding boxes of the element's boxes, in client coordinates.
    KRYS_NODISCARD Ref<dom::DOMRectList> GetClientRects() noexcept;

    /// @brief The union of the element's boxes, in client coordinates.
    KRYS_NODISCARD Ref<dom::DOMRect> GetBoundingClientRect() noexcept;

    /// @brief `GetBoundingClientRect()` as a rect, without making a `DOMRect`.
    KRYS_NODISCARD RectF BoundingClientRect() noexcept;

    /// @brief Where the element's renderer's bounding box is on the screen; empty without a renderer.
    KRYS_NODISCARD RectI ScreenRect() const noexcept;

    /// @brief The element's renderer and the union of its boxes in absolute coordinates, without updating
    /// layout; null if it has no boxes.
    KRYS_NODISCARD Maybe<Pair<RenderElement *, RectF>> BoundingAbsoluteRectWithoutLayout() const noexcept;

    /// @brief Scrolls the element's ancestors so the element is aligned as @p options ask.
    void ScrollIntoView(const ScrollIntoViewOptions &options = {}) noexcept;

    /// @brief Scrolls the element's ancestors so its start (or, if @p alignToTop is false, its end) is
    /// aligned with theirs.
    void ScrollIntoView(bool alignToTop) noexcept;

    /// @brief Scrolls the element's ancestors so the element is visible, if it isn't: centred in them if
    /// @p centerIfNotVisible, otherwise to their nearest edge.
    void ScrollIntoViewIfNotVisible(bool centerIfNotVisible = true) noexcept;

    /// @brief Scrolls the element's contents to @p options' position (or to @p x, @p y).
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scroll
    void Scroll(const ScrollToOptions &options = {}) noexcept;

    void Scroll(double x, double y) noexcept;

    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scrollto
    void ScrollTo(const ScrollToOptions &options = {}) noexcept;

    void ScrollTo(double x, double y) noexcept;

    /// @brief Scrolls the element's contents by @p options' offsets (or by @p x, @p y).
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scrollby
    void ScrollBy(const ScrollToOptions &options = {}) noexcept;

    void ScrollBy(double x, double y) noexcept;

    /// @brief How far the contents are scrolled, from the top and left.
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scrolltop
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scrollleft
    KRYS_NODISCARD int32 ScrollTop() noexcept;

    void SetScrollTop(int32 newTop) noexcept;

    KRYS_NODISCARD int32 ScrollLeft() noexcept;

    void SetScrollLeft(int32 newLeft) noexcept;

    /// @brief The size of the scrollable overflow.
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scrollwidth
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-scrollheight
    KRYS_NODISCARD int32 ScrollWidth() noexcept;

    KRYS_NODISCARD int32 ScrollHeight() noexcept;

    /// @brief The width of the top and left border (and of a left scrollbar).
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-clienttop
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-clientleft
    KRYS_NODISCARD int32 ClientTop() noexcept;

    KRYS_NODISCARD int32 ClientLeft() noexcept;

    /// @brief The size of the padding box, without scrollbars; the viewport's for the root.
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-clientwidth
    /// @see https://drafts.csswg.org/cssom-view/#dom-element-clientheight
    KRYS_NODISCARD int32 ClientWidth() noexcept;

    KRYS_NODISCARD int32 ClientHeight() noexcept;

    /// @brief The nearest positioned (or table) ancestor the offset values are relative to, retargeted out of
    /// shadow trees the element isn't in.
    /// @see https://drafts.csswg.org/cssom-view/#extensions-to-the-htmlelement-interface
    KRYS_NODISCARD Element *OffsetParent() noexcept;

    /// @brief The offset parent, without retargeting it out of shadow trees.
    KRYS_NODISCARD Element *OffsetParentIgnoringShadowTrees() noexcept;

    /// @brief The border box's position from the offset parent's padding edge, and its size, in the
    /// element's own CSS pixels.
    /// @see https://drafts.csswg.org/cssom-view/#dom-htmlelement-offsettop
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
    ExceptionOr<void> TextContent(DOMString value) noexcept final;

#pragma endregion

#pragma region ParentNode Mixin - https://dom.spec.whatwg.org/#parentnode

    /// @brief Returns the child elements.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-children
    KRYS_NODISCARD Ref<dom::HTMLCollection> Children() noexcept;

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
    ExceptionOr<void> Prepend(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` after the last child of `this`, while replacing strings in `nodes` with
    /// equivalent Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-append
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> Append(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Replace all children of `this` with `nodes`, while replacing strings in `nodes` with equivalent
    /// Text nodes.
    /// @see https://dom.spec.whatwg.org/#dom-parentnode-replacechildren
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    ExceptionOr<void> ReplaceChildren(const List<dom::NodeOrString> &nodes) noexcept;

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
    KRYS_NODISCARD ExceptionOr<Ref<dom::NodeList>> QuerySelectorAll(DOMStringView selectors) noexcept;

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
    ExceptionOr<void> Before(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Inserts `nodes` just after `this`, while replacing strings in `nodes` with equivalent Text
    /// nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-after
    ExceptionOr<void> After(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Replaces `this` with `nodes`, while replacing strings in `nodes` with equivalent Text nodes.
    /// @throws HierarchyRequestError if the constraints of the node tree are violated.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-replacewith
    ExceptionOr<void> ReplaceWith(const List<dom::NodeOrString> &nodes) noexcept;

    /// @brief Removes `this`.
    /// @see https://dom.spec.whatwg.org/#dom-childnode-remove
    ExceptionOr<void> Remove() noexcept;

#pragma endregion

#pragma region Slottable Mixin - https://dom.spec.whatwg.org/#slotable

    /// @see https://dom.spec.whatwg.org/#dom-slotable-assignedslot
    KRYS_NODISCARD RefPtr<HTMLSlotElement> AssignedSlot() noexcept;

#pragma endregion

    /// @brief Whether the parser has finished inserting this element's children. Elements not created by the
    /// parser have always finished.
    KRYS_NODISCARD bool IsFinishedParsingChildren() const noexcept
    {
      return !_nodeFlags.contains(dom::NodeFlag::IsParsingChildren);
    }

    /// @brief The element's qualified name: its namespace, prefix and local name.
    KRYS_NODISCARD const QualifiedName &TagQualifiedName() const noexcept
    {
      return _qualifiedName;
    }

    /// @brief The locale the element's form controls use, from its language.
    KRYS_NODISCARD i18n::Locale &GetLocale() const noexcept;

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
    /// @see https://dom.spec.whatwg.org/#concept-class
    KRYS_NODISCARD List<DOMStringAtom> ClassNames() const noexcept;

    /// @brief The classes a `class` attribute with @p value would give the element (none for null), as
    /// `ClassNames` gives them.
    KRYS_NODISCARD List<DOMStringAtom> ClassNamesForValue(Maybe<DOMStringView> value) const noexcept;

    /// @brief The element's ID, ASCII-lowercased in quirks mode; empty if it has none.
    KRYS_NODISCARD DOMString IdForStyleResolution() const noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-state
    KRYS_NODISCARD dom::CustomElementState GetCustomElementState() const noexcept
    {
      return _customElementState;
    }

    /// @see https://dom.spec.whatwg.org/#concept-element-is-value
    KRYS_NODISCARD const DOMStringAtom &IsValue() const noexcept KRYS_LIFETIME_BOUND
    {
      return _is;
    }

#pragma region Custom elements

    /// @see https://dom.spec.whatwg.org/#concept-element-custom
    KRYS_NODISCARD bool IsDefinedCustomElement() const noexcept
    {
      return _customElementState == dom::CustomElementState::Custom;
    }

    /// @brief Whether the element waits for a definition to upgrade it (its state is `undefined`).
    KRYS_NODISCARD bool IsCustomElementUpgradeCandidate() const noexcept
    {
      return _customElementState == dom::CustomElementState::Undefined;
    }

    /// @brief Whether the element is a custom element, or being constructed as one.
    KRYS_NODISCARD bool IsPrecustomizedOrDefinedCustomElement() const noexcept
    {
      return _customElementState == dom::CustomElementState::Precustomized
             || _customElementState == dom::CustomElementState::Custom;
    }

    /// @brief The behaviour of the custom element, created by its definition; null unless the element is
    /// defined.
    KRYS_NODISCARD html::CustomElement *GetCustomElement() const noexcept;

    /// @brief The element's custom element reaction queue, which holds the definition it's defined by or is
    /// to be upgraded to; null if it has neither.
    /// @see https://html.spec.whatwg.org/#custom-element-reaction-queue
    /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-definition
    KRYS_NODISCARD html::CustomElementReactionQueue *ReactionQueue() const noexcept;

    /// @brief The custom element state changes of creating and upgrading custom elements: defined (with the
    /// definition's queue), failed, precustomized while being constructed, and undefined while it waits for
    /// a definition. A failed element's reaction queue is cleared rather than dropped, as the upgrade that
    /// failed may be running its reactions.
    /// @see https://dom.spec.whatwg.org/#concept-element-custom-element-state
    void SetIsDefinedCustomElement(html::CustomElementInterface &elementInterface) noexcept;

    void SetIsFailedCustomElement() noexcept;

    void SetIsFailedCustomElementWithoutClearingReactionQueue() noexcept;

    void SetIsPrecustomizedCustomElement() noexcept;

    void ClearReactionQueueFromFailedCustomElement() noexcept;

    void SetIsCustomElementUpgradeCandidate() noexcept;

    /// @brief Enqueues an upgrade of the element to @p elementInterface's definition.
    /// @see https://html.spec.whatwg.org/#enqueue-a-custom-element-upgrade-reaction
    void EnqueueToUpgrade(html::CustomElementInterface &elementInterface) noexcept;

    /// @brief Sets the behaviour the element's definition created for it, and the registry it uses.
    void SetCustomElement(UniquePtr<html::CustomElement> &&customElement) noexcept;

    void SetCustomElementRegistry(RefPtr<html::CustomElementRegistry> &&registry) noexcept;

    /// @brief Whether the element's custom states include @p state, as `:state()` matches.
    KRYS_NODISCARD bool HasCustomState(DOMStringAtom state) const noexcept;

    /// @brief The element's custom states, created if it has none.
    KRYS_NODISCARD html::CustomStateSet &EnsureCustomStateSet() noexcept;

#pragma endregion

    /// @brief Writes attributes that are updated lazily (the `style` attribute after a change to the inline
    /// style, an SVG element's attributes after a change to the properties that reflect them) back to the
    /// attribute list. Everything that reads the attribute list calls this first.
    void SynchronizeAllAttributes() const noexcept
    {
      if (_nodeFlags.contains(dom::NodeFlag::StyleAttributeIsDirty)) KRYS_UNLIKELY
      {
        SynchronizeStyleAttribute();
      }
      if (_nodeFlags.contains(dom::NodeFlag::AnimatedSVGAttributesAreDirty)) KRYS_UNLIKELY
      {
        SynchronizeAnimatedSVGAttributes();
      }
    }

    /// @brief Whether the element is a replaced element (its content is outside the scope of CSS), given its
    /// style if there is one.
    /// @see https://html.spec.whatwg.org/#replaced-elements
    KRYS_NODISCARD virtual bool IsReplaced(const RenderStyle * = nullptr) const noexcept
    {
      return false;
    }

#pragma region Form Controls

    /// @brief Whether the element is a listed form-associated element, and whether it's one that can be
    /// validated.
    /// @see https://html.spec.whatwg.org/#category-listed
    KRYS_NODISCARD virtual bool IsFormListedElement() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsValidatedFormListedElement() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#concept-fe-disabled
    KRYS_NODISCARD virtual bool IsDisabledFormControl() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-valid
    KRYS_NODISCARD virtual bool MatchesValidPseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-invalid
    KRYS_NODISCARD virtual bool MatchesInvalidPseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-user-valid
    KRYS_NODISCARD virtual bool MatchesUserValidPseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-user-invalid
    KRYS_NODISCARD virtual bool MatchesUserInvalidPseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-read-write
    KRYS_NODISCARD virtual bool MatchesReadWritePseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-indeterminate
    KRYS_NODISCARD virtual bool MatchesIndeterminatePseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-default
    KRYS_NODISCARD virtual bool MatchesDefaultPseudoClass() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-optional
    KRYS_NODISCARD virtual bool IsOptionalFormControl() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-required
    KRYS_NODISCARD virtual bool IsRequiredFormControl() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-in-range
    KRYS_NODISCARD virtual bool IsInRange() const noexcept
    {
      return false;
    }

    /// @see https://html.spec.whatwg.org/#selector-out-of-range
    KRYS_NODISCARD virtual bool IsOutOfRange() const noexcept
    {
      return false;
    }

#pragma endregion

    /// @brief Whether the element is relevant to the user, for `content-visibility: auto`.
    /// @see https://drafts.csswg.org/css-contain/#relevant-to-the-user
    KRYS_NODISCARD bool IsRelevantToUser() const noexcept;

    /// @brief Why the element is relevant to the user, once its relevancy has been determined.
    KRYS_NODISCARD Maybe<EnumSet<dom::ContentRelevancy>> GetContentRelevancy() const noexcept;

    /// @brief Records why the element is relevant to the user, as content visibility determined.
    void SetContentRelevancy(EnumSet<dom::ContentRelevancy> contentRelevancy) noexcept;

    /// @brief The base value of a `random()` of the element (or of one of its pseudo-elements) that isn't
    /// shared between elements.
    KRYS_NODISCARD double
      LookupCSSRandomBaseValue(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                               const css::calc::RandomCachingKey &key) const noexcept;

    /// @brief Whether the element or a pseudo-element has `random()` base values.
    KRYS_NODISCARD bool HasRandomCachingKeyMap() const noexcept;

#pragma region Style

    /// @brief What `ResolveComputedStyle` is for: any style, only that of rendered elements, or deciding
    /// editability (which doesn't need container queries resolved).
    enum class ResolveComputedStyleMode : uint8
    {
      Normal,
      RenderedOnly,
      Editability,
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

    /// @brief The same, for the element or one of its pseudo-elements.
    KRYS_NODISCARD const RenderStyle *RenderOrDisplayContentsStyle(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    /// @brief Drops the computed styles of the element and its descendants.
    void ResetComputedStyle() noexcept;

    /// @brief Forgets the relations selector matching recorded on the element (`:empty`, its child index),
    /// on its children's behalf, and on its descendants' too.
    void ResetStyleRelations() noexcept;

    void ResetChildStyleRelations() noexcept;

    void ResetAllDescendantStyleRelations() noexcept;

    /// @brief Whether the element's stored style (it has no renderer) is `display: contents` or `none`.
    KRYS_NODISCARD bool HasDisplayContents() const noexcept;

    KRYS_NODISCARD bool HasDisplayNone() const noexcept;

    /// @brief Keeps the style of an element with `display: contents` or `none`, which no renderer holds;
    /// clearing it as the element gets a renderer.
    void StoreDisplayContentsOrNoneStyle(UniquePtr<RenderStyle> style) noexcept;

    void ClearDisplayContentsOrNoneStyle() noexcept;

    /// @brief The renderer the element gets with @p style, or null if it gets none.
    // FIXME: The insertion position in the render tree, and media elements' and frame sets' overrides.
    KRYS_NODISCARD virtual RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept;

    /// @brief Whether the element would have a renderer with @p style.
    // FIXME: Media elements' overrides, once they have renderers.
    KRYS_NODISCARD virtual bool RendererIsNeeded(const RenderStyle &style) noexcept;

    /// @brief Whether @p child, a child of the element in the composed tree, gets a renderer when the element
    /// has one.
    // FIXME: Media elements' override, once they have renderers.
    KRYS_NODISCARD virtual bool ChildShouldCreateRenderer(const Node &) const noexcept
    {
      return true;
    }

    /// @brief Called by the render tree updater before the element's renderer is created.
    virtual void WillAttachRenderers() noexcept
    {
    }

    /// @brief Called by the render tree updater after the element's renderer (and its descendants') was
    /// created or replaced.
    virtual void DidAttachRenderers() noexcept
    {
    }

    /// @brief Called by the render tree updater before the element's renderer (and its descendants') is torn
    /// down (a control releases the mouse it captured).
    virtual void WillDetachRenderers() noexcept
    {
    }

    /// @brief Called by the render tree updater after the element's renderer was torn down (a control closes
    /// its picker).
    virtual void DidDetachRenderers() noexcept
    {
    }

    /// @brief Called before the element's computed style (kept for elements without a renderer) is dropped.
    virtual void WillResetComputedStyle() noexcept
    {
    }

    /// @brief Called when the element's document is suspended or resumed, if the element registered for it.
    virtual void PrepareForDocumentSuspension() noexcept
    {
    }

    virtual void ResumeFromDocumentSuspension() noexcept
    {
    }

    /// @brief Called around the element's style resolution, for elements with custom style resolve callbacks.
    virtual void WillRecalcStyle(EnumSet<style::Change>) noexcept;

    virtual void DidRecalcStyle(EnumSet<style::Change>) noexcept;

    /// @brief Whether the element is a text field: a text area, or an input whose type is text-like.
    KRYS_NODISCARD virtual bool IsTextField() const noexcept
    {
      return false;
    }

    /// @brief Whether the element's `dir` attribute is `ltr`, `rtl` or `auto`.
    KRYS_NODISCARD bool HasValidTextDirectionState() const noexcept;

    /// @brief Whether the element's directionality is its auto directionality.
    /// @see https://html.spec.whatwg.org/#auto-directionality
    KRYS_NODISCARD bool HasAutoTextDirectionState() const noexcept;

    /// @brief Updates the directionality of the element, of what it decides, and of its ancestors whose auto
    /// directionality it decides.
    /// @see https://html.spec.whatwg.org/#the-directionality
    void UpdateEffectiveTextDirection() noexcept;

    /// @brief Takes the parent's directionality, or updates the element's own if it may differ; called as
    /// the element is inserted.
    void UpdateEffectiveTextDirectionIfNeeded() noexcept;

    /// @brief Whether the element takes text from the host's input method while it's focused: it's editable,
    /// or a text field (not a password field).
    KRYS_NODISCARD virtual bool ShouldUseInputMethod() noexcept;

    /// @brief Whether the element's style is resolved by `ResolveCustomStyle`.
    KRYS_NODISCARD bool HasCustomStyleResolveCallbacks() const noexcept
    {
      return _nodeFlags.contains(dom::NodeFlag::HasCustomStyleResolveCallbacks);
    }

    /// @brief The element's style, for elements with custom style resolve callbacks (user agent shadow tree
    /// elements that style themselves from their host); nothing to resolve it from the style sheets.
    /// @param shadowHostStyle The style of the shadow host of the element's tree, or null outside one.
    KRYS_NODISCARD virtual Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept;

    /// @brief The element's style resolved from the style sheets, without adjusting it.
    KRYS_NODISCARD style::UnadjustedStyle
      ResolveStyle(const style::ResolutionContext &resolutionContext) noexcept;

    /// @brief The URL of the image the element shows: its `src` attribute, or null if it has none.
    KRYS_NODISCARD virtual Maybe<DOMString> ImageSourceUrl() const noexcept;

    /// @brief The URL the element links to, if it's a link, resolved against its document's base URL.
    KRYS_NODISCARD net::Url AbsoluteLinkUrl() const noexcept;

    /// @brief Fires `focus` (then `focusin`) at the element as the focus update steps focus it; text controls
    /// update their placeholder and begin editing first.
    virtual void DispatchFocusEvent(EventTarget *relatedTarget, const dom::FocusOptions &options) noexcept;

    /// @brief Fires `blur` (then `focusout`) at the element as the focus update steps unfocus it; text
    /// controls update their placeholder and end editing first.
    virtual void DispatchBlurEvent(EventTarget *relatedTarget) noexcept;

    /// @brief Called after the element lost the focus and `blur` was fired at it.
    virtual void DidBlur() noexcept
    {
    }

    /// @brief Shows that the element is focused: places a caret at the start of an editing host (unless the
    /// selection is already in it), and scrolls the element (or the caret) into view.
    virtual void UpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                                       SelectionRevealMode revealMode = SelectionRevealMode::Reveal) noexcept;

    /// @brief Shows that the element is focused, through the element that shows it.
    void FindTargetAndUpdateFocusAppearance(
      dom::SelectionRestorationMode restorationMode,
      SelectionRevealMode revealMode = SelectionRevealMode::Reveal) noexcept;

    /// @brief The element that shows that this one is focused (the image of an image map's area), or null if
    /// none does.
    /// @see https://html.spec.whatwg.org/#focus-processing-model
    KRYS_NODISCARD virtual RefPtr<Element> FocusAppearanceUpdateTarget() noexcept;

    /// @brief The element's renderer, or null if it isn't rendered.
    KRYS_NODISCARD RenderElement *Renderer() const noexcept;

    /// @brief The element's `::before` pseudo-element, or null if it has none.
    KRYS_NODISCARD PseudoElement *BeforePseudoElement() const noexcept;

    /// @brief The scroll position of the element's layer, kept while the element has no renderer.
    KRYS_NODISCARD PointI SavedLayerScrollPosition() const noexcept;

    /// @brief Keeps the layer's scroll position while the element has no renderer.
    void SetSavedLayerScrollPosition(const PointI &position) noexcept;

    /// @brief The element's `::after` pseudo-element, or null if it has none.
    KRYS_NODISCARD PseudoElement *AfterPseudoElement() const noexcept;

    /// @brief The element's `::before` or `::after` pseudo-element (@p type), created if it doesn't exist.
    PseudoElement &EnsurePseudoElement(PseudoElementType type) noexcept;

    /// @brief The pseudo-element @p pseudoElementIdentifier if it's a `::before` or `::after` that exists.
    KRYS_NODISCARD RefPtr<PseudoElement>
      PseudoElementIfExists(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept;

    KRYS_NODISCARD RefPtr<const PseudoElement>
      PseudoElementIfExists(const style::PseudoElementIdentifier &pseudoElementIdentifier) const noexcept;

    /// @brief Destroys the element's `::before` or `::after` pseudo-element, if it has one.
    void ClearBeforePseudoElement() noexcept;

    void ClearAfterPseudoElement() noexcept;

    /// @brief Marks the element's style (or its subtree's) out of date, with the siblings whose style
    /// depends on it; also its layer's composition, or its renderers, as named.
    void InvalidateStyle() noexcept;

    void InvalidateStyleAndLayerComposition() noexcept;

    void InvalidateStyleForSubtree() noexcept;

    void InvalidateStyleAndRenderersForSubtree() noexcept;

    /// @brief Rebuilds the element's renderer, with its style as it is.
    void InvalidateRenderer() noexcept;

    /// @brief Marks the element's style (or its subtree's) out of date, without its siblings.
    void InvalidateStyleInternal() noexcept;

    void InvalidateStyleForSubtreeInternal() noexcept;

    /// @brief Restyles a size container's subtree when its size changes; and resumes the resolution that
    /// waited for its layout.
    void InvalidateForQueryContainerSizeChange() noexcept;

    void InvalidateForResumingQueryContainerResolution() noexcept;

    /// @brief Restyles an anchor-positioned element when its anchor moves; and resumes the resolution that
    /// waited for the anchor's layout.
    void InvalidateForAnchorRectChange() noexcept;

    void InvalidateForResumingAnchorPositionedElementResolution() noexcept;

    /// @brief Whether a change to the element needs to invalidate style: false when the element isn't
    /// connected or its style (or the whole document's) will be resolved again anyway.
    KRYS_NODISCARD bool NeedsStyleInvalidation() const noexcept;

    /// @brief The style resolver of the element's tree: its shadow root's, or its document's.
    KRYS_NODISCARD style::Resolver &StyleResolver() noexcept;

    /// @brief Whether styles that depend on the element as a query container are to be updated.
    KRYS_NODISCARD bool NeedsUpdateQueryContainerDependentStyle() const noexcept
    {
      return _nodeFlags.contains(dom::NodeFlag::NeedsUpdateQueryContainerDependentStyle);
    }

    void ClearNeedsUpdateQueryContainerDependentStyle() noexcept
    {
      _nodeFlags.Remove(dom::NodeFlag::NeedsUpdateQueryContainerDependentStyle);
    }

    /// @brief The relations selector matching recorded on the element, which say what a change to it or to
    /// its children restyles: `:empty`, sibling combinators, and the positional pseudo-classes of its
    /// children and descendants.
    KRYS_NODISCARD bool StyleAffectedByEmpty() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::StyleAffectedByEmpty);
    }

    KRYS_NODISCARD bool StyleIsAffectedByPreviousSibling() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::StyleIsAffectedByPreviousSibling);
    }

    KRYS_NODISCARD bool DescendantsAffectedByPreviousSibling() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::DescendantsAffectedByPreviousSibling);
    }

    KRYS_NODISCARD bool AffectsNextSiblingElementStyle() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::AffectsNextSiblingElementStyle);
    }

    KRYS_NODISCARD bool ChildrenAffectedByFirstChildRules() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::ChildrenAffectedByFirstChildRules);
    }

    KRYS_NODISCARD bool ChildrenAffectedByLastChildRules() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::ChildrenAffectedByLastChildRules);
    }

    KRYS_NODISCARD bool ChildrenAffectedByForwardPositionalRules() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::ChildrenAffectedByForwardPositionalRules);
    }

    KRYS_NODISCARD bool DescendantsAffectedByForwardPositionalRules() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::DescendantsAffectedByForwardPositionalRules);
    }

    KRYS_NODISCARD bool ChildrenAffectedByBackwardPositionalRules() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::ChildrenAffectedByBackwardPositionalRules);
    }

    KRYS_NODISCARD bool DescendantsAffectedByBackwardPositionalRules() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::DescendantsAffectedByBackwardPositionalRules);
    }

    KRYS_NODISCARD bool AffectedByHasWithPositionalPseudoClass() const noexcept
    {
      return _nodeStyleFlags.contains(dom::NodeStyleFlag::AffectedByHasWithPositionalPseudoClass);
    }

    /// @brief The element's index among its parent's element children, plus one, as cached by selector
    /// matching; zero if it isn't known.
    KRYS_NODISCARD uint32 ChildIndex() const noexcept;

    /// @brief Records the relations, as selector matching finds them.
    void SetStyleAffectedByEmpty() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::StyleAffectedByEmpty);
    }

    void SetStyleIsAffectedByPreviousSibling() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::StyleIsAffectedByPreviousSibling);
    }

    void SetDescendantsAffectedByPreviousSibling() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::DescendantsAffectedByPreviousSibling);
    }

    void SetAffectsNextSiblingElementStyle() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::AffectsNextSiblingElementStyle);
    }

    void SetChildrenAffectedByFirstChildRules() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::ChildrenAffectedByFirstChildRules);
    }

    void SetChildrenAffectedByLastChildRules() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::ChildrenAffectedByLastChildRules);
    }

    void SetChildrenAffectedByForwardPositionalRules() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::ChildrenAffectedByForwardPositionalRules);
    }

    void SetDescendantsAffectedByForwardPositionalRules() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::DescendantsAffectedByForwardPositionalRules);
    }

    void SetChildrenAffectedByBackwardPositionalRules() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::ChildrenAffectedByBackwardPositionalRules);
    }

    void SetDescendantsAffectedByBackwardPositionalRules() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::DescendantsAffectedByBackwardPositionalRules);
    }

    void SetAffectedByHasWithPositionalPseudoClass() noexcept
    {
      _nodeStyleFlags.Add(dom::NodeStyleFlag::AffectedByHasWithPositionalPseudoClass);
    }

    /// @brief Caches the element's index among its parent's element children, plus one.
    void SetChildIndex(uint32 index) noexcept;

#pragma endregion

#pragma region Animations

    /// @brief Animates the element with @p keyframes and plays the animation, as `animate()` does.
    /// @param options A duration in milliseconds, or the animation's options.
    /// @return A `TypeError` if the keyframes or the timing are invalid, or a `SyntaxError` if the
    /// pseudo-element selector is.
    /// @see https://drafts.csswg.org/web-animations-1/#dom-animatable-animate
    KRYS_NODISCARD ExceptionOr<Ref<WebAnimation>>
      Animate(boo::KeyframesInput &&keyframes,
              Maybe<Variant<double, KeyframeAnimationOptions>> options) noexcept;

    /// @brief Animates the element with @p keyframes and no options.
    /// @note An overload rather than a default argument, which would need the options' definition here.
    KRYS_NODISCARD ExceptionOr<Ref<WebAnimation>> Animate(boo::KeyframesInput &&keyframes) noexcept;

    /// @brief The relevant animations whose effects target the element (or, with `subtree`, its descendants
    /// too), in composite order.
    /// @see https://drafts.csswg.org/web-animations-1/#dom-animatable-getanimations
    KRYS_NODISCARD List<RefPtr<WebAnimation>>
      GetAnimations(const Maybe<GetAnimationsOptions> &options = null) noexcept;

    /// @brief Whether the element, or one of its pseudo-elements, may have keyframe effects.
    KRYS_NODISCARD bool MayHaveKeyframeEffects() const noexcept;

    /// @brief The animation state of the element (or a pseudo-element), if it has any; the `Ensure` form
    /// creates it.
    KRYS_NODISCARD ElementAnimationRareData *
      AnimationRareData(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    KRYS_NODISCARD ElementAnimationRareData &
      EnsureAnimationRareData(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    /// @brief The keyframe effects targeting the element (or a pseudo-element), in composite order.
    KRYS_NODISCARD boo::KeyframeEffectStack *GetKeyframeEffectStack(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    KRYS_NODISCARD boo::KeyframeEffectStack &EnsureKeyframeEffectStack(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    KRYS_NODISCARD bool
      HasKeyframeEffects(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    /// @brief The animations whose effects target the element (or a pseudo-element).
    KRYS_NODISCARD const AnimationCollection *
      Animations(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    KRYS_NODISCARD AnimationCollection &
      EnsureAnimations(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    /// @brief The element's CSS transitions by property, completed and running.
    /// @see https://drafts.csswg.org/css-transitions-1/#completed-transitions
    KRYS_NODISCARD bool
      HasCompletedTransitionForProperty(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                        const AnimatableCSSProperty &property) const noexcept;

    KRYS_NODISCARD bool
      HasRunningTransitionForProperty(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                      const AnimatableCSSProperty &property) const noexcept;

    KRYS_NODISCARD bool HasRunningTransitions(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    KRYS_NODISCARD const AnimatableCSSPropertyToTransitionMap *CompletedTransitionsByProperty(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    KRYS_NODISCARD const AnimatableCSSPropertyToTransitionMap *RunningTransitionsByProperty(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    KRYS_NODISCARD AnimatableCSSPropertyToTransitionMap &EnsureCompletedTransitionsByProperty(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    KRYS_NODISCARD AnimatableCSSPropertyToTransitionMap &EnsureRunningTransitionsByProperty(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    /// @brief The style at the element's last style change event, which transitions start from.
    /// @see https://drafts.csswg.org/css-transitions-1/#style-change-event
    KRYS_NODISCARD const RenderStyle *LastStyleChangeEventStyle(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    void SetLastStyleChangeEventStyle(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                      UniquePtr<const RenderStyle> &&style) noexcept;

    /// @brief Whether the cascade overrode a property the element's animations set, so an animation-only
    /// update resolves its style again rather than reusing the last style change event's.
    KRYS_NODISCARD bool HasPropertiesOverridenAfterAnimation(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    void SetHasPropertiesOverridenAfterAnimation(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier, bool value) noexcept;

    /// @brief The element's CSS animations, in `animation-name` order.
    KRYS_NODISCARD CSSAnimationCollection &AnimationsCreatedByMarkup(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    void SetAnimationsCreatedByMarkup(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier,
                                      CSSAnimationCollection &&animations) noexcept;

    /// @brief Called when the element's CSS animations change, or a `@keyframes` rule they use; the latter
    /// leaves a keyframes update pending until style next resolves.
    void
      CSSAnimationsDidUpdate(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    void
      KeyframesRuleDidChange(const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

    KRYS_NODISCARD bool HasPendingKeyframesUpdate(
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) const noexcept;

    /// @brief Invalidates the element's style because one of its animations changed.
    void InvalidateStyleForAnimation() noexcept;

#pragma endregion

#pragma region Mouse Events

    enum class EventIsDispatched : bool
    {
      No,
      Yes,
    };

    enum class EventIsDefaultPrevented : bool
    {
      No,
      Yes,
    };

    struct DispatchMouseEventResult
    {
      EventIsDispatched Dispatched;
      EventIsDefaultPrevented DefaultPrevented;
    };

    /// @brief Dispatches a trusted mouse event of type @p eventType from @p platformEvent at this element,
    /// after the pointer event it maps to (`click`, `auxclick` and `contextmenu` are only pointer events); a
    /// `click` with a @p detail of 2 is followed by a `dblclick`.
    /// @return Whether the event was dispatched without being cancelled, and whether it was cancelled.
    DispatchMouseEventResult DispatchMouseEvent(const PlatformMouseEvent &platformEvent,
                                                const DOMStringAtom &eventType, int32 detail = 0,
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

    /// @brief The element's assigned access key: the first token of its `accesskey` attribute that's a single
    /// code point, or empty if it has none.
    /// @see https://html.spec.whatwg.org/#assigned-access-key
    KRYS_NODISCARD DOMString AssignedAccessKey() const noexcept;

    /// @brief Runs the command the element's access key stands for. This focuses it if it's focusable.
    /// @param sendMouseEvents Whether a click it fires comes with a press and a release.
    /// @return Whether a click was dispatched.
    /// @see https://html.spec.whatwg.org/#command-facet-action
    virtual bool AccessKeyAction(bool sendMouseEvents) noexcept;

    /// @brief Dispatches a click at the element from the user agent, standing in for @p underlyingEvent.
    /// @return False if the element is a disabled form control, or is already being clicked.
    bool DispatchSimulatedClick(
      Event *underlyingEvent,
      dom::SimulatedClickMouseEventOptions eventOptions = dom::SimulatedClickMouseEventOptions::SendNoEvents,
      dom::SimulatedClickVisualOptions visualOptions =
        dom::SimulatedClickVisualOptions::ShowPressedLook) noexcept;

#pragma endregion

#pragma region Top Layer

    /// @brief Whether the element is in its document's top layer.
    /// @see https://drafts.csswg.org/css-position-4/#in-top-layer
    KRYS_NODISCARD bool IsInTopLayer() const noexcept
    {
      return _eventTargetFlags.contains(dom::EventTargetFlag::IsInTopLayer);
    }

    /// @brief Adds the element, which must be connected, to the top of its document's top layer, where it's
    /// rendered above the rest of the document with its `::backdrop`. An element that's leaving the top layer
    /// is moved to the top; otherwise it mustn't be in it already.
    /// @see https://drafts.csswg.org/css-position-4/#add-an-element-to-the-top-layer
    void AddToTopLayer() noexcept;

    /// @brief Asks for the element to leave its document's top layer: it stays rendered there until its
    /// `overlay` (which then computes to `none`) stops being `auto`, so a transition on `overlay` can keep it
    /// there while it animates out. Nothing happens if it isn't in the top layer or is already leaving.
    /// @see https://drafts.csswg.org/css-position-4/#request-an-element-to-be-removed-from-the-top-layer
    void RequestRemoveFromTopLayer() noexcept;

    /// @brief Removes the element, which must be in it, from its document's top layer at once.
    /// @see https://drafts.csswg.org/css-position-4/#remove-an-element-from-the-top-layer-immediately
    void RemoveFromTopLayer() noexcept;

#pragma endregion

#pragma region Popovers

    /// @brief The element's popover state, if it has a `popover` attribute.
    KRYS_NODISCARD dom::PopoverData *GetPopoverData() const noexcept;

    /// @brief Creates or drops the popover state, as the `popover` attribute comes or goes.
    dom::PopoverData &EnsurePopoverData() noexcept;

    void ClearPopoverData() noexcept;

    /// @brief Whether the element is in the popover showing state.
    /// @see https://html.spec.whatwg.org/#popover-showing-state
    KRYS_NODISCARD bool IsPopoverShowing() const noexcept;

    /// @brief The state of the element's `popover` attribute.
    KRYS_NODISCARD dom::PopoverState GetPopoverState() const noexcept;

    /// @brief The popover the element (a button) showed and is still the popover trigger of, if any.
    KRYS_NODISCARD Element *InvokedPopover() const noexcept;

    /// @brief Records the popover the element showed.
    void SetInvokedPopover(Element *element) noexcept;

    /// @brief The topmost showing auto or hint popover that's a flat tree ancestor of the element, or (if
    /// @p isPopover) of @p source, the element's invoker, inclusive.
    /// @param isPopover Whether the element is a popover about to show, rather than another element about to
    /// enter the top layer (a dialog), whose source isn't considered.
    /// @see https://html.spec.whatwg.org/#topmost-popover-ancestor
    KRYS_NODISCARD RefPtr<HTMLElement> TopmostPopoverAncestor(HTMLElement *source, bool isPopover) noexcept;

#pragma endregion

#pragma region Intersection Observers

    /// @brief The intersection observers whose root the element is, and those observing it.
    IntersectionObserverData &EnsureIntersectionObserverData() noexcept;

    /// @brief The same, if the element has any.
    KRYS_NODISCARD IntersectionObserverData *IntersectionObserverDataIfExists() const noexcept;

#pragma endregion

#pragma region Resize Observers

    /// @brief The resize observers observing the element.
    ResizeObserverData &EnsureResizeObserverData() noexcept;

    /// @brief The same, if the element has any.
    KRYS_NODISCARD ResizeObserverData *ResizeObserverDataIfExists() const noexcept;

    /// @brief The element's last remembered size in each axis, for `contain-intrinsic-size: auto`.
    /// @see https://drafts.csswg.org/css-sizing-4/#last-remembered
    KRYS_NODISCARD Maybe<LayoutUnit> LastRememberedLogicalWidth() const noexcept;

    KRYS_NODISCARD Maybe<LayoutUnit> LastRememberedLogicalHeight() const noexcept;

    void SetLastRememberedLogicalWidth(LayoutUnit width) noexcept;

    void ClearLastRememberedLogicalWidth() noexcept;

    void SetLastRememberedLogicalHeight(LayoutUnit height) noexcept;

    void ClearLastRememberedLogicalHeight() noexcept;

#pragma endregion

#pragma region Commands

    /// @brief Whether the element carries out a built-in command for the buttons whose `commandfor` element
    /// it is; buttons don't send it the others.
    KRYS_NODISCARD virtual bool IsValidCommandType(dom::CommandType) const noexcept
    {
      return false;
    }

    /// @brief Carries out a built-in command, sent by a button, if it applies to the element's state.
    /// @return Whether the element carried it out.
    virtual bool HandleCommandInternal(HTMLButtonElement &, dom::CommandType) noexcept
    {
      return false;
    }

#pragma endregion

#pragma region Element Reflection

    /// @brief The element an element-reference attribute's target resolves to: the unresolved target
    /// (`ElementForAttributeInternal`) with its reference target resolved, as behaviour uses it.
    /// @see https://html.spec.whatwg.org/#get-the-resolved-attribute-target-element
    KRYS_NODISCARD RefPtr<Element> ResolvedElementForAttribute(DOMStringAtom attributeName) const noexcept;

    /// @brief @p element as an IDL attribute of this one exposes it: retargeted against this, so an element
    /// in a shadow tree this element isn't in shows as the host holding it.
    /// @see https://dom.spec.whatwg.org/#retarget
    KRYS_NODISCARD RefPtr<Element> RetargetForBindings(Element *element) const noexcept;

    /// @brief The element a reference to this one targets: this, or, through its shadow root's reference
    /// target, the element in its shadow tree with that ID (resolved in turn); null if there's no such
    /// element.
    /// @see https://html.spec.whatwg.org/#resolve-the-reference-target
    KRYS_NODISCARD RefPtr<Element> ResolveReferenceTarget() noexcept;

    /// @brief The element an element reflection attribute refers to: the one set through its IDL attribute,
    /// if it's still in the element's shadow-including ancestors' trees, else the one with the attribute's
    /// value as its ID in the element's tree.
    /// @see https://html.spec.whatwg.org/#attr-associated-element
    KRYS_NODISCARD RefPtr<Element>
      ElementForAttributeInternal(const DOMStringAtom &attributeName) const noexcept;

    /// @brief Sets an element reflection attribute's element: the content attribute becomes empty, or is
    /// removed if @p element is null.
    /// @see https://html.spec.whatwg.org/#reflecting-content-attributes-in-idl-attributes:element
    void SetElementAttribute(const DOMStringAtom &attributeName, Element *element) noexcept;

    /// @brief Whether @p attributeName (in the null namespace) is reflected by an `Element?` IDL attribute.
    KRYS_NODISCARD static bool IsElementReflectionAttribute(const DOMStringAtom &attributeName) noexcept;

    /// @brief The elements a `FrozenArray<Element>?` reflection attribute refers to: the ones set through its
    /// IDL attribute that are still in the element's shadow-including ancestors' trees, else the ones with
    /// the attribute's IDs in the element's tree; null if neither is set.
    /// @see https://html.spec.whatwg.org/#attr-associated-elements
    KRYS_NODISCARD Maybe<List<Ref<Element>>>
      ElementsForAttributeInternal(const DOMStringAtom &attributeName) const noexcept;

    /// @brief The elements a set of element references attribute resolves to: its unresolved targets
    /// (`ElementsForAttributeInternal`), each with its reference target resolved, leaving out those that
    /// resolve to nothing.
    /// @see https://html.spec.whatwg.org/#resolved-attribute-target-elements
    KRYS_NODISCARD List<Ref<Element>>
      ResolvedElementsForAttribute(DOMStringAtom attributeName) const noexcept;

    /// @brief Sets a `FrozenArray<Element>?` reflection attribute's elements: the content attribute becomes
    /// empty, or is removed if @p elements is null.
    /// @see https://html.spec.whatwg.org/#attr-associated-elements
    void SetElementsArrayAttribute(const DOMStringAtom &attributeName,
                                   const Maybe<List<Ref<Element>>> &elements) noexcept;

    /// @brief Whether @p attributeName (in the null namespace) is reflected by a `FrozenArray<Element>?` IDL
    /// attribute.
    KRYS_NODISCARD static bool
      IsElementsArrayReflectionAttribute(const DOMStringAtom &attributeName) noexcept;

    /// @brief The first element in tree order whose root is this element's root and whose ID is @p id.
    KRYS_NODISCARD RefPtr<Element> ElementInTreeWithId(DOMStringView id) const noexcept;

#pragma endregion

#pragma region User Action State

    /// @brief Whether the element has user action state in its document (`Document::UserActionElements`).
    KRYS_NODISCARD bool IsUserActionElement() const noexcept
    {
      return _nodeFlags.contains(dom::NodeFlag::IsUserActionElement);
    }

    /// @brief Records whether the document has user action state for the element.
    void SetUserActionElement(bool flag) noexcept
    {
      flag ? _nodeFlags.Add(dom::NodeFlag::IsUserActionElement)
           : _nodeFlags.Remove(dom::NodeFlag::IsUserActionElement);
    }

    /// @brief Whether the element is in the chain of elements that became `:active` when the pointer was
    /// pressed.
    KRYS_NODISCARD bool IsInActiveChain() const noexcept
    {
      return IsUserActionElement() && IsUserActionElementInActiveChain();
    }

    /// @brief The element's user action state: whether it's `:active`, `:hover`, `:focus`,
    /// `:focus-visible` and `:focus-within`.
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

    /// @brief Sets the element's user action state, restyling what depends on it.
    virtual void
      SetActive(bool value,
                style::InvalidationScope invalidationScope = style::InvalidationScope::All) noexcept;

    virtual void SetHovered(bool value,
                            style::InvalidationScope invalidationScope = style::InvalidationScope::All,
                            HitTestRequest request = {}) noexcept;

    /// @brief Sets the element's focus, and the focus within of its ancestors in the composed tree.
    virtual void SetFocus(bool value,
                          dom::FocusVisibility visibility = dom::FocusVisibility::Invisible) noexcept;

    void SetHasFocusVisible(bool value) noexcept;

    void SetHasFocusWithin(bool value) noexcept;

    /// @brief Clears the element's hover and active state, moving the document's hovered element and active
    /// chain to its ancestors, before its renderer goes.
    void ClearHoverAndActiveStatusBeforeDetachingRenderer() noexcept;

#pragma endregion

  private:
    void ScrollTo(const ScrollToOptions &options,
                  ScrollSnapPointSelectionMethod snapPointSelectionMethod) noexcept;

    /// @brief The element's rare data, if it has any.
    KRYS_NODISCARD dom::ElementRareData *GetElementRareData() const noexcept;

    /// @brief The element's rare data, created if it has none.
    KRYS_NODISCARD dom::ElementRareData &EnsureElementRareData() noexcept;

    /// @brief Sets the custom element state, restyling `:defined` and refreshing the listed elements of
    /// forms.
    void SetCustomElementState(dom::CustomElementState state) noexcept;

    /// @brief Stops the element being observed, or being an observer's root, as it's destroyed.
    void DisconnectFromIntersectionObservers() noexcept;

    void DisconnectFromResizeObservers() noexcept;

    /// @brief The user action state the document holds for the element.
    KRYS_NODISCARD bool IsUserActionElementInActiveChain() const noexcept;

    KRYS_NODISCARD bool IsUserActionElementActive() const noexcept;

    KRYS_NODISCARD bool IsUserActionElementFocused() const noexcept;

    KRYS_NODISCARD bool IsUserActionElementHovered() const noexcept;

    KRYS_NODISCARD bool IsUserActionElementHasFocusVisible() const noexcept;

    KRYS_NODISCARD bool IsUserActionElementHasFocusWithin() const noexcept;

    /// @brief Resolves the style of a pseudo-element the element has no renderer for, caching it on the
    /// element's style.
    KRYS_NODISCARD const RenderStyle &
      ResolvePseudoElementStyle(const style::PseudoElementIdentifier &pseudoElementIdentifier) noexcept;

  protected:
    /// @brief Sets an attribute in the null namespace to `value` without running the attribute change steps
    /// or queueing a mutation record, for lazily synchronised attributes.
    void SetSynchronizedLazyAttribute(DOMStringAtom localName, DOMString value) noexcept;

    /// @brief Sets the attribute @p name (matched by its namespace and local name) to `value` without running
    /// the attribute change steps or queueing a mutation record, for lazily synchronised attributes.
    void SetSynchronizedLazyAttribute(const QualifiedName &name, DOMString value) noexcept;

    /// @brief The inline style's property map (`attributeStyleMap`), once it's been asked for.
    KRYS_NODISCARD StylePropertyMap *ExistingAttributeStyleMap() const noexcept;

    /// @brief Keeps the `attributeStyleMap` once it's made.
    void SetAttributeStyleMap(Ref<StylePropertyMap> map) noexcept;

#pragma region Parser Hooks

    /// @brief Called by the parser when it pushes this element onto the stack of open elements.
    void BeginParsingChildren() noexcept
    {
      _nodeFlags.Add(dom::NodeFlag::IsParsingChildren);
    }

    /// @brief Called by the parser when it pops this element off the stack of open elements. Overrides must
    /// call the base implementation.
    /// @see https://html.spec.whatwg.org/#stack-of-open-elements
    virtual void FinishParsingChildren() noexcept;

#pragma endregion

    /// @brief Called after a user agent shadow root is attached to the element.
    virtual void DidAddUserAgentShadowRoot(boo::ShadowRoot &) noexcept
    {
    }

#pragma region Extension Hooks

    /// @brief The attribute change steps, run whenever one of the element's attributes is set, changed or
    /// removed. Overrides must call the base implementation.
    /// @param oldValue The previous value, or null if the attribute was added.
    /// @param value The new value, or null if the attribute was removed.
    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change-ext
    virtual void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                                    Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept;

#pragma endregion

  private:
    /// @brief Writes the inline style, or the animated SVG attributes, back to the attribute list.
    void SynchronizeStyleAttribute() const noexcept;

    void SynchronizeAnimatedSVGAttributes() const noexcept;

    /// @brief Replaces the element's children (a template's content's) with @p markup parsed in its context.
    ExceptionOr<void>
      ReplaceChildrenWithMarkup(DOMStringView markup,
                                EnumSet<dom::ParserContentPolicy> parserContentPolicy) noexcept;
  };
}

namespace krys::boo::dom
{
  /// @brief Invalidates the style of @p sibling and its following siblings that sibling combinators make
  /// depend on their previous siblings, up to the first one that doesn't affect the next.
  void InvalidateForSiblingCombinators(Element *sibling) noexcept;

  /// @brief Whether an element with @p style is in the top layer, or is a `::backdrop`.
  KRYS_NODISCARD bool IsInTopLayerOrBackdrop(const RenderStyle &style, const Element *element) noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::Element)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &node) noexcept
  {
    return node.IsElementNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
