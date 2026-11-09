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

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Symbol.hpp"
#include <brigand/algorithms/remove.hpp>
#include <brigand/algorithms/wrap.hpp>
#include <brigand/sequences/append.hpp>
#include <brigand/sequences/list.hpp>

namespace krys::boo::css
{
  class SymbolTable;

  // MARK: Add/Remove Symbol

  /// @brief Transform `brigand::list<css1, css2, ...>`  -> `brigand::list<css1, css2, ..., symbol>`.
  template <typename TypeList>
  struct PlusSymbolLazy
  {
    using type = brigand::append<TypeList, brigand::list<Symbol>>;
  };

  template <typename TypeList>
  using PlusSymbol = typename PlusSymbolLazy<TypeList>::type;

  /// @brief Transform `brigand::list<css1, css2, ..., symbol>`  -> `brigand::list<css1, css2, ...>`.
  template <typename TypeList>
  struct MinusSymbolLazy
  {
    using type = brigand::remove_if<TypeList, IsSymbol<brigand::_1>>;
  };

  template <typename TypeList>
  using MinusSymbol = typename MinusSymbolLazy<TypeList>::type;

  // MARK: - Symbol replacement

  // Replaces the symbol with a value from the symbol table. This is only relevant
  // for Symbol, so a catchall overload that implements the identity function is
  // provided to allow generic replacement.
  KRYS_NODISCARD Number<> ReplaceSymbol(Symbol, const SymbolTable &) noexcept;

  template <typename T>
  KRYS_NODISCARD constexpr T ReplaceSymbol(T value, const SymbolTable &)
  {
    return value;
  }

  template <typename... Ts>
  using TypesMinusSymbol = brigand::wrap<MinusSymbol<brigand::list<Ts...>>, VariantOrSingle>;

  template <typename... Ts>
  KRYS_NODISCARD constexpr TypesMinusSymbol<Ts...> ReplaceSymbol(const Variant<Ts...> &component,
                                                                 const SymbolTable &symbolTable) noexcept
  {
    return krys::SwitchOn(component, [&](auto part) -> TypesMinusSymbol<Ts...>
                          { return ReplaceSymbol(part, symbolTable); });
  }

  template <typename T>
  KRYS_NODISCARD constexpr decltype(auto) ReplaceSymbol(const Maybe<T> &component,
                                                        const SymbolTable &symbolTable) noexcept
  {
    return component ? std::make_optional(ReplaceSymbol(*component, symbolTable)) : null;
  }
}