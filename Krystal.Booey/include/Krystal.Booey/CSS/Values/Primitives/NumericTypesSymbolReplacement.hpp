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