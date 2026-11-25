#pragma once

#include "Krystal.Booey/DOM/Events/ToggleEventTask.hpp"
#include "Krystal.Booey/DOM/Internals/PopoverData.hpp"
#include "Krystal.Booey/DOM/Nodes/StyledElement.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Attributes/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Types/BoolOr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class Document;
  class HTMLElement;
  class NodeList;
}

namespace krys::boo::html
{
  class ElementInternals;
  class FormAssociatedElement;
  class FormListedElement;
  class ValidatedFormListedElement;

  /// @see https://html.spec.whatwg.org/multipage/dom.html#showpopoveroptions
  struct ShowPopoverOptions
  {
    RefPtr<HTMLElement> Source;
  };

  /// @see https://html.spec.whatwg.org/multipage/dom.html#togglepopoveroptions
  struct TogglePopoverOptions : public ShowPopoverOptions
  {
    Maybe<bool> Force;
  };

  using TogglePopoverOptionsOrBool = BoolOr<TogglePopoverOptions>;

  enum class HTMLElementFlag : uint8
  {
    IsMediaElement,
  };

  consteval HTMLElementFlag MaxEnumValue(HTMLElementFlag) noexcept
  {
    return HTMLElementFlag::IsMediaElement;
  }

  using HTMLElementFlags = EnumSet<HTMLElementFlag>;

  enum class HTMLElementInterface : uint8
  {
    None,
    Anchor,
    Area,
    Audio,
    Base,
    BDI,
    Body,
    BR,
    Button,
    Canvas,
    Data,
    DataList,
    Details,
    Dialog,
    Directory,
    Div,
    DList,
    Embed,
    Font,
    Form,
    FieldSet,
    FrameSet,
    Head,
    Heading,
    HR,
    Html,
    IFrame,
    Image,
    Input,
    Link,
    Label,
    Legend,
    LI,
    Map,
    Marquee,
    MaybeFormAssociatedCustom,
    Media,
    Menu,
    Meta,
    Meter,
    Mod,
    Object,
    OList,
    OptGroup,
    Option,
    Output,
    Paragraph,
    Param,
    Picture,
    Pre,
    Progress,
    Quote,
    Script,
    Select,
    SelectedContent,
    Slot,
    Source,
    Span,
    Style,
    Summary,
    Table,
    TableCaption,
    TableCell,
    TableCol,
    TableRow,
    TableSection,
    Template,
    TextArea,
    Title,
    Time,
    Track,
    UList,
    Unknown,
    Video,
    WBR,
  };

  consteval HTMLElementInterface MaxEnumValue(HTMLElementInterface) noexcept
  {
    return HTMLElementInterface::WBR;
  }
}

namespace krys::boo
{
  /// @see https://html.spec.whatwg.org/multipage/dom.html#htmlelement
  class HTMLElement : public StyledElement
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLElement);

  private:
    html::HTMLElementFlags _htmlElementFlags;
    html::HTMLElementInterface _interface
      : EnumBitWidth<html::HTMLElementInterface> {html::HTMLElementInterface::None};

  protected:
    HTMLElement(Document &document, html::HTMLElementInterface interface,
                html::HTMLElementFlags flags = html::HTMLElementFlags {},
                dom::NodeFlags nodeFlags = dom::NodeFlags {}) noexcept;

    /// @brief Constructs an element with its tag name, for elements made outside the element factory (the
    /// elements of user agent shadow trees), which otherwise gives elements their names.
    HTMLElement(Document &document, html::HTMLElementInterface interface, DOMStringAtom localName,
                html::HTMLElementFlags flags, dom::NodeFlags nodeFlags) noexcept;

  public:
    HTMLElement(Document &document) noexcept;

#pragma region HTMLElement - https://html.spec.whatwg.org/multipage/dom.html#htmlelement

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-title
    KRYS_NODISCARD DOMString Title() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-title
    void Title(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-lang
    KRYS_NODISCARD DOMString Lang() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-lang
    void Lang(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-translate
    KRYS_NODISCARD bool Translate() const noexcept;

    /// @brief Whether @p element's translation mode is translate-enabled, rather than no-translate: the
    /// state of the nearest inclusive ancestor HTML element whose `translate` attribute is `yes` or `no`, or
    /// translate-enabled if there's none.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#translation-mode
    KRYS_NODISCARD static bool IsTranslateEnabled(const Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-translate
    void Translate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-dir
    KRYS_NODISCARD DOMString Dir() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-dir
    void Dir(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-hidden
    KRYS_NODISCARD BoolOr<DOMString> Hidden() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-hidden
    void Hidden(const DOMString &value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-hidden
    void Hidden(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-hidden
    void Hidden(double value) noexcept;

    /// @brief Whether the `hidden` attribute is in the hidden until found state.
    KRYS_NODISCARD bool IsHiddenUntilFound() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-inert
    KRYS_NODISCARD bool Inert() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-inert
    void Inert(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-click
    void Click() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-accesskey
    KRYS_NODISCARD DOMString AccessKey() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-accesskey
    void AccessKey(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-accesskeylabel
    KRYS_NODISCARD DOMString AccessKeyLabel() const noexcept;

    /// @brief The state of the `contenteditable` attribute: `true`, `plaintext-only`, `false` or `inherit`.
    /// @note `isContentEditable` is `Node::IsContentEditable`.
    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-contenteditable
    KRYS_NODISCARD DOMString ContentEditable() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-contenteditable
    ExceptionOr<void> ContentEditable(DOMStringView value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-draggable
    KRYS_NODISCARD bool Draggable() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dnd.html#dom-draggable
    void Draggable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-spellcheck
    KRYS_NODISCARD bool SpellCheck() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-spellcheck
    void SpellCheck(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-writingsuggestions
    KRYS_NODISCARD DOMString WritingSuggestions() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-writingsuggestions
    void WritingSuggestions(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-autocapitalize
    KRYS_NODISCARD DOMString Autocapitalize() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-autocapitalize
    void Autocapitalize(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-autocorrect
    KRYS_NODISCARD bool Autocorrect() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-autocorrect
    void Autocorrect(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-inputmode
    KRYS_NODISCARD DOMString InputMode() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-inputmode
    void InputMode(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-enterkeyhint
    KRYS_NODISCARD DOMString EnterKeyHint() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-enterkeyhint
    void EnterKeyHint(DOMString value) noexcept;

    /// @brief The element's rendered text: its text as laid out, with line breaks and tabs for its boxes, or
    /// its descendant text content if it isn't being rendered.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-innertext
    /// @see https://html.spec.whatwg.org/multipage/dom.html#get-the-text-steps
    /// @see https://html.spec.whatwg.org/multipage/dom.html#rendered-text-collection-steps
    KRYS_NODISCARD DOMString InnerText() const noexcept;

    /// @brief Replaces the element's children with @p value's text, with a `<br>` for each line break.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-innertext
    /// @see https://html.spec.whatwg.org/multipage/dom.html#set-the-inner-text-steps
    void InnerText(const DOMString &value) noexcept;

    /// @brief The same as `InnerText()`.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-outertext
    KRYS_NODISCARD DOMString OuterText() const noexcept;

    /// @brief Replaces the element itself with @p value's text, with a `<br>` for each line break, merged
    /// with the text around it; `NoModificationAllowedError` if the element has no parent.
    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-outertext
    ExceptionOr<void> OuterText(const DOMString &value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/custom-elements.html#dom-attachinternals
    KRYS_NODISCARD ExceptionOr<Ref<html::ElementInternals>> AttachInternals() noexcept;

    /// @brief Shows the popover, invoked by @p options' source.
    /// @return A `NotSupportedError` if the element isn't a popover; an `InvalidStateError` if it isn't
    /// connected, its document isn't fully active, it's a modal dialog, another popover is being shown or
    /// hidden, or its `popover` attribute changes as other popovers hide.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-showpopover
    ExceptionOr<void> ShowPopover(const html::ShowPopoverOptions &options = {}) noexcept;

    /// @return A `NotSupportedError` if the element isn't a popover; an `InvalidStateError` if it's a modal
    /// dialog.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-hidepopover
    ExceptionOr<void> HidePopover() noexcept;

    /// @brief Hides the popover if it's showing, else shows it, unless @p options forces one or the other.
    /// @return Whether the popover is showing, or the exceptions of `ShowPopover` and `HidePopover`.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-togglepopover
    ExceptionOr<bool>
      TogglePopover(const html::TogglePopoverOptionsOrBool &options = html::TogglePopoverOptions {}) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-popover
    KRYS_NODISCARD Maybe<DOMString> Popover() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-popover
    void Popover(Maybe<DOMString> value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-headingoffset
    KRYS_NODISCARD uint32 HeadingOffset() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-headingoffset
    void HeadingOffset(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-headingreset
    KRYS_NODISCARD bool HeadingReset() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-headingreset
    void HeadingReset(bool value) noexcept;

#pragma endregion

    /// @brief The colour a presentational attribute (`bgcolor`, `color`, `text`, ...) specifies, if any.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-a-legacy-colour-value
    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseLegacyColorValue(DOMStringView string) noexcept;

#pragma region HTMLOrSVGOrMathMLElement Mixin - https://html.spec.whatwg.org/#htmlorsvgormathmlelement

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-tabindex
    KRYS_NODISCARD int32 TabIndex() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this);
    }

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-tabindex
    void TabIndex(int32 value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this, value);
    }

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-fe-autofocus
    KRYS_NODISCARD bool Autofocus() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this);
    }

    /// @see https://html.spec.whatwg.org/multipage/dom.html#dom-fe-autofocus
    void Autofocus(bool value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this, value);
    }

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-focus
    void Focus(const dom::FocusOptions &options = {}) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Focus(*this, options);
    }

    /// @brief Runs the element's access key action: focuses it if it's focusable, then clicks it.
    /// @return Whether the click was dispatched.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/interaction.html#dom-blur
    void Blur() noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Blur(*this);
    }

#pragma endregion

#pragma region GlobalEventHandlers, DocumentAndElementEventHandlers - https://html.spec.whatwg.org/multipage/webappapis.html#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

    // FIXME: HTMLElement mixins:
    // HTMLElement includes ElementContentEditable;

    /// @brief The editability of @p node from the `contenteditable` attributes of it and its ancestors, for
    /// nodes in shadow trees (whose style isn't used for editability).
    KRYS_NODISCARD static Node::Editability EditabilityFromContentEditableAttr(const Node &node) noexcept;

  protected:
#pragma region Presentational hints - https://html.spec.whatwg.org/multipage/rendering.html#presentational-hints

    enum class AllowZeroValue : bool
    {
      No,
      Yes,
    };

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    /// @param value The `border` attribute's value, or null if there isn't one.
    KRYS_NODISCARD uint32 ParseBorderWidthAttribute(Maybe<DOMStringView> value) const noexcept;

    /// @brief Maps a `border` attribute to a solid border of its width.
    void ApplyBorderAttributeToStyle(DOMStringView value, css::MutableStyleProperties &style) noexcept;

    /// @brief Maps an image-like element's `align` attribute to `vertical-align`, or to `float` for left and
    /// right.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#attributes-for-embedded-content-and-images
    void ApplyAlignmentAttributeToStyle(DOMStringView alignment, css::MutableStyleProperties &style) noexcept;

    /// @brief Maps `width` and `height` attributes to `aspect-ratio: auto w / h`, if both are numbers of
    /// pixels.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#map-to-the-aspect-ratio-property-(using-dimension-rules)
    void ApplyAspectRatioFromWidthAndHeightAttributesToStyle(DOMStringView widthAttribute,
                                                             DOMStringView heightAttribute,
                                                             css::MutableStyleProperties &style) noexcept;

    /// @brief As above, for a canvas's attributes, which are non-negative integers.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#map-to-the-aspect-ratio-property
    void ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(
      DOMStringView widthAttribute, DOMStringView heightAttribute,
      css::MutableStyleProperties &style) noexcept;

    /// @brief Maps a dimension attribute (a number of pixels or a percentage) to @p property.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#maps-to-the-dimension-property
    void AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                              DOMStringView value,
                              AllowZeroValue allowZeroValue = AllowZeroValue::Yes) noexcept;

    /// @brief Maps a dimension attribute that may also be a relative length (`2*`) to @p property.
    void AddHTMLMultiLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                   DOMStringView value) noexcept;

    /// @brief Maps a dimension attribute that's a number of pixels (not a percentage) to @p property.
    /// @see https://html.spec.whatwg.org/multipage/rendering.html#maps-to-the-pixel-length-property
    void AddHTMLPixelsToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                              DOMStringView value) noexcept;

    /// @brief Maps a legacy color attribute to @p property, if it parses.
    /// @see https://html.spec.whatwg.org/multipage/common-microsyntaxes.html#rules-for-parsing-a-legacy-colour-value
    static void AddHTMLColorToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                    DOMStringView attributeValue) noexcept;

    /// @brief The value of the attribute in the null namespace, or an empty view if there isn't one.
    KRYS_NODISCARD DOMString AttributeOrEmpty(DOMStringView localName) const noexcept;

#pragma endregion

#pragma region HTMLElement Flags

#pragma endregion

#pragma region Popovers

  public:
    /// @brief Shows the popover, invoked by @p source; the exceptions of `ShowPopover` are returned only if
    /// @p throwExceptions.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#show-popover
    ExceptionOr<void> ShowPopoverInternal(HTMLElement *source, bool throwExceptions = true) noexcept;

    /// @brief Hides the popover, and the popovers above it in its stack; the exceptions of `HidePopover` are
    /// returned only if @p throwExceptions.
    /// @param source The element that hid it, which `beforetoggle` and `toggle` carry.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#hide-popover-algorithm
    ExceptionOr<void> HidePopoverInternal(dom::FocusPreviousElement focusPreviousElement,
                                          dom::FireEvents fireEvents, bool throwExceptions,
                                          HTMLElement *source) noexcept;

    /// @brief The popover commands, for a popover.
    KRYS_NODISCARD bool IsValidCommandType(dom::CommandType command) const noexcept override;

    /// @brief Shows or hides the element, if it's a popover, for the popover commands.
    bool HandleCommandInternal(HTMLButtonElement &invoker, dom::CommandType command) noexcept override;

  protected:
#pragma endregion

#pragma region Extensibility Hooks

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/infrastructure.html#html-element-insertion-steps
    void OnInsert() noexcept override
    {
      // 1. If insertedNode is an element whose namespace is the HTML namespace, and this standard defines
      //    HTML element insertion steps for insertedNode's local name, then run the corresponding HTML
      //    element insertion steps given insertedNode.
      // (Elements with insertion steps override this, and the autofocus insertion steps run for all of them.)
      html::FocusAlgorithms::AutofocusInsertionSteps(*this);

      // 2. If insertedNode is a form-associated element or the ancestor of a form-associated element: if the
      //    form-associated element's parser inserted flag is set, then return; reset the form owner of the
      //    form-associated element.
      // (Each form-associated element resets its own form owner when inserted, in FormAssociatedElement.)

      // 3. If insertedNode is an Element that is not on the stack of open elements of an HTML parser, then
      //    process internal resource links given insertedNode's node document.
      // (Skipped: internal resource links aren't processed; see KNOWN-ISSUES.md.)
    }

    /// @see https://html.spec.whatwg.org/multipage/infrastructure.html#html-element-removing-steps
    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override
    {
      (void)isSubtreeRoot;
      (void)oldAncestor;

      // 1. Let document be removedNode's node document.
      // 2. If document's focused area is removedNode, then set document's focused area to document's
      //    viewport, and set document's relevant global object's navigation API's focus changed during
      //    ongoing navigation to false.
      // (Done before the removal, in Document::NodeWillBeRemoved.)

      // 3. If removedNode is an element whose namespace is the HTML namespace, and this standard defines HTML
      //    element removing steps for removedNode's local name, then run the corresponding HTML element
      //    removing steps given removedNode, isSubtreeRoot, and oldAncestor.
      // (Elements with removing steps override this.)

      // 4. If removedNode is a form-associated element with a non-null form owner and removedNode and its
      //    form owner are no longer in the same tree, then reset the form owner of removedNode.
      // (In FormAssociatedElement::ElementRemovedFromAncestor.)

      // 5. If removedNode's popover attribute is not in the No Popover state, then run the hide popover
      //    algorithm given removedNode, false, false, false, and null.
      if (GetPopoverData())
      {
        (void)HidePopoverInternal(dom::FocusPreviousElement::No, dom::FireEvents::No, false, nullptr);
      }
    }

    /// @see https://html.spec.whatwg.org/multipage/infrastructure.html#html-element-moving-steps
    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override
    {
      (void)isSubtreeRoot;
      (void)oldAncestor;

      // 1. If movedNode is an element whose namespace is the HTML namespace, and this standard defines HTML
      //    element moving steps for movedNode's local name, then run the corresponding HTML element moving
      //    steps given movedNode, isSubtreeRoot, and oldAncestor.
      // (Elements with moving steps override this.)

      // 2. If movedNode is a form-associated element with a non-null form owner and movedNode and its form
      //    owner are no longer in the same tree, then reset the form owner of movedNode.
      // (Each form-associated element's override treats a move as a removal and an insertion.)
    }

#pragma endregion

  public:
    /// @brief Whether the element is the HTML element named by @p name.
    KRYS_NODISCARD bool HasTagName(html::TagName name) const noexcept
    {
      return TagQualifiedName().TagName() == name;
    }

    /// @brief Whether the element is a text control's inner text element, which holds its editable text.
    KRYS_NODISCARD virtual bool IsTextControlInnerTextElement() const noexcept
    {
      return false;
    }

    /// @brief Whether the element is one of these parts of an input's shadow tree.
    KRYS_NODISCARD virtual bool IsSearchFieldResultsButtonElement() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSpinButtonElement() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSliderThumbElement() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSliderContainerElement() const noexcept
    {
      return false;
    }

#pragma region Form Controls

    /// @brief The element as a form-associated, listed or validated listed element, or null if it isn't one.
    KRYS_NODISCARD virtual html::FormAssociatedElement *AsFormAssociatedElement() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual html::FormListedElement *AsFormListedElement() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual html::ValidatedFormListedElement *AsValidatedFormListedElement() noexcept
    {
      return nullptr;
    }

    /// @see https://html.spec.whatwg.org/multipage/forms.html#category-label
    KRYS_NODISCARD virtual bool IsLabelable() const noexcept
    {
      return false;
    }

    /// @brief Whether the element can be actually disabled: a form control, an optgroup or an option.
    /// @see https://html.spec.whatwg.org/multipage/semantics-other.html#concept-element-disabled
    KRYS_NODISCARD bool CanBeActuallyDisabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/semantics-other.html#concept-element-disabled
    KRYS_NODISCARD virtual bool IsActuallyDisabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/dom.html#interactive-content-2
    KRYS_NODISCARD virtual bool IsInteractiveContent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept override;

    /// @brief The labels of a labelable element, in tree order; null for other elements.
    /// @see https://html.spec.whatwg.org/multipage/forms.html#dom-lfe-labels
    KRYS_NODISCARD RefPtr<NodeList> Labels() noexcept;

#pragma endregion

  protected:
#pragma region Type Checks

    // Each IsHTMLXElement says whether the element's interface is HTMLXElement.

    /// @brief Whether the element is a form control: a button, fieldset, input, output, select or textarea.
    KRYS_NODISCARD bool IsFormControlElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Button
             || _interface == html::HTMLElementInterface::FieldSet
             || _interface == html::HTMLElementInterface::Input
             || _interface == html::HTMLElementInterface::Output
             || _interface == html::HTMLElementInterface::Select
             || _interface == html::HTMLElementInterface::TextArea;
    }

    KRYS_NODISCARD bool IsHTMLFontElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Font;
    }

    KRYS_NODISCARD bool IsHTMLFrameSetElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::FrameSet;
    }

    KRYS_NODISCARD bool IsHTMLMarqueeElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Marquee;
    }

    KRYS_NODISCARD bool IsHTMLTablePartElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TableCell
             || _interface == html::HTMLElementInterface::TableCol
             || _interface == html::HTMLElementInterface::TableRow
             || _interface == html::HTMLElementInterface::TableSection;
    }

    KRYS_NODISCARD bool IsHTMLPlugInElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Embed
             || _interface == html::HTMLElementInterface::Object;
    }

    KRYS_NODISCARD bool IsHTMLAreaElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Area;
    }

    KRYS_NODISCARD bool IsHTMLAudioElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Audio;
    }

    KRYS_NODISCARD bool IsHTMLAnchorElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Anchor;
    }

    KRYS_NODISCARD bool IsHTMLBaseElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Base;
    }

    KRYS_NODISCARD bool IsHTMLBodyElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Body;
    }

    KRYS_NODISCARD bool IsHTMLBRElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::BR;
    }

    KRYS_NODISCARD bool IsHTMLButtonElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Button;
    }

    KRYS_NODISCARD bool IsHTMLCanvasElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Canvas;
    }

    KRYS_NODISCARD bool IsHTMLDataElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Data;
    }

    KRYS_NODISCARD bool IsHTMLDataListElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::DataList;
    }

    KRYS_NODISCARD bool IsHTMLDetailsElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Details;
    }

    KRYS_NODISCARD bool IsHTMLDialogElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Dialog;
    }

    KRYS_NODISCARD bool IsHTMLDirectoryElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Directory;
    }

    KRYS_NODISCARD bool IsHTMLDivElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Div;
    }

    KRYS_NODISCARD bool IsHTMLDListElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::DList;
    }

    KRYS_NODISCARD bool IsHTMLEmbedElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Embed;
    }

    KRYS_NODISCARD bool IsHTMLFieldSetElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::FieldSet;
    }

    KRYS_NODISCARD bool IsHTMLFormElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Form;
    }

    KRYS_NODISCARD bool IsHTMLHeadElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Head;
    }

    KRYS_NODISCARD bool IsHTMLHeadingElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Heading;
    }

    KRYS_NODISCARD bool IsHTMLHRElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::HR;
    }

    KRYS_NODISCARD bool IsHTMLHtmlElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Html;
    }

    KRYS_NODISCARD bool IsHTMLIFrameElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::IFrame;
    }

    KRYS_NODISCARD bool IsHTMLImageElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Image;
    }

    KRYS_NODISCARD bool IsHTMLInputElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Input;
    }

    KRYS_NODISCARD bool IsHTMLLabelElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Label;
    }

    KRYS_NODISCARD bool IsHTMLLegendElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Legend;
    }

    KRYS_NODISCARD bool IsHTMLLIElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::LI;
    }

    KRYS_NODISCARD bool IsHTMLLinkElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Link;
    }

    KRYS_NODISCARD bool IsHTMLMapElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Map;
    }

    KRYS_NODISCARD bool IsHTMLMediaElement() const noexcept
    {
      return _htmlElementFlags.contains(html::HTMLElementFlag::IsMediaElement);
    }

    KRYS_NODISCARD bool IsHTMLMenuElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Menu;
    }

    KRYS_NODISCARD bool IsHTMLMetaElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Meta;
    }

    KRYS_NODISCARD bool IsHTMLMeterElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Meter;
    }

    KRYS_NODISCARD bool IsHTMLModElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Mod;
    }

    KRYS_NODISCARD bool IsHTMLObjectElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Object;
    }

    KRYS_NODISCARD bool IsHTMLOListElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::OList;
    }

    KRYS_NODISCARD bool IsHTMLOptGroupElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::OptGroup;
    }

    KRYS_NODISCARD bool IsHTMLOptionElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Option;
    }

    KRYS_NODISCARD bool IsHTMLOutputElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Output;
    }

    KRYS_NODISCARD bool IsHTMLParagraphElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Paragraph;
    }

    KRYS_NODISCARD bool IsHTMLParamElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Param;
    }

    KRYS_NODISCARD bool IsHTMLPictureElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Picture;
    }

    KRYS_NODISCARD bool IsHTMLPreElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Pre;
    }

    KRYS_NODISCARD bool IsHTMLProgressElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Progress;
    }

    KRYS_NODISCARD bool IsHTMLQuoteElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Quote;
    }

    KRYS_NODISCARD bool IsHTMLScriptElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Script;
    }

    KRYS_NODISCARD bool IsHTMLSelectElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Select;
    }

    KRYS_NODISCARD bool IsHTMLSelectedContentElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::SelectedContent;
    }

    // NOTE: HTMLSlotElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLSourceElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Source;
    }

    KRYS_NODISCARD bool IsHTMLSpanElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Span;
    }

    KRYS_NODISCARD bool IsHTMLStyleElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Style;
    }

    KRYS_NODISCARD bool IsHTMLSummaryElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Summary;
    }

    KRYS_NODISCARD bool IsHTMLTableElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Table;
    }

    KRYS_NODISCARD bool IsHTMLTableCaptionElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TableCaption;
    }

    KRYS_NODISCARD bool IsHTMLTableCellElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TableCell;
    }

    KRYS_NODISCARD bool IsHTMLTableColElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TableCol;
    }

    KRYS_NODISCARD bool IsHTMLTableRowElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TableRow;
    }

    KRYS_NODISCARD bool IsHTMLTableSectionElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TableSection;
    }

    KRYS_NODISCARD bool IsHTMLTemplateElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Template;
    }

    KRYS_NODISCARD bool IsHTMLTextAreaElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::TextArea;
    }

    KRYS_NODISCARD bool IsHTMLTitleElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Title;
    }

    KRYS_NODISCARD bool IsHTMLTimeElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Time;
    }

    KRYS_NODISCARD bool IsHTMLTrackElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Track;
    }

    KRYS_NODISCARD bool IsHTMLUListElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::UList;
    }

    KRYS_NODISCARD bool IsHTMLMaybeFormAssociatedCustomElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::MaybeFormAssociatedCustom;
    }

    // NOTE: HTMLUnknownElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLVideoElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::Video;
    }

    KRYS_NODISCARD bool IsHTMLBDIElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::BDI;
    }

    KRYS_NODISCARD bool IsHTMLWBRElement() const noexcept
    {
      return _interface == html::HTMLElementInterface::WBR;
    }

#pragma endregion

  private:
    enum class AllowPercentage : bool
    {
      No,
      Yes,
    };

    enum class UseCSSPXAsUnitType : bool
    {
      No,
      Yes,
    };

    enum class IsMultiLength : bool
    {
      No,
      Yes,
    };

    /// @brief Maps a dimension attribute to @p property, allowing the forms the parameters say.
    void AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                              DOMStringView value, AllowPercentage allowPercentage,
                              UseCSSPXAsUnitType useCSSPX, IsMultiLength isMultiLength,
                              AllowZeroValue allowZeroValue = AllowZeroValue::Yes) noexcept;

    /// @brief Sets `aspect-ratio: auto width / height`.
    static void AddParsedWidthAndHeightToAspectRatioList(double width, double height,
                                                         css::MutableStyleProperties &style) noexcept;

    /// @brief Maps a `lang` attribute to the locale the style uses (auto for an empty one).
    void MapLanguageAttributeToLocale(DOMStringView value, css::MutableStyleProperties &style) noexcept;

    /// @brief Whether the popover can be shown (@p expectedToBeShowing false) or hidden: false if it already
    /// is.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#check-popover-validity
    KRYS_NODISCARD ExceptionOr<bool> CheckPopoverValidity(bool expectedToBeShowing,
                                                          Document *expectedDocument) const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#popover-focusing-steps
    void RunPopoverFocusingSteps() noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#queue-a-popover-toggle-event-task
    void QueuePopoverToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState,
                                     HTMLElement *source) noexcept;

    /// @return Whether the event wasn't canceled.
    bool FirePopoverBeforeToggleEvent(dom::ToggleState oldState, dom::ToggleState newState, bool cancelable,
                                      HTMLElement *source) noexcept;

    /// @brief Sets the popover's trigger, which becomes its invoked popover's invoker.
    void SetInvoker(HTMLElement *invoker) noexcept;

    /// @brief Follows a change to the `popover` attribute, hiding the popover if it's showing.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#the-popover-attribute
    void PopoverAttributeChanged(Maybe<DOMStringView> value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
