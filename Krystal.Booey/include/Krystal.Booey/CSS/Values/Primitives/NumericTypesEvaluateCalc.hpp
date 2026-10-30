#pragma once

#include "Krystal.Booey/CSS/Values/NoConversionDataRequiredTag.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/UnevaluatedCalc.hpp"

namespace krys::boo::css
{
  class SymbolTable;

  // MARK: - Requires Conversion Data

  KRYS_NODISCARD inline bool RequiresConversionData(Numeric auto const &primitive) noexcept
  {
    return krys::SwitchOn(primitive, [&](const auto &value) { return RequiresConversionData(value); });
  }

  // MARK: - Evaluation

  // FIXME: Remove "EvaluateCalc" family of functions once color code has moved to the "ToStyle" family of
  // functions.

  template <Calc T>
  KRYS_NODISCARD typename T::Raw EvaluateCalc(const T &calc, NoConversionDataRequiredTag tag,
                                              const SymbolTable &symbolTable) noexcept
  {
    // Evaluation produces a value in the canonical unit.
    return {T::UnitTraits::Canonical, calc.Evaluate(T::Category, tag, symbolTable)};
  }

  template <typename T>
  KRYS_NODISCARD constexpr T EvaluateCalc(const T &component, NoConversionDataRequiredTag,
                                          const SymbolTable &) noexcept
  {
    return component;
  }

  template <typename... Ts>
  KRYS_NODISCARD Variant<Ts...>
    EvaluateCalcIfNoConversionDataRequired(const Variant<Ts...> &component,
                                           const SymbolTable &symbolTable) noexcept
  {
    return krys::SwitchOn(component,
                          [&](const auto &alternative) -> Variant<Ts...>
                          {
                            if (RequiresConversionData(alternative))
                            {
                              return alternative;
                            }

                            return EvaluateCalc(alternative, NoConversionDataRequiredTag {}, symbolTable);
                          });
  }

  template <Numeric T>
  KRYS_NODISCARD T EvaluateCalcIfNoConversionDataRequired(const T &component,
                                                          const SymbolTable &symbolTable) noexcept
  {
    return krys::SwitchOn(component,
                          [&](const auto &alternative) -> T
                          {
                            if (RequiresConversionData(alternative))
                            {
                              return {alternative};
                            }

                            return {EvaluateCalc(alternative, NoConversionDataRequiredTag {}, symbolTable)};
                          });
  }

  template <typename T>
  decltype(auto) EvaluateCalcIfNoConversionDataRequired(const Maybe<T> &component,
                                                        const SymbolTable &symbolTable) noexcept
  {
    return component ? std::make_optional(EvaluateCalcIfNoConversionDataRequired(*component, symbolTable))
                     : null;
  }

  KRYS_NODISCARD inline bool IsUnevaluatedCalc(Numeric auto const &value) noexcept
  {
    return krys::SwitchOn(value, [&](const auto &alternative) { return IsUnevaluatedCalc(alternative); });
  }
}