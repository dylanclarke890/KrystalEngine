/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Shapes/PathFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Shapes/FillRule.hpp"
#include "Krystal.Booey/Style/Values/Shapes/PathComputation.hpp"
#include "Krystal.Booey/Style/Values/Shapes/WindRuleComputation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  enum class PathConversion : bool
  {
    None,
    ForceAbsolute
  };

  struct Path
  {
    struct PathData
    {
      svg::SVGPathByteStream ByteStream;

      KRYS_NODISCARD bool operator==(const PathData &) const noexcept = default;
    };

    Maybe<style::FillRule> FillRule;
    PathData Data;

    float Zoom {1};

    KRYS_NODISCARD bool operator==(const Path &) const noexcept = default;
  };
  using PathFunction = css::FunctionNotation<css::ValueId::Path, Path>;

  template <size_t I>
  const auto &get(const Path &value)
  {
    if constexpr (!I)
    {
      return value.FillRule;
    }
    else if constexpr (I == 1)
    {
      return value.Data;
    }
    else if constexpr (I == 2)
    {
      return value.Zoom;
    }
  }

  // MARK: - Conversion

  // Non-standard parameters, `conversion` and `zoom`, are needed in some instances of Style <-> CSS
  // conversions for Path.

  template <>
  struct ToCSS<Path>
  {
    KRYS_NODISCARD css::Path operator()(const Path &, const RenderStyle &,
                                        PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct ToStyle<css::Path>
  {
    KRYS_NODISCARD Path operator()(const css::Path &, const BuilderState &,
                                   Maybe<float> zoom = 1.0f) noexcept;
  };

  template <>
  struct ToCSS<Path::PathData>
  {
    KRYS_NODISCARD css::Path::PathData operator()(const Path::PathData &, const RenderStyle &,
                                                  PathConversion = PathConversion::None) noexcept;
  };
  template <>
  struct ToStyle<css::Path::PathData>
  {
    KRYS_NODISCARD Path::PathData operator()(const css::Path::PathData &, const BuilderState &) noexcept;
  };

  template <>
  struct CSSValueCreation<PathFunction>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const PathFunction &,
                                              PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<Path>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &, const Path &,
                    PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Path

  template <>
  struct PathComputation<Path>
  {
    KRYS_NODISCARD boo::Path operator()(const Path &, const FloatRect &) noexcept;
  };

  // MARK: - Wind Rule

  template <>
  struct WindRuleComputation<Path>
  {
    KRYS_NODISCARD boo::WindRule operator()(const Path &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<Path>
  {
    KRYS_NODISCARD bool CanBlend(const Path &, const Path &) noexcept;
    KRYS_NODISCARD Path Blend(const Path &, const Path &, const BlendingContext &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Path, 2)
