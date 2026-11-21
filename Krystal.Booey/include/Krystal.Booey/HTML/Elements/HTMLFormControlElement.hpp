/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2023 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Forms/ValidatedFormListedElement.hpp"

namespace krys::boo
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
    KRYS_NODISCARD virtual Maybe<DOMString> ResultForDialogSubmit() const noexcept;

    /// @brief The popover the control (a button) shows or hides, if it can.
    /// @see https://html.spec.whatwg.org/#popover-target-element
    KRYS_NODISCARD RefPtr<HTMLElement> PopoverTargetElement() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popovertargetelement
    KRYS_NODISCARD RefPtr<boo::Element> PopoverTargetElementForBindings() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popovertargetelement
    void PopoverTargetElementForBindings(boo::Element *element) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popovertargetaction
    KRYS_NODISCARD DOMString PopoverTargetAction() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-popovertargetaction
    void PopoverTargetAction(DOMString value) noexcept;

    /// @brief Shows or hides the control's popover target element as its activation, unless the activation
    /// came from inside the popover (@p eventTarget is the event's target).
    /// @see https://html.spec.whatwg.org/#popover-target-attribute-activation-behavior
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

    void ReadOnlyStateChanged() noexcept override;

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

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLFormControlElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsFormControlElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsFormControlElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
