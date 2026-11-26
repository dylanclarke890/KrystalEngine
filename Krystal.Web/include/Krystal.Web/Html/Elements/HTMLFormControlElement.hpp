#pragma once

#include "Krystal.Web/Html/Elements/HTMLElement.hpp"
#include "Krystal.Web/Html/Forms/ValidatedFormListedElement.hpp"

namespace krys::web
{
  /// @brief A form control: a button, fieldset, input, output, select or textarea.
  class HTMLFormControlElement : public HTMLElement, public html::ValidatedFormListedElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFormControlElement);

  private:
    bool _isRequired : 1 {false};
    bool _valueMatchesRenderer : 1 {false};
    bool _wasChangedSinceLastFormControlChangeEvent : 1 {false};

  public:
    ~HTMLFormControlElement() noexcept override;

    KRYS_NODISCARD bool IsValidatedFormListedElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool IsFormListedElement() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept override
    {
      return html::ValidatedFormListedElement::MatchesValidPseudoClass();
    }

    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept override
    {
      return html::ValidatedFormListedElement::MatchesInvalidPseudoClass();
    }

    KRYS_NODISCARD bool MatchesUserValidPseudoClass() const noexcept override
    {
      return html::ValidatedFormListedElement::MatchesUserValidPseudoClass();
    }

    KRYS_NODISCARD bool MatchesUserInvalidPseudoClass() const noexcept override
    {
      return html::ValidatedFormListedElement::MatchesUserInvalidPseudoClass();
    }

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept override
    {
      return IsDisabled();
    }

    /// @see https://html.spec.whatwg.org/multipage/input.html#dom-fs-formnovalidate
    KRYS_NODISCARD bool FormNoValidate() const noexcept;

    /// @brief Whether the value shown (the inner text) matches the value, or was changed and needs updating.
    KRYS_NODISCARD bool FormControlValueMatchesRenderer() const noexcept
    {
      return _valueMatchesRenderer;
    }

    void SetFormControlValueMatchesRenderer(bool matches) noexcept
    {
      _valueMatchesRenderer = matches;
    }

    /// @brief Whether the value changed (with an input event) since the last change event.
    KRYS_NODISCARD bool WasChangedSinceLastFormControlChangeEvent() const noexcept
    {
      return _wasChangedSinceLastFormControlChangeEvent;
    }

    void SetChangedSinceLastFormControlChangeEvent(bool changed) noexcept;

    /// @brief Fires change, as the user commits a value; the value counts as unchanged since then.
    virtual void DispatchFormControlChangeEvent() noexcept;

    /// @brief Fires change (queued while events are held back).
    void DispatchChangeEvent() noexcept;

    /// @brief Fires cancel, as the user dismisses a picker.
    void DispatchCancelEvent() noexcept;

    /// @brief Fires input, as the user changes the value; the value counts as changed until change fires.
    void DispatchFormControlInputEvent() noexcept;

    /// @brief Whether the control has a `required` attribute.
    KRYS_NODISCARD bool IsRequired() const noexcept
    {
      return _isRequired;
    }

    /// @brief Whether pressing Enter in the control submits its form implicitly.
    /// @see https://html.spec.whatwg.org/multipage/form-control-infrastructure.html#implicit-submission
    KRYS_NODISCARD virtual bool CanTriggerImplicitSubmission() const noexcept
    {
      return false;
    }

    /// @brief Whether it's a submit button, which the form's implicit submission activates.
    KRYS_NODISCARD virtual bool IsSuccessfulSubmitButton() const noexcept
    {
      return false;
    }

    /// @brief Whether the button is the submitter of the entry list being constructed, so it appends its
    /// entry; set around the construction.
    KRYS_NODISCARD virtual bool IsActivatedSubmit() const noexcept
    {
      return false;
    }

    virtual void SetActivatedSubmit(bool activated) noexcept
    {
      (void)activated;
    }

    /// @see https://html.spec.whatwg.org/multipage/forms.html#concept-submit-button
    KRYS_NODISCARD virtual bool IsSubmitButton() const noexcept
    {
      return false;
    }

    /// @brief The return value of a dialog this control submits.
    KRYS_NODISCARD virtual Maybe<DOMString> ResultForDialogSubmit() const noexcept;

    /// @brief The popover the control (a button) shows or hides, if it can.
    /// @see https://html.spec.whatwg.org/multipage/popover.html#popover-target-element
    KRYS_NODISCARD RefPtr<HTMLElement> PopoverTargetElement() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-popovertargetelement
    KRYS_NODISCARD RefPtr<web::Element> PopoverTargetElementForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-popovertargetelement
    void PopoverTargetElementForBindings(web::Element *element) noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-popovertargetaction
    KRYS_NODISCARD DOMString PopoverTargetAction() const noexcept;

    /// @see https://html.spec.whatwg.org/multipage/popover.html#dom-popovertargetaction
    void PopoverTargetAction(DOMString value) noexcept;

    /// @brief Shows or hides the control's popover target element as its activation, unless the activation
    /// came from inside the popover (@p eventTarget is the event's target).
    /// @see https://html.spec.whatwg.org/multipage/popover.html#popover-target-attribute-activation-behavior
    void HandlePopoverTargetAction(EventTarget *eventTarget) noexcept;

    KRYS_NODISCARD html::FormAssociatedElement *AsFormAssociatedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD html::FormListedElement *AsFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD html::ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD HTMLElement &AsHTMLElement() noexcept final
    {
      return *this;
    }

    KRYS_NODISCARD const HTMLElement &AsHTMLElement() const noexcept final
    {
      return *this;
    }

  protected:
    HTMLFormControlElement(Document &document, html::HTMLElementInterface interface,
                           html::HTMLElementFlags flags = html::HTMLElementFlags {}) noexcept;

    void OnInsert() noexcept override;

    void OnPostConnection() noexcept override;

    void DidAttachRenderers() noexcept override;

    void DidRecalcStyle(EnumSet<style::Change>) noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnMove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAdopt(Document &oldDocument) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

    void DisabledStateChanged() noexcept override;

    /// @brief Called when the `required` attribute comes or goes.
    virtual void RequiredStateChanged() noexcept;

    /// @brief Losing the focus hides the control's validation message.
    void DispatchBlurEvent(EventTarget *relatedTarget) noexcept override;

  private:
    KRYS_NODISCARD HTMLElement *ValidationAnchorElement() noexcept final
    {
      return this;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::HTMLFormControlElement)

  KRYS_NODISCARD static bool IsType(const krys::web::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::web::HTMLElement>(target).IsFormControlElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::web::HTMLElement &target) noexcept
  {
    return target.IsFormControlElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
