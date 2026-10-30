#include "Krystal.Booey/HTML/NumberInputType.hpp"
#include "Krystal.Booey/DOM/BeforeTextInsertedEvent.hpp"
#include "Krystal.Booey/DOM/KeyboardEvent.hpp"
#include "Krystal.Booey/HTML/HTMLInputElement.hpp"
#include "Krystal.Booey/HTML/Parser/HTMLParserIdioms.hpp"
#include "Krystal.Booey/Infra/StringAlgorithms.hpp"
#include "Krystal.Booey/Platform/LocalizedStrings.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Text/CodePointIterator.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <cmath>

namespace krys::boo::html
{
  namespace
  {
    constexpr int NumberDefaultStep = 1;
    constexpr int NumberDefaultStepBase = 0;
    constexpr int NumberStepScaleFactor = 1;

    struct RealNumberRenderSize
    {
      uint32 SizeBeforeDecimalPoint;
      uint32 SizeAfteDecimalPoint;

      KRYS_NODISCARD RealNumberRenderSize Max(const RealNumberRenderSize &other) const noexcept
      {
        return {std::max(SizeBeforeDecimalPoint, other.SizeBeforeDecimalPoint),
                std::max(SizeAfteDecimalPoint, other.SizeAfteDecimalPoint)};
      }
    };

    KRYS_NODISCARD RealNumberRenderSize CalculateRenderSize(const Decimal &value) noexcept
    {
      krys_debug_assert(value.IsFinite());
      const auto sizeOfDigits = static_cast<uint32>(std::to_string(value.Value().Coefficient()).size());
      const uint32 sizeOfSign = value.IsNegative() ? 1 : 0;
      const int32 exponent = value.Exponent();
      if (exponent >= 0)
      {
        return {sizeOfSign + sizeOfDigits, 0};
      }

      const int32 sizeBeforeDecimalPoint = exponent + static_cast<int32>(sizeOfDigits);
      if (sizeBeforeDecimalPoint > 0)
      {
        // In case of "123.456"
        return {sizeOfSign + static_cast<uint32>(sizeBeforeDecimalPoint),
                sizeOfDigits - static_cast<uint32>(sizeBeforeDecimalPoint)};
      }

      // In case of "0.00012345"
      const uint32 sizeOfZero = 1;
      const auto numberOfZeroAfterDecimalPoint = static_cast<uint32>(-sizeBeforeDecimalPoint);
      return {sizeOfSign + sizeOfZero, numberOfZeroAfterDecimalPoint + sizeOfDigits};
    }

    KRYS_NODISCARD bool IsE(char32 ch) noexcept
    {
      return ch == U'e' || ch == U'E';
    }

    KRYS_NODISCARD bool IsPlusSign(char32 ch) noexcept
    {
      return ch == U'+';
    }

    KRYS_NODISCARD bool IsSignPrefix(char32 ch) noexcept
    {
      return ch == U'+' || ch == U'-';
    }

    KRYS_NODISCARD bool IsDigit(char32 ch) noexcept
    {
      return ch >= U'0' && ch <= U'9';
    }

    KRYS_NODISCARD bool IsDecimalSeparator(char32 ch) noexcept
    {
      return ch == U'.';
    }

    KRYS_NODISCARD bool HasTwoSignChars(dom::DOMStringView string) noexcept
    {
      return std::ranges::count_if(string, [](char8 character) { return IsSignPrefix(character); }) >= 2;
    }

    KRYS_NODISCARD bool HasDecimalSeparator(dom::DOMStringView string) noexcept
    {
      return string.find(u8'.') != dom::DOMStringView::npos;
    }

    KRYS_NODISCARD bool HasE(dom::DOMStringView string) noexcept
    {
      return string.find_first_of(u8"eE") != dom::DOMStringView::npos;
    }

    KRYS_NODISCARD bool HasSignNotAfterE(dom::DOMStringView string) noexcept
    {
      for (size_t i = 0; i < string.size(); ++i)
      {
        if (IsSignPrefix(string[i]))
        {
          return !i || !IsE(string[i - 1]);
        }
      }
      return false;
    }

    /// @brief @p input with the characters a number can't have dropped.
    KRYS_NODISCARD dom::DOMString StripInvalidNumberCharacters(dom::DOMStringView input) noexcept
    {
      constexpr dom::DOMStringView allowedChars = u8"0123456789.Ee-+";
      dom::DOMString result;
      result.reserve(input.size());
      for (char8 character : input)
      {
        if (allowedChars.find(character) != dom::DOMStringView::npos)
        {
          result.push_back(character);
        }
      }
      return result;
    }

    /// @brief @p input with full-width digits, full stops and minus-like characters made ASCII.
    KRYS_NODISCARD dom::DOMString NormalizeFullWidthNumberChars(dom::DOMStringView input) noexcept
    {
      constexpr char32 fullwidthDigitZero = 0xFF10;
      constexpr char32 fullwidthDigitNine = 0xFF19;
      constexpr char32 katakanaHiraganaProlongedSoundMark = 0x30FC;
      constexpr char32 fullwidthHyphenMinus = 0xFF0D;
      constexpr char32 minusSign = 0x2212;
      constexpr char32 fullwidthFullStop = 0xFF0E;

      StringBuilder result;
      result.ReserveCapacity(input.size());
      for (text::CodePointIterator iterator(input); !iterator.AtEnd(); ++iterator)
      {
        char32 character = *iterator;
        if (character >= fullwidthDigitZero && character <= fullwidthDigitNine)
        {
          // Convert full-width digits (U+FF10-U+FF19) to ASCII digits (0-9)
          result.Append(static_cast<char32>(character - fullwidthDigitZero + U'0'));
        }
        else if (character == katakanaHiraganaProlongedSoundMark || character == fullwidthHyphenMinus
                 || character == minusSign)
        {
          // Convert minus-like characters commonly produced by IMEs to ASCII '-'.
          //
          // Note: On Japanese IMEs, typing a minus sign in full-width mode can
          // produce U+30FC, U+FF0D, or U+2212, depending on the platform and input mode.
          //
          // Since users generally intend to input negative numbers in such cases,
          // we normalize U+30FC, U+FF0D, and U+2212 to ASCII minus '-' (U+002D).
          result.Append(U'-');
        }
        else if (character == fullwidthFullStop)
        {
          // Convert full-width full stop (U+FF0E) to ASCII dot (.)
          result.Append(U'.');
        }
        else
        {
          // Preserve other characters
          result.Append(character);
        }
      }
      return krys::move(result).ToString();
    }
  }

  void NumberInputType::SetValue(dom::DOMStringView sanitizedValue, bool valueChanged,
                                 TextFieldEventBehavior eventBehavior,
                                 TextControlSetValueSelection selection) noexcept
  {
    krys_debug_assert(Element());
    if (!valueChanged && sanitizedValue.empty() && !Element()->InnerTextValue().empty())
    {
      UpdateInnerTextValue();
    }
    TextFieldInputType::SetValue(sanitizedValue, valueChanged, eventBehavior, selection);
  }

  bool NumberInputType::SizeShouldIncludeDecoration(int32 defaultSize, int32 &preferredSize) const noexcept
  {
    preferredSize = defaultSize;

    krys_debug_assert(Element());
    auto &element = *Element();
    auto stepString = element.GetAttribute(u8"step").value_or(dom::DOMString {});
    if (infra::StringAlgorithms::ASCIICaseInsensitiveMatch(stepString, u8"any"))
    {
      return false;
    }

    const Decimal minimum =
      ParseToDecimalForNumberType(element.GetAttribute(u8"min").value_or(dom::DOMString {}));
    if (!minimum.IsFinite())
    {
      return false;
    }

    const Decimal maximum =
      ParseToDecimalForNumberType(element.GetAttribute(u8"max").value_or(dom::DOMString {}));
    if (!maximum.IsFinite())
    {
      return false;
    }

    const Decimal step = ParseToDecimalForNumberType(stepString, Decimal(1));
    krys_debug_assert(step.IsFinite());

    RealNumberRenderSize size =
      CalculateRenderSize(minimum).Max(CalculateRenderSize(maximum).Max(CalculateRenderSize(step)));

    preferredSize = static_cast<int32>(size.SizeBeforeDecimalPoint + size.SizeAfteDecimalPoint
                                       + (size.SizeAfteDecimalPoint ? 1 : 0));

    return true;
  }

  float NumberInputType::DecorationWidth(float inputWidth) const noexcept
  {
    krys_debug_assert(Element());

    float width = 0;
    auto *spinButton = Element()->InnerSpinButtonElement();
    if (auto *spinRenderer = spinButton ? spinButton->GetRenderBox() : nullptr)
    {
      width += spinRenderer->BorderAndPaddingLogicalWidth().ToFloat();

      // Since the width of spinRenderer is not calculated yet, spinRenderer->logicalWidth() returns 0.
      // So computedStyle()->logicalWidth() is used instead.

      // FIXME: This is incorrect for anything other than fixed widths.
      const auto *computedStyle = spinButton->ExistingComputedStyle();
      if (auto fixedLogicalWidth = computedStyle->LogicalWidth().TryFixed())
      {
        width += fixedLogicalWidth->ResolveZoom(computedStyle->UsedZoomForLength());
      }
      else if (auto percentageLogicalWidth = computedStyle->LogicalWidth().TryPercentage())
      {
        auto percentageLogicalWidthValue = percentageLogicalWidth->Value;
        if (percentageLogicalWidthValue != 100.f)
        {
          width += inputWidth * percentageLogicalWidthValue / (100 - percentageLogicalWidthValue);
        }
      }
    }
    return width;
  }

  InputType::ShouldCallBaseEventHandler
    NumberInputType::HandleKeydownEvent(dom::KeyboardEvent &event) noexcept
  {
    HandleKeydownEventForSpinButton(event);
    if (!event.DefaultHandled())
    {
      return TextFieldInputType::HandleKeydownEvent(event);
    }
    return ShouldCallBaseEventHandler::Yes;
  }

  void NumberInputType::HandleBeforeTextInsertedEvent(dom::BeforeTextInsertedEvent &event) noexcept
  {
    // Normalize full-width digits and minus sign to ASCII
    auto normalizedText = NormalizeFullWidthNumberChars(event.Text());

    // FIXME: Numbers in the element's locale (convertFromLocalizedNumber), with locales.

    // If the cleaned up text doesn't match input text, don't insert partial input
    // since it could be an incorrect paste.
    auto updatedEventText = StripInvalidNumberCharacters(normalizedText);

    // Get left and right of cursor
    krys_debug_assert(Element());
    auto &element = *Element();

    auto originalValue = element.InnerTextValue();
    auto selectionStart = std::min<size_t>(element.SelectionStart(), originalValue.size());
    auto selectionEnd =
      std::max(selectionStart, std::min<size_t>(element.SelectionEnd(), originalValue.size()));

    auto leftHalf = originalValue.substr(0, selectionStart);
    auto rightHalf = originalValue.substr(selectionEnd);

    // Process 1 char at a time
    dom::DOMString finalEventText;
    for (char8 character : updatedEventText)
    {
      if (IsDecimalSeparator(character))
      {
        // For a decimal point input:
        // - Reject if the editing value already contains another decimal point
        // - Reject if the editing value contains 'e' and the caret is placed
        // after the 'e'.
        // - Reject if the editing value contains '+' or '-' and the caret is
        // placed before it unless it's after an e
        if (HasDecimalSeparator(leftHalf) || HasDecimalSeparator(rightHalf) || HasE(leftHalf)
            || HasSignNotAfterE(rightHalf))
        {
          continue;
        }
      }
      else if (IsE(character))
      {
        // For 'e' input:
        // - Reject if the editing value already contains another 'e'
        // - Reject if the editing value contains a decimal point, and the caret
        // is placed before it

        // Disallow inserting 'e' if the first character is '+'
        if (!leftHalf.empty() && IsPlusSign(leftHalf[0]))
        {
          continue;
        }

        // Reject inserting 'e' at the beginning if the first character is sign
        if (leftHalf.empty() && !rightHalf.empty() && IsSignPrefix(rightHalf[0]))
        {
          continue;
        }

        if (HasE(leftHalf) || HasE(rightHalf) || HasDecimalSeparator(rightHalf))
        {
          continue;
        }
      }
      else if (IsSignPrefix(character))
      {
        // For '-' or '+' input:
        // - Reject if the editing value already contains two signs
        // - Reject if the editing value contains 'e' and the caret is placed
        // neither at the beginning of the value nor just after 'e'
        if (HasTwoSignChars(leftHalf + rightHalf))
        {
          continue;
        }

        auto hasE = HasE(leftHalf) || HasE(rightHalf);

        if (leftHalf.empty())
        {
          // Caret is at the start of the value
          if (!rightHalf.empty() && IsSignPrefix(rightHalf[0]))
          {
            // Reject if there's already a sign at the start (to avoid --1 or ++1)
            continue;
          }
          if (hasE && IsPlusSign(character))
          {
            // If there is already an 'e' in the value, disallow inserting a leading '+'
            continue;
          }
          // Otherwise, allow inserting '-' or other allowed characters at the start
        }
        else
        {
          if (hasE)
          {
            // Must be just after 'e'
            if (!IsE(leftHalf.back()))
            {
              continue;
            }
            // Reject if there is already a sign after 'e'
            if (!rightHalf.empty() && IsSignPrefix(rightHalf[0]))
            {
              continue;
            }
          }
          else
          {
            // No 'e' and not at start: reject
            continue;
          }
        }
      }
      else if (IsDigit(character))
      {
        // For a digit input:
        // - Reject if the first letter of the editing value is a sign and the
        // caret is placed just before it
        // - Reject if the editing value contains 'e' + a sign, and the caret is
        // placed between them.
        if (leftHalf.empty() && !rightHalf.empty() && IsSignPrefix(rightHalf[0]))
        {
          continue;
        }

        if (!leftHalf.empty() && IsE(leftHalf.back()) && !rightHalf.empty() && IsSignPrefix(rightHalf[0]))
        {
          continue;
        }
      }

      // Add character
      leftHalf.push_back(character);
      finalEventText.push_back(character);
    }
    event.SetText(krys::move(finalEventText));
  }

  dom::DOMString NumberInputType::ConvertFromVisibleValue(dom::DOMStringView visibleValue) const noexcept
  {
    // FIXME: Numbers in the element's locale (convertFromLocalizedNumber), with locales.
    return dom::DOMString(visibleValue);
  }

  void NumberInputType::AttributeChanged(dom::DOMStringAtom name) noexcept
  {
    krys_debug_assert(Element());
    if (name == u8"max" || name == u8"min")
    {
      if (auto *element = Element())
      {
        element->InvalidateStyleForSubtree();
        if (auto *renderer = element->Renderer())
        {
          renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
        }
      }
    }
    else if (name == u8"class" || name == u8"step")
    {
      if (auto *element = Element())
      {
        if (auto *renderer = element->Renderer())
        {
          renderer->SetNeedsLayoutAndPreferredWidthsUpdate();
        }
      }
    }

    TextFieldInputType::AttributeChanged(name);
  }

  double NumberInputType::ValueAsDouble() const noexcept
  {
    krys_debug_assert(Element());
    return ParseToDoubleForNumberType(Element()->Value());
  }

  dom::ExceptionOr<void>
    NumberInputType::SetValueAsDouble(double newValue, TextFieldEventBehavior eventBehavior) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(SerializeForNumberType(newValue), eventBehavior);
    return {};
  }

  dom::ExceptionOr<void>
    NumberInputType::SetValueAsDecimal(const Decimal &newValue,
                                       TextFieldEventBehavior eventBehavior) const noexcept
  {
    krys_debug_assert(Element());
    (void)Element()->SetValue(SerializeForNumberType(newValue), eventBehavior);
    return {};
  }

  bool NumberInputType::TypeMismatchFor(dom::DOMStringView value) const noexcept
  {
    return !value.empty() && !std::isfinite(ParseToDoubleForNumberType(value));
  }

  bool NumberInputType::TypeMismatch() const noexcept
  {
    krys_debug_assert(Element());
    krys_debug_assert(!TypeMismatchFor(Element()->Value()));
    return false;
  }

  StepRange NumberInputType::CreateStepRange(AnyStepHandling anyStepHandling) const noexcept
  {
    static const StepRange::StepDescription stepDescription {NumberDefaultStep, NumberDefaultStepBase,
                                                             NumberStepScaleFactor};

    krys_debug_assert(Element());
    const Decimal stepBase = FindStepBase(Decimal(NumberDefaultStepBase));

    const Decimal doubleMax = Decimal::DoubleMax();
    auto &element = *Element();

    RangeLimitations rangeLimitations = RangeLimitations::Invalid;
    auto extractBound = [&](dom::DOMStringView attributeName, const Decimal &defaultValue) -> Decimal
    {
      Decimal valueFromAttribute =
        ParseToNumberOrNaN(element.GetAttribute(attributeName).value_or(dom::DOMString()));
      if (valueFromAttribute.IsFinite())
      {
        rangeLimitations = RangeLimitations::Valid;
        return valueFromAttribute;
      }
      return defaultValue;
    };
    Decimal minimum = extractBound(u8"min", -doubleMax);
    Decimal maximum = extractBound(u8"max", doubleMax);

    const Decimal step =
      StepRange::ParseStep(anyStepHandling, stepDescription, element.GetAttribute(u8"step"));
    return StepRange(stepBase, rangeLimitations, minimum, maximum, step, stepDescription);
  }

  Decimal NumberInputType::ParseToNumber(dom::DOMStringView string,
                                         const Decimal &defaultValue) const noexcept
  {
    return ParseToDecimalForNumberType(string, defaultValue);
  }

  dom::DOMString NumberInputType::Serialize(const Decimal &value) const noexcept
  {
    if (!value.IsFinite())
    {
      return {};
    }
    return SerializeForNumberType(value);
  }

  dom::DOMString NumberInputType::SanitizeValue(dom::DOMStringView proposedValue) const noexcept
  {
    if (proposedValue.empty())
    {
      return {};
    }
    if (std::isfinite(ParseToDoubleForNumberType(proposedValue)))
    {
      return dom::DOMString(proposedValue);
    }
    return {};
  }

  bool NumberInputType::HasBadInput() const noexcept
  {
    krys_debug_assert(Element());
    auto standardValue = ConvertFromVisibleValue(Element()->InnerTextValue());
    return !standardValue.empty() && !std::isfinite(ParseToDoubleForNumberType(standardValue));
  }

  dom::DOMString NumberInputType::BadInputText() const noexcept
  {
    return ValidationMessageBadInputForNumberText();
  }

  bool NumberInputType::SupportsPlaceholder() const noexcept
  {
    return true;
  }
}
