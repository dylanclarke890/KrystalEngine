/*
 * Copyright (C) 2012 Google Inc. All rights reserved.
 * Copyright (C) 2020 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/HTML/Elements/HTMLDivElement.hpp"

namespace krys::boo
{
  class DateComponents;
  class Locale;
  class RenderStyle;
}

namespace krys::boo::dom
{
  class KeyboardEvent;
}

namespace krys::boo::html
{
  class DateTimeFieldElement;
  struct DateTimeFieldsState;

  enum class DateTimePlaceholderIfNoValue : bool
  {
    No,
    Yes
  };

  /// @brief What a date or time field is part of: a date or time input's editable fields.
  class DateTimeFieldElementFieldOwner
  {
  public:
    virtual ~DateTimeFieldElementFieldOwner() noexcept = default;
    virtual void DidBlurFromField(dom::Event &event) noexcept = 0;
    virtual void FieldValueChanged() noexcept = 0;
    virtual bool FocusOnNextField(const DateTimeFieldElement &field) noexcept = 0;
    virtual bool FocusOnPreviousField(const DateTimeFieldElement &field) noexcept = 0;
    KRYS_NODISCARD virtual bool IsFieldOwnerDisabled() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsFieldOwnerReadOnly() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsFieldOwnerHorizontal() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString LocaleIdentifier() const noexcept = 0;
    /// @brief The date fields show (greyed) until they have values: today.
    KRYS_NODISCARD virtual const DateComponents &PlaceholderDate() const noexcept = 0;
  };

  /// @brief One of a date or time input's editable fields: a number or a name (a month, AM or PM) that the
  /// arrow keys step and typing sets, showing a placeholder until it has a value.
  class DateTimeFieldElement : public HTMLDivElement
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeFieldElement);

  public:
    enum class EventBehavior : bool
    {
      DispatchNoEvent,
      DispatchInputAndChangeEvents
    };

  private:
    // The owner clears this when it goes (RemoveFieldOwner).
    DateTimeFieldElementFieldOwner *_fieldOwner;

  public:
    void DefaultEventHandler(dom::Event &event) noexcept override;

    KRYS_NODISCARD dom::DOMString VisibleValue() const noexcept;

    void RemoveFieldOwner() noexcept
    {
      _fieldOwner = nullptr;
    }

    KRYS_NODISCARD virtual bool HasValue() const noexcept = 0;
    virtual void PopulateDateTimeFieldsState(
      DateTimeFieldsState &state,
      DateTimePlaceholderIfNoValue placeholderIfNoValue = DateTimePlaceholderIfNoValue::No) noexcept = 0;
    virtual void SetEmptyValue(EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept = 0;
    virtual void SetValueAsDate(const DateComponents &date) noexcept = 0;
    virtual void SetValueAsInteger(int value,
                                   EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept = 0;
    virtual void StepDown() noexcept = 0;
    virtual void StepUp() noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString Value() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString PlaceholderValue() const noexcept = 0;

  protected:
    DateTimeFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner) noexcept;

    /// @brief Makes @p field the part named @p part, focusable (its parts are what a date input focuses),
    /// and a spin button named @p label for accessibility.
    static void Initialize(DateTimeFieldElement &field, dom::DOMStringAtom part,
                           utf8_stringview label = {}) noexcept;

    KRYS_NODISCARD Locale &LocaleForOwner() const noexcept;
    KRYS_NODISCARD dom::DOMString LocaleIdentifier() const noexcept;
    void UpdateVisibleValue(EventBehavior eventBehavior) noexcept;
    /// @brief Sets `aria-valuenow` and `aria-valuetext` to the field's value, or removes them without one.
    void UpdateAriaValueAttributes() noexcept;
    virtual void AdjustMinInlineSize(RenderStyle &style) const noexcept = 0;
    KRYS_NODISCARD virtual int ValueAsInteger() const noexcept = 0;
    KRYS_NODISCARD virtual int PlaceholderValueAsInteger() const noexcept = 0;
    virtual void HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept = 0;
    virtual void HandleBlurEvent(dom::Event &event) noexcept;

    KRYS_NODISCARD DateTimeFieldElementFieldOwner *FieldOwner() const noexcept
    {
      return _fieldOwner;
    }

  private:
    KRYS_NODISCARD Maybe<style::UnadjustedStyle>
      ResolveCustomStyle(const style::ResolutionContext &resolutionContext,
                         const RenderStyle *shadowHostStyle) noexcept final;

    void DefaultKeyboardEventHandler(dom::KeyboardEvent &event) noexcept;
    KRYS_NODISCARD bool IsFieldOwnerDisabled() const noexcept;
    KRYS_NODISCARD bool IsFieldOwnerReadOnly() const noexcept;
    KRYS_NODISCARD bool IsFieldOwnerHorizontal() const noexcept;
  };
}
