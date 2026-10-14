#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::css
{
  class StyleProperties;
  class Value;

  using CounterStyleName = CSSOMStringAtom;
  using CounterStyleRanges = List<Pair<int32, int32>>;
  using CounterStyleSystemData = Pair<CounterStyleName, int32>;

  /// @brief A counter symbol: a string or an identifier.
  struct CounterStyleSymbol
  {
    bool IsCustomIdent {false};
    CSSOMString Text;

    KRYS_NODISCARD bool operator==(const CounterStyleSymbol &) const noexcept = default;
    KRYS_NODISCARD CSSOMString CssText() const noexcept;
  };

  using CounterStyleAdditiveSymbols = List<Pair<CounterStyleSymbol, uint32>>;

  /// @brief The keywords that can be used as values for the counter-style `system` descriptor.
  /// @see https://www.w3.org/TR/css-counter-styles-3/#counter-style-system
  enum class CounterStyleSystem : uint8
  {
    Cyclic,
    Numeric,
    Alphabetic,
    Symbolic,
    Additive,
    Fixed,
    DisclosureClosed,
    DisclosureOpen,
    SimplifiedChineseInformal,
    SimplifiedChineseFormal,
    TraditionalChineseInformal,
    TraditionalChineseFormal,
    EthiopicNumeric,
    Extends
  };

  enum class CounterStyleSpeakAs : uint8
  {
    Auto,
    Bullets,
    Numbers,
    Words,
    SpellOut,
    CounterStyleNameReference,
  };

  struct CounterStylePad
  {
    uint32 PadMinimumLength {0};
    CounterStyleSymbol PadSymbol;

    KRYS_NODISCARD bool operator==(const CounterStylePad &) const noexcept = default;
    KRYS_NODISCARD CSSOMString CssText() const noexcept;
  };

  struct CounterStyleNegativeSymbols
  {
    CounterStyleSymbol Prefix {false, u8"-"};
    CounterStyleSymbol Suffix;

    KRYS_NODISCARD bool operator==(const CounterStyleNegativeSymbols &) const noexcept = default;
  };

  /// @brief The descriptors explicitly set in a `@counter-style` rule.
  enum class CounterStyleDescriptor : uint16
  {
    None = 0,
    System = 1 << 0,
    Negative = 1 << 1,
    Prefix = 1 << 2,
    Suffix = 1 << 3,
    Range = 1 << 4,
    Pad = 1 << 5,
    Fallback = 1 << 6,
    Symbols = 1 << 7,
    AdditiveSymbols = 1 << 8,
    SpeakAs = 1 << 9
  };
}

KRYS_DEFINE_FLAGS_ENUM_TRAITS(krys::boo::css::CounterStyleDescriptor, 10uz);

namespace krys::boo::css
{
  /// @brief The descriptors of a `@counter-style` rule.
  struct CounterStyleDescriptors
  {
    CounterStyleName Name;
    CounterStyleSystem System;
    CounterStyleNegativeSymbols NegativeSymbols;
    CounterStyleSymbol Prefix;
    CounterStyleSymbol Suffix;
    CounterStyleRanges Ranges;
    CounterStylePad Pad;
    CounterStyleName FallbackName;
    List<CounterStyleSymbol> Symbols;
    CounterStyleAdditiveSymbols AdditiveSymbols;
    CounterStyleSpeakAs SpeakAs;
    CounterStyleName ExtendsName;
    int32 FixedSystemFirstSymbolValue;
    CounterStyleDescriptor ExplicitlySetDescriptors;
    bool IsExtendedResolved {false};

    // Create() is preferred here rather than a custom constructor, so that the struct still classifies as an
    // aggregate.
    KRYS_NODISCARD static CounterStyleDescriptors Create(CounterStyleName name,
                                                         const StyleProperties &properties) noexcept;

    KRYS_NODISCARD bool operator==(const CounterStyleDescriptors &other) const noexcept
    {
      // Intentionally doesn't check IsExtendedResolved.
      return Name == other.Name && System == other.System && NegativeSymbols == other.NegativeSymbols
             && Prefix == other.Prefix && Suffix == other.Suffix && Ranges == other.Ranges && Pad == other.Pad
             && FallbackName == other.FallbackName && Symbols == other.Symbols
             && AdditiveSymbols == other.AdditiveSymbols && SpeakAs == other.SpeakAs
             && ExtendsName == other.ExtendsName
             && FixedSystemFirstSymbolValue == other.FixedSystemFirstSymbolValue
             && ExplicitlySetDescriptors == other.ExplicitlySetDescriptors;
    }

    void SetExplicitlySetDescriptors(const StyleProperties &properties) noexcept;
    KRYS_NODISCARD bool IsValid() const noexcept;
    KRYS_NODISCARD static bool
      AreSymbolsValidForSystem(CounterStyleSystem system, const List<CounterStyleSymbol> &symbols,
                               const CounterStyleAdditiveSymbols &additiveSymbols) noexcept;

    void SetName(CounterStyleName name) noexcept;
    void SetSystemData(CounterStyleSystemData systemData) noexcept;
    void SetNegative(CounterStyleNegativeSymbols negative) noexcept;
    void SetPrefix(CounterStyleSymbol prefix) noexcept;
    void SetSuffix(CounterStyleSymbol suffix) noexcept;
    void SetRanges(CounterStyleRanges ranges) noexcept;
    void SetPad(CounterStylePad pad) noexcept;
    void SetFallbackName(CounterStyleName name) noexcept;
    void SetSymbols(List<CounterStyleSymbol> symbols) noexcept;
    void SetAdditiveSymbols(CounterStyleAdditiveSymbols additiveSymbols) noexcept;

    KRYS_NODISCARD CSSOMString NameCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString SystemCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString NegativeCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString PrefixCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString SuffixCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString RangesCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString PadCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString FallbackCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString SymbolsCSSText() const noexcept;
    KRYS_NODISCARD CSSOMString AdditiveSymbolsCSSText() const noexcept;
  };

  /// @brief The system named by a `system` descriptor value (`symbolic` if there is none).
  KRYS_NODISCARD CounterStyleSystem ToCounterStyleSystemEnum(const Value *system) noexcept;

  KRYS_NODISCARD CounterStyleRanges RangeFromValue(Ref<Value> value) noexcept;
  KRYS_NODISCARD CounterStyleAdditiveSymbols AdditiveSymbolsFromValue(Ref<Value> value) noexcept;
  KRYS_NODISCARD CounterStylePad PadFromValue(Ref<Value> value) noexcept;
  KRYS_NODISCARD CounterStyleNegativeSymbols NegativeSymbolsFromValue(Ref<Value> value) noexcept;
  KRYS_NODISCARD CounterStyleSymbol SymbolFromValue(RefPtr<Value> value) noexcept;
  KRYS_NODISCARD List<CounterStyleSymbol> SymbolsFromValue(Ref<Value> value) noexcept;
  KRYS_NODISCARD CounterStyleName FallbackNameFromValue(Ref<Value> value) noexcept;
  KRYS_NODISCARD CounterStyleSystemData ExtractSystemDataFromValue(RefPtr<Value> systemValue,
                                                                   CounterStyleSystem system) noexcept;
}
