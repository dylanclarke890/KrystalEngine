#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/Markup.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include <algorithm>
#include <limits>
#include <string>

namespace krys::boo::css
{
  using namespace std::string_view_literals;

  namespace
  {
    KRYS_NODISCARD CSSOMString NameText(const CounterStyleName &name) noexcept
    {
      return name == CounterStyleName::Null() ? CSSOMString() : CSSOMString(name.View());
    }

    KRYS_NODISCARD CounterStyleRanges RangeFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto ranges = properties.GetPropertyCSSValue(PropertyId::Range);
      if (!ranges)
      {
        return {};
      }
      return RangeFromValue(krys::move(ranges));
    }

    KRYS_NODISCARD CounterStyleSymbol SymbolFromValue(const Value *value) noexcept
    {
      auto *primitiveValue = DynamicDowncast<PrimitiveValue>(value);
      if (!primitiveValue)
      {
        return {};
      }

      return {primitiveValue->IsCustomIdent(), primitiveValue->StringValue()};
    }

    KRYS_NODISCARD CounterStyleName NameFromValue(Ref<Value> value) noexcept
    {
      auto *primitiveValue = DynamicDowncast<PrimitiveValue>(*value);
      if (!primitiveValue)
      {
        return CounterStyleName::Null();
      }

      return CounterStyleName(primitiveValue->StringValue());
    }

    KRYS_NODISCARD CounterStyleAdditiveSymbols
      AdditiveSymbolsFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto value = properties.GetPropertyCSSValue(PropertyId::AdditiveSymbols);
      if (!value)
      {
        return {};
      }
      return AdditiveSymbolsFromValue(krys::move(value));
    }

    KRYS_NODISCARD CounterStylePad PadFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto value = properties.GetPropertyCSSValue(PropertyId::Pad);
      if (!value)
      {
        return {};
      }
      return PadFromValue(krys::move(value));
    }

    KRYS_NODISCARD CounterStyleNegativeSymbols
      NegativeSymbolsFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto negative = properties.GetPropertyCSSValue(PropertyId::Negative);
      if (!negative)
      {
        return {};
      }
      return NegativeSymbolsFromValue(krys::move(negative));
    }

    KRYS_NODISCARD List<CounterStyleSymbol>
      SymbolsFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto symbolsValues = properties.GetPropertyCSSValue(PropertyId::Symbols);
      if (!symbolsValues)
      {
        return {};
      }
      return SymbolsFromValue(krys::move(symbolsValues));
    }

    KRYS_NODISCARD CounterStyleName
      FallbackNameFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto fallback = properties.GetPropertyCSSValue(PropertyId::Fallback);
      if (!fallback)
      {
        return CounterStyleName(u8"decimal"sv);
      }
      return FallbackNameFromValue(krys::move(fallback));
    }

    KRYS_NODISCARD CounterStyleSymbol PrefixFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto prefix = properties.GetPropertyCSSValue(PropertyId::Prefix);
      if (!prefix)
      {
        return {};
      }
      return SymbolFromValue(prefix.get());
    }

    KRYS_NODISCARD CounterStyleSymbol SuffixFromStyleProperties(const StyleProperties &properties) noexcept
    {
      auto suffix = properties.GetPropertyCSSValue(PropertyId::Suffix);
      // https://www.w3.org/TR/css-counter-styles-3/#counter-style-suffix
      // ("." full stop followed by a space)
      if (!suffix)
      {
        return {false, u8". "};
      }
      return SymbolFromValue(suffix.get());
    }

    KRYS_NODISCARD CounterStyleSystemData ExtractSystemDataFromStyleProperties(
      const StyleProperties &properties, CounterStyleSystem system) noexcept
    {
      auto systemValue = properties.GetPropertyCSSValue(PropertyId::System);
      // If no value is provided after `fixed`, the first synbol value is implicitly 1
      // (https://www.w3.org/TR/css-counter-styles-3/#first-symbol-value).
      if (!systemValue)
      {
        return {CounterStyleName(u8"decimal"sv), 1};
      }

      return ExtractSystemDataFromValue(krys::move(systemValue), system);
    }
  }

  CounterStyleSystem ToCounterStyleSystemEnum(const Value *system) noexcept
  {
    if (!system)
    {
      return CounterStyleSystem::Symbolic;
    }

    krys_debug_assert(system->IsValueId() || system->IsValuePairValue());
    auto systemKeyword = ValueId::Invalid;
    if (system->IsValueId())
    {
      systemKeyword = system->ValueId();
    }
    else if (system->IsValuePairValue())
    {
      // This value must be `fixed` or `extends`, both of which can or must have an additional component.
      systemKeyword = system->First().ValueId();
    }
    switch (systemKeyword)
    {
      case ValueId::Cyclic:
      {
        return CounterStyleSystem::Cyclic;
      }
      case ValueId::Fixed:
      {
        return CounterStyleSystem::Fixed;
      }
      case ValueId::Symbolic:
      {
        return CounterStyleSystem::Symbolic;
      }
      case ValueId::Alphabetic:
      {
        return CounterStyleSystem::Alphabetic;
      }
      case ValueId::Numeric:
      {
        return CounterStyleSystem::Numeric;
      }
      case ValueId::Additive:
      {
        return CounterStyleSystem::Additive;
      }
      case ValueId::InternalDisclosureClosed:
      {
        return CounterStyleSystem::DisclosureClosed;
      }
      case ValueId::InternalDisclosureOpen:
      {
        return CounterStyleSystem::DisclosureOpen;
      }
      case ValueId::InternalSimplifiedChineseInformal:
      {
        return CounterStyleSystem::SimplifiedChineseInformal;
      }
      case ValueId::InternalSimplifiedChineseFormal:
      {
        return CounterStyleSystem::SimplifiedChineseFormal;
      }
      case ValueId::InternalTraditionalChineseInformal:
      {
        return CounterStyleSystem::TraditionalChineseInformal;
      }
      case ValueId::InternalTraditionalChineseFormal:
      {
        return CounterStyleSystem::TraditionalChineseFormal;
      }
      case ValueId::InternalEthiopicNumeric:
      {
        return CounterStyleSystem::EthiopicNumeric;
      }
      case ValueId::Extends:
      {
        return CounterStyleSystem::Extends;
      }
      default:
      {
        krys_debug_assert_msg(false, "Not a counter style system.");
        return CounterStyleSystem::Symbolic;
      }
    }
  }

  CounterStyleRanges RangeFromValue(Ref<Value> value) noexcept
  {
    auto *list = DynamicDowncast<ValueList>(*value);
    if (!list)
    {
      return {};
    }
    CounterStyleRanges result;
    for (const auto &rangeValue : *list)
    {
      if (!rangeValue.IsValuePairValue())
      {
        return {};
      }
      auto &low = Downcast<PrimitiveValue>(rangeValue.First());
      auto &high = Downcast<PrimitiveValue>(rangeValue.Second());
      int32 convertedLow {std::numeric_limits<int32>::min()};
      int32 convertedHigh {std::numeric_limits<int32>::max()};
      if (low.IsInteger())
      {
        convertedLow = low.ResolveAsIntegerDeprecated<int32>();
      }
      if (high.IsInteger())
      {
        convertedHigh = high.ResolveAsIntegerDeprecated<int32>();
      }
      result.push_back({convertedLow, convertedHigh});
    }
    return result;
  }

  CounterStyleSymbol SymbolFromValue(RefPtr<Value> value) noexcept
  {
    return SymbolFromValue(value.get());
  }

  CounterStyleAdditiveSymbols AdditiveSymbolsFromValue(Ref<Value> value) noexcept
  {
    CounterStyleAdditiveSymbols result;
    for (const auto &additiveSymbol : Downcast<ValueList>(*value))
    {
      auto &pair = Downcast<ValuePair>(additiveSymbol);
      auto weight = Downcast<PrimitiveValue>(pair.First()).ResolveAsIntegerDeprecated<uint32>();
      auto symbol = SymbolFromValue(&pair.Second());
      result.emplace_back(symbol, weight);
    }
    return result;
  }

  CounterStylePad PadFromValue(Ref<Value> value) noexcept
  {
    auto &list = Downcast<ValueList>(*value);
    krys_debug_assert(list.size() == 2);
    auto length = Downcast<PrimitiveValue>(list[0]).ResolveAsIntegerDeprecated<int32>();
    krys_debug_assert(length >= 0);
    return {static_cast<uint32>(std::max(0, length)), SymbolFromValue(&list[1])};
  }

  CounterStyleNegativeSymbols NegativeSymbolsFromValue(Ref<Value> value) noexcept
  {
    CounterStyleNegativeSymbols result;
    if (auto *list = DynamicDowncast<ValueList>(*value))
    {
      krys_debug_assert(list->size() == 2);
      result.Prefix = SymbolFromValue(list->Item(0));
      result.Suffix = SymbolFromValue(list->Item(1));
    }
    else
    {
      result.Prefix = SymbolFromValue(value.get());
    }
    return result;
  }

  List<CounterStyleSymbol> SymbolsFromValue(Ref<Value> value) noexcept
  {
    List<CounterStyleSymbol> result;
    for (const auto &symbolValue : Downcast<ValueList>(*value))
    {
      // Only primitive values give a (non-null) symbol.
      if (Is<PrimitiveValue>(symbolValue))
      {
        result.push_back(SymbolFromValue(&symbolValue));
      }
    }
    return result;
  }

  CounterStyleName FallbackNameFromValue(Ref<Value> value) noexcept
  {
    return NameFromValue(krys::move(value));
  }

  CounterStyleSystemData ExtractSystemDataFromValue(RefPtr<Value> systemValue,
                                                    CounterStyleSystem system) noexcept
  {
    CounterStyleSystemData result {CounterStyleName(u8"decimal"sv), 1};
    if (!systemValue)
    {
      return result;
    }
    krys_debug_assert(systemValue->IsValueId() || systemValue->IsValuePairValue());
    if (systemValue->IsValuePairValue())
    {
      // This value must be `fixed` or `extends`, both of which can or must have an additional component.
      auto &secondValue = systemValue->Second();
      if (system == CounterStyleSystem::Extends)
      {
        krys_debug_assert(secondValue.IsCustomIdent());
        result.first = secondValue.IsCustomIdent() ? CounterStyleName(secondValue.CustomIdent())
                                                   : CounterStyleName(u8"decimal"sv);
      }
      else if (system == CounterStyleSystem::Fixed)
      {
        krys_debug_assert(secondValue.IsInteger());
        result.second = secondValue.IsInteger() ? static_cast<int32>(secondValue.IntegerDeprecated()) : 1;
      }
    }
    return result;
  }

  void CounterStyleDescriptors::SetExplicitlySetDescriptors(const StyleProperties &properties) noexcept
  {
    auto set = [&](PropertyId id, CounterStyleDescriptor descriptor)
    {
      if (properties.GetPropertyCSSValue(id))
      {
        ExplicitlySetDescriptors |= descriptor;
      }
    };

    set(PropertyId::System, CounterStyleDescriptor::System);
    set(PropertyId::Negative, CounterStyleDescriptor::Negative);
    set(PropertyId::Prefix, CounterStyleDescriptor::Prefix);
    set(PropertyId::Suffix, CounterStyleDescriptor::Suffix);
    set(PropertyId::Range, CounterStyleDescriptor::Range);
    set(PropertyId::Pad, CounterStyleDescriptor::Pad);
    set(PropertyId::Fallback, CounterStyleDescriptor::Fallback);
    set(PropertyId::AdditiveSymbols, CounterStyleDescriptor::AdditiveSymbols);
    set(PropertyId::Symbols, CounterStyleDescriptor::Symbols);
    set(PropertyId::SpeakAs, CounterStyleDescriptor::SpeakAs);
  }

  CounterStyleDescriptors CounterStyleDescriptors::Create(CounterStyleName name,
                                                          const StyleProperties &properties) noexcept
  {
    auto systemValue = properties.GetPropertyCSSValue(PropertyId::System);
    auto system = ToCounterStyleSystemEnum(systemValue.get());
    auto systemData = ExtractSystemDataFromStyleProperties(properties, system);
    CounterStyleDescriptors descriptors {
      .Name = name,
      .System = system,
      .NegativeSymbols = NegativeSymbolsFromStyleProperties(properties),
      .Prefix = PrefixFromStyleProperties(properties),
      .Suffix = SuffixFromStyleProperties(properties),
      .Ranges = RangeFromStyleProperties(properties),
      .Pad = PadFromStyleProperties(properties),
      .FallbackName = FallbackNameFromStyleProperties(properties),
      .Symbols = SymbolsFromStyleProperties(properties),
      .AdditiveSymbols = AdditiveSymbolsFromStyleProperties(properties),
      .SpeakAs = CounterStyleSpeakAs::Auto,
      .ExtendsName = systemData.first,
      .FixedSystemFirstSymbolValue = systemData.second,
      .ExplicitlySetDescriptors = CounterStyleDescriptor::None,
    };
    descriptors.SetExplicitlySetDescriptors(properties);
    return descriptors;
  }

  bool CounterStyleDescriptors::AreSymbolsValidForSystem(
    CounterStyleSystem system, const List<CounterStyleSymbol> &symbols,
    const CounterStyleAdditiveSymbols &additiveSymbols) noexcept
  {
    switch (system)
    {
      case CounterStyleSystem::Cyclic:
      case CounterStyleSystem::Fixed:
      case CounterStyleSystem::Symbolic:
      {
        return !symbols.empty();
      }
      case CounterStyleSystem::Alphabetic:
      case CounterStyleSystem::Numeric:
      {
        return symbols.size() >= 2u;
      }
      case CounterStyleSystem::Additive:
      {
        return !additiveSymbols.empty();
      }
      case CounterStyleSystem::SimplifiedChineseInformal:
      case CounterStyleSystem::SimplifiedChineseFormal:
      case CounterStyleSystem::TraditionalChineseInformal:
      case CounterStyleSystem::TraditionalChineseFormal:
      case CounterStyleSystem::EthiopicNumeric:
      case CounterStyleSystem::Extends:
      {
        return symbols.empty() && additiveSymbols.empty();
      }
      case CounterStyleSystem::DisclosureClosed:
      case CounterStyleSystem::DisclosureOpen:
      {
        return true;
      }
    }
    krys_unreachable();
  }

  bool CounterStyleDescriptors::IsValid() const noexcept
  {
    return AreSymbolsValidForSystem(System, Symbols, AdditiveSymbols);
  }

  void CounterStyleDescriptors::SetName(CounterStyleName name) noexcept
  {
    if (name == CounterStyleName::Null() || Name == name)
    {
      return;
    }
    Name = krys::move(name);
  }

  void CounterStyleDescriptors::SetSystemData(CounterStyleSystemData systemData) noexcept
  {
    if (ExtendsName == systemData.first && FixedSystemFirstSymbolValue == systemData.second)
    {
      return;
    }
    ExtendsName = systemData.first;
    FixedSystemFirstSymbolValue = systemData.second;
  }

  void CounterStyleDescriptors::SetNegative(CounterStyleNegativeSymbols negative) noexcept
  {
    if (NegativeSymbols == negative)
    {
      return;
    }
    NegativeSymbols = krys::move(negative);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Negative;
  }

  void CounterStyleDescriptors::SetPrefix(CounterStyleSymbol prefix) noexcept
  {
    if (Prefix == prefix)
    {
      return;
    }
    Prefix = krys::move(prefix);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Prefix;
  }

  void CounterStyleDescriptors::SetSuffix(CounterStyleSymbol suffix) noexcept
  {
    if (Suffix == suffix)
    {
      return;
    }
    Suffix = krys::move(suffix);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Suffix;
  }

  void CounterStyleDescriptors::SetRanges(CounterStyleRanges ranges) noexcept
  {
    if (Ranges == ranges)
    {
      return;
    }
    Ranges = krys::move(ranges);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Range;
  }

  void CounterStyleDescriptors::SetPad(CounterStylePad pad) noexcept
  {
    if (Pad == pad)
    {
      return;
    }
    Pad = krys::move(pad);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Pad;
  }

  void CounterStyleDescriptors::SetFallbackName(CounterStyleName name) noexcept
  {
    if (FallbackName == name)
    {
      return;
    }
    FallbackName = krys::move(name);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Fallback;
  }

  void CounterStyleDescriptors::SetSymbols(List<CounterStyleSymbol> symbols) noexcept
  {
    if (Symbols == symbols || !AreSymbolsValidForSystem(System, symbols, AdditiveSymbols))
    {
      return;
    }
    Symbols = krys::move(symbols);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::Symbols;
  }

  void CounterStyleDescriptors::SetAdditiveSymbols(CounterStyleAdditiveSymbols additiveSymbols) noexcept
  {
    if (AdditiveSymbols == additiveSymbols || !AreSymbolsValidForSystem(System, Symbols, additiveSymbols))
    {
      return;
    }
    AdditiveSymbols = krys::move(additiveSymbols);
    ExplicitlySetDescriptors |= CounterStyleDescriptor::AdditiveSymbols;
  }

  CSSOMString CounterStyleSymbol::CssText() const noexcept
  {
    StringBuilder builder;
    if (IsCustomIdent)
    {
      SerialiseIdentifier(Text, builder);
    }
    else
    {
      SerialiseString(Text, builder);
    }
    return krys::move(builder).ToString();
  }

  CSSOMString CounterStyleDescriptors::NameCSSText() const noexcept
  {
    return NameText(Name);
  }

  CSSOMString CounterStyleDescriptors::SystemCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::System))
    {
      return {};
    }
    if (IsExtendedResolved)
    {
      return CSSOMString(u8"extends ") + NameText(ExtendsName);
    }

    switch (System)
    {
      case CounterStyleSystem::Cyclic:
      {
        return u8"cyclic";
      }
      case CounterStyleSystem::Numeric:
      {
        return u8"numeric";
      }
      case CounterStyleSystem::Alphabetic:
      {
        return u8"alphabetic";
      }
      case CounterStyleSystem::Symbolic:
      {
        return u8"symbolic";
      }
      case CounterStyleSystem::Additive:
      {
        return u8"additive";
      }
      case CounterStyleSystem::Fixed:
      {
        StringBuilder result;
        result.Append(u8"fixed ");
        result.Append(FixedSystemFirstSymbolValue);
        return krys::move(result).ToString();
      }
      case CounterStyleSystem::Extends:
      {
        return CSSOMString(u8"extends ") + NameText(ExtendsName);
      }
      // Internal values should not be exposed.
      case CounterStyleSystem::SimplifiedChineseInformal:
      case CounterStyleSystem::SimplifiedChineseFormal:
      case CounterStyleSystem::TraditionalChineseInformal:
      case CounterStyleSystem::TraditionalChineseFormal:
      case CounterStyleSystem::EthiopicNumeric:
      case CounterStyleSystem::DisclosureClosed:
      case CounterStyleSystem::DisclosureOpen:
      {
        return {};
      }
    }
    return {};
  }

  CSSOMString CounterStyleDescriptors::NegativeCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Negative))
    {
      return {};
    }
    return NegativeSymbols.Prefix.CssText() + u8" " + NegativeSymbols.Suffix.CssText();
  }

  CSSOMString CounterStyleDescriptors::PrefixCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Prefix))
    {
      return {};
    }
    return Prefix.CssText();
  }

  CSSOMString CounterStyleDescriptors::SuffixCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Suffix))
    {
      return {};
    }
    return Suffix.CssText();
  }

  CSSOMString CounterStyleDescriptors::RangesCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Range))
    {
      return {};
    }
    if (Ranges.empty())
    {
      return u8"auto";
    }

    StringBuilder builder;
    for (size_t i = 0uz; i < Ranges.size(); ++i)
    {
      if (i)
      {
        builder.Append(u8", "sv);
      }
      const auto &range = Ranges[i];
      if (range.first == std::numeric_limits<int32>::min())
      {
        builder.Append(u8"infinite"sv);
      }
      else
      {
        builder.Append(range.first);
      }
      builder.Append(u8' ');
      if (range.second == std::numeric_limits<int32>::max())
      {
        builder.Append(u8"infinite"sv);
      }
      else
      {
        builder.Append(range.second);
      }
    }
    return krys::move(builder).ToString();
  }

  CSSOMString CounterStylePad::CssText() const noexcept
  {
    StringBuilder result;
    result.Append(PadMinimumLength);
    result.Append(u8' ');
    result.Append(PadSymbol.CssText());
    return krys::move(result).ToString();
  }

  CSSOMString CounterStyleDescriptors::PadCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Pad))
    {
      return {};
    }
    return Pad.CssText();
  }

  CSSOMString CounterStyleDescriptors::FallbackCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Fallback))
    {
      return {};
    }
    return NameText(FallbackName);
  }

  CSSOMString CounterStyleDescriptors::SymbolsCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::Symbols))
    {
      return {};
    }
    StringBuilder builder;
    for (size_t i = 0uz; i < Symbols.size(); ++i)
    {
      if (i)
      {
        builder.Append(u8' ');
      }
      builder.Append(Symbols[i].CssText());
    }
    return krys::move(builder).ToString();
  }

  CSSOMString CounterStyleDescriptors::AdditiveSymbolsCSSText() const noexcept
  {
    if (!HasFlag(ExplicitlySetDescriptors, CounterStyleDescriptor::AdditiveSymbols))
    {
      return {};
    }
    StringBuilder builder;
    for (size_t i = 0uz; i < AdditiveSymbols.size(); ++i)
    {
      if (i)
      {
        builder.Append(u8", "sv);
      }
      builder.Append(AdditiveSymbols[i].second);
      builder.Append(u8' ');
      builder.Append(AdditiveSymbols[i].first.CssText());
    }
    return krys::move(builder).ToString();
  }
}
