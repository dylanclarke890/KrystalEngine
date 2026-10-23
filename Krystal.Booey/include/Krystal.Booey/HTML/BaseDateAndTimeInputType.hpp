#pragma once

#include "Krystal.Booey/Core/DateTimeFormat.hpp"
#include "Krystal.Booey/HTML/InputType.hpp"
#include "Krystal.Booey/HTML/Shadow/DateTimeEditElement.hpp"
#include "Krystal.Booey/Platform/DateTimeChooser.hpp"
#include "Krystal.Booey/Platform/DateTimeChooserClient.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  struct DateTimeChooserParameters;
}

namespace krys::boo::html
{
  /// @brief The base of the date and time input types: date, datetime-local, month, time and week. Their
  /// values are edited in fields laid out by the locale's format, and chosen in the host's date picker.
  class BaseDateAndTimeInputType : public InputType,
                                   public DateTimeChooserClient,
                                   private DateTimeEditElementEditControlOwner
  {
  public:
    KRYS_NODISCARD bool TypeMismatchFor(dom::DOMStringView value) const noexcept final;
    KRYS_NODISCARD bool ValueMissing(dom::DOMStringView value) const noexcept final;
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

    KRYS_NODISCARD Decimal ParseToNumber(dom::DOMStringView source,
                                         const Decimal &defaultValue) const noexcept override;
    KRYS_NODISCARD dom::DOMString Serialize(const Decimal &value) const noexcept final;
    KRYS_NODISCARD dom::DOMString SerializeWithComponents(const DateComponents &date) const noexcept;

    KRYS_NODISCARD bool ShouldHaveSecondField(const DateComponents &date) const noexcept;
    KRYS_NODISCARD bool ShouldHaveMillisecondField(const DateComponents &date) const noexcept;

    /// @brief @p value in decimal, zero padded to @p width digits.
    KRYS_NODISCARD static dom::DOMString PadNumber(uint32 value, size_t width) noexcept;

    void ShowPicker() noexcept override;

  private:
    class DateTimeFormatValidator final : public DateTimeFormat::TokenHandler
    {
    private:
      EnumSet<DateTimeFormatValidationResults> _results;

    public:
      void VisitField(DateTimeFormat::FieldType fieldType, int count) noexcept final;

      void VisitLiteral(utf8_stringview) noexcept final
      {
      }

      bool ValidateFormat(dom::DOMStringView format, const BaseDateAndTimeInputType &inputType) noexcept;
    };

    RefPtr<DateTimeChooser> _dateTimeChooser;
    RefPtr<DateTimeEditElement> _dateTimeEditElement;
    bool _pickerWasActivatedByKeyboard {false};

    KRYS_NODISCARD virtual Maybe<DateComponents>
      ParseToDateComponents(dom::DOMStringView source) const noexcept = 0;
    KRYS_NODISCARD virtual Maybe<DateComponents>
      SetMillisecondToDateComponents(double value) const noexcept = 0;
    virtual void SetupLayoutParameters(DateTimeEditElement::LayoutParameters &layoutParameters,
                                       const DateComponents &date) const noexcept = 0;
    KRYS_NODISCARD virtual bool
      IsValidFormat(EnumSet<DateTimeFormatValidationResults> results) const noexcept = 0;
    KRYS_NODISCARD virtual dom::DOMString SerializeWithMilliseconds(double value) const noexcept;

    // InputType functions:
    KRYS_NODISCARD dom::DOMString VisibleValue() const noexcept final;
    KRYS_NODISCARD dom::DOMString SanitizeValue(dom::DOMStringView proposedValue) const noexcept override;
    void SetValue(dom::DOMStringView value, bool valueChanged, TextFieldEventBehavior eventBehavior,
                  TextControlSetValueSelection selection) noexcept final;
    KRYS_NODISCARD double ValueAsDate() const noexcept override;
    dom::ExceptionOr<void> SetValueAsDate(double value) const noexcept override;
    KRYS_NODISCARD double ValueAsDouble() const noexcept final;
    dom::ExceptionOr<void> SetValueAsDecimal(const Decimal &value,
                                             TextFieldEventBehavior eventBehavior) const noexcept final;
    KRYS_NODISCARD Decimal DefaultValueForStepUp() const noexcept override;
    KRYS_NODISCARD dom::DOMString LocalizeValue(dom::DOMStringView proposedValue) const noexcept final;
    KRYS_NODISCARD bool SupportsReadOnly() const noexcept final;

    void HandleActivationBehavior(dom::Event &event) noexcept override;
    void CreateShadowSubtree() noexcept final;
    void RemoveShadowSubtree() noexcept final;
    void UpdateInnerTextValue() noexcept final;
    void AttributeChanged(dom::DOMStringAtom name) noexcept final;
    KRYS_NODISCARD bool IsPresentingAttachedView() const noexcept final;
    void ElementDidBlur() noexcept final;
    void Detach() noexcept final;

    ShouldCallBaseEventHandler HandleKeydownEvent(dom::KeyboardEvent &event) noexcept final;
    void HandleKeypressEvent(dom::KeyboardEvent &event) noexcept final;
    void HandleKeyupEvent(dom::KeyboardEvent &event) noexcept final;

    // DateTimeEditElementEditControlOwner functions:
    void DidBlurFromControl() noexcept final;
    void DidChangeValueFromControl() noexcept final;
    void DidReceiveSpaceKeyFromControl() noexcept final;
    KRYS_NODISCARD bool IsEditControlOwnerDisabled() const noexcept final;
    KRYS_NODISCARD bool IsEditControlOwnerReadOnly() const noexcept final;
    KRYS_NODISCARD dom::DOMString LocaleIdentifier() const noexcept final;

    // DateTimeChooserClient functions:
    void DidChooseValue(utf8_stringview value) noexcept final;

    void DidEndChooser() noexcept final
    {
      _dateTimeChooser = nullptr;
    }

    bool SetupDateTimeChooserParameters(DateTimeChooserParameters &parameters) noexcept;
    void CloseDateTimeChooser() noexcept;
  };
}
