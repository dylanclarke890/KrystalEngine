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