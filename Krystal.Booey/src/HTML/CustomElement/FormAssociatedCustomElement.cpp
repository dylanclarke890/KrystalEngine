/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/HTML/CustomElement/FormAssociatedCustomElement.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementReactionQueue.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLFormElement.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLMaybeFormAssociatedCustomElement.hpp"
#include "Krystal.Booey/HTML/Forms/DOMFormData.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"

namespace krys::boo::html
{
  namespace
  {
    CustomElementFormValue CloneIfIsFormData(CustomElementFormValue &&value) noexcept
    {
      if (auto *formData = std::get_if<RefPtr<DOMFormData>>(&value); formData && *formData)
      {
        return RefPtr<DOMFormData>((*formData)->Clone());
      }
      return krys::move(value);
    }
  }

  FormAssociatedCustomElement::FormAssociatedCustomElement(
    HTMLMaybeFormAssociatedCustomElement &element) noexcept
      : _element(element)
  {
  }

  FormAssociatedCustomElement::~FormAssociatedCustomElement() noexcept
  {
    ClearForm();
  }

  HTMLElement &FormAssociatedCustomElement::AsHTMLElement() noexcept
  {
    return _element;
  }

  const HTMLElement &FormAssociatedCustomElement::AsHTMLElement() const noexcept
  {
    return _element;
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-setvalidity
  dom::ExceptionOr<void> FormAssociatedCustomElement::SetValidity(ValidityStateFlags flags,
                                                                  dom::DOMString message,
                                                                  HTMLElement *validationAnchor) noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());

    // 3. If message is not given, then set it to the empty string.
    // 4. If flags contains one or more true values and message is the empty string, then throw a TypeError.
    if (!flags.IsValid() && message.empty())
    {
      return dom::Exception {dom::ExceptionCode::TypeError};
    }

    // 5. Set message to the result of normalizing newlines given message.
    // 6. For each entry flag → value of flags, set element's validity flag with the name flag to value.
    // 7. Set element's validation message to the empty string if all of element's validity flags are false;
    //    otherwise to message.
    // 8. If element's customError validity flag is true, then set element's custom validity error message to
    //    element's validation message. Otherwise, set element's custom validity error message to the empty
    //    string.
    // (The validation message is the custom validity error message, and customError is a flag.)
    _validityStateFlags = flags;
    SetCustomValidationMessage(flags.IsValid() ? dom::DOMString()
                                               : infra::StringAlgorithms::NormalizeNewlines(message));

    // 9. If anchor is not given, then set it to element.
    // 10. Otherwise, if anchor is not a shadow-including inclusive descendant of element, then throw a
    //     "NotFoundError" DOMException.
    if (validationAnchor
        && !dom::TreeQueries::IsShadowIncludingInclusiveAncestor(_element, *validationAnchor))
    {
      return dom::Exception {dom::ExceptionCode::NotFoundError};
    }

    // 11. Set element's validation anchor to anchor.
    // (Null for the element, as ValidationAnchorElement gives.)
    _validationAnchor = CreateWeakPtr(validationAnchor);

    return {};
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-validationmessage
  dom::DOMString FormAssociatedCustomElement::ValidationMessage() const noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());

    // 3. Return element's validation message.
    return CustomValidationMessage();
  }

  // https://html.spec.whatwg.org/#dom-elementinternals-setformvalue
  void FormAssociatedCustomElement::SetFormValue(CustomElementFormValue &&submissionValue,
                                                 Maybe<CustomElementFormValue> &&state) noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());

    // 3. Set element's submission value to value if value is not a FormData object, or to a clone of value's
    //    entry list otherwise.
    _submissionValue = CloneIfIsFormData(krys::move(submissionValue));

    // 4. If the state argument of the function is omitted, set element's state to its submission value.
    // 5. Otherwise, if state is a FormData object, set element's state to a clone of state's entry list.
    // 6. Otherwise, set element's state to state.
    _state = state.has_value() ? CloneIfIsFormData(krys::move(*state)) : _submissionValue;
  }

  HTMLElement *FormAssociatedCustomElement::ValidationAnchorElement() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    auto *anchor = _validationAnchor.get();
    return anchor ? anchor : &_element;
  }

  bool FormAssociatedCustomElement::ComputeValidity() const noexcept
  {
    krys_debug_assert(_element.IsPrecustomizedOrDefinedCustomElement());
    return _validityStateFlags.IsValid();
  }

  // https://html.spec.whatwg.org/#face-entry-construction
  bool FormAssociatedCustomElement::AppendFormData(DOMFormData &formData) noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());

    // 1. If element's submission value is a list of entries, then append each item of element's submission
    //    value to entry list, and return.
    // 2. If the element does not have a name attribute specified, or its name attribute's value is the empty
    //    string, then return.
    // 3. If the element's submission value is not null, create an entry with the name attribute value and the
    //    submission value, and append it to entry list.
    if (auto *entries = std::get_if<RefPtr<DOMFormData>>(&_submissionValue))
    {
      if (*entries)
      {
        for (auto &item : (*entries)->Items())
        {
          if (auto *value = std::get_if<dom::DOMString>(&item.Data))
          {
            formData.Append(item.Name, *value);
          }
          else
          {
            formData.Append(item.Name, *std::get<RefPtr<File>>(item.Data));
          }
        }
      }
    }
    else if (auto *value = std::get_if<dom::DOMString>(&_submissionValue))
    {
      if (auto name = Name(); !name.empty())
      {
        formData.Append(name, *value);
      }
    }
    else if (auto *file = std::get_if<RefPtr<File>>(&_submissionValue))
    {
      if (auto name = Name(); !name.empty() && *file)
      {
        formData.Append(name, **file);
      }
    }

    return true;
  }

  bool FormAssociatedCustomElement::IsEnumeratable() const noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    return true;
  }

  void FormAssociatedCustomElement::Reset() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    SetInteractedWithSinceLastFormSubmitEvent(false);
    CustomElementReactionQueue::EnqueueFormResetCallbackIfNeeded(_element);
  }

  void FormAssociatedCustomElement::DisabledStateChanged() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    ValidatedFormListedElement::DisabledStateChanged();
    CustomElementReactionQueue::EnqueueFormDisabledCallbackIfNeeded(_element, IsDisabled());
  }

  void FormAssociatedCustomElement::DidChangeForm() noexcept
  {
    krys_debug_assert(_element.IsDefinedCustomElement());
    ValidatedFormListedElement::DidChangeForm();
    if (!BelongsToFormThatIsBeingDestroyed())
    {
      CustomElementReactionQueue::EnqueueFormAssociatedCallbackIfNeeded(_element, Form());
    }
  }

  void FormAssociatedCustomElement::WillUpgrade() noexcept
  {
    SetDataListAncestorState(TriState::False);
  }

  void FormAssociatedCustomElement::DidUpgrade() noexcept
  {
    krys_debug_assert(!Form());

    ParseFormAttribute(_element.GetAttribute(u8"form"));
    ParseDisabledAttribute(_element.GetAttribute(u8"disabled"));
    ParseReadOnlyAttribute(_element.GetAttribute(u8"readonly"));

    SetDataListAncestorState(TriState::Indeterminate);
    UpdateWillValidateAndValidity();
    SyncWithFieldsetAncestors(_element.ParentNode());
  }

  dom::DOMString FormAssociatedCustomElement::FormControlType() const noexcept
  {
    return dom::DOMString(_element.LocalName().View());
  }
}
