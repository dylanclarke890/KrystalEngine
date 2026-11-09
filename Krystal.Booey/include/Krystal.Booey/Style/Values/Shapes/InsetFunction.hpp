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

#include "Krystal.Booey/CSS/Values/Shapes/InsetFunction.hpp"
#include "Krystal.Booey/Style/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"

namespace krys::boo
{
  class Path;

  namespace style
  {
    struct Inset
    {
      using InsetsType = css::MinimallySerialisingSpaceSeparatedRectEdges<LengthPercentage<>>;

      InsetsType Insets;
      BorderRadius Radii;

      KRYS_NODISCARD bool operator==(const Inset &) const noexcept = default;
    };
    using InsetFunction = css::FunctionNotation<css::ValueId::Inset, Inset>;

    template <size_t I>
    const auto &get(const Inset &value)
    {
      if constexpr (!I)
      {
        return value.Insets;
      }
      else if constexpr (I == 1)
      {
        return value.Radii;
      }
    }

    DEFINE_TYPE_MAPPING(css::Inset, Inset)

    template <>
    struct PathComputation<Inset>
    {
      KRYS_NODISCARD boo::Path operator()(const Inset &, const FloatRect &) noexcept;
    };

  }
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Inset, 2)
