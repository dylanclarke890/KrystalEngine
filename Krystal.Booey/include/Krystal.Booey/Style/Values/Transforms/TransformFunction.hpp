/*
 * Copyright (C) 2012 Adobe Systems Incorporated. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
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

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionWrapper.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // Any <transform-function>.
  // https://www.w3.org/TR/css-transforms-1/#typedef-transform-function
  struct TransformFunction : TransformFunctionWrapper<TransformFunctionBase>
  {
    using TransformFunctionWrapper<TransformFunctionBase>::TransformFunctionWrapper;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<TransformFunction>
  {
    KRYS_NODISCARD TransformFunction operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<TransformFunction>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const TransformFunction &) noexcept;
  };
  template <>
  struct CSSValueCreation<TransformationMatrix>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const TransformationMatrix &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<TransformFunction>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const TransformFunction &) noexcept;
  };
  template <>
  struct Serialiser<TransformationMatrix>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const TransformationMatrix &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<TransformFunction>
  {
    KRYS_NODISCARD TransformFunction Blend(const TransformFunction &, const TransformFunction &,
                                           const BlendingContext &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<TransformFunction>
  {
    KRYS_NODISCARD Ref<TransformOperation> operator()(const TransformFunction &, const FloatSize &) noexcept;
  };
}
