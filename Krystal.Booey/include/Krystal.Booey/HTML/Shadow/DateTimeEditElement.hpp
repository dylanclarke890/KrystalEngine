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

#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElement.hpp"
#include "Krystal.Booey/Platform/DateComponents.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"

namespace krys::boo::html
{
  /// @brief What a date or time input's editable fields belong to: its input type.
  class DateTimeEditElementEditControlOwner
  {
  public:
    virtual ~DateTimeEditElementEditControlOwner() noexcept = default;
    virtual void DidBlurFromControl() noexcept = 0;
    virtual void DidChangeValueFromControl() noexcept = 0;
    virtual void DidReceiveSpaceKeyFromControl() noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString
      FormatDateTimeFieldsState(const DateTimeFieldsState &state) const noexcept = 0;
    KRYS_NODISCARD virtual bool IsEditControlOwnerDisabled() const noexcept = 0;
    KRYS_NODISCARD virtual bool IsEditControlOwnerReadOnly() const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString LocaleIdentifier() const noexcept = 0;
  };

  /// @brief A date or time input's editable fields (the `::-webkit-datetime-edit` part): one per part of the
  /// locale's format, with the format's literals between them.
  class DateTimeEditElement final : public HTMLDivElement, public DateTimeFieldElementFieldOwner
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(DateTimeEditElement);

  public:
    struct LayoutParameters
    {
      dom::DOMString DateTimeFormat;
      dom::DOMString FallbackDateTimeFormat;
      boo::Locale &FormatLocale;
      bool ShouldHaveMillisecondField {false};

      explicit LayoutParameters(boo::Locale &locale) noexcept : FormatLocale(locale)
      {
      }
    };

  private:
    // Datetime can be represented by at most 8 fields:
    // 1. year
    // 2. month
    // 3. day-of-month
    // 4. hour
    // 5. minute
    // 6. second
    // 7. millisecond
    // 8. AM/PM
    static constexpr size_t MaximumNumberOfFields = 8;

    SmallList<Ref<DateTimeFieldElement>, MaximumNumberOfFields> _fields;
    // The owner clears this when it goes (RemoveEditControlOwner).
    DateTimeEditElementEditControlOwner *_editControlOwner;
    DateComponents _placeholderDate;

  public:
    DateTimeEditElement(dom::Document &document,
                        DateTimeEditElementEditControlOwner &editControlOwner) noexcept;
    ~DateTimeEditElement() noexcept override;

    KRYS_NODISCARD static Ref<DateTimeEditElement>
      Create(dom::Document &document, DateTimeEditElementEditControlOwner &editControlOwner) noexcept;

    void AddField(Ref<DateTimeFieldElement> field) noexcept;
    KRYS_NODISCARD dom::Element &FieldsWrapperElement() const noexcept;
    void FocusByOwner() noexcept;
    void ResetFields() noexcept;
    void SetEmptyValue(const LayoutParameters &layoutParameters) noexcept;
    void SetValueAsDate(const LayoutParameters &layoutParameters, const DateComponents &date) noexcept;
    KRYS_NODISCARD dom::DOMString Value() const noexcept;
    KRYS_NODISCARD dom::DOMString PlaceholderValue() const noexcept;
    KRYS_NODISCARD bool EditableFieldsHaveValues() const noexcept;

    void RemoveEditControlOwner() noexcept
    {
      _editControlOwner = nullptr;
    }

    /// @brief The fields, in order.
    KRYS_NODISCARD Span<const Ref<DateTimeFieldElement>> Fields() const noexcept
    {
      return _fields;
    }

    void DefaultEventHandler(dom::Event &event) noexcept final;

  private:
    KRYS_NODISCARD size_t FieldIndexOf(const DateTimeFieldElement &field) const noexcept;
    KRYS_NODISCARD DateTimeFieldElement *FocusedFieldElement() const noexcept;
    void Layout(const LayoutParameters &layoutParameters) noexcept;
    KRYS_NODISCARD DateTimeFieldsState ValueAsDateTimeFieldsState(
      DateTimePlaceholderIfNoValue placeholderIfNoValue = DateTimePlaceholderIfNoValue::No) const noexcept;

    bool FocusOnNextFocusableField(size_t startIndex) noexcept;

    // DateTimeFieldElementFieldOwner functions:
    void DidBlurFromField(dom::Event &event) noexcept final;
    void FieldValueChanged() noexcept final;
    bool FocusOnNextField(const DateTimeFieldElement &field) noexcept final;
    bool FocusOnPreviousField(const DateTimeFieldElement &field) noexcept final;
    KRYS_NODISCARD bool IsFieldOwnerDisabled() const noexcept final;
    KRYS_NODISCARD bool IsFieldOwnerReadOnly() const noexcept final;
    KRYS_NODISCARD bool IsFieldOwnerHorizontal() const noexcept final;
    KRYS_NODISCARD dom::DOMString LocaleIdentifier() const noexcept final;
    KRYS_NODISCARD const DateComponents &PlaceholderDate() const noexcept final;
  };
}
