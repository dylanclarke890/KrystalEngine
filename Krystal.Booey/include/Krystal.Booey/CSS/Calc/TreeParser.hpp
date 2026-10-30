#pragma once

#include "Krystal.Booey/CSS/Calc/AllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserOptions.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::css
{
  class TokenRange;

  struct ParserContext;
  struct PropertyParserState;

  enum class Category : uint8;
}

namespace krys::boo::css::calc
{
  struct SimplificationOptions;
  struct Tree;

  struct ParserOptions
  {
    /// @brief The context in which the parse is taking place.
    css::Category Category;

    /// @brief The allowed numeric range for the calculated result.
    css::Range Range;

    /// @brief Contains additional symbols that can be used in the calculation. These will need to be resolved
    /// before the calculation can be resolved.
    css::AllowedSymbols AllowedSymbols;

    /// @brief Contains options about the specific property the calc() is intended to be used with.
    css::PropertyParserOptions PropertyOptions;
  };

  /// @brief Parses and simplifies the provided TokenRange into a Tree. Returns `null` on failure.
  KRYS_NODISCARD Maybe<Tree> ParseAndSimplify(TokenRange &tokens, css::PropertyParserState &state,
                                              const ParserOptions &options,
                                              const SimplificationOptions &simplifyOptions) noexcept;

  /// @brief Returns whether the provided ValueId is one of the functions that should be parsed as a calc().
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
}