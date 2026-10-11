#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"

namespace krys::boo::css
{
  enum class Category : uint8;
}

namespace krys::boo::css::calc
{
  struct Child;
  struct Tree;

  struct Abs;
  struct Acos;
  struct Anchor;
  struct AnchorSize;
  struct Asin;
  struct Atan2;
  struct Atan;
  struct CanonicalDimension;
  struct Clamp;
  struct Cos;
  struct Exp;
  struct Hypot;
  struct Invert;
  struct Log;
  struct Max;
  struct Min;
  struct Mod;
  struct Negate;
  struct NonCanonicalDimension;
  struct Number;
  struct Percentage;
  struct Pow;
  struct Product;
  struct Progress;
  struct Random;
  struct Rem;
  struct RoundDown;
  struct RoundNearest;
  struct RoundToZero;
  struct RoundUp;
  struct SiblingCount;
  struct SiblingIndex;
  struct Sign;
  struct Sin;
  struct Sqrt;
  struct Sum;
  struct Symbol;
  struct Tan;

  /// @see https://drafts.csswg.org/css-values-4/#calc-simplification
  struct SimplificationOptions
  {
    /// @brief The context in which the simplification is taking place.
    css::Category Category;

    /// @brief The allowed numeric range for the calculated result.
    css::Range Range;

    /// @brief Contains information needed to convert length units into their canonical forms.
    Maybe<ToLengthConversionData> ConversionData;

    /// @brief Contains information needed to convert unresolved symbols into Numeric values.
    SymbolTable SymbolTable;

    /// @brief Allows removal of 0 value lengths (px, em, etc.) from Sum operations.
    bool AllowZeroValueLengthRemovalFromSum = false;
  };

  KRYS_NODISCARD Maybe<CanonicalDimension>
    Canonicalize(NonCanonicalDimension dimension,
                 const Maybe<ToLengthConversionData> &conversionData) noexcept;

  KRYS_NODISCARD bool CanSimplify(const Tree &tree, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Tree CopyAndSimplify(const Tree &tree, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Child CopyAndSimplify(const Child &child, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Number &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Percentage &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(NonCanonicalDimension &value,
                                       const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(CanonicalDimension &value,
                                       const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Symbol &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(SiblingCount &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(SiblingIndex &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sum &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Product &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Negate &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Invert &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Min &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Max &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Clamp &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundNearest &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundUp &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundDown &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundToZero &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Mod &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Rem &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sin &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Cos &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Tan &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Asin &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Acos &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Atan &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Atan2 &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Pow &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sqrt &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Hypot &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Log &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Exp &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Abs &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sign &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Random &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Progress &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Anchor &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(AnchorSize &value, const SimplificationOptions &options) noexcept;
}