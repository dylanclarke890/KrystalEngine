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

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"

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

    /// @brief Contains information needed to convert unresolved symbols into numeric values.
    css::SymbolTable SymbolTable;

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