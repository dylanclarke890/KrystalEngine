/*
 * Copyright (C) 2021 Tyler Wilcock <twilco.o@protonmail.com>.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Css/CSSOM/CSSCounterStyleRule.hpp"
#include "Krystal.Booey/Css/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/Css/CSSOM/CSSStyleSheet.hpp"
#include "Krystal.Booey/Css/Parser/InputStream.hpp"
#include "Krystal.Booey/Css/Parser/Tokenizer.hpp"
#include "Krystal.Booey/Css/Properties/Consumers/CounterStyles.hpp"
#include "Krystal.Booey/Css/Properties/PropertyParser.hpp"
#include "Krystal.Booey/Css/Rules/CounterStyleRule.hpp"
#include "Krystal.Booey/Css/Values/Value.hpp"

namespace krys::boo
{
  CSSCounterStyleRule::CSSCounterStyleRule(css::CounterStyleRule &counterStyleRule,
                                           CSSStyleSheet *parent) noexcept
      : CSSRule(parent), _counterStyleRule(ShareRef(counterStyleRule))
  {
  }

  CSSCounterStyleRule::~CSSCounterStyleRule() noexcept = default;

  Ref<CSSCounterStyleRule> CSSCounterStyleRule::Create(css::CounterStyleRule &rule,
                                                       CSSStyleSheet *sheet) noexcept
  {
    return AdoptRef(*new CSSCounterStyleRule(rule, sheet));
  }

  const css::CounterStyleDescriptors &CSSCounterStyleRule::Descriptors() const noexcept
  {
    return _counterStyleRule->Descriptors();
  }

  css::CounterStyleDescriptors &CSSCounterStyleRule::MutableDescriptors() noexcept
  {
    return _counterStyleRule->MutableDescriptors();
  }

  CSSOMString CSSCounterStyleRule::Name() const noexcept
  {
    return _counterStyleRule->Name().ToString();
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

  void CSSCounterStyleRule::Reattach(css::Rule &rule) noexcept
  {
    _counterStyleRule = ShareRef(Downcast<css::CounterStyleRule>(rule));
  }

  RefPtr<css::Value> CSSCounterStyleRule::CSSValueFromText(css::PropertyId property,
                                                           const CSSOMString &text) const noexcept
  {
    return css::PropertyParser::ParseCounterStyleDescriptor(property, text, GetParserContext());
  }

  // https://drafts.csswg.org/css-counter-styles-3/#dom-csscounterstylerule-name
  void CSSCounterStyleRule::SetName(const CSSOMString &text) noexcept
  {
    css::InputStream inputStream {CSSOMString(text)};
    css::Tokenizer tokenizer(inputStream);
    std::ignore = tokenizer.PumpTokenizer();
    auto tokenRange = tokenizer.Tokens();
    auto name = css::parse_property::ConsumeCounterStyleNameInPrelude(tokenRange);
    if (name == null)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    _counterStyleRule->SetName(name);
    MutableDescriptors().SetName(krys::move(name));
  }

  void CSSCounterStyleRule::SetSystem(const CSSOMString &text) noexcept
  {
    auto systemValue = CSSValueFromText(css::PropertyId::System, text);
    if (!systemValue)
    {
      return;
    }

    auto system = css::ToCounterStyleSystemEnum(systemValue.get());
    // If the attribute being set is `system`, and the new value would change the algorithm used, do nothing
    // and abort these steps.
    // (It's okay to change an aspect of the algorithm, like the first symbol value of a `fixed` system.)
    // https://www.w3.org/TR/css-counter-styles-3/#the-csscounterstylerule-interface
    if (system != Descriptors().System)
    {
      return;
    }

    auto systemData = css::ExtractSystemDataFromValue(krys::move(systemValue), system);
    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetSystemData(krys::move(systemData));
  }

  void CSSCounterStyleRule::SetNegative(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Negative, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetNegative(css::NegativeSymbolsFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetPrefix(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Prefix, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetPrefix(SymbolFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetSuffix(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Suffix, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetSuffix(SymbolFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetRange(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Range, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetRanges(css::RangeFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetPad(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Pad, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetPad(css::PadFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetFallback(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Fallback, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetFallbackName(css::FallbackNameFromValue(newValue));
  }

  void CSSCounterStyleRule::SetSymbols(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::Symbols, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetSymbols(css::SymbolsFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetAdditiveSymbols(const CSSOMString &text) noexcept
  {
    auto newValue = CSSValueFromText(css::PropertyId::AdditiveSymbols, text);
    if (!newValue)
    {
      return;
    }

    CSSStyleSheet::RuleMutationScope mutationScope(this);
    MutableDescriptors().SetAdditiveSymbols(css::AdditiveSymbolsFromValue(krys::move(newValue)));
  }

  void CSSCounterStyleRule::SetSpeakAs(const CSSOMString &) noexcept
  {
    // FIXME: @counter-style speak-as not supported (rdar://103019111).
  }
}
