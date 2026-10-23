#pragma once

#include "Krystal.Booey/HTML/HTMLElement.hpp"
#include "Krystal.Booey/HTML/ValidatedFormListedElement.hpp"

namespace krys::boo::html
{
  /// @brief A form control: a button, fieldset, input, output, select or textarea.
  class HTMLFormControlElement : public HTMLElement, public ValidatedFormListedElement
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
      return ValidatedFormListedElement::MatchesValidPseudoClass();
    }

    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept override
    {
      return ValidatedFormListedElement::MatchesInvalidPseudoClass();
    }

    KRYS_NODISCARD bool MatchesUserValidPseudoClass() const noexcept override
    {
      return ValidatedFormListedElement::MatchesUserValidPseudoClass();
    }

    KRYS_NODISCARD bool MatchesUserInvalidPseudoClass() const noexcept override
    {
      return ValidatedFormListedElement::MatchesUserInvalidPseudoClass();
    }

    KRYS_NODISCARD bool IsDisabledFormControl() const noexcept override
    {
      return IsDisabled();
    }

    /// @see https://html.spec.whatwg.org/#dom-fs-formnovalidate
    KRYS_NODISCARD bool FormNoValidate() const noexcept;

    KRYS_NODISCARD bool FormControlValueMatchesRenderer() const noexcept
    {
      return _valueMatchesRenderer;
    }

    void SetFormControlValueMatchesRenderer(bool matches) noexcept
    {
      _valueMatchesRenderer = matches;
    }

    KRYS_NODISCARD bool WasChangedSinceLastFormControlChangeEvent() const noexcept
    {
      return _wasChangedSinceLastFormControlChangeEvent;
    }

    void SetChangedSinceLastFormControlChangeEvent(bool changed) noexcept;

    virtual void DispatchFormControlChangeEvent() noexcept;
    void DispatchChangeEvent() noexcept;
    void DispatchCancelEvent() noexcept;
    void DispatchFormControlInputEvent() noexcept;

    KRYS_NODISCARD bool IsRequired() const noexcept
    {
      return _isRequired;
    }

    /// @brief Whether pressing Enter in the control submits its form implicitly.
    /// @see https://html.spec.whatwg.org/#implicit-submission
    KRYS_NODISCARD virtual bool CanTriggerImplicitSubmission() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsSuccessfulSubmitButton() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual bool IsActivatedSubmit() const noexcept
    {
      return false;
    }

    virtual void SetActivatedSubmit(bool activated) noexcept
    {
      (void)activated;
    }

    /// @see https://html.spec.whatwg.org/#concept-submit-button
    KRYS_NODISCARD virtual bool IsSubmitButton() const noexcept
    {
      return false;
    }

    /// @brief The return value of a dialog this control submits.
    KRYS_NODISCARD virtual dom::DOMString ResultForDialogSubmit() const noexcept;

    KRYS_NODISCARD FormAssociatedElement *AsFormAssociatedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD FormListedElement *AsFormListedElement() noexcept final
    {
      return this;
    }

    KRYS_NODISCARD ValidatedFormListedElement *AsValidatedFormListedElement() noexcept final
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
    HTMLFormControlElement(dom::Document &document, HTMLElementInterface interface,
                           HTMLElementFlags flags = HTMLElementFlags::None) noexcept;

    void OnInsert() noexcept override;
    void OnPostConnection() noexcept override;
    void DidAttachRenderers() noexcept override;
    void DidRecalcStyle(EnumSet<style::Change>) noexcept override;
    void OnRemove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnMove(bool isSubtreeRoot, dom::ContainerNode &oldAncestor) noexcept override;
    void OnAdopt(dom::Document &oldDocument) noexcept override;
    void OnAttributeChanged(dom::DOMStringAtom localName, Maybe<dom::DOMStringView> oldValue,
                            Maybe<dom::DOMStringView> value,
                            dom::DOMStringAtom namespaceURI) noexcept override;

    void DisabledStateChanged() noexcept override;
    void ReadOnlyStateChanged() noexcept override;
    virtual void RequiredStateChanged() noexcept;

  private:
    KRYS_NODISCARD HTMLElement *ValidationAnchorElement() noexcept final
    {
      return this;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::html::HTMLFormControlElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::html::HTMLElement>(target).IsFormControlElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::html::HTMLElement &target) noexcept
  {
    return target.IsFormControlElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
