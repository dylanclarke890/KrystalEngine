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

#include "Krystal.Web/Css/Calc/SymbolTable.hpp"
#include "Krystal.Web/Css/Values/Primitives/Range.hpp"
#include "Krystal.Web/Css/Values/ToLengthConversionData.hpp"

namespace krys::web::css
{
  enum class Category : uint8;
}

namespace krys::web::css::calc
{
  struct Child;
  struct Tree;

  struct Abs;
  struct Acos;
  struct Anchor;
  struct AnchorSize;
  struct Asin;
  struct Atan;
  struct Atan2;
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
    Canonicalize(NonCanonicalDimension root, const Maybe<ToLengthConversionData> &conversionData) noexcept;

  KRYS_NODISCARD bool CanSimplify(const Tree &tree, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Tree CopyAndSimplify(const Tree &tree, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Child CopyAndSimplify(const Child &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Number &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Percentage &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(NonCanonicalDimension &root,
                                       const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(CanonicalDimension &value,
                                       const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Symbol &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(SiblingCount &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(SiblingIndex &value, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sum &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Product &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Negate &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Invert &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Min &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Max &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Clamp &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundNearest &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundUp &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundDown &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(RoundToZero &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Mod &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Rem &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sin &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Cos &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Tan &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Asin &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Acos &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Atan &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Atan2 &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Pow &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sqrt &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Hypot &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Log &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Exp &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Abs &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Sign &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Random &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Progress &root, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(Anchor &anchor, const SimplificationOptions &options) noexcept;

  KRYS_NODISCARD Maybe<Child> Simplify(AnchorSize &anchorSize, const SimplificationOptions &options) noexcept;
}
