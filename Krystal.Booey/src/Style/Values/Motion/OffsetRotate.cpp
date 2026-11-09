/*
 * Copyright (C) 2021 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/Motion/OffsetRotate.hpp"

#include "Krystal.Booey/CSS/Values/OffsetRotateValue.hpp"

#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  OffsetRotate CSSValueConversion<OffsetRotate>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    const css::PrimitiveValue *modifierValue = nullptr;
    const css::PrimitiveValue *angleValue = nullptr;

    if (const auto *offsetRotateValue = DynamicDowncast<css::OffsetRotateValue>(value))
    {
      modifierValue = offsetRotateValue->Modifier();
      angleValue = offsetRotateValue->GetAngle();
    }
    else if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      // Values coming from CSSTypedOM didn't go through the parser and may not have been converted to a
      // css::OffsetRotateValue.
      if (primitiveValue->ValueId() == css::ValueId::Auto
          || primitiveValue->ValueId() == css::ValueId::Reverse)
      {
        modifierValue = primitiveValue;
      }
      else if (primitiveValue->IsAngle())
      {
        angleValue = primitiveValue;
      }
    }

    Maybe<css::keywords::Auto> autoKeyword;
    auto angleInDegrees = 0.0f;

    if (angleValue)
    {
      angleInDegrees = angleValue->ResolveAsAngle<float>(state.CssToLengthConversionData());
    }

    if (modifierValue)
    {
      switch (modifierValue->ValueId())
      {
        case css::ValueId::Auto: autoKeyword = css::keywords::Auto {}; break;
        case css::ValueId::Reverse:
          autoKeyword = css::keywords::Auto {};
          angleInDegrees += 180.0f;
          break;
        default: krys_debug_assert_msg(false, "Unreachable");
      }
    }

    return OffsetRotate {autoKeyword, OffsetRotate::Angle {angleInDegrees}};
  }

  // MARK: - Blending

  bool Blending<OffsetRotate>::CanBlend(const OffsetRotate &from, const OffsetRotate &to) noexcept
  {
    return from.HasAuto() == to.HasAuto();
  }

  OffsetRotate Blending<OffsetRotate>::Blend(const OffsetRotate &from, const OffsetRotate &to,
                                             const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? to : from;
    }

    krys_debug_assert(CanBlend(from, to));
    return OffsetRotate {from.AutoKeyword(),
                         krys::boo::style::Blend(from.GetAngle(), to.GetAngle(), context)};
  }

  // MARK: - Evaluation
}
