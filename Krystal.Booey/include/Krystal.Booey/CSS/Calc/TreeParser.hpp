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

#include "CSSCalcSymbolsAllowed.h"
#include "CSSPrimitiveNumericRange.h"
#include "CSSPropertyParserOptions.h"
#include <optional>

namespace krys::boo::css
{

  namespace CSS
  {
    enum class Category : uint8_t;
  }

  namespace CSS
  {
    struct PropertyParserState;
  }

  class CSSParserTokenRange;
  struct ParserContext;

  enum CSSValueID : uint16_t;

  namespace CSSCalc
  {

    struct SimplificationOptions;
    struct Tree;

    struct ParserOptions
    {
      // `category` represents the context in which the parse is taking place.
      CSS::Category category;

      // `range` represents the allowed numeric range for the calculated result.
      CSS::Range range;

      // `allowedSymbols` contains additional symbols that can be used in the calculation. These will need to
      // be resolved before the calculation can be resolved.
      CSSCalcSymbolsAllowed allowedSymbols;

      // `propertyOptions` contains options about the specific property the calc() is intended to be used
      // with.
      CSSPropertyParserOptions propertyOptions;
    };

    // Parses and simplifies the provided `CSSParserTokenRange` into a CSSCalc::Tree. Returns `std::nullopt`
    // on failure.
    std::optional<Tree> parseAndSimplify(CSSParserTokenRange &, CSS::PropertyParserState &,
                                         const ParserOptions &, const SimplificationOptions &);

    // Returns whether the provided `CSSValueID` is one of the functions that should be parsed as a `calc()`.
    KRYS_NODISCARD constexpr bool IsCalcFunction(ValueId functionId) noexcept
    {
      switch (functionId)
      {
        case ValueId::Calc:
        case ValueId::WebkitCalc:
        case ValueId::Min:
        case ValueId::Max:
        case ValueId::Clamp:
        case ValueId::Pow:
        case ValueId::Sqrt:
        case ValueId::Hypot:
        case ValueId::Sin:
        case ValueId::Cos:
        case ValueId::Tan:
        case ValueId::Exp:
        case ValueId::Log:
        case ValueId::Asin:
        case ValueId::Acos:
        case ValueId::Atan:
        case ValueId::Atan2:
        case ValueId::Abs:
        case ValueId::Sign:
        case ValueId::Round:
        case ValueId::Mod:
        case ValueId::Rem:
        case ValueId::Progress:
        case ValueId::Random:
        case ValueId::SiblingCount:
        case ValueId::SiblingIndex:
        case ValueId::Anchor:
        case ValueId::AnchorSize:
        {
          return true;
        }
        default:
        {
          return false;
        }
      }
    }

  } // namespace CSSCalc
} // namespace krys::boo::css


#pragma once

#include "Krystal.Booey/CSS/Values/ValueId.hpp"

namespace krys::boo::css
{

}