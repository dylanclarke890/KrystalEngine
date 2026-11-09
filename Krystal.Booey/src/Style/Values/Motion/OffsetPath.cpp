/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/Style/Values/Motion/OffsetPath.hpp"

#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  OffsetPath CSSValueConversion<OffsetPath>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    return OffsetPath {
      ConvertCSSValueToStyle<RefPtr<PathOperation>>(state, value, SupportRayPathOperation::Yes)};
  }

  Ref<css::Value> CSSValueCreation<OffsetPath>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                           const OffsetPath &value) noexcept
  {
    return krys::SwitchOn(
      value, [&](const BasicShapePath &path)
      { return CreateCSSValue(pool, style, path, PathConversion::ForceAbsolute); },
      [&](const auto &path) { return CreateCSSValue(pool, style, path); });
  }

  // MARK: - Serialization

  void Serialiser<OffsetPath>::operator()(StringBuilder &builder, const css::SerialisationContext &context,
                                          const RenderStyle &style, const OffsetPath &value) noexcept
  {
    return krys::SwitchOn(
      value, [&](const BasicShapePath &path)
      { Serialise(builder, context, style, path, PathConversion::ForceAbsolute); },
      [&](const auto &path) { Serialise(builder, context, style, path); });
  }

  // MARK: - Blending

  bool Blending<OffsetPath>::CanBlend(const OffsetPath &a, const OffsetPath &b) noexcept
  {
    if (a.IsNone() || b.IsNone())
    {
      return false;
    }

    return a._operation->CanBlend(*b._operation);
  }

  OffsetPath Blending<OffsetPath>::Blend(const OffsetPath &a, const OffsetPath &b,
                                         const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return OffsetPath {a._operation->Blend(b._operation.get(), context)};
  }

  // MARK: - Platform

  RefPtr<PathOperation> ToPlatform<OffsetPath>::operator()(const OffsetPath &value) noexcept
  {
    return value._operation;
  }
}
