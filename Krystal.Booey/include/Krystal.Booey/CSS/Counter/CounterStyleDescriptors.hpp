#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::css
{
  class Value;
  class StyleProperties;

  struct CSSCounterStyleDescriptors
  {
    using Name = CSSOMStringAtom;
    using Ranges = SmallList<std::pair<int, int>>;
    using SystemData = std::pair<CSSCounterStyleDescriptors::Name, int>;
    // The keywords that can be used as values for the counter-style `system` descriptor.
    // https://www.w3.org/TR/css-counter-styles-3/#counter-style-system
    struct Symbol
    {
      bool isCustomIdent {false};
      CSSOMString text;
      friend bool operator==(const Symbol &, const Symbol &) = default;
      CSSOMString cssText() const;
    };
    using AdditiveSymbols = SmallList<std::pair<Symbol, unsigned>>;
    enum class System : uint8_t
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
    enum class SpeakAs : uint8_t
    {
      Auto,
      Bullets,
      Numbers,
      Words,
      SpellOut,
      CounterStyleNameReference,
    };
    struct Pad
    {
      unsigned m_padMinimumLength = 0;
      Symbol m_padSymbol;
      friend bool operator==(const Pad &, const Pad &) = default;
      CSSOMString cssText() const;
    };
    struct NegativeSymbols
    {
      Symbol m_prefix = {false, "-"};
      Symbol m_suffix;
      friend bool operator==(const NegativeSymbols &, const NegativeSymbols &) = default;
    };
    enum class ExplicitlySetDescriptors : uint16_t
    {
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

    // create() is prefered here rather than a custom constructor, so that the Struct still classifies as an
    // aggregate.
    static CSSCounterStyleDescriptors create(CSSOMStringAtom name, const StyleProperties &);
    bool operator==(const CSSCounterStyleDescriptors &other) const
    {
      // Intentionally doesn't check m_isExtendedResolved.
      return m_name == other.m_name && m_system == other.m_system
             && m_negativeSymbols == other.m_negativeSymbols && m_prefix == other.m_prefix
             && m_suffix == other.m_suffix && m_ranges == other.m_ranges && m_pad == other.m_pad
             && m_fallbackName == other.m_fallbackName && m_symbols == other.m_symbols
             && m_additiveSymbols == other.m_additiveSymbols && m_speakAs == other.m_speakAs
             && m_extendsName == other.m_extendsName
             && m_fixedSystemFirstSymbolValue == other.m_fixedSystemFirstSymbolValue
             && m_explicitlySetDescriptors == other.m_explicitlySetDescriptors;
    }
    void setExplicitlySetDescriptors(const StyleProperties &);
    bool isValid() const;
    static bool areSymbolsValidForSystem(System, const SmallList<Symbol> &, const AdditiveSymbols &);

    void setName(Name);
    void setSystem(System);
    void setSystemData(SystemData);
    void setNegative(NegativeSymbols);
    void setPrefix(Symbol);
    void setSuffix(Symbol);
    void setRanges(Ranges);
    void setPad(Pad);
    void setFallbackName(Name);
    void setSymbols(SmallList<Symbol>);
    void setAdditiveSymbols(AdditiveSymbols);

    CSSOMString nameCSSText() const;
    CSSOMString systemCSSText() const;
    CSSOMString negativeCSSText() const;
    CSSOMString prefixCSSText() const;
    CSSOMString suffixCSSText() const;
    CSSOMString rangesCSSText() const;
    CSSOMString padCSSText() const;
    CSSOMString fallbackCSSText() const;
    CSSOMString symbolsCSSText() const;
    CSSOMString additiveSymbolsCSSText() const;

    Name m_name;
    System m_system;
    NegativeSymbols m_negativeSymbols;
    Symbol m_prefix;
    Symbol m_suffix;
    Ranges m_ranges;
    Pad m_pad;
    Name m_fallbackName;
    SmallList<Symbol> m_symbols;
    AdditiveSymbols m_additiveSymbols;
    SpeakAs m_speakAs;
    Name m_extendsName;
    int m_fixedSystemFirstSymbolValue;
    OptionSet<ExplicitlySetDescriptors> m_explicitlySetDescriptors;
    bool m_isExtendedResolved {false};
  };

  CSSCounterStyleDescriptors::Ranges rangeFromCSSValue(Ref<Value>);
  CSSCounterStyleDescriptors::AdditiveSymbols additiveSymbolsFromCSSValue(Ref<Value>);
  CSSCounterStyleDescriptors::Pad padFromCSSValue(Ref<Value>);
  CSSCounterStyleDescriptors::NegativeSymbols negativeSymbolsFromCSSValue(Ref<Value>);
  CSSCounterStyleDescriptors::Symbol symbolFromCSSValue(RefPtr<Value>);
  SmallList<CSSCounterStyleDescriptors::Symbol> symbolsFromCSSValue(Ref<Value>);
  CSSCounterStyleDescriptors::Name fallbackNameFromCSSValue(Ref<Value>);
  CSSCounterStyleDescriptors::SystemData extractSystemDataFromCSSValue(RefPtr<Value>,
                                                                       CSSCounterStyleDescriptors::System);
} // namespace krys::boo::css
