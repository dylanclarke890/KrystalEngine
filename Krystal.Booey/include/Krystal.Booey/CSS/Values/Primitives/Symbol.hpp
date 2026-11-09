/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct SymbolRaw
  {
    ValueId Value;

    constexpr bool operator==(const SymbolRaw &) const = default;
  };

  struct Symbol
  {
    using Raw = SymbolRaw;

    ValueId Value;

    constexpr Symbol(SymbolRaw &&value) : Value {value.Value}
    {
    }

    constexpr Symbol(const SymbolRaw &value) : Value {value.Value}
    {
    }

    constexpr bool operator==(const Symbol &) const = default;
  };

  template <typename T>
  struct IsSymbol : public IntegralConstant<bool, SameType<T, Symbol>>
  {
  };

  template <>
  struct Serialiser<SymbolRaw>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const SymbolRaw &value) noexcept;
  };

  template <>
  struct Serialiser<Symbol>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const Symbol &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<SymbolRaw>
  {
    constexpr void operator()(ComputedStyleDependencies &, const SymbolRaw &) noexcept
    {
    }
  };
  template <>
  struct ComputedStyleDependenciesCollector<Symbol>
  {
    constexpr void operator()(ComputedStyleDependencies &, const Symbol &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<SymbolRaw>
  {
    KRYS_NODISCARD constexpr IterationStatus operator()(const Func<IterationStatus(css::Value &)> &,
                                                        const SymbolRaw &) noexcept
    {
      return IterationStatus::Continue;
    }
  };

  template <>
  struct ValueChildrenVisitor<Symbol>
  {
    KRYS_NODISCARD constexpr IterationStatus operator()(const Func<IterationStatus(css::Value &)> &,
                                                        const Symbol &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}