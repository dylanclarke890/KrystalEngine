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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/UI/AccentColor.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo::style
{
  const Color &AccentColor::ColorOrCurrentColor() const
  {
    if (_value)
    {
      return *_value;
    }
    return Color::GetCurrentColor();
  }

  // MARK: - Blending

  bool Blending<AccentColor>::Equals(const AccentColor &a, const AccentColor &b, const RenderStyle &aStyle,
                                     const RenderStyle &bStyle) noexcept
  {
    bool aAuto = a.IsAuto();
    bool bAuto = b.IsAuto();

    if (aAuto || bAuto)
    {
      return aAuto == bAuto;
    }

    return style::EqualsForBlending(*a.TryColor(), *b.TryColor(), aStyle, bStyle);
  }

  bool Blending<AccentColor>::CanBlend(const AccentColor &a, const AccentColor &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto() && style::CanBlend(*a.TryColor(), *b.TryColor());
  }

  AccentColor Blending<AccentColor>::Blend(const AccentColor &a, const AccentColor &b,
                                           const RenderStyle &aStyle, const RenderStyle &bStyle,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return style::Blend(*a.TryColor(), *b.TryColor(), aStyle, bStyle, context);
  }
}
