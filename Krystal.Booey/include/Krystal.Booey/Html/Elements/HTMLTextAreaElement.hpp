#pragma once

#include "Krystal.Booey/Html/Elements/HTMLTextFormControlElement.hpp"

namespace krys::boo
{
  class BeforeTextInsertedEvent;

  /// @see https://html.spec.whatwg.org/multipage/form-elements.html#htmltextareaelement
  class HTMLTextAreaElement : public HTMLTextFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTextAreaElement);

  private:
    constexpr static uint32 DefaultRows = 2;
    constexpr static uint32 DefaultCols = 20;

    enum WrapMethod : uint8
    {
      NoWrap,
      SoftWrap,
      HardWrap,
    };

    mutable DOMString _value;
    RefPtr<HTMLElement> _placeholder;
    WrapMethod _wrap {SoftWrap};
    mutable bool _isDirty {false};
    mutable bool _wasModifiedByUser {false};

  public:
    HTMLTextAreaElement(Document &document) noexcept;

    /// @brief A text area with its user agent shadow tree.
    KRYS_NODISCARD static Ref<HTMLTextAreaElement> Create(Document &document) noexcept;

#pragma region HTMLTextAreaElement - https://html.spec.whatwg.org/multipage/form-elements.html#htmltextareaelement

    /// @see https://html.spec.whatwg.org/#dom-textarea-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-autocomplete
    void Autocomplete(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-cols
    KRYS_NODISCARD uint32 Cols() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-cols
    void Cols(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-dirname
    KRYS_NODISCARD DOMString DirName() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-dirname
    void DirName(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-disabled
    void Disabled(bool value) noexcept;

    using HTMLTextFormControlElement::MaxLength;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-maxlength
    void MaxLength(int32 value) noexcept;

    using HTMLTextFormControlElement::MinLength;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-minlength
    void MinLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-name
    KRYS_NODISCARD DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-placeholder
    KRYS_NODISCARD DOMString Placeholder() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-placeholder
    void Placeholder(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-readonly
    KRYS_NODISCARD bool ReadOnly() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-readonly
    void ReadOnly(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-rows
    KRYS_NODISCARD uint32 Rows() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-rows
    void Rows(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-wrap
    KRYS_NODISCARD DOMString Wrap() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-wrap
    void Wrap(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-type
    KRYS_NODISCARD DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @brief The text content of the element.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-defaultvalue
    KRYS_NODISCARD DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-defaultvalue
    void DefaultValue(DOMString value) noexcept;

    /// @brief The raw value.
    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-value
    KRYS_NODISCARD DOMString Value() const noexcept final;

    ExceptionOr<void>
      SetValue(DOMStringView value,
               html::TextFieldEventBehavior eventBehavior = html::TextFieldEventBehavior::DispatchNoEvent,
               html::TextControlSetValueSelection selection =
                 html::TextControlSetValueSelection::SetSelectionToEnd) noexcept final;

    /// @see https://html.spec.whatwg.org/multipage/form-elements.html#dom-textarea-textlength
    KRYS_NODISCARD uint32 TextLength() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange
    void SetSelectionRangeForBindings(uint32 start, uint32 end, DOMStringView direction = {}) noexcept;

#pragma endregion

    KRYS_NODISCARD DOMString FormControlType() const noexcept final
    {
      return u8"textarea";
    }

    KRYS_NODISCARD bool IsTextField() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool DirAutoUsesValue() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool ShouldUseInputMethod() noexcept final
    {
      return true;
    }

    KRYS_NODISCARD HTMLElement *PlaceholderElement() const noexcept final
    {
      return _placeholder.get();
    }

    KRYS_NODISCARD RefPtr<html::TextControlInnerTextElement> InnerTextElement() const noexcept final;

    RefPtr<html::TextControlInnerTextElement> InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept final;

    KRYS_NODISCARD RenderStyle CreateInnerTextStyle(const RenderStyle &style) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;

    /// @brief Focuses the text area, without clicking it.
    bool AccessKeyAction(bool sendMouseEvents) noexcept override;

    void UpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                               SelectionRevealMode revealMode = SelectionRevealMode::Reveal) noexcept final;

    void DefaultEventHandler(Event &event) noexcept final;

    KRYS_NODISCARD bool IsEnumeratable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsLabelable() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsInteractiveContent() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool SupportsPlaceholder() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsOptionalFormControl() const noexcept final
    {
      return !IsRequiredFormControl();
    }

    KRYS_NODISCARD bool IsRequiredFormControl() const noexcept final
    {
      return IsRequired();
    }

    KRYS_NODISCARD bool MatchesReadWritePseudoClass() const noexcept final
    {
      return IsMutable();
    }

    KRYS_NODISCARD bool ValueMissing() const noexcept final;

    KRYS_NODISCARD bool TooShort() const noexcept final;

    KRYS_NODISCARD bool TooLong() const noexcept final;

    /// @brief Whether @p candidate would be a valid value.
    KRYS_NODISCARD bool IsValidValue(DOMStringView candidate) const noexcept;

    bool AppendFormData(html::DOMFormData &formData) noexcept final;

    void Reset() noexcept final;

  protected:
    KRYS_NODISCARD bool IsEmptyValue() const noexcept final
    {
      return Value().empty();
    }

    void DidAddUserAgentShadowRoot(boo::ShadowRoot &root) noexcept final;

    void UpdatePlaceholderText() noexcept final;

    void SubtreeHasChanged() noexcept final;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept final;

    void OnChildrenChanged() noexcept final;

    void OnClone(Node &copy, bool subtree) noexcept final;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                         const DOMStringAtom &namespaceURI) const noexcept override;

    void CollectPresentationalHintsForAttribute(const DOMStringAtom &localName,
                                                const DOMStringAtom &namespaceURI, DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

  private:
    /// @brief Takes the text the user edited as the value, if the inner text changed since the value was set.
    void UpdateValue() const noexcept;

    /// @brief Cuts text the user inserts to what `maxlength` leaves room for.
    void HandleBeforeTextInsertedEvent(BeforeTextInsertedEvent &event) const noexcept;

    /// @brief @p proposedValue cut to at most @p maxLength, as maxlength counts it.
    KRYS_NODISCARD static DOMString SanitizeUserInputValue(DOMStringView proposedValue,
                                                           size_t maxLength) noexcept;

    /// @brief Sets the value from the default value, leaving the dirty flag unset.
    void SetNonDirtyValue(DOMStringView value, html::TextControlSetValueSelection selection) noexcept;

    /// @brief Sets the value (with its line breaks normalized) and the inner text, moving the selection as
    /// @p selection asks.
    void SetValueCommon(DOMStringView newValue, html::TextFieldEventBehavior eventBehavior,
                        html::TextControlSetValueSelection selection) noexcept;

    /// @brief The validity states for @p value, or for the value if it's null.
    KRYS_NODISCARD bool ValueMissing(Maybe<DOMStringView> value) const noexcept;

    KRYS_NODISCARD bool TooShort(Maybe<DOMStringView> value, NeedsToCheckDirtyFlag check) const noexcept;

    KRYS_NODISCARD bool TooLong(Maybe<DOMStringView> value, NeedsToCheckDirtyFlag check) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLTextAreaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLTextAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLTextAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
