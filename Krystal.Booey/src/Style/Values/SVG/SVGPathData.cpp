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

#include "Krystal.Booey/Style/Values/SVG/SVGPathData.hpp"

#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  SVGPathData CSSValueConversion<SVGPathData>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    auto *pathValue = RequiredDowncast<css::PathValue>(state, value);
    if (!pathValue)
    {
      return css::keywords::None {};
    }

    return ConvertToStyle(pathValue->GetPath(), state);
  }

  Ref<css::Value> CSSValueCreation<SVGPathData>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                            const SVGPathData &value) noexcept
  {
    if (value.IsNone())
    {
      return CreateCSSValue(pool, style, css::keywords::None {});
    }
    return css::PathValue::Create(ConvertToCSS(*value.TryPath(), style, PathConversion::ForceAbsolute));
  }

  // MARK: - Serialization

  void Serialiser<SVGPathData>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                           const RenderStyle &style, const SVGPathData &value) noexcept
  {
    if (value.IsNone())
    {
      Serialise(builder, context, style, css::keywords::None {});
      return;
    }
    Serialise(builder, context, style, *value.TryPath(), PathConversion::ForceAbsolute);
  }

  // MARK: - Blending

  bool Blending<SVGPathData>::CanBlend(const SVGPathData &a, const SVGPathData &b) noexcept
  {
    auto aPath = a.TryPath();
    auto bPath = b.TryPath();
    return aPath && bPath && style::CanBlend(*aPath, *bPath);
  }

  SVGPathData Blending<SVGPathData>::Blend(const SVGPathData &a, const SVGPathData &b,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      return context.Progress < 0.5 ? a : b;
    }

    krys_debug_assert(style::CanBlend(a, b));
    return style::Blend(*a.TryPath(), *b.TryPath(), context);
  }
}
