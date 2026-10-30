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