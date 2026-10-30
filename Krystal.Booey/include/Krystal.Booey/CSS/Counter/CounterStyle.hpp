#pragma once

#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/CanMakeWeakPtr.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::css
{
  /// @brief A counter style: the algorithm, from a `@counter-style` rule or a predefined style, that turns a
  /// counter value into its text.
  /// @see https://www.w3.org/TR/css-counter-styles-3/#counter-styles
  class CounterStyle : public RefCounted<CounterStyle>, public CanMakeWeakPtr<CounterStyle>
  {
  private:
    CounterStyleDescriptors _descriptors;
    bool _predefinedCounterStyle {false};
    WeakPtr<const CounterStyle> _fallbackReference;
    bool _isFallingBack {false};

    CounterStyle(const CounterStyleDescriptors &descriptors, bool isPredefinedCounterStyle) noexcept;

  public:
    KRYS_NODISCARD static Ref<CounterStyle> Create(const CounterStyleDescriptors &descriptors,
                                                   bool isPredefinedCounterStyle) noexcept;

    KRYS_NODISCARD bool operator==(const CounterStyle &other) const noexcept
    {
      return _descriptors == other._descriptors && _predefinedCounterStyle == other._predefinedCounterStyle;
    }

    /// @brief The representation of @p value.
    /// @see https://www.w3.org/TR/css-counter-styles-3/#generate-a-counter
    KRYS_NODISCARD CSSOMString Text(int32 value, WritingMode writingMode) noexcept;

    KRYS_NODISCARD const CounterStyleName &Name() const noexcept
    {
      return _descriptors.Name;
    }

    KRYS_NODISCARD CounterStyleSystem System() const noexcept
    {
      return _descriptors.System;
    }

    KRYS_NODISCARD const CounterStyleNegativeSymbols &Negative() const noexcept
    {
      return _descriptors.NegativeSymbols;
    }

    KRYS_NODISCARD const CounterStyleSymbol &Prefix() const noexcept
    {
      return _descriptors.Prefix;
    }

    KRYS_NODISCARD const CounterStyleSymbol &Suffix() const noexcept
    {
      return _descriptors.Suffix;
    }

    KRYS_NODISCARD const CounterStyleRanges &Ranges() const noexcept
    {
      return _descriptors.Ranges;
    }

    KRYS_NODISCARD const CounterStylePad &Pad() const noexcept
    {
      return _descriptors.Pad;
    }

    KRYS_NODISCARD const CounterStyleName &FallbackName() const noexcept
    {
      return _descriptors.FallbackName;
    }

    KRYS_NODISCARD const List<CounterStyleSymbol> &Symbols() const noexcept
    {
      return _descriptors.Symbols;
    }

    KRYS_NODISCARD const CounterStyleAdditiveSymbols &AdditiveSymbols() const noexcept
    {
      return _descriptors.AdditiveSymbols;
    }

    KRYS_NODISCARD CounterStyleSpeakAs SpeakAs() const noexcept
    {
      return _descriptors.SpeakAs;
    }

    KRYS_NODISCARD const CounterStyleName &ExtendsName() const noexcept
    {
      return _descriptors.ExtendsName;
    }

    KRYS_NODISCARD int32 FirstSymbolValueForFixedSystem() const noexcept
    {
      return _descriptors.FixedSystemFirstSymbolValue;
    }

    KRYS_NODISCARD CounterStyleDescriptor ExplicitlySetDescriptors() const noexcept
    {
      return _descriptors.ExplicitlySetDescriptors;
    }

    void SetSystem(CounterStyleSystem system) noexcept
    {
      _descriptors.System = system;
    }

    void SetNegative(const CounterStyleNegativeSymbols &negative) noexcept
    {
      _descriptors.NegativeSymbols = negative;
    }

    void SetPrefix(const CounterStyleSymbol &prefix) noexcept
    {
      _descriptors.Prefix = prefix;
    }

    void SetSuffix(const CounterStyleSymbol &suffix) noexcept
    {
      _descriptors.Suffix = suffix;
    }

    void SetRanges(const CounterStyleRanges &ranges) noexcept
    {
      _descriptors.Ranges = ranges;
    }

    void SetPad(const CounterStylePad &pad) noexcept
    {
      _descriptors.Pad = pad;
    }

    void SetFallbackName(const CounterStyleName &fallbackName) noexcept
    {
      _descriptors.FallbackName = fallbackName;
    }

    void SetSymbols(const List<CounterStyleSymbol> &symbols) noexcept
    {
      _descriptors.Symbols = symbols;
    }

    void SetAdditiveSymbols(const CounterStyleAdditiveSymbols &additiveSymbols) noexcept
    {
      _descriptors.AdditiveSymbols = additiveSymbols;
    }

    void SetSpeakAs(CounterStyleSpeakAs speakAs) noexcept
    {
      _descriptors.SpeakAs = speakAs;
    }

    void SetFirstSymbolValueForFixedSystem(int32 firstSymbolValue) noexcept
    {
      _descriptors.FixedSystemFirstSymbolValue = firstSymbolValue;
    }

    void SetFallbackReference(Ref<CounterStyle> &&fallback) noexcept;

    KRYS_NODISCARD bool IsFallbackUnresolved() const noexcept
    {
      return !_fallbackReference;
    }

    KRYS_NODISCARD bool IsExtendsUnresolved() const noexcept
    {
      return !_descriptors.IsExtendedResolved;
    }

    KRYS_NODISCARD bool IsExtendsSystem() const noexcept
    {
      return System() == CounterStyleSystem::Extends;
    }

    void ExtendAndResolve(const CounterStyle &extendedCounterStyle) noexcept;

    KRYS_NODISCARD static CSSOMString CounterForSystemSimplifiedChineseInformal(int32 value) noexcept;
    KRYS_NODISCARD static CSSOMString CounterForSystemSimplifiedChineseFormal(int32 value) noexcept;
    KRYS_NODISCARD static CSSOMString CounterForSystemTraditionalChineseInformal(int32 value) noexcept;
    KRYS_NODISCARD static CSSOMString CounterForSystemTraditionalChineseFormal(int32 value) noexcept;
    KRYS_NODISCARD static CSSOMString CounterForSystemEthiopicNumeric(uint32 value) noexcept;
    KRYS_NODISCARD static CSSOMString CounterForSystemDisclosureClosed(WritingMode writingMode) noexcept;
    KRYS_NODISCARD static CSSOMString CounterForSystemDisclosureOpen(WritingMode writingMode) noexcept;

  private:
    // https://www.w3.org/TR/css-counter-styles-3/#counter-style-range
    KRYS_NODISCARD bool IsInRange(int32 value) const noexcept;
    // https://www.w3.org/TR/css-counter-styles-3/#counter-style-negative
    KRYS_NODISCARD bool ShouldApplyNegativeSymbols(int32 value) const noexcept;

    // https://www.w3.org/TR/css-counter-styles-3/#counter-style-fallback
    KRYS_NODISCARD const CounterStyle *Fallback() const noexcept
    {
      return _fallbackReference.get();
    }

    KRYS_NODISCARD CSSOMString FallbackText(int32 value, WritingMode writingMode) noexcept;

    void ApplyPadSymbols(CSSOMString &text, int32 value) const noexcept;
    void ApplyNegativeSymbols(CSSOMString &text) const noexcept;

    // Initial text representation for the counter, before applying pad and/or negative symbols. Suffix and
    // Prefix are also not considered as described by
    // https://www.w3.org/TR/css-counter-styles-3/#counter-styles. Null when the counter style can't represent
    // the value.
    KRYS_NODISCARD Maybe<CSSOMString> InitialRepresentation(int32 value,
                                                            WritingMode writingMode) const noexcept;
    KRYS_NODISCARD CSSOMString CounterForSystemCyclic(int32 value) const noexcept;
    KRYS_NODISCARD Maybe<CSSOMString> CounterForSystemFixed(int32 value) const noexcept;
    KRYS_NODISCARD Maybe<CSSOMString> CounterForSystemSymbolic(uint32 value) const noexcept;
    KRYS_NODISCARD Maybe<CSSOMString> CounterForSystemAlphabetic(uint32 value) const noexcept;
    KRYS_NODISCARD CSSOMString CounterForSystemNumeric(uint32 value) const noexcept;
    KRYS_NODISCARD Maybe<CSSOMString> CounterForSystemAdditive(uint32 value) const noexcept;

    KRYS_NODISCARD bool IsPredefinedCounterStyle() const noexcept
    {
      return _predefinedCounterStyle;
    }

    KRYS_NODISCARD bool IsAutoRange() const noexcept
    {
      return Ranges().empty();
    }
  };
}
