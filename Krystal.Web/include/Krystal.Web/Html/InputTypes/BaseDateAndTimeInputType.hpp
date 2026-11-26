/*
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2016 Apple Inc. All rights reserved.
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

#include "Krystal.Core/Containers/EnumSet.hpp"
#include "Krystal.I18n/DateTimeFormat.hpp"
#include "Krystal.Web/Html/InputTypes/InputType.hpp"
#include "Krystal.Web/Html/Shadow/DateTimeEditElement.hpp"
#include "Krystal.Web/Platform/Choosers/DateTimeChooser.hpp"
#include "Krystal.Web/Platform/Choosers/DateTimeChooserClient.hpp"

namespace krys::web
{
  struct DateTimeChooserParameters;
}

namespace krys::web::html
{
  /// @brief The base of the date and time input types: date, datetime-local, month, time and week. Their
  /// values are edited in fields laid out by the locale's format, and chosen in the host's date picker.
  class BaseDateAndTimeInputType : public InputType,
                                   public DateTimeChooserClient,
                                   private DateTimeEditElementEditControlOwner
  {
  public:
    KRYS_NODISCARD bool ShouldRespectListAttribute() const noexcept final
    {
      return true;
    }

    KRYS_NODISCARD bool TypeMismatchFor(DOMStringView value) const noexcept final;

    KRYS_NODISCARD bool ValueMissing(DOMStringView value) const noexcept final;

    KRYS_NODISCARD bool TypeMismatch() const noexcept final;

    KRYS_NODISCARD bool HasBadInput() const noexcept final;

  protected:
    enum class DateTimeFormatValidationResults : uint8
    {
      HasYear,
      HasMonth,
      HasWeek,
      HasDay,
      HasHour,
      HasMinute,
      HasSecond,
      HasMeridiem,
    };

    BaseDateAndTimeInputType(Type type, HTMLInputElement &element) noexcept : InputType(type, element)
    {
    }

    ~BaseDateAndTimeInputType() noexcept override;

    KRYS_NODISCARD Decimal ParseToNumber(DOMStringView source,
                                         const Decimal &defaultValue) const noexcept override;

    KRYS_NODISCARD DOMString Serialize(const Decimal &value) const noexcept final;

    KRYS_NODISCARD DOMString SerializeWithComponents(const DateComponents &date) const noexcept;

    KRYS_NODISCARD bool ShouldHaveSecondField(const DateComponents &date) const noexcept;

    KRYS_NODISCARD bool ShouldHaveMillisecondField(const DateComponents &date) const noexcept;

    /// @brief @p value in decimal, zero padded to @p width digits.
    KRYS_NODISCARD static DOMString PadNumber(uint32 value, size_t width) noexcept;

    void ShowPicker() noexcept override;

  private:
    class DateTimeFormatValidator final : public i18n::DateTimeFormat::TokenHandler
    {
    private:
      EnumSet<DateTimeFormatValidationResults> _results;

    public:
      void VisitField(i18n::DateTimeFormat::FieldType fieldType, int count) noexcept final;

      void VisitLiteral(utf8_stringview) noexcept final
      {
      }

      bool ValidateFormat(DOMStringView format, const BaseDateAndTimeInputType &inputType) noexcept;
    };

    RefPtr<DateTimeChooser> _dateTimeChooser;
    RefPtr<DateTimeEditElement> _dateTimeEditElement;
    bool _pickerWasActivatedByKeyboard {false};

    KRYS_NODISCARD virtual Maybe<DateComponents>
      ParseToDateComponents(DOMStringView source) const noexcept = 0;

    KRYS_NODISCARD virtual Maybe<DateComponents>
      SetMillisecondToDateComponents(double value) const noexcept = 0;

    virtual void SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                                       const DateComponents &date) const noexcept = 0;

    KRYS_NODISCARD virtual bool
      IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept = 0;

    KRYS_NODISCARD virtual DOMString SerializeWithMilliseconds(double value) const noexcept;

    // InputType functions:
    KRYS_NODISCARD DOMString VisibleValue() const noexcept final;

    KRYS_NODISCARD DOMString SanitizeValue(DOMStringView proposedValue) const noexcept override;

    void SetValue(DOMStringView value, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;

    KRYS_NODISCARD double ValueAsDate() const noexcept override;

    ExceptionOr<void> SetValueAsDate(double value) const noexcept override;

    KRYS_NODISCARD double ValueAsDouble() const noexcept final;

    ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                        TextFieldEventBehavior eventBehavior) const noexcept final;

    KRYS_NODISCARD Decimal DefaultValueForStepUp() const noexcept override;

    KRYS_NODISCARD DOMString LocalizeValue(DOMStringView proposedValue) const noexcept final;

    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final;

    void HandleActivationBehavior(Event &event) noexcept override;

    void CreateShadowSubtree() noexcept final;

    void RemoveShadowSubtree() noexcept final;

    void UpdateInnerTextValue() noexcept final;

    void AttributeChanged(const DOMStringAtom &name) noexcept final;

    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept final;

    void ElementDidBlur() noexcept final;

    void Detach() noexcept final;

    ShouldCallBaseEventHandler HandleKeydownEvent(KeyboardEvent &event) noexcept final;

    void HandleKeypressEvent(KeyboardEvent &event) noexcept final;

    void HandleKeyupEvent(KeyboardEvent &event) noexcept final;

    // DateTimeEditElementEditControlOwner functions:
    void DidBlurFromControl() noexcept final;

    void DidChangeValueFromControl() noexcept final;

    void DidReceiveSpaceKeyFromControl() noexcept final;

    KRYS_NODISCARD bool IsEditControlOwnerDisabled() const noexcept final;

    KRYS_NODISCARD bool IsEditControlOwnerReadOnly() const noexcept final;

    KRYS_NODISCARD DOMString LocaleIdentifier() const noexcept final;

    // DateTimeChooserClient functions:
    void DidChooseValue(utf8_stringview value) noexcept final;

    void DidEndChooser() noexcept final;

    bool SetupDateTimeChooserParameters(DateTimeChooserParameters &parameters) noexcept;

    void CloseDateTimeChooser() noexcept;
  };
}
