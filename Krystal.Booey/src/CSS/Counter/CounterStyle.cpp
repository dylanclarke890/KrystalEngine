#include "Krystal.Booey/CSS/Counter/CounterStyle.hpp"
#include "Krystal.Booey/Core/TextBreakIterator.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleRegistry.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Text/Encodings/UTF.hpp"
#include "Krystal.Core/Text/Unicode.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include <cmath>
#include <cstdlib>
#include <limits>

namespace krys::boo::css
{
  namespace
  {
    void AppendCodePoint(CSSOMString &string, char32 codePoint) noexcept
    {
      auto result = text::UTF8::EncodeOne(codePoint);
      string.append(result.Output.data(), result.CodeUnitsProduced);
    }

    KRYS_NODISCARD CSSOMString StringFromCodePoint(char32 codePoint) noexcept
    {
      CSSOMString string;
      AppendCodePoint(string, codePoint);
      return string;
    }

    enum class Formality : bool
    {
      Informal,
      Formal
    };

    // This table format was derived from an old draft of the CSS specification: 3 group markers, 3 digit
    // markers, 10 digits, negative sign.
    KRYS_NODISCARD CSSOMString CounterForSystemCJK(int32 number, const Array<char32, 17> &table,
                                                   Formality formality) noexcept
    {
      enum AbstractCJKCharacter
      {
        NoChar,
        SecondGroupMarker,
        ThirdGroupMarker,
        FourthGroupMarker,
        SecondDigitMarker,
        ThirdDigitMarker,
        FourthDigitMarker,
        Digit0,
        Digit1,
        Digit2,
        Digit3,
        Digit4,
        Digit5,
        Digit6,
        Digit7,
        Digit8,
        Digit9,
        NegativeSign
      };

      if (!number)
      {
        return StringFromCodePoint(table[Digit0 - 1]);
      }

      krys_debug_assert(number != std::numeric_limits<int32>::min());
      bool needsNegativeSign = number < 0;
      if (needsNegativeSign)
      {
        number = -number;
      }

      constexpr uint32 groupLength = 8; // 4 digits, 3 digit markers, and a group marker
      constexpr uint32 bufferLength = 4 * groupLength;
      Array<AbstractCJKCharacter, bufferLength> buffer;
      buffer.fill(NoChar);

      for (int32 i = 0; i < 4; ++i)
      {
        int32 groupValue = number % 10'000;
        number /= 10'000;

        // Process least-significant group first, but put it in the buffer last.
        auto *group = buffer.data() + (3 - i) * groupLength;

        if (groupValue && i)
        {
          group[7] = static_cast<AbstractCJKCharacter>(SecondGroupMarker - 1 + i);
        }

        // Put in the four digits and digit markers for any non-zero digits.
        group[6] = static_cast<AbstractCJKCharacter>(Digit0 + (groupValue % 10));
        if (number || groupValue > 9)
        {
          int32 digitValue = ((groupValue / 10) % 10);
          group[4] = static_cast<AbstractCJKCharacter>(Digit0 + digitValue);
          if (digitValue)
          {
            group[5] = SecondDigitMarker;
          }
        }
        if (number || groupValue > 99)
        {
          int32 digitValue = ((groupValue / 100) % 10);
          group[2] = static_cast<AbstractCJKCharacter>(Digit0 + digitValue);
          if (digitValue)
          {
            group[3] = ThirdDigitMarker;
          }
        }
        if (number || groupValue > 999)
        {
          int32 digitValue = groupValue / 1'000;
          group[0] = static_cast<AbstractCJKCharacter>(Digit0 + digitValue);
          if (digitValue)
          {
            group[1] = FourthDigitMarker;
          }
        }

        if (formality == Formality::Informal && groupValue < 20)
        {
          // Remove the tens digit, but leave the marker.
          krys_debug_assert(group[4] == NoChar || group[4] == Digit0 || group[4] == Digit1);
          group[4] = NoChar;
        }

        if (!number)
        {
          break;
        }
      }

      // Convert into characters, omitting consecutive runs of digit0 and trailing digit0.
      uint32 length = 0;
      Array<char32, bufferLength + 1> characters;
      auto last = NoChar;
      if (needsNegativeSign)
      {
        characters[length++] = table[NegativeSign - 1];
      }
      for (uint32 i = 0; i < bufferLength; ++i)
      {
        auto character = buffer[i];
        if (character != NoChar)
        {
          if (character != Digit0 || last != Digit0)
          {
            characters[length++] = table[character - 1];
          }
          last = character;
        }
      }
      if (last == Digit0)
      {
        --length;
      }

      CSSOMString result;
      for (uint32 i = 0; i < length; ++i)
      {
        AppendCodePoint(result, characters[i]);
      }
      return result;
    }
  }

  // https://www.w3.org/TR/css-counter-styles-3/#cyclic-system
  CSSOMString CounterStyle::CounterForSystemCyclic(int32 value) const noexcept
  {
    auto amountOfSymbols = Symbols().size();
    krys_debug_assert(amountOfSymbols > 0);

    // For avoiding subtracting -1 from INT_MAX we will sum-up amountOfSymbols in case the value is not
    // positive. This works because x % y = (x + y) % y
    auto symbolIndex = static_cast<uint32>(value > 0 ? value : value + static_cast<int32>(amountOfSymbols));
    symbolIndex = (symbolIndex - 1) % amountOfSymbols;
    krys_debug_assert(symbolIndex < amountOfSymbols);

    return Symbols().at(symbolIndex).Text;
  }

  // https://www.w3.org/TR/css-counter-styles-3/#fixed-system
  Maybe<CSSOMString> CounterStyle::CounterForSystemFixed(int32 value) const noexcept
  {
    if (value < FirstSymbolValueForFixedSystem())
    {
      return {};
    }

    auto valueOffset = static_cast<uint32>(value - FirstSymbolValueForFixedSystem());
    if (valueOffset >= Symbols().size())
    {
      return {};
    }

    return Symbols().at(valueOffset).Text;
  }

  // https://www.w3.org/TR/css-counter-styles-3/#symbolic-system
  Maybe<CSSOMString> CounterStyle::CounterForSystemSymbolic(uint32 value) const noexcept
  {
    auto amountOfSymbols = Symbols().size();
    krys_debug_assert(amountOfSymbols > 0);

    if (value < 1)
    {
      return {};
    }

    auto symbolIndex = (value - 1) % amountOfSymbols;
    auto frequency =
      static_cast<uint32>(std::ceil(static_cast<float>(value) / static_cast<float>(amountOfSymbols)));

    CSSOMString result;
    for (uint32 i = 0; i < frequency; ++i)
    {
      result.append(Symbols().at(symbolIndex).Text);
    }
    return result;
  }

  // https://www.w3.org/TR/css-counter-styles-3/#alphabetic-system
  Maybe<CSSOMString> CounterStyle::CounterForSystemAlphabetic(uint32 value) const noexcept
  {
    auto amountOfSymbols = Symbols().size();
    krys_debug_assert(amountOfSymbols >= 2);

    if (value < 1)
    {
      return {};
    }

    List<const CSSOMString *> reversed;
    while (value)
    {
      value -= 1;
      reversed.push_back(&Symbols().at(value % amountOfSymbols).Text);
      value = static_cast<uint32>(value / amountOfSymbols);
    }

    CSSOMString result;
    for (auto iter = reversed.rbegin(); iter != reversed.rend(); ++iter)
    {
      result.append(**iter);
    }
    return result;
  }

  // https://www.w3.org/TR/css-counter-styles-3/#numeric-system
  CSSOMString CounterStyle::CounterForSystemNumeric(uint32 value) const noexcept
  {
    auto amountOfSymbols = Symbols().size();
    krys_debug_assert(amountOfSymbols >= 2);

    if (!value)
    {
      return Symbols().at(0).Text;
    }

    List<const CSSOMString *> reversed;
    while (value)
    {
      reversed.push_back(&Symbols().at(value % amountOfSymbols).Text);
      value = static_cast<uint32>(value / amountOfSymbols);
    }

    CSSOMString result;
    for (auto iter = reversed.rbegin(); iter != reversed.rend(); ++iter)
    {
      result.append(**iter);
    }
    return result;
  }

  // https://www.w3.org/TR/css-counter-styles-3/#additive-system
  Maybe<CSSOMString> CounterStyle::CounterForSystemAdditive(uint32 value) const noexcept
  {
    auto &additiveSymbols = AdditiveSymbols();
    if (!value)
    {
      for (auto &[symbol, weight] : additiveSymbols)
      {
        if (!weight)
        {
          return symbol.Text;
        }
      }
      return {};
    }

    CSSOMString result;
    auto appendToResult = [&](const CSSOMString &symbol, uint32 frequency)
    {
      for (uint32 i = 0; i < frequency; ++i)
      {
        result.append(symbol);
      }
    };

    for (auto &[symbol, weight] : additiveSymbols)
    {
      if (!weight || weight > value)
      {
        continue;
      }
      auto repetitions = value / weight;
      appendToResult(symbol.Text, repetitions);
      value -= weight * repetitions;
      if (!value)
      {
        return result;
      }
    }
    return {};
  }

  CSSOMString CounterStyle::CounterForSystemDisclosureClosed(WritingMode writingMode) noexcept
  {
    if (writingMode.IsVerticalTypographic())
    {
      return StringFromCodePoint(writingMode.IsInlineTopToBottom() ? text::BlackDownPointingSmallTriangle
                                                                   : text::BlackUpPointingSmallTriangle);
    }
    return StringFromCodePoint(writingMode.IsBidiLTR() ? text::BlackRightPointingSmallTriangle
                                                       : text::BlackLeftPointingSmallTriangle);
  }

  CSSOMString CounterStyle::CounterForSystemDisclosureOpen(WritingMode writingMode) noexcept
  {
    switch (writingMode.BlockDirection())
    {
      case FlowDirection::TopToBottom: return StringFromCodePoint(text::BlackDownPointingSmallTriangle);
      case FlowDirection::BottomToTop: return StringFromCodePoint(text::BlackUpPointingSmallTriangle);
      case FlowDirection::LeftToRight: return StringFromCodePoint(text::BlackRightPointingSmallTriangle);
      case FlowDirection::RightToLeft: return StringFromCodePoint(text::BlackLeftPointingSmallTriangle);
    }
    krys_debug_assert(false);
    return {};
  }

  CSSOMString CounterStyle::CounterForSystemSimplifiedChineseInformal(int32 value) noexcept
  {
    static constexpr Array<char32, 17> simplifiedChineseInformalTable {
      0x842C, 0x5104, 0x5146, // These three group markers are probably wrong; OK because we don't use this on
                              // big enough numbers.
      0x5341, 0x767E, 0x5343, 0x96F6, 0x4E00, 0x4E8C, 0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D,
      0x8D1F};
    return CounterForSystemCJK(value, simplifiedChineseInformalTable, Formality::Informal);
  }

  CSSOMString CounterStyle::CounterForSystemSimplifiedChineseFormal(int32 value) noexcept
  {
    static constexpr Array<char32, 17> simplifiedChineseFormalTable {
      0x842C, 0x5104, 0x5146, // These three group markers are probably wrong; OK because we don't use this on
                              // big enough numbers.
      0x62FE, 0x4F70, 0x4EDF, 0x96F6, 0x58F9, 0x8D30, 0x53C1, 0x8086, 0x4F0D, 0x9646, 0x67D2, 0x634C, 0x7396,
      0x8D1F};
    return CounterForSystemCJK(value, simplifiedChineseFormalTable, Formality::Formal);
  }

  CSSOMString CounterStyle::CounterForSystemTraditionalChineseInformal(int32 value) noexcept
  {
    static constexpr Array<char32, 17> traditionalChineseInformalTable {
      0x842C, 0x5104, 0x5146, 0x5341, 0x767E, 0x5343, 0x96F6, 0x4E00, 0x4E8C,
      0x4E09, 0x56DB, 0x4E94, 0x516D, 0x4E03, 0x516B, 0x4E5D, 0x8CA0};
    return CounterForSystemCJK(value, traditionalChineseInformalTable, Formality::Informal);
  }

  CSSOMString CounterStyle::CounterForSystemTraditionalChineseFormal(int32 value) noexcept
  {
    static constexpr Array<char32, 17> traditionalChineseFormalTable {
      0x842C, 0x5104, 0x5146, // These three group markers are probably wrong; OK because we don't use this on
                              // big enough numbers.
      0x62FE, 0x4F70, 0x4EDF, 0x96F6, 0x58F9, 0x8CB3, 0x53C3, 0x8086, 0x4F0D, 0x9678, 0x67D2, 0x634C, 0x7396,
      0x8CA0};
    return CounterForSystemCJK(value, traditionalChineseFormalTable, Formality::Formal);
  }

  CSSOMString CounterStyle::CounterForSystemEthiopicNumeric(uint32 value) noexcept
  {
    krys_debug_assert(value >= 1);

    if (value == 1)
    {
      constexpr char32 ethiopicDigitOne = 0x1369;
      return StringFromCodePoint(ethiopicDigitOne);
    }

    // Split the number into groups of two digits, starting with the least significant decimal digit.
    Array<uint8, 5> groups;
    for (auto &group : groups)
    {
      group = static_cast<uint8>(value % 100);
      value /= 100;
    }

    CSSOMString result;
    bool isMostSignificantGroup = true;
    for (int32 i = static_cast<int32>(groups.size()) - 1; i >= 0; --i)
    {
      auto groupValue = groups[i];
      bool isOddIndex = i & 1;
      // If the group has the value zero, or if the group is the most significant one and has the value 1,
      // or if the group has an odd index (as given in the previous step) and has the value 1,
      // then remove the digits (but leave the group, so it still has a separator appended below).
      if (!(groupValue == 1 && (isMostSignificantGroup || isOddIndex)))
      {
        if (auto tens = groupValue / 10)
        {
          AppendCodePoint(result, 0x1371 + tens);
        }
        if (auto ones = groupValue % 10)
        {
          AppendCodePoint(result, 0x1368 + ones);
        }
      }
      if (groupValue && isOddIndex)
      {
        AppendCodePoint(result, 0x137B);
      }
      if ((groupValue || !isMostSignificantGroup) && !isOddIndex && i)
      {
        AppendCodePoint(result, 0x137C);
      }
      if (groupValue)
      {
        isMostSignificantGroup = false;
      }
    }

    return result;
  }

  Maybe<CSSOMString> CounterStyle::InitialRepresentation(int32 value, WritingMode writingMode) const noexcept
  {
    auto absoluteValue = static_cast<uint32>(std::abs(static_cast<int64>(value)));
    switch (System())
    {
      case CounterStyleSystem::Cyclic:           return CounterForSystemCyclic(value);
      case CounterStyleSystem::Numeric:          return CounterForSystemNumeric(absoluteValue);
      case CounterStyleSystem::Alphabetic:       return CounterForSystemAlphabetic(absoluteValue);
      case CounterStyleSystem::Symbolic:         return CounterForSystemSymbolic(absoluteValue);
      case CounterStyleSystem::Additive:         return CounterForSystemAdditive(absoluteValue);
      case CounterStyleSystem::Fixed:            return CounterForSystemFixed(value);
      case CounterStyleSystem::DisclosureClosed: return CounterForSystemDisclosureClosed(writingMode);
      case CounterStyleSystem::DisclosureOpen:   return CounterForSystemDisclosureOpen(writingMode);
      case CounterStyleSystem::SimplifiedChineseInformal:
        return CounterForSystemSimplifiedChineseInformal(value);
      case CounterStyleSystem::SimplifiedChineseFormal: return CounterForSystemSimplifiedChineseFormal(value);
      case CounterStyleSystem::TraditionalChineseInformal:
        return CounterForSystemTraditionalChineseInformal(value);
      case CounterStyleSystem::TraditionalChineseFormal:
        return CounterForSystemTraditionalChineseFormal(value);
      case CounterStyleSystem::EthiopicNumeric:
        return CounterForSystemEthiopicNumeric(static_cast<uint32>(value));
      case CounterStyleSystem::Extends:
        // CounterStyle with extends system should have been promoted to another system at this point
        krys_debug_assert(false);
        break;
    }
    return {};
  }

  CSSOMString CounterStyle::FallbackText(int32 value, WritingMode writingMode) noexcept
  {
    if (_isFallingBack || !Fallback())
    {
      _isFallingBack = false;
      return CounterStyleRegistry::DecimalCounter()->Text(value, writingMode);
    }
    _isFallingBack = true;
    auto fallbackText = const_cast<CounterStyle &>(*Fallback()).Text(value, writingMode);
    _isFallingBack = false;
    return fallbackText;
  }

  CSSOMString CounterStyle::Text(int32 value, WritingMode writingMode) noexcept
  {
    if (!IsInRange(value))
    {
      return FallbackText(value, writingMode);
    }

    auto result = InitialRepresentation(value, writingMode);
    if (!result)
    {
      return FallbackText(value, writingMode);
    }
    ApplyPadSymbols(*result, value);
    if (ShouldApplyNegativeSymbols(value))
    {
      ApplyNegativeSymbols(*result);
    }

    return krys::move(*result);
  }

  bool CounterStyle::ShouldApplyNegativeSymbols(int32 value) const noexcept
  {
    auto system = System();
    return value < 0
           && (system == CounterStyleSystem::Symbolic || system == CounterStyleSystem::Numeric
               || system == CounterStyleSystem::Alphabetic || system == CounterStyleSystem::Additive);
  }

  void CounterStyle::ApplyNegativeSymbols(CSSOMString &text) const noexcept
  {
    text = Negative().Prefix.Text + text + Negative().Suffix.Text;
  }

  void CounterStyle::ApplyPadSymbols(CSSOMString &text, int32 value) const noexcept
  {
    // FIXME: should we cap pad minimum length?
    if (Pad().PadMinimumLength <= 0)
    {
      return;
    }

    auto numberOfSymbolsToAdd =
      static_cast<int32>(Pad().PadMinimumLength) - static_cast<int32>(NumGraphemeClusters(text));
    if (ShouldApplyNegativeSymbols(value))
    {
      numberOfSymbolsToAdd -= static_cast<int32>(NumGraphemeClusters(Negative().Prefix.Text)
                                                 + NumGraphemeClusters(Negative().Suffix.Text));
    }

    CSSOMString padText;
    for (int32 i = 0; i < numberOfSymbolsToAdd; ++i)
    {
      padText += Pad().PadSymbol.Text;
    }

    text = padText + text;
  }

  bool CounterStyle::IsInRange(int32 value) const noexcept
  {
    if (IsAutoRange())
    {
      switch (System())
      {
        case CounterStyleSystem::Cyclic:
        case CounterStyleSystem::Numeric:
        case CounterStyleSystem::Fixed:
        case CounterStyleSystem::DisclosureClosed:
        case CounterStyleSystem::DisclosureOpen:             return true;
        case CounterStyleSystem::Alphabetic:
        case CounterStyleSystem::Symbolic:
        case CounterStyleSystem::EthiopicNumeric:            return value >= 1;
        case CounterStyleSystem::Additive:                   return value >= 0;
        case CounterStyleSystem::SimplifiedChineseInformal:
        case CounterStyleSystem::SimplifiedChineseFormal:
        case CounterStyleSystem::TraditionalChineseInformal:
        case CounterStyleSystem::TraditionalChineseFormal:   return value >= -9'999 && value <= 9'999;
        case CounterStyleSystem::Extends:                    krys_debug_assert(false); return true;
      }
    }

    for (const auto &[lowerBound, higherBound] : Ranges())
    {
      if (value >= lowerBound && value <= higherBound)
      {
        return true;
      }
    }
    return false;
  }

  CounterStyle::CounterStyle(const CounterStyleDescriptors &descriptors,
                             bool isPredefinedCounterStyle) noexcept
      : _descriptors(descriptors), _predefinedCounterStyle(isPredefinedCounterStyle)
  {
  }

  Ref<CounterStyle> CounterStyle::Create(const CounterStyleDescriptors &descriptors,
                                         bool isPredefinedCounterStyle) noexcept
  {
    return AdoptRef(*new CounterStyle(descriptors, isPredefinedCounterStyle));
  }

  void CounterStyle::SetFallbackReference(Ref<CounterStyle> &&fallback) noexcept
  {
    _fallbackReference = WeakPtr<const CounterStyle>::WithRef(*fallback);
  }

  // The counter's system value is promoted to the value of the counter we are extending.
  void CounterStyle::ExtendAndResolve(const CounterStyle &extendedCounterStyle) noexcept
  {
    _descriptors.IsExtendedResolved = true;

    auto isExplicitlySet = [&](CounterStyleDescriptor descriptor)
    {
      return HasFlag(ExplicitlySetDescriptors(), descriptor);
    };

    SetSystem(extendedCounterStyle.System());
    SetFirstSymbolValueForFixedSystem(extendedCounterStyle.FirstSymbolValueForFixedSystem());

    if (!isExplicitlySet(CounterStyleDescriptor::Negative))
    {
      SetNegative(extendedCounterStyle.Negative());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::Prefix))
    {
      SetPrefix(extendedCounterStyle.Prefix());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::Suffix))
    {
      SetSuffix(extendedCounterStyle.Suffix());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::Range))
    {
      SetRanges(extendedCounterStyle.Ranges());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::Pad))
    {
      SetPad(extendedCounterStyle.Pad());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::Fallback))
    {
      SetFallbackName(extendedCounterStyle.FallbackName());
      _fallbackReference = extendedCounterStyle._fallbackReference;
    }
    if (!isExplicitlySet(CounterStyleDescriptor::Symbols))
    {
      SetSymbols(extendedCounterStyle.Symbols());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::AdditiveSymbols))
    {
      SetAdditiveSymbols(extendedCounterStyle.AdditiveSymbols());
    }
    if (!isExplicitlySet(CounterStyleDescriptor::SpeakAs))
    {
      SetSpeakAs(extendedCounterStyle.SpeakAs());
    }
  }
}
