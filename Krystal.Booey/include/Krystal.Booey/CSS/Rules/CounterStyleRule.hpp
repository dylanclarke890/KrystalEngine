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

#pragma once

#include "Krystal.Booey/CSS/Counter/CounterStyleDescriptors.hpp"
#include "Krystal.Booey/CSS/Rules/Rule.hpp"

namespace krys::boo::css
{
  /// @brief `@counter-style`.
  class CounterStyleRule final : public Rule
  {
  private:
    CSSOMStringAtom _name;
    CounterStyleDescriptors _descriptors;

    CounterStyleRule(const CSSOMStringAtom &name, CounterStyleDescriptors &&descriptors) noexcept;
    CounterStyleRule(const CounterStyleRule &other) noexcept = default;

  public:
    ~CounterStyleRule() noexcept;

    KRYS_NODISCARD static Ref<CounterStyleRule> Create(const CSSOMStringAtom &name,
                                                       CounterStyleDescriptors &&descriptors) noexcept;

    KRYS_NODISCARD Ref<CounterStyleRule> Copy() const noexcept;

    KRYS_NODISCARD const CounterStyleDescriptors &Descriptors() const noexcept
    {
      return _descriptors;
    }

    KRYS_NODISCARD CounterStyleDescriptors &MutableDescriptors() noexcept
    {
      return _descriptors;
    }

    KRYS_NODISCARD const CSSOMStringAtom &Name() const noexcept
    {
      return _name;
    }

    KRYS_NODISCARD CSSOMString System() const noexcept
    {
      return _descriptors.SystemCSSText();
    }

    KRYS_NODISCARD CSSOMString Negative() const noexcept
    {
      return _descriptors.NegativeCSSText();
    }

    KRYS_NODISCARD CSSOMString Prefix() const noexcept
    {
      return _descriptors.PrefixCSSText();
    }

    KRYS_NODISCARD CSSOMString Suffix() const noexcept
    {
      return _descriptors.SuffixCSSText();
    }

    KRYS_NODISCARD CSSOMString Range() const noexcept
    {
      return _descriptors.RangesCSSText();
    }

    KRYS_NODISCARD CSSOMString Pad() const noexcept
    {
      return _descriptors.PadCSSText();
    }

    KRYS_NODISCARD CSSOMString Fallback() const noexcept
    {
      return _descriptors.FallbackCSSText();
    }

    KRYS_NODISCARD CSSOMString Symbols() const noexcept
    {
      return _descriptors.SymbolsCSSText();
    }

    KRYS_NODISCARD CSSOMString AdditiveSymbols() const noexcept
    {
      return _descriptors.AdditiveSymbolsCSSText();
    }

    KRYS_NODISCARD CSSOMString SpeakAs() const noexcept
    {
      return {};
    }

    void SetName(const CSSOMStringAtom &name) noexcept
    {
      _name = name;
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::css::CounterStyleRule)
  KRYS_NODISCARD static bool IsType(const krys::boo::css::Rule &rule) noexcept
  {
    return rule.IsCounterStyleRule();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
