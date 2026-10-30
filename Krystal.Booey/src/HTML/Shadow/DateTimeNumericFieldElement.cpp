#include "Krystal.Booey/HTML/Shadow/DateTimeNumericFieldElement.hpp"
#include "Krystal.Booey/Constants/EventNames.hpp"
#include "Krystal.Booey/Core/PlatformLocale.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Core/Time/MonotonicTime.hpp"
#include <algorithm>
#include <charconv>
#include <format>
#include <unicode/utf8.h>

namespace krys::boo::html
{
  namespace
  {
    constexpr MonotonicTime::duration TypeAheadTimeout = std::chrono::seconds(1);

    /// @brief @p value in decimal, zero padded to @p width digits.
    KRYS_NODISCARD dom::DOMString PadNumber(int value, size_t width) noexcept
    {
      auto digits = std::format("{:0{}}", value, width);
      return dom::DOMString(reinterpret_cast<const char8 *>(digits.data()), digits.size());
    }
  }

  int DateTimeNumericFieldElement::Range::ClampValue(int value) const noexcept
  {
    return std::clamp(value, Minimum, Maximum);
  }

  bool DateTimeNumericFieldElement::Range::IsInRange(int value) const noexcept
  {
    return value >= Minimum && value <= Maximum;
  }

  DateTimeNumericFieldElement::DateTimeNumericFieldElement(dom::Document &document,
                                                           DateTimeFieldElementFieldOwner &fieldOwner,
                                                           const Range &range, int placeholder) noexcept
      : DateTimeFieldElement(document, fieldOwner), _range(range), _placeholder(FormatValue(placeholder)),
        _placeholderValue(placeholder)
  {
  }

  void DateTimeNumericFieldElement::AdjustMinInlineSize(RenderStyle &style) const noexcept
  {
    auto &font = style.GetFontCascade();

    size_t length = 2;
    if (_range.Maximum > 999)
    {
      length = 4;
    }
    else if (_range.Maximum > 99)
    {
      length = 3;
    }

    auto &locale = LocaleForOwner();

    float inlineSize = 0;
    for (char8 c = '0'; c <= '9'; ++c)
    {
      auto numberString = locale.ConvertToLocalizedNumber(dom::DOMString(length, c));
      inlineSize = std::max(inlineSize, font.Width(RenderBlock::ConstructTextRun(numberString, style)));
    }

    style.SetLogicalMinWidth(style::MinimumSize::Fixed {inlineSize / style.UsedZoomForLength().Value});
  }

  int DateTimeNumericFieldElement::Maximum() const noexcept
  {
    return _range.Maximum;
  }

  dom::DOMString DateTimeNumericFieldElement::FormatValue(int value) const noexcept
  {
    auto &locale = LocaleForOwner();

    if (_range.Maximum > 999)
    {
      return locale.ConvertToLocalizedNumber(PadNumber(value, 4));
    }
    if (_range.Maximum > 99)
    {
      return locale.ConvertToLocalizedNumber(PadNumber(value, 3));
    }
    return locale.ConvertToLocalizedNumber(PadNumber(value, 2));
  }

  bool DateTimeNumericFieldElement::HasValue() const noexcept
  {
    return _hasValue;
  }

  void DateTimeNumericFieldElement::SetEmptyValue(EventBehavior eventBehavior) noexcept
  {
    _value = 0;
    _hasValue = false;
    _typeAheadBuffer.clear();
    UpdateVisibleValue(eventBehavior);
  }

  void DateTimeNumericFieldElement::SetValueAsInteger(int value, EventBehavior eventBehavior) noexcept
  {
    _value = _range.ClampValue(value);
    _hasValue = true;
    UpdateVisibleValue(eventBehavior);
  }

  void DateTimeNumericFieldElement::SetValueAsIntegerByStepping(int value) noexcept
  {
    _typeAheadBuffer.clear();
    SetValueAsInteger(value, EventBehavior::DispatchInputAndChangeEvents);
  }

  void DateTimeNumericFieldElement::StepDown() noexcept
  {
    int newValue = _hasValue ? _value - 1 : _range.Maximum;
    if (!_range.IsInRange(newValue))
    {
      newValue = _range.Maximum;
    }
    SetValueAsIntegerByStepping(newValue);
  }

  void DateTimeNumericFieldElement::StepUp() noexcept
  {
    int newValue = _hasValue ? _value + 1 : _range.Minimum;
    if (!_range.IsInRange(newValue))
    {
      newValue = _range.Minimum;
    }
    SetValueAsIntegerByStepping(newValue);
  }

  dom::DOMString DateTimeNumericFieldElement::Value() const noexcept
  {
    if (_hasValue)
    {
      return FormatValue(_value);
    }
    return {};
  }

  dom::DOMString DateTimeNumericFieldElement::PlaceholderValue() const noexcept
  {
    return _placeholder;
  }

  void DateTimeNumericFieldElement::HandleKeyboardEvent(dom::KeyboardEvent &keyboardEvent) noexcept
  {
    if (keyboardEvent.Type() != EventNames::KeyPress)
    {
      return;
    }

    auto charCode = static_cast<UChar32>(keyboardEvent.CharCode());
    if (charCode < 0 || charCode > 0x10FFFF)
    {
      return;
    }
    char8 encoded[U8_MAX_LENGTH];
    size_t encodedLength = 0;
    U8_APPEND_UNSAFE(encoded, encodedLength, charCode);
    auto number = LocaleForOwner().ConvertFromLocalizedNumber(dom::DOMStringView(encoded, encodedLength));
    int digit = number.empty() ? -1 : number[0] - '0';
    if (digit < 0 || digit > 9)
    {
      return;
    }

    auto timeSinceLastDigitChar = keyboardEvent.TimeStamp() - _lastDigitCharTime;
    _lastDigitCharTime = keyboardEvent.TimeStamp();

    if (timeSinceLastDigitChar > TypeAheadTimeout)
    {
      _typeAheadBuffer.clear();
    }
    else if (auto length = _typeAheadBuffer.size())
    {
      auto maxLength = FormatValue(_range.Maximum).size();
      if (length == maxLength)
      {
        _typeAheadBuffer.clear();
      }
    }

    _typeAheadBuffer.append(number);
    int value = 0;
    auto *begin = reinterpret_cast<const char *>(_typeAheadBuffer.data());
    (void)std::from_chars(begin, begin + _typeAheadBuffer.size(), value);
    SetValueAsInteger(value, EventBehavior::DispatchInputAndChangeEvents);

    keyboardEvent.SetDefaultHandled();
  }

  void DateTimeNumericFieldElement::HandleBlurEvent(dom::Event &event) noexcept
  {
    _typeAheadBuffer.clear();
    DateTimeFieldElement::HandleBlurEvent(event);
  }
}
