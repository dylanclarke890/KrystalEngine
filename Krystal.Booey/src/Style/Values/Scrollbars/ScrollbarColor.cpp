/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Style/Values/Scrollbars/ScrollbarColor.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ScrollbarColor CSSValueConversion<ScrollbarColor>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (css::IsValueId(value, css::ValueId::Auto))
    {
      return css::keywords::Auto {};
    }

    auto *pair = RequiredDowncast<css::ValuePair>(state, value);
    if (!pair)
    {
      return css::keywords::Auto {};
    }

    const auto &thumb = pair->First();
    const auto &track = pair->Second();

    return ScrollbarColor::Parts {
      .Thumb = ConvertCSSValueToStyle<Color>(state, thumb),
      .Track = ConvertCSSValueToStyle<Color>(state, track),
    };
  }

  // MARK: - Blending

  bool Blending<ScrollbarColor>::Equals(const ScrollbarColor &a, const ScrollbarColor &b,
                                        const RenderStyle &aStyle, const RenderStyle &bStyle) noexcept
  {
    bool aAuto = a.IsAuto();
    bool bAuto = b.IsAuto();

    if (aAuto || bAuto)
    {
      return aAuto == bAuto;
    }

    return style::EqualsForBlending(a.TryValue()->Thumb, b.TryValue()->Thumb, aStyle, bStyle)
           && style::EqualsForBlending(a.TryValue()->Track, b.TryValue()->Track, aStyle, bStyle);
  }

  bool Blending<ScrollbarColor>::CanBlend(const ScrollbarColor &a, const ScrollbarColor &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto() && style::CanBlend(a.TryValue()->Thumb, b.TryValue()->Thumb)
           && style::CanBlend(a.TryValue()->Track, b.TryValue()->Track);
  }

  ScrollbarColor Blending<ScrollbarColor>::Blend(const ScrollbarColor &a, const ScrollbarColor &b,
                                                 const RenderStyle &aStyle, const RenderStyle &bStyle,
                                                 const BlendingContext &context) noexcept
  {
    if (!style::CanBlend(a, b))
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    return ScrollbarColor::Parts {
      .Thumb = style::Blend(a.TryValue()->Thumb, b.TryValue()->Thumb, aStyle, bStyle, context),
      .Track = style::Blend(a.TryValue()->Track, b.TryValue()->Track, aStyle, bStyle, context),
    };
  }
}
