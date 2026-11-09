/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2018 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/HTML/InputTypes/InputType.hpp"
#include "Krystal.Booey/HTML/Shadow/DataListButtonElement.hpp"
#include "Krystal.Booey/HTML/Shadow/SpinButtonElement.hpp"
#include "Krystal.Booey/Platform/Choosers/DataListSuggestionsClient.hpp"

namespace krys::boo::html
{
  /// @brief The base of the text-like input types and number: a field with an inner text element the user
  /// edits, and a placeholder.
  class TextFieldInputType : public InputType,
                             protected SpinButtonOwner,
                             private DataListSuggestionsClient,
                             private DataListButtonOwner
  {
  private:
    RefPtr<HTMLElement> _container;
    RefPtr<HTMLElement> _innerBlock;
    RefPtr<TextControlInnerTextElement> _innerText;
    RefPtr<HTMLElement> _placeholder;
    RefPtr<SpinButtonElement> _innerSpinButton;
    /// @brief The list button, made once the field's datalist may have changed, and shown while it has one.
    RefPtr<html::DataListButtonElement> _dataListDropdownIndicator;
    /// @brief The list of suggestions, while it's shown.
    RefPtr<DataListSuggestionPicker> _suggestionPicker;
    /// @brief The suggestions for the text they were found for.
    Maybe<Pair<dom::DOMString, List<DataListSuggestion>>> _cachedSuggestions;

  public:
    ~TextFieldInputType() noexcept override;
    KRYS_NODISCARD bool ShouldRespectListAttribute() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;

    void DataListMayHaveChanged() noexcept final;
    KRYS_NODISCARD HTMLElement *DataListButtonElement() const noexcept final;
    /// @brief Shows the datalist's suggestions.
    void ShowPicker() noexcept override;
    /// @brief Whether the suggestions are shown.
    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept override;

  protected:
    TextFieldInputType(Type type, HTMLInputElement &element) noexcept;

    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept override;
    /// @brief Steps the value with the up and down arrow keys, as the spin button would.
    void HandleKeydownEventForSpinButton(dom::KeyboardEvent &event) noexcept;
    void HandleClickEvent(dom::MouseEvent &event) noexcept final;

    KRYS_NODISCARD HTMLElement *ContainerElement() const noexcept final;
    KRYS_NODISCARD HTMLElement *InnerBlockElement() const noexcept final;
    KRYS_NODISCARD RefPtr<TextControlInnerTextElement> InnerTextElement() const noexcept final;
    KRYS_NODISCARD HTMLElement *InnerSpinButtonElement() const noexcept final;

    KRYS_NODISCARD virtual bool NeedsContainer() const noexcept;
    void CreateShadowSubtree() noexcept override;
    void RemoveShadowSubtree() noexcept override;
    void AttributeChanged(dom::DOMStringAtom name) noexcept override;
    void DisabledStateChanged() noexcept final;
    void ReadOnlyStateChanged() noexcept final;
    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final;
    KRYS_NODISCARD bool ShouldUseInputMethod() const noexcept override;
    void HandleFocusEvent(dom::Node *oldFocusedNode) noexcept final;
    void HandleBlurEvent() noexcept final;
    void SetValue(dom::DOMStringView sanitizedValue, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept override;
    void UpdateInnerTextValue() noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept override;

    KRYS_NODISCARD virtual dom::DOMString
      ConvertFromVisibleValue(dom::DOMStringView visibleValue) const noexcept;
    virtual void DidSetValueByUserEdit() noexcept;

  private:
    KRYS_NODISCARD bool IsEmptyValue() const noexcept final;
    void HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &event) noexcept override;
    void ForwardEvent(dom::Event &event) noexcept final;
    KRYS_NODISCARD bool ShouldSubmitImplicitly(dom::Event &event) noexcept final;
    KRYS_NODISCARD RenderPtr<RenderElement> CreateInputRenderer(RenderStyle &&style) noexcept override;
    KRYS_NODISCARD HTMLElement *PlaceholderElement() const noexcept final;
    void UpdatePlaceholderText() noexcept final;
    bool AppendFormData(DOMFormData &formData) const noexcept final;
    void SubtreeHasChanged() noexcept final;
    void ElementDidBlur() noexcept final;

    // SpinButtonOwner functions.
    void FocusAndSelectSpinButtonOwner() noexcept final;
    KRYS_NODISCARD bool ShouldSpinButtonRespondToMouseEvents() const noexcept final;
    void SpinButtonStepDown() noexcept final;
    void SpinButtonStepUp() noexcept final;

    KRYS_NODISCARD bool ShouldHaveSpinButton() const noexcept;

    // DataListSuggestionsClient functions.
    KRYS_NODISCARD IntRect ElementRectInRootViewCoordinates() const noexcept final;
    KRYS_NODISCARD List<DataListSuggestion> Suggestions() noexcept final;
    KRYS_NODISCARD bool IsElementRTL() const noexcept final;
    void DidSelectDataListOption(utf8_stringview value) noexcept final;
    void DidCloseSuggestions() noexcept final;

    // DataListButtonOwner functions.
    void DataListButtonElementWasClicked() noexcept final;

    void CreateDataListDropdownIndicator() noexcept;
    void DisplaySuggestions(DataListSuggestionActivationType type) noexcept;
    void CloseSuggestions() noexcept;

    enum class PreserveSelectionRange : bool
    {
      No,
      Yes
    };

    void CreateContainer(PreserveSelectionRange preserveSelection = PreserveSelectionRange::Yes) noexcept;
  };
}
