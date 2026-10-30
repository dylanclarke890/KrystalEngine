#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::css
{
  enum class Category : uint8;
}

namespace krys::boo::css::calc
{
  struct Tree;
}

namespace krys::boo::style::calc
{
  struct Tree;

  struct ToCSSOptions
  {
    /// @brief The context in which the conversion is taking place.
    css::Category Category;

    /// @brief The allowed numeric range for the calculated result.
    css::Range Range;

    /// @brief The RenderStyle the Tree is from, for zoom calculations.
    const RenderStyle &Style;
  };

  struct ToStyleOptions
  {
    /// @brief The context in which the conversion is taking place.
    css::Category Category;

    /// @brief The allowed numeric range for the calculated result.
    css::Range Range;

    /// @brief Contains information needed to convert length units into their canonical forms.
    Maybe<css::ToLengthConversionData> ConversionData;

    /// @brief Contains information needed to convert unresolved symbols into numeric values.
    css::SymbolTable SymbolTable;
  };

  /// @brief Converts from `style::calc::Tree` to `css::calc::Tree`.
  KRYS_NODISCARD css::calc::Tree ToCSS(const Tree &, const ToCSSOptions &) noexcept;

  /// @brief Converts from `css::calc::Tree` to `style::calc::Tree`.
  KRYS_NODISCARD Tree ToStyle(const css::calc::Tree &, const ToStyleOptions &) noexcept;
}
