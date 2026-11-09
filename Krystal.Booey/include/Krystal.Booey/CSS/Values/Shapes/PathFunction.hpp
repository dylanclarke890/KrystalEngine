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

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/SVG/Internals/SVGPathByteStream.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  // <path()> = path( <'fill-rule'>? , <string> )
  // https://drafts.csswg.org/css-shapes-1/#funcdef-basic-shape-path
  struct Path
  {
    struct PathData
    {
      svg::SVGPathByteStream ByteStream;

      KRYS_NODISCARD bool operator==(const PathData &) const noexcept = default;
    };

    Maybe<boo::css::FillRule> FillRule;
    PathData Data;

    KRYS_NODISCARD bool operator==(const Path &) const noexcept = default;
  };

  using PathFunction = FunctionNotation<ValueId::Path, Path>;

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Path &value) noexcept
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.Data;
    }
  }

  template <>
  struct Serialiser<Path>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context, const Path &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<Path::PathData>
  {
    void operator()(ComputedStyleDependencies &dependencies, const Path::PathData &data) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<Path::PathData>
  {
    KRYS_NODISCARD IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                              const Path::PathData &data) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Path, 2)
