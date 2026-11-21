/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Types/USVString.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLElement.hpp"
#include "Krystal.Booey/HTML/Forms/RadioButtonGroups.hpp"
#include "Krystal.Booey/Loader/FormSubmission.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo::html
{
  class DOMFormData;
  class FormAssociatedElement;
  class FormListedElement;
  class HTMLFormControlsCollection;
  class RadioNodeList;
  enum class RadioNodeListCandidates : uint8;
  class ValidatedFormListedElement;
}

namespace krys::boo
{
  class Event;
  class HTMLFormControlElement;
  class HTMLImageElement;

  /// @see https://html.spec.whatwg.org/#htmlformelement
  class HTMLFormElement : public HTMLElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(HTMLFormElement);

  private:
    UniquePtr<dom::DOMTokenList> _relList;
    WeakPtr<dom::HTMLCollection> _elements;
    FormSubmission::Attributes _attributes;
    HashMap<DOMStringAtom, WeakPtr<HTMLElement>> _pastNamesMap;
    html::RadioButtonGroups _radioButtonGroups;
    mutable WeakPtr<HTMLFormControlElement> _defaultButton;
    List<WeakPtr<HTMLElement>> _listedElements;
    /// @brief The form's RadioNodeLists by name, which remove themselves as they're destroyed.
    HashMap<DOMString, WeakPtr<dom::NodeList>> _radioNodeLists;
    /// @brief The same for images, which are candidates only if no listed element is.
    HashMap<DOMString, WeakPtr<dom::NodeList>> _imageRadioNodeLists;
    /// @brief The images whose form owner this is, in no particular order.
    List<WeakPtr<HTMLImageElement>> _imageElements;
    List<WeakPtr<HTMLElement>> _invalidFormControls;
    WeakPtr<FormSubmission> _plannedFormSubmission;
    uint32 _listedElementsBeforeIndex {0};
    uint32 _listedElementsAfterIndex {0};
    bool _wasUserSubmitted {false};
    bool _isSubmittingOrPreparingForSubmission {false};
    bool _shouldSubmit {false};
    bool _isInResetFunction {false};
    bool _isConstructingEntryList {false};

  public:
    HTMLFormElement(Document &document) noexcept;

    ~HTMLFormElement() noexcept override;

#pragma region HTMLFormElement - https://html.spec.whatwg.org/#htmlformelement

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    KRYS_NODISCARD DOMString AcceptCharset() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-acceptcharset
    void AcceptCharset(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    KRYS_NODISCARD dom::USVString Action() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-action
    void Action(dom::USVString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    KRYS_NODISCARD DOMString Autocomplete() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-autocomplete
    void Autocomplete(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    KRYS_NODISCARD DOMString EncType() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-enctype
    void EncType(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    KRYS_NODISCARD DOMString Encoding() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-encoding
    void Encoding(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    KRYS_NODISCARD DOMString Method() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-method
    void Method(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    KRYS_NODISCARD DOMString Name() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-name
    void Name(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    KRYS_NODISCARD bool NoValidate() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-novalidate
    void NoValidate(bool value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    KRYS_NODISCARD DOMString Target() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-target
    void Target(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    KRYS_NODISCARD DOMString Rel() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rel
    void Rel(DOMString value) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-rellist
    KRYS_NODISCARD dom::DOMTokenList &RelList() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-elements
    KRYS_NODISCARD Ref<html::HTMLFormControlsCollection> Elements() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-length
    KRYS_NODISCARD size_t Length() const noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-item
    KRYS_NODISCARD RefPtr<HTMLElement> Item(size_t index) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-nameditem
    KRYS_NODISCARD Maybe<Variant<RefPtr<html::RadioNodeList>, RefPtr<Element>>>
      NamedItem(DOMStringView name) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-submit
    void Submit() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-requestsubmit
    ExceptionOr<void> RequestSubmit(HTMLElement *submitter = nullptr) noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-reset
    void Reset() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-checkvalidity
    bool CheckValidity() noexcept;

    /// @see https://html.spec.whatwg.org/#dom-form-reportvalidity
    bool ReportValidity() noexcept;

#pragma endregion

    /// @brief The elements of the form named @p name, falling back to the one last found by that name.
    KRYS_NODISCARD List<Ref<Element>> NamedElements(DOMStringView name) noexcept;

    KRYS_NODISCARD bool IsSupportedPropertyName(DOMStringView name) noexcept;

    KRYS_NODISCARD bool ShouldAutocomplete() const noexcept;

    void RegisterImageElement(HTMLImageElement &element) noexcept;

    void UnregisterImageElement(HTMLImageElement &element) noexcept;

    /// @brief The images whose form owner this is with an ID or name of @p name, in tree order.
    KRYS_NODISCARD List<Ref<Element>> NamedImages(DOMStringView name) const noexcept;

    void RegisterFormListedElement(html::FormListedElement &element) noexcept;

    void UnregisterFormListedElement(html::FormListedElement &element) noexcept;

    void AddInvalidFormControl(const HTMLElement &formControlElement) noexcept;

    void RemoveInvalidFormControlIfNeeded(const HTMLElement &formControlElement) noexcept;

    /// @see https://html.spec.whatwg.org/#form-submission-algorithm
    void SubmitIfPossible(
      Event *event, HTMLFormControlElement *submitter = nullptr,
      FormSubmissionTrigger trigger = FormSubmissionTrigger::NotSubmittedByJavaScript) noexcept;

    /// @brief Submits the form as script's `submit()` does.
    void SubmitFromJavaScript() noexcept;

    /// @see https://html.spec.whatwg.org/#implicit-submission
    void SubmitImplicitly(Event &event, bool fromImplicitSubmissionTrigger) noexcept;

    KRYS_NODISCARD bool IsMethodPost() const noexcept
    {
      return _attributes.GetMethod() == FormSubmission::Method::Post;
    }

    /// @brief The browsing context name the form submits to: the submitter's `formtarget`, the form's
    /// `target`, or the document's base target, with `_blank` for a target with dangling markup.
    /// @see https://html.spec.whatwg.org/#get-an-element's-target
    KRYS_NODISCARD DOMString EffectiveTarget(const Event *event,
                                             HTMLFormControlElement *submitter) const noexcept;

    KRYS_NODISCARD bool WasUserSubmitted() const noexcept;

    /// @brief The form control the event that submits the form was targeted at, if any.
    KRYS_NODISCARD static HTMLFormControlElement *FindSubmitter(const Event *event) noexcept;

    /// @see https://html.spec.whatwg.org/#default-button
    KRYS_NODISCARD HTMLFormControlElement *DefaultButton() const noexcept;

    void ResetDefaultButton() noexcept;

    KRYS_NODISCARD html::RadioButtonGroups &RadioButtonGroups() noexcept
    {
      return _radioButtonGroups;
    }

    KRYS_NODISCARD List<Ref<HTMLElement>> CopyListedElementsVector() const noexcept;

    KRYS_NODISCARD List<Ref<HTMLElement>> CopyValidatedListedElementsVector() const noexcept;

    /// @brief The names and values of the form's text fields.
    KRYS_NODISCARD List<Pair<DOMString, DOMString>> TextFieldValues() const noexcept;

    KRYS_NODISCARD static HTMLFormElement *FindClosestFormAncestor(const Element &element) noexcept;

    /// @see https://html.spec.whatwg.org/#constructing-the-form-data-set
    /// @return The entry list, or null if the form is already constructing one.
    KRYS_NODISCARD RefPtr<html::DOMFormData>
      ConstructEntryList(const RefPtr<HTMLFormControlElement> &submitter,
                         const Ref<html::DOMFormData> &domFormData,
                         List<Pair<DOMString, DOMString>> *formValues) noexcept;

    KRYS_NODISCARD const FormSubmission::Attributes &Attributes() const noexcept
    {
      return _attributes;
    }

    /// @brief The form's elements with the ID or name @p name.
    /// @brief The form's listed elements, in tree order.
    KRYS_NODISCARD const List<WeakPtr<HTMLElement>> &ListedElements() const noexcept
    {
      return _listedElements;
    }

    /// @brief Forgets the RadioNodeList for @p name, as it's being destroyed.
    void RadioNodeListDestroyed(DOMStringView name, html::RadioNodeListCandidates candidates) noexcept;

    KRYS_NODISCARD Ref<html::RadioNodeList> RadioNodeListForName(DOMStringView name) noexcept;

    KRYS_NODISCARD Ref<html::RadioNodeList>
      RadioNodeListForName(DOMStringView name, html::RadioNodeListCandidates candidates) noexcept;

    KRYS_NODISCARD bool MatchesValidPseudoClass() const noexcept final;

    KRYS_NODISCARD bool MatchesInvalidPseudoClass() const noexcept final;

  protected:
    void OnInsert() noexcept override;

    void OnRemove(bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept override;

    void OnAttributeChanged(const DOMStringAtom &localName, Maybe<DOMStringView> oldValue,
                            Maybe<DOMStringView> value, const DOMStringAtom &namespaceURI) noexcept override;

  private:
    void Submit(Event *event, bool processingUserGesture, FormSubmissionTrigger trigger,
                HTMLFormControlElement *submitter = nullptr) noexcept;

    void SubmitDialog(const Ref<FormSubmission> &formSubmission) noexcept;

    KRYS_NODISCARD uint32 FormElementIndexWithFormAttribute(Element &element, uint32 rangeStart,
                                                            uint32 rangeEnd) noexcept;

    KRYS_NODISCARD uint32 FormElementIndex(html::FormListedElement &listedElement) noexcept;

    bool ValidateInteractively() noexcept;

    /// @brief Checks each control's validity, storing those whose `invalid` event wasn't canceled.
    /// @return Whether any control is invalid.
    bool CheckInvalidControlsAndCollectUnhandled(List<Ref<HTMLElement>> &unhandledInvalidControls) noexcept;

    KRYS_NODISCARD HTMLElement *ElementFromPastNamesMap(DOMStringView pastName) const noexcept;

    void AddToPastNamesMap(html::FormAssociatedElement &element, DOMStringView pastName) noexcept;

    void RemoveFromPastNamesMap(html::FormAssociatedElement &item) noexcept;

    void ResetListedFormControlElements() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::HTMLFormElement)

  KRYS_NODISCARD static bool IsType(const krys::boo::Node &target) noexcept
  {
    return target.IsHTMLElement() && Downcast<krys::boo::HTMLElement>(target).IsHTMLFormElement();
  }

  KRYS_NODISCARD static bool IsType(const krys::boo::HTMLElement &target) noexcept
  {
    return target.IsHTMLFormElement();
  }

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();
