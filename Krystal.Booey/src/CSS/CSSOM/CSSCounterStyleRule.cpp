#include "Krystal.Booey/CSS/CSSOM/CSSCounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/CSSStyleSheet.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/CSS/Values/Value.hpp"

namespace krys::boo::css
{
  CSSCounterStyleRule::CSSCounterStyleRule(CounterStyleRule &counterStyleRule, CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _counterStyleRule(ShareRef(counterStyleRule))
  {
  }

  CSSCounterStyleRule::~CSSCounterStyleRule() noexcept = default;

  Ref<CSSCounterStyleRule> CSSCounterStyleRule::Create(CounterStyleRule &rule, CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSCounterStyleRule(rule, sheet));
  }

  const CounterStyleDescriptors &CSSCounterStyleRule::Descriptors() const noexcept
  {
    return _counterStyleRule->Descriptors();
  }

  CounterStyleDescriptors &CSSCounterStyleRule::MutableDescriptors() noexcept
  {
    return _counterStyleRule->MutableDescriptors();
  }

  CSSOMString CSSCounterStyleRule::Name() const noexcept
  {
    return CSSOMString(_counterStyleRule->Name().View());
  }

  CSSOMString CSSCounterStyleRule::System() const noexcept
  {
    return _counterStyleRule->System();
  }

  CSSOMString CSSCounterStyleRule::Negative() const noexcept
  {
    return _counterStyleRule->Negative();
  }

  CSSOMString CSSCounterStyleRule::Prefix() const noexcept
  {
    return _counterStyleRule->Prefix();
  }

  CSSOMString CSSCounterStyleRule::Suffix() const noexcept
  {
    return _counterStyleRule->Suffix();
  }

  CSSOMString CSSCounterStyleRule::Range() const noexcept
  {
    return _counterStyleRule->Range();
  }

  CSSOMString CSSCounterStyleRule::Pad() const noexcept
  {
    return _counterStyleRule->Pad();
  }

  CSSOMString CSSCounterStyleRule::Fallback() const noexcept
  {
    return _counterStyleRule->Fallback();
  }

  CSSOMString CSSCounterStyleRule::Symbols() const noexcept
  {
    return _counterStyleRule->Symbols();
  }

  CSSOMString CSSCounterStyleRule::AdditiveSymbols() const noexcept
  {
    return _counterStyleRule->AdditiveSymbols();
  }

  CSSOMString CSSCounterStyleRule::SpeakAs() const noexcept
  {
    return _counterStyleRule->SpeakAs();
  }

  CSSOMString CSSCounterStyleRule::CssText() const noexcept
  {
    CSSOMString result;
    result.append(u8"@counter-style ").append(Name()).append(u8" {");

    auto appendDescriptor = [&](CSSOMStringView name, const CSSOMString &text)
    {
      if (!text.empty())
      {
        result.append(u8" ").append(name).append(u8": ").append(text).push_back(u8';');
      }
    };

    appendDescriptor(u8"system", System());
    appendDescriptor(u8"symbols", Symbols());
    appendDescriptor(u8"additive-symbols", AdditiveSymbols());
    appendDescriptor(u8"negative", Negative());
    appendDescriptor(u8"prefix", Prefix());
    appendDescriptor(u8"suffix", Suffix());
    appendDescriptor(u8"pad", Pad());
    appendDescriptor(u8"range", Range());
    appendDescriptor(u8"fallback", Fallback());
    appendDescriptor(u8"speak-as", SpeakAs());

    result.append(u8" }");
    return result;
  }

  void CSSCounterStyleRule::Reattach(Rule &rule) noexcept
  {
    _counterStyleRule = ShareRef(Downcast<CounterStyleRule>(rule));
  }

  RefPtr<Value> CSSCounterStyleRule::CSSValueFromText(PropertyId property,
                                                      const CSSOMString &text) const noexcept
  {
    return PropertyParser::ParseCounterStyleDescriptor(property, text, GetParserContext());
  }

  // https://drafts.csswg.org/css-counter-styles-3/#dom-csscounterstylerule-name
  void CSSCounterStyleRule::SetName(const CSSOMString &text) noexcept
  {
    InputStream inputStream {CSSOMString(text)};
    Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto tokenRange = tokenizer.Tokens();
    auto name = PropertyParserHelpers::ConsumeCounterStyleNameInPrelude(tokenRange);
    if (name == CSSOMStringAtom::Null())
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    _counterStyleRule->SetName(name);
    MutableDescriptors().SetName(krys::move(name));
  }

  void CSSCounterStyleRule::SetSystem(const CSSOMString &text) noexcept
  {
    auto systemValue = CSSValueFromText(PropertyId::System, text);
    if (!systemValue)
    {
      return;
    }

    auto system = ToCounterStyleSystemEnum(systemValue.get());
    // If the attribute being set is `system`, and the new value would change the algorithm used, do nothing
    // and abort these steps.
    // (It's okay to change an aspect of the algorithm, like the first symbol value of a `fixed` system.)
    // https://www.w3.org/TR/css-counter-styles-3/#the-csscounterstylerule-interface
    if (system != Descriptors().System)
    {
      return;
    }

    auto systemData = ExtractSystemDataFromValue(krys::move(systemValue), system);
    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetSystemData(krys::move(systemData));
  }

  void CSSCounterStyleRule::SetNegative(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Negative, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetNegative(NegativeSymbolsFromValue(Ref<Value>(krys::move(newValue))));
  }

  void CSSCounterStyleRule::SetPrefix(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Prefix, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetPrefix(SymbolFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetSuffix(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Suffix, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetSuffix(SymbolFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetRange(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Range, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetRanges(RangeFromValue(Ref<Value>(krys::move(newValue))));
  }

  void CSSCounterStyleRule::SetPad(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Pad, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetPad(PadFromValue(Ref<Value>(krys::move(newValue))));
  }

  void CSSCounterStyleRule::SetFallback(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Fallback, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetFallbackName(FallbackNameFromValue(Ref<Value>(krys::move(newValue))));
  }

  void CSSCounterStyleRule::SetSymbols(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::Symbols, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetSymbols(SymbolsFromValue(Ref<Value>(krys::move(newValue))));
  }

  void CSSCounterStyleRule::SetAdditiveSymbols(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(PropertyId::AdditiveSymbols, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetAdditiveSymbols(AdditiveSymbolsFromValue(Ref<Value>(krys::move(newValue))));
  }

  void CSSCounterStyleRule::SetSpeakAs(const CSSOMString &) noexcept
  {
    // FIXME: @counter-style speak-as not supported (rdar://103019111).
  }
}
