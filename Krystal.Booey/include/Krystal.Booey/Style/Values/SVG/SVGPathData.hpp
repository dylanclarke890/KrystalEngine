/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Shapes/PathFunction.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'d'> = none | <path()>
  // https://svgwg.org/svg2-draft/paths.html#DProperty
  // NOTE: The type is `SVGPathData` as `D` is just a bit too opaque.
  struct SVGPathData
  {
    SVGPathData(css::keywords::None) noexcept
    {
    }

    SVGPathData(PathFunction &&path) noexcept : _path {krys::move(path)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_path;
    }
    KRYS_NODISCARD bool IsPath() const noexcept
    {
      return !!_path;
    }
    KRYS_NODISCARD const Maybe<PathFunction> &TryPath() const noexcept KRYS_LIFETIME_BOUND
    {
      return _path;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(*_path);
    }

    KRYS_NODISCARD bool operator==(const SVGPathData &) const noexcept = default;

  private:
    Maybe<PathFunction> _path {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGPathData>
  {
    KRYS_NODISCARD SVGPathData operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<SVGPathData>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const SVGPathData &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<SVGPathData>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const SVGPathData &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<SVGPathData>
  {
    KRYS_NODISCARD bool CanBlend(const SVGPathData &, const SVGPathData &) noexcept;
    KRYS_NODISCARD SVGPathData Blend(const SVGPathData &, const SVGPathData &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGPathData)
