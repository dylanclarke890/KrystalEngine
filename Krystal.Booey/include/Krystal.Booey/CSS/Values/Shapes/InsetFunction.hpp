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

#include "Krystal.Booey/CSS/Values/Borders/BorderRadius.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <inset()> = inset( <length-percentage>{1,4} [ round <'border-radius'> ]? )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-inset
  struct Inset
  {
    using InsetsType = MinimallySerialisingSpaceSeparatedRectEdges<LengthPercentage<>>;

    InsetsType Insets;
    BorderRadius Radii;

    KRYS_NODISCARD bool operator==(const Inset &) const noexcept = default;
  };

  using InsetFunction = FunctionNotation<ValueId::Inset, Inset>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Inset &value) noexcept
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

  template <>
  struct Serialiser<Inset>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const Inset &value) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Inset, 2)
