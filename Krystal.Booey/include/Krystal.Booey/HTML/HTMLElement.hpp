#pragma once

#include "Krystal.Booey/DOM/Enums/PopoverState.hpp"
#include "Krystal.Booey/DOM/StyledElement.hpp"
#include "Krystal.Booey/DOM/ToggleEventTask.hpp"
#include "Krystal.Booey/HTML/Algorithms/FocusAlgorithms.hpp"
#include "Krystal.Booey/HTML/Dicts/ShowPopoverOptions.hpp"
#include "Krystal.Booey/HTML/Dicts/TogglePopoverOptions.hpp"
#include "Krystal.Booey/HTML/Enums/HTMLElementFlags.hpp"
#include "Krystal.Booey/HTML/Enums/HTMLElementInterface.hpp"
#include "Krystal.Booey/HTML/EventHandlerAttributes.hpp"
#include "Krystal.Booey/HTML/Mixins/HTMLOrSVGOrMathMLElement.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::dom
{
  class NodeList;
}

namespace krys::boo::html
{
  class ElementInternals;
  class FormAssociatedElement;
  class FormListedElement;
  class ValidatedFormListedElement;

  /// @see https://html.spec.whatwg.org/#htmlelement
  class HTMLElement : public dom::StyledElement
  {
    KRYS_TYPE_CAST_TRAITS_ACCESS();
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLElement);

  private:
    HTMLElementFlags _flags : BitCount<HTMLElementFlags>() {HTMLElementFlags::None};
    HTMLElementInterface _interface : BitCount<HTMLElementInterface>() {HTMLElementInterface::None};

  protected:
    HTMLElement(dom::Document &document, HTMLElementInterface interface,
                HTMLElementFlags flags = HTMLElementFlags::None,
                dom::NodeFlags nodeFlags = dom::NodeFlags::None) noexcept;
    /// @brief Constructs an element with its tag name, for elements made outside the element factory (the
    /// elements of user agent shadow trees), which otherwise gives elements their names.
    HTMLElement(dom::Document &document, HTMLElementInterface interface, dom::DOMStringAtom localName,
                HTMLElementFlags flags, dom::NodeFlags nodeFlags) noexcept;

  public:
    HTMLElement(dom::Document &document) noexcept;

#pragma region HTMLElement - https://html.spec.whatwg.org/#htmlelement

    /// @see https://html.spec.whatwg.org/#dom-title
    KRYS_NODISCARD dom::DOMString Title() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-title
    void Title(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-lang
    KRYS_NODISCARD dom::DOMString Lang() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-lang
    void Lang(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-translate
    KRYS_NODISCARD bool Translate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-translate
    void Translate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dir
    KRYS_NODISCARD dom::DOMString Dir() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-dir
    void Dir(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    KRYS_NODISCARD BoolOr<dom::DOMString> Hidden() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    void Hidden(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    void Hidden(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-hidden
    void Hidden(double value) noexcept;

    /// @brief Whether the `hidden` attribute is in the hidden until found state.
    KRYS_NODISCARD bool IsHiddenUntilFound() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-inert
    KRYS_NODISCARD bool Inert() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-inert
    void Inert(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-click
    void Click() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-accesskey
    KRYS_NODISCARD dom::DOMString AccessKey() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-accesskey
    void AccessKey(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-accesskeylabel
    KRYS_NODISCARD dom::DOMString AccessKeyLabel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-draggable
    KRYS_NODISCARD bool Draggable() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-draggable
    void Draggable(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-spellcheck
    KRYS_NODISCARD bool SpellCheck() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-spellcheck
    void SpellCheck(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-writingsuggestions
    KRYS_NODISCARD dom::DOMString WritingSuggestions() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-writingsuggestions
    void WritingSuggestions(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocapitalize
    KRYS_NODISCARD dom::DOMString Autocapitalize() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocapitalize
    void Autocapitalize(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocorrect
    KRYS_NODISCARD bool Autocorrect() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-autocorrect
    void Autocorrect(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-innertext
    KRYS_NODISCARD dom::DOMString InnerText() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-innertext
    void InnerText(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-outertext
    KRYS_NODISCARD dom::DOMString OuterText() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-outertext
    void OuterText(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-attachinternals
    KRYS_NODISCARD dom::ExceptionOr<Ref<ElementInternals>> AttachInternals() noexcept;

    /// @brief Shows the popover, invoked by @p options' source.
    /// @return A `NotSupportedError` if the element isn't a popover; an `InvalidStateError` if it isn't
    /// connected, its document isn't fully active, it's a modal dialog, another popover is being shown or
    /// hidden, or its `popover` attribute changes as other popovers hide.
    /// @see https://html.spec.whatwg.org/#dom-showpopover
    dom::ExceptionOr<void> ShowPopover(const ShowPopoverOptions &options = {}) noexcept;

    /// @return A `NotSupportedError` if the element isn't a popover; an `InvalidStateError` if it's a modal
    /// dialog.
    /// @see https://html.spec.whatwg.org/#dom-hidepopover
    dom::ExceptionOr<void> HidePopover() noexcept;

    /// @brief Hides the popover if it's showing, else shows it, unless @p options forces one or the other.
    /// @return Whether the popover is showing, or the exceptions of `ShowPopover` and `HidePopover`.
    /// @see https://html.spec.whatwg.org/#dom-togglepopover
    dom::ExceptionOr<bool>
      TogglePopover(const TogglePopoverOptionsOrBool &options = TogglePopoverOptions {}) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popover
    KRYS_NODISCARD Maybe<dom::DOMString> Popover() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popover
    void Popover(Maybe<dom::DOMString> &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingoffset
    KRYS_NODISCARD uint32 HeadingOffset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingoffset
    void HeadingOffset(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingreset
    KRYS_NODISCARD bool HeadingReset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-headingreset
    void HeadingReset(bool value) noexcept;

#pragma endregion

    /// @brief The colour a presentational attribute (`bgcolor`, `color`, `text`, ...) specifies, if any.
    /// @see https://html.spec.whatwg.org/#rules-for-parsing-a-legacy-colour-value
    KRYS_NODISCARD static Maybe<SRGBA<uint8>> ParseLegacyColorValue(dom::DOMStringView string) noexcept;

#pragma region HTMLOrSVGOrMathMLElement Mixin - https://html.spec.whatwg.org/#htmlorsvgormathmlelement

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    KRYS_NODISCARD int32 TabIndex() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-tabindex
    void TabIndex(int32 value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::TabIndex(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    KRYS_NODISCARD bool Autofocus() const noexcept
    {
      return html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this);
    }

    /// @see https://html.spec.whatwg.org/#dom-fe-autofocus
    void Autofocus(bool value) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Autofocus(*this, value);
    }

    /// @see https://html.spec.whatwg.org/#dom-focus
    void Focus(const dom::FocusOptions &options = {}) noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Focus(*this, options);
    }

    /// @brief Runs the element's access key action: focuses it if it's focusable, then clicks it.
    /// @return Whether the click was dispatched.
    virtual bool AccessKeyAction(bool sendMouseEvents) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-blur
    void Blur() noexcept
    {
      html::mixins::HTMLOrSVGOrMathMLElement::Blur(*this);
    }

#pragma endregion

#pragma region GlobalEventHandlers, DocumentAndElementEventHandlers - https://html.spec.whatwg.org/#event-handler-attributes

    KRYS_GLOBAL_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)
    KRYS_DOCUMENT_AND_ELEMENT_EVENT_HANDLERS(KRYS_DECLARE_EVENT_HANDLER)

#pragma endregion

    // TODO(HTMLELEMENT, HTML): HTMLElement mixins:
    // HTMLElement includes ElementContentEditable;

    /// @brief The editability of @p node from the `contenteditable` attributes of it and its ancestors, for
    /// nodes in shadow trees (whose style isn't used for editability).
    KRYS_NODISCARD static dom::Node::Editability
      EditabilityFromContentEditableAttr(const dom::Node &node) noexcept;

  protected:
#pragma region Presentational hints - https://html.spec.whatwg.org/#presentational-hints

    enum class AllowZeroValue : bool
    {
      No,
      Yes
    };

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

    /// @param value The `border` attribute's value, or null if there isn't one.
    KRYS_NODISCARD uint32 ParseBorderWidthAttribute(Maybe<dom::DOMStringView> value) const noexcept;
    void ApplyBorderAttributeToStyle(dom::DOMStringView value, css::MutableStyleProperties &style) noexcept;
    void ApplyAlignmentAttributeToStyle(dom::DOMStringView alignment,
                                        css::MutableStyleProperties &style) noexcept;
    void ApplyAspectRatioFromWidthAndHeightAttributesToStyle(dom::DOMStringView widthAttribute,
                                                             dom::DOMStringView heightAttribute,
                                                             css::MutableStyleProperties &style) noexcept;
    void ApplyAspectRatioWithoutDimensionalRulesFromWidthAndHeightAttributesToStyle(
      dom::DOMStringView widthAttribute, dom::DOMStringView heightAttribute,
      css::MutableStyleProperties &style) noexcept;

    // https://www.w3.org/TR/html4/sgml/dtd.html#Length, including pixel and percentage values.
    void AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                              dom::DOMStringView value,
                              AllowZeroValue allowZeroValue = AllowZeroValue::Yes) noexcept;
    // https://www.w3.org/TR/html4/sgml/dtd.html#MultiLength, including pixel, percentage, and relative
    // values.
    void AddHTMLMultiLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                                   dom::DOMStringView value) noexcept;
    // https://www.w3.org/TR/html4/sgml/dtd.html#Pixels, including pixel value.
    void AddHTMLPixelsToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                              dom::DOMStringView value) noexcept;
    void AddHTMLColorToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                             dom::DOMStringView attributeValue) noexcept;

    /// @brief The value of the attribute in the null namespace, or an empty view if there isn't one.
    KRYS_NODISCARD dom::DOMString AttributeOrEmpty(dom::DOMStringView localName) const noexcept;

#pragma endregion

#pragma region HTMLElement Flags

    void SetHTMLElementFlag(HTMLElementFlags flag) noexcept
    {
      _flags = _flags | flag;
    }

    void ClearHTMLElementFlag(HTMLElementFlags flag) noexcept
    {
      _flags = _flags & ~flag;
    }

    KRYS_NODISCARD bool HasHTMLElementFlag(HTMLElementFlags flag) const noexcept
    {
      return HasFlag(_flags, flag);
    }

#pragma endregion

#pragma region Popovers

  public:
    /// @brief Shows the popover, invoked by @p source; the exceptions of `ShowPopover` are returned only if
    /// @p throwExceptions.
    /// @see https://html.spec.whatwg.org/#show-popover
    dom::ExceptionOr<void> ShowPopoverInternal(HTMLElement *source, bool throwExceptions = true) noexcept;

    /// @brief Hides the popover, and the popovers above it in its stack; the exceptions of `HidePopover` are
    /// returned only if @p throwExceptions.
    /// @param source The element that hid it, which `beforetoggle` and `toggle` carry.
    /// @see https://html.spec.whatwg.org/#hide-popover-algorithm
    dom::ExceptionOr<void> HidePopoverInternal(dom::FocusPreviousElement focusPreviousElement,
                                               dom::FireEvents fireEvents, bool throwExceptions,
                                               HTMLElement *source) noexcept;

    /// @brief The popover commands, for a popover.
    KRYS_NODISCARD bool IsValidCommandType(dom::CommandType command) const noexcept override;
    /// @brief Shows or hides the element, if it's a popover, for the popover commands.
    bool HandleCommandInternal(HTMLButtonElement &invoker, dom::CommandType command) noexcept override;

  protected:
#pragma endregion

#pragma region Extensibility Hooks

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    void OnInsert() noexcept override
    {
      FocusAlgorithms::AutofocusInsertionSteps(*this);

      // TODO(HTMLELEMENT): Implement the insertion steps for an HTMLElement.
      // If insertedNode is a form-associated element or the ancestor of a form-associated element:
      // - If the form-associated element's parser inserted flag is set, then return.
      // - Reset the form owner of the form-associated element.
      //
      // If insertedNode is an Element that is not on the stack of open elements of an HTML parser, then
      // process internal resource links given insertedNode's node document.
    }

    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override
    {
      (void)isSubtreeRoot;
      (void)oldAncestor;
      // TODO(HTMLELEMENT): Implement the removal steps for an HTMLElement.

      // Let document be removedNode's node document.

      // If document's focused area is removedNode, then set document's focused area to document's viewport,
      // and set document's relevant global object's navigation API's focus changed during ongoing navigation
      // to false.
      //
      // This does not perform the unfocusing steps, focusing steps, or focus update steps, and thus no blur
      // or change events are fired.
      //
      // If removedNode is an element whose namespace is the HTML namespace, and this standard defines HTML
      // element removing steps for removedNode's local name, then run the corresponding HTML element removing
      // steps given removedNode, isSubtreeRoot, and oldAncestor.
      //
      // If removedNode is a form-associated element with a non-null form owner and removedNode and its form
      // owner are no longer in the same tree, then reset the form owner of removedNode.
      //

      // If removedNode's popover attribute is not in the No Popover state, then run the hide popover
      // algorithm given removedNode, false, false, false, and null.
      if (GetPopoverData())
      {
        (void)HidePopoverInternal(dom::FocusPreviousElement::No, dom::FireEvents::No, false, nullptr);
      }
    }

    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override
    {
      (void)isSubtreeRoot;
      (void)oldAncestor;
      // TODO(HTMLELEMENT): Implement the move steps for an HTMLElement.

      // If movedNode is an element whose namespace is the HTML namespace, and this standard defines HTML
      // element moving steps for movedNode's local name, then run the corresponding HTML element moving steps
      // given movedNode, isSubtreeRoot, and oldAncestor.

      // If movedNode is a form-associated element with a non-null form owner and movedNode and its form owner
      // are no longer in the same tree, then reset the form owner of movedNode.
    }

#pragma endregion

  public:
    /// @brief Whether the element is a text control's inner text element, which holds its editable text.
    KRYS_NODISCARD virtual bool IsTextControlInnerTextElement() const noexcept
    {
      return false;
    }

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

    KRYS_NODISCARD virtual FormAssociatedElement *AsFormAssociatedElement() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual FormListedElement *AsFormListedElement() noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual ValidatedFormListedElement *AsValidatedFormListedElement() noexcept
    {
      return nullptr;
    }

    /// @see https://html.spec.whatwg.org/#category-label
    KRYS_NODISCARD virtual bool IsLabelable() const noexcept
    {
      return false;
    }

    /// @brief Whether the element can be actually disabled: a form control, an optgroup or an option.
    /// @see https://html.spec.whatwg.org/#concept-element-disabled
    KRYS_NODISCARD bool CanBeActuallyDisabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#concept-element-disabled
    KRYS_NODISCARD virtual bool IsActuallyDisabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#interactive-content
    KRYS_NODISCARD virtual bool IsInteractiveContent() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept override;

    /// @brief The labels of a labelable element, in tree order; null for other elements.
    /// @see https://html.spec.whatwg.org/#dom-lfe-labels
    KRYS_NODISCARD RefPtr<dom::NodeList> Labels() noexcept;

#pragma endregion

  protected:
#pragma region Type Checks

    /// @brief Whether the element is a form control: a button, fieldset, input, output, select or textarea.
    KRYS_NODISCARD bool IsFormControlElement() const noexcept
    {
      return _interface == HTMLElementInterface::Button || _interface == HTMLElementInterface::FieldSet
             || _interface == HTMLElementInterface::Input || _interface == HTMLElementInterface::Output
             || _interface == HTMLElementInterface::Select || _interface == HTMLElementInterface::TextArea;
    }

    KRYS_NODISCARD bool IsHTMLFontElement() const noexcept
    {
      return _interface == HTMLElementInterface::Font;
    }

    KRYS_NODISCARD bool IsHTMLFrameSetElement() const noexcept
    {
      return _interface == HTMLElementInterface::FrameSet;
    }

    KRYS_NODISCARD bool IsHTMLMarqueeElement() const noexcept
    {
      return _interface == HTMLElementInterface::Marquee;
    }

    KRYS_NODISCARD bool IsHTMLTablePartElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCell || _interface == HTMLElementInterface::TableCol
             || _interface == HTMLElementInterface::TableRow
             || _interface == HTMLElementInterface::TableSection;
    }

    KRYS_NODISCARD bool IsHTMLPlugInElement() const noexcept
    {
      return _interface == HTMLElementInterface::Embed || _interface == HTMLElementInterface::Object;
    }

    KRYS_NODISCARD bool IsHTMLAreaElement() const noexcept
    {
      return _interface == HTMLElementInterface::Area;
    }

    KRYS_NODISCARD bool IsHTMLAudioElement() const noexcept
    {
      return _interface == HTMLElementInterface::Audio;
    }

    KRYS_NODISCARD bool IsHTMLAnchorElement() const noexcept
    {
      return _interface == HTMLElementInterface::Anchor;
    }

    KRYS_NODISCARD bool IsHTMLBaseElement() const noexcept
    {
      return _interface == HTMLElementInterface::Base;
    }

    KRYS_NODISCARD bool IsHTMLBodyElement() const noexcept
    {
      return _interface == HTMLElementInterface::Body;
    }

    KRYS_NODISCARD bool IsHTMLBRElement() const noexcept
    {
      return _interface == HTMLElementInterface::BR;
    }

    KRYS_NODISCARD bool IsHTMLButtonElement() const noexcept
    {
      return _interface == HTMLElementInterface::Button;
    }

    KRYS_NODISCARD bool IsHTMLCanvasElement() const noexcept
    {
      return _interface == HTMLElementInterface::Canvas;
    }

    KRYS_NODISCARD bool IsHTMLDataElement() const noexcept
    {
      return _interface == HTMLElementInterface::Data;
    }

    KRYS_NODISCARD bool IsHTMLDataListElement() const noexcept
    {
      return _interface == HTMLElementInterface::DataList;
    }

    KRYS_NODISCARD bool IsHTMLDetailsElement() const noexcept
    {
      return _interface == HTMLElementInterface::Details;
    }

    KRYS_NODISCARD bool IsHTMLDialogElement() const noexcept
    {
      return _interface == HTMLElementInterface::Dialog;
    }

    KRYS_NODISCARD bool IsHTMLDivElement() const noexcept
    {
      return _interface == HTMLElementInterface::Div;
    }

    KRYS_NODISCARD bool IsHTMLDListElement() const noexcept
    {
      return _interface == HTMLElementInterface::DList;
    }

    KRYS_NODISCARD bool IsHTMLEmbedElement() const noexcept
    {
      return _interface == HTMLElementInterface::Embed;
    }

    KRYS_NODISCARD bool IsHTMLFieldSetElement() const noexcept
    {
      return _interface == HTMLElementInterface::FieldSet;
    }

    KRYS_NODISCARD bool IsHTMLFormElement() const noexcept
    {
      return _interface == HTMLElementInterface::Form;
    }

    KRYS_NODISCARD bool IsHTMLHeadElement() const noexcept
    {
      return _interface == HTMLElementInterface::Head;
    }

    KRYS_NODISCARD bool IsHTMLHeadingElement() const noexcept
    {
      return _interface == HTMLElementInterface::Heading;
    }

    KRYS_NODISCARD bool IsHTMLHRElement() const noexcept
    {
      return _interface == HTMLElementInterface::HR;
    }

    KRYS_NODISCARD bool IsHTMLHtmlElement() const noexcept
    {
      return _interface == HTMLElementInterface::Html;
    }

    KRYS_NODISCARD bool IsHTMLIFrameElement() const noexcept
    {
      return _interface == HTMLElementInterface::IFrame;
    }

    KRYS_NODISCARD bool IsHTMLImageElement() const noexcept
    {
      return _interface == HTMLElementInterface::Image;
    }

    KRYS_NODISCARD bool IsHTMLInputElement() const noexcept
    {
      return _interface == HTMLElementInterface::Input;
    }

    KRYS_NODISCARD bool IsHTMLLabelElement() const noexcept
    {
      return _interface == HTMLElementInterface::Label;
    }

    KRYS_NODISCARD bool IsHTMLLegendElement() const noexcept
    {
      return _interface == HTMLElementInterface::Legend;
    }

    KRYS_NODISCARD bool IsHTMLLIElement() const noexcept
    {
      return _interface == HTMLElementInterface::LI;
    }

    KRYS_NODISCARD bool IsHTMLLinkElement() const noexcept
    {
      return _interface == HTMLElementInterface::Link;
    }

    KRYS_NODISCARD bool IsHTMLMapElement() const noexcept
    {
      return _interface == HTMLElementInterface::Map;
    }

    KRYS_NODISCARD bool IsHTMLMediaElement() const noexcept
    {
      return HasFlag(_flags, HTMLElementFlags::IsMediaElement);
    }

    KRYS_NODISCARD bool IsHTMLMenuElement() const noexcept
    {
      return _interface == HTMLElementInterface::Menu;
    }

    KRYS_NODISCARD bool IsHTMLMetaElement() const noexcept
    {
      return _interface == HTMLElementInterface::Meta;
    }

    KRYS_NODISCARD bool IsHTMLMeterElement() const noexcept
    {
      return _interface == HTMLElementInterface::Meter;
    }

    KRYS_NODISCARD bool IsHTMLModElement() const noexcept
    {
      return _interface == HTMLElementInterface::Mod;
    }

    KRYS_NODISCARD bool IsHTMLObjectElement() const noexcept
    {
      return _interface == HTMLElementInterface::Object;
    }

    KRYS_NODISCARD bool IsHTMLOListElement() const noexcept
    {
      return _interface == HTMLElementInterface::OList;
    }

    KRYS_NODISCARD bool IsHTMLOptGroupElement() const noexcept
    {
      return _interface == HTMLElementInterface::OptGroup;
    }

    KRYS_NODISCARD bool IsHTMLOptionElement() const noexcept
    {
      return _interface == HTMLElementInterface::Option;
    }

    KRYS_NODISCARD bool IsHTMLOutputElement() const noexcept
    {
      return _interface == HTMLElementInterface::Output;
    }

    KRYS_NODISCARD bool IsHTMLParagraphElement() const noexcept
    {
      return _interface == HTMLElementInterface::Paragraph;
    }

    KRYS_NODISCARD bool IsHTMLPictureElement() const noexcept
    {
      return _interface == HTMLElementInterface::Picture;
    }

    KRYS_NODISCARD bool IsHTMLPreElement() const noexcept
    {
      return _interface == HTMLElementInterface::Pre;
    }

    KRYS_NODISCARD bool IsHTMLProgressElement() const noexcept
    {
      return _interface == HTMLElementInterface::Progress;
    }

    KRYS_NODISCARD bool IsHTMLQuoteElement() const noexcept
    {
      return _interface == HTMLElementInterface::Quote;
    }

    KRYS_NODISCARD bool IsHTMLScriptElement() const noexcept
    {
      return _interface == HTMLElementInterface::Script;
    }

    KRYS_NODISCARD bool IsHTMLSelectElement() const noexcept
    {
      return _interface == HTMLElementInterface::Select;
    }

    KRYS_NODISCARD bool IsHTMLSelectedContentElement() const noexcept
    {
      return _interface == HTMLElementInterface::SelectedContent;
    }

    // NOTE: HTMLSlotElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLSourceElement() const noexcept
    {
      return _interface == HTMLElementInterface::Source;
    }

    KRYS_NODISCARD bool IsHTMLSpanElement() const noexcept
    {
      return _interface == HTMLElementInterface::Span;
    }

    KRYS_NODISCARD bool IsHTMLStyleElement() const noexcept
    {
      return _interface == HTMLElementInterface::Style;
    }

    KRYS_NODISCARD bool IsHTMLSummaryElement() const noexcept
    {
      return _interface == HTMLElementInterface::Summary;
    }

    KRYS_NODISCARD bool IsHTMLTableElement() const noexcept
    {
      return _interface == HTMLElementInterface::Table;
    }

    KRYS_NODISCARD bool IsHTMLTableCaptionElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCaption;
    }

    KRYS_NODISCARD bool IsHTMLTableCellElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCell;
    }

    KRYS_NODISCARD bool IsHTMLTableColElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableCol;
    }

    KRYS_NODISCARD bool IsHTMLTableRowElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableRow;
    }

    KRYS_NODISCARD bool IsHTMLTableSectionElement() const noexcept
    {
      return _interface == HTMLElementInterface::TableSection;
    }

    KRYS_NODISCARD bool IsHTMLTemplateElement() const noexcept
    {
      return _interface == HTMLElementInterface::Template;
    }

    KRYS_NODISCARD bool IsHTMLTextAreaElement() const noexcept
    {
      return _interface == HTMLElementInterface::TextArea;
    }

    KRYS_NODISCARD bool IsHTMLTitleElement() const noexcept
    {
      return _interface == HTMLElementInterface::Title;
    }

    KRYS_NODISCARD bool IsHTMLTimeElement() const noexcept
    {
      return _interface == HTMLElementInterface::Time;
    }

    KRYS_NODISCARD bool IsHTMLTrackElement() const noexcept
    {
      return _interface == HTMLElementInterface::Track;
    }

    KRYS_NODISCARD bool IsHTMLUListElement() const noexcept
    {
      return _interface == HTMLElementInterface::UList;
    }

    KRYS_NODISCARD bool IsHTMLMaybeFormAssociatedCustomElement() const noexcept
    {
      return _interface == HTMLElementInterface::MaybeFormAssociatedCustom;
    }

    // NOTE: HTMLUnknownElement type check is not needed here as Node already has it.

    KRYS_NODISCARD bool IsHTMLVideoElement() const noexcept
    {
      return _interface == HTMLElementInterface::Video;
    }

    KRYS_NODISCARD bool IsHTMLWBRElement() const noexcept
    {
      return _interface == HTMLElementInterface::WBR;
    }

#pragma endregion
  private:
    enum class AllowPercentage : bool
    {
      No,
      Yes
    };

    enum class UseCSSPXAsUnitType : bool
    {
      No,
      Yes
    };

    enum class IsMultiLength : bool
    {
      No,
      Yes
    };

    void AddHTMLLengthToStyle(css::MutableStyleProperties &style, css::PropertyId property,
                              dom::DOMStringView value, AllowPercentage allowPercentage,
                              UseCSSPXAsUnitType useCSSPX, IsMultiLength isMultiLength,
                              AllowZeroValue allowZeroValue = AllowZeroValue::Yes) noexcept;
    void AddParsedWidthAndHeightToAspectRatioList(double width, double height,
                                                  css::MutableStyleProperties &style) noexcept;
    void MapLanguageAttributeToLocale(dom::DOMStringView value, css::MutableStyleProperties &style) noexcept;

    /// @brief Whether the popover can be shown (@p expectedToBeShowing false) or hidden: false if it already
    /// is.
    /// @see https://html.spec.whatwg.org/#check-popover-validity
    KRYS_NODISCARD dom::ExceptionOr<bool>
      CheckPopoverValidity(bool expectedToBeShowing, dom::Document *expectedDocument) const noexcept;
    /// @see https://html.spec.whatwg.org/#popover-focusing-steps
    void RunPopoverFocusingSteps() noexcept;
    /// @see https://html.spec.whatwg.org/#queue-a-popover-toggle-event-task
    void QueuePopoverToggleEventTask(dom::ToggleState oldState, dom::ToggleState newState,
                                     HTMLElement *source) noexcept;
    /// @return Whether the event wasn't canceled.
    bool FirePopoverBeforeToggleEvent(dom::ToggleState oldState, dom::ToggleState newState, bool cancelable,
                                      HTMLElement *source) noexcept;
    /// @brief Sets the popover's trigger, which becomes its invoked popover's invoker.
    void SetInvoker(HTMLElement *invoker) noexcept;
    void PopoverAttributeChanged(Maybe<dom::DOMStringView> value) noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();