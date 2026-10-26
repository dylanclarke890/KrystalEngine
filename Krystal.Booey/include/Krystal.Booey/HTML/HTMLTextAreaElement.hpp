#pragma once

#include "Krystal.Booey/HTML/HTMLTextFormControlElement.hpp"

namespace krys::boo::dom
{
  class BeforeTextInsertedEvent;
}

namespace krys::boo::html
{
  /// @see https://html.spec.whatwg.org/#htmltextareaelement
  class HTMLTextAreaElement : public HTMLTextFormControlElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLTextAreaElement);

  private:
    static constexpr uint32 DefaultRows = 2;
    static constexpr uint32 DefaultCols = 20;

    enum WrapMethod : uint8
    {
      NoWrap,
      SoftWrap,
      HardWrap
    };

    mutable dom::DOMString _value;
    RefPtr<HTMLElement> _placeholder;
    WrapMethod _wrap {SoftWrap};
    mutable bool _isDirty {false};
    mutable bool _wasModifiedByUser {false};

  public:
    HTMLTextAreaElement(dom::Document &document) noexcept;

    /// @brief A text area with its user agent shadow tree.
    KRYS_NODISCARD static Ref<HTMLTextAreaElement> Create(dom::Document &document) noexcept;

#pragma region HTMLTextAreaElement - https://html.spec.whatwg.org/#htmltextareaelement

    /// @see https://html.spec.whatwg.org/#dom-textarea-autocomplete
    KRYS_NODISCARD dom::DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-autocomplete
    void Autocomplete(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-cols
    KRYS_NODISCARD uint32 Cols() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-cols
    void Cols(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-dirname
    KRYS_NODISCARD dom::DOMString DirName() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-dirname
    void DirName(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-disabled
    KRYS_NODISCARD bool Disabled() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-disabled
    void Disabled(bool value) noexcept;

    using HTMLTextFormControlElement::MaxLength;

    /// @see https://html.spec.whatwg.org/#dom-textarea-maxlength
    void MaxLength(int32 value) noexcept;

    using HTMLTextFormControlElement::MinLength;

    /// @see https://html.spec.whatwg.org/#dom-textarea-minlength
    void MinLength(int32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-name
    KRYS_NODISCARD dom::DOMString Name() const noexcept override;

    /// @see https://html.spec.whatwg.org/#dom-textarea-name
    void Name(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-placeholder
    KRYS_NODISCARD dom::DOMString Placeholder() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-placeholder
    void Placeholder(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-readonly
    KRYS_NODISCARD bool ReadOnly() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-readonly
    void ReadOnly(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-required
    KRYS_NODISCARD bool Required() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-required
    void Required(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-rows
    KRYS_NODISCARD uint32 Rows() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-rows
    void Rows(uint32 value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-wrap
    KRYS_NODISCARD dom::DOMString Wrap() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-wrap
    void Wrap(dom::DOMString &&value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-type
    KRYS_NODISCARD dom::DOMString Type() const noexcept
    {
      return FormControlType();
    }

    /// @brief The text content of the element.
    /// @see https://html.spec.whatwg.org/#dom-textarea-defaultvalue
    KRYS_NODISCARD dom::DOMString DefaultValue() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea-defaultvalue
    void DefaultValue(dom::DOMString &&value) noexcept;

    /// @brief The raw value.
    /// @see https://html.spec.whatwg.org/#dom-textarea-value
    KRYS_NODISCARD dom::DOMString Value() const noexcept final;

    dom::ExceptionOr<void>
      SetValue(dom::DOMStringView value,
               TextFieldEventBehavior eventBehavior = TextFieldEventBehavior::DispatchNoEvent,
               TextControlSetValueSelection selection =
                 TextControlSetValueSelection::SetSelectionToEnd) noexcept final;

    /// @see https://html.spec.whatwg.org/#dom-textarea-textlength
    KRYS_NODISCARD uint32 TextLength() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-textarea/input-setselectionrange
    void SetSelectionRangeForBindings(uint32 start, uint32 end, dom::DOMStringView direction = {}) noexcept;

#pragma endregion

    KRYS_NODISCARD dom::DOMString FormControlType() const noexcept final
    {
      return u8"textarea";
    }

    KRYS_NODISCARD bool ShouldSaveAndRestoreFormControlState() const noexcept final
    {
      return true;
    }

    /// @brief The text area's value, if the user changed it.
    KRYS_NODISCARD FormControlState SaveFormControlState() const noexcept final;
    void RestoreFormControlState(const FormControlState &state) noexcept final;

    KRYS_NODISCARD bool IsTextField() const noexcept final
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

    KRYS_NODISCARD RefPtr<TextControlInnerTextElement> InnerTextElement() const noexcept final;
    RefPtr<TextControlInnerTextElement> InnerTextElementCreatingShadowSubtreeIfNeeded() noexcept final;
    KRYS_NODISCARD RenderStyle CreateInnerTextStyle(const RenderStyle &style) noexcept final;

    KRYS_NODISCARD RenderPtr<RenderElement> CreateElementRenderer(RenderStyle &&style) noexcept final;
    void UpdateFocusAppearance(dom::SelectionRestorationMode restorationMode,
                               SelectionRevealMode revealMode = SelectionRevealMode::Reveal) noexcept final;
    void DefaultEventHandler(dom::Event &event) noexcept final;

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
    KRYS_NODISCARD bool IsValidValue(dom::DOMStringView candidate) const noexcept;

    bool AppendFormData(DOMFormData &formData) noexcept final;
    void Reset() noexcept final;

  protected:
    KRYS_NODISCARD bool IsEmptyValue() const noexcept final
    {
      return Value().empty();
    }

    void DidAddUserAgentShadowRoot(dom::ShadowRoot &root) noexcept final;
    void UpdatePlaceholderText() noexcept final;
    void SubtreeHasChanged() noexcept final;

    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value, dom::DOMStringAtom namespaceURI) noexcept final;
    void OnChildrenChanged() noexcept final;
    void OnClone(dom::Node &copy, bool subtree) noexcept final;

    KRYS_NODISCARD bool
      HasPresentationalHintsForAttribute(dom::DOMStringAtom localName,
                                         dom::DOMStringAtom namespaceURI) const noexcept override;
    void CollectPresentationalHintsForAttribute(dom::DOMStringAtom localName, dom::DOMStringAtom namespaceURI,
                                                dom::DOMStringView value,
                                                css::MutableStyleProperties &style) noexcept override;

  private:
    /// @brief Takes the text the user edited as the value, if the inner text changed since the value was set.
    void UpdateValue() const noexcept;

    void HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &event) const noexcept;
    KRYS_NODISCARD static dom::DOMString SanitizeUserInputValue(dom::DOMStringView proposedValue,
                                                                size_t maxLength) noexcept;

    void SetNonDirtyValue(dom::DOMStringView value, TextControlSetValueSelection selection) noexcept;
    void SetValueCommon(dom::DOMStringView newValue, TextFieldEventBehavior eventBehavior,
                        TextControlSetValueSelection selection) noexcept;

    KRYS_NODISCARD bool ValueMissing(Maybe<dom::DOMStringView> value) const noexcept;
    KRYS_NODISCARD bool TooShort(Maybe<dom::DOMStringView> value, NeedsToCheckDirtyFlag check) const noexcept;
    KRYS_NODISCARD bool TooLong(Maybe<dom::DOMStringView> value, NeedsToCheckDirtyFlag check) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLTextAreaElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsHTMLTextAreaElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsHTMLTextAreaElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();