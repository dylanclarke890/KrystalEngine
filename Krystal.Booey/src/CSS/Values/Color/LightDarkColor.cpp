/*
 * Copyright (C) 2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  bool LightDarkColor::operator==(const LightDarkColor &) const noexcept = default;

  boo::Color CreateColor(const LightDarkColor &unresolved, PlatformColorResolutionState &state) noexcept
  {
    if (!state.Appearance)
    {
      return {};
    }

    PlatformColorResolutionStateNester nester {state};

    switch (*state.Appearance)
    {
      case LightDarkColorAppearance::Light: return CreateColor(unresolved.LightColor, state);
      case LightDarkColorAppearance::Dark:  return CreateColor(unresolved.DarkColor, state);
    }

    krys_debug_assert_msg(false, "Unknown light-dark() appearance.");
    return {};
  }

  bool ContainsCurrentColor(const LightDarkColor &unresolved) noexcept
  {
    return ContainsCurrentColor(unresolved.LightColor) || ContainsCurrentColor(unresolved.DarkColor);
  }

  void Serialiser<LightDarkColor>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                              const LightDarkColor &value) noexcept
  {
    builder.Append(u8"light-dark(");
    Serialise(builder, context, value.LightColor);
    builder.Append(u8", ");
    Serialise(builder, context, value.DarkColor);
    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<LightDarkColor>::operator()(ComputedStyleDependencies &dependencies,
                                                                      const LightDarkColor &value) noexcept
  {
    CollectComputedStyleDependencies(dependencies, value.LightColor);
    CollectComputedStyleDependencies(dependencies, value.DarkColor);
  }

  IterationStatus
    ValueChildrenVisitor<LightDarkColor>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                                     const LightDarkColor &value) noexcept
  {
    if (VisitValueChildren(func, value.LightColor) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    if (VisitValueChildren(func, value.DarkColor) == IterationStatus::Done)
    {
      return IterationStatus::Done;
    }
    return IterationStatus::Continue;
  }
}
