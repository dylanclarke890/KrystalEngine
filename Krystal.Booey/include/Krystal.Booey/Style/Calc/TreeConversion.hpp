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
