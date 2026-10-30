#pragma once

#include "Krystal.Booey/DOM/Types/DOMHighResTimeStamp.hpp"
#include "Krystal.Booey/HTML/Shadow/DateTimeFieldElement.hpp"

namespace krys::boo::html
{
  /// @brief A date or time field holding a number in a range: typing digits sets it.
  class DateTimeNumericFieldElement : public DateTimeFieldElement
  {
  public:
    struct Range
    {
      int Minimum;
      int Maximum;

      KRYS_NODISCARD int ClampValue(int value) const noexcept;
      KRYS_NODISCARD bool IsInRange(int value) const noexcept;
    };

  private:
    const Range _range;
    const dom::DOMString _placeholder;
    int _placeholderValue {0};
    int _value {0};
    bool _hasValue {false};
    dom::DOMString _typeAheadBuffer;
    dom::DOMHighResTimeStamp _lastDigitCharTime {};

  protected:
    DateTimeNumericFieldElement(dom::Document &document, DateTimeFieldElementFieldOwner &fieldOwner,
                                const Range &range, int placeholder) noexcept;

    KRYS_NODISCARD int Maximum() const noexcept;

    // DateTimeFieldElement functions:
    KRYS_NODISCARD bool HasValue() const noexcept final;
    void SetEmptyValue(EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept final;
    void SetValueAsInteger(int value,
                           EventBehavior eventBehavior = EventBehavior::DispatchNoEvent) noexcept final;
    void StepDown() noexcept override;
    void StepUp() noexcept override;

    KRYS_NODISCARD int ValueAsInteger() const noexcept final
    {
      return _hasValue ? _value : -1;
    }

    KRYS_NODISCARD int PlaceholderValueAsInteger() const noexcept final
    {
      return _placeholderValue;
    }

  private:
    // DateTimeFieldElement functions:
    void AdjustMinInlineSize(RenderStyle &style) const noexcept final;
    KRYS_NODISCARD dom::DOMString Value() const noexcept final;
    KRYS_NODISCARD dom::DOMString PlaceholderValue() const noexcept final;
    void HandleKeyboardEvent(dom::KeyboardEvent &event) noexcept final;
    void HandleBlurEvent(dom::Event &event) noexcept final;

    KRYS_NODISCARD dom::DOMString FormatValue(int value) const noexcept;
    void SetValueAsIntegerByStepping(int value) noexcept;
  };
}
