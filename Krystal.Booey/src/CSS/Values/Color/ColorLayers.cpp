/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  boo::Color CreateColor(const ColorLayers &value, PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    // FIXME: This should be made into a lazy transformed range to avoid the unnecessary temporary allocation.
    List<boo::Color> colors;
    colors.reserve(value.Colors.size());
    for (const auto &color : value.Colors)
    {
      colors.push_back(CreateColor(color, state));
    }

    auto resolver = ColorLayersResolver {.BlendMode = value.BlendMode, .Colors = krys::move(colors)};

    return BlendSourceOver(krys::move(resolver));
  }

  bool ContainsCurrentColor(const ColorLayers &value) noexcept
  {
    return std::ranges::any_of(value.Colors, [](const auto &color) { return ContainsCurrentColor(color); });
  }

  bool ContainsColorSchemeDependentColor(const ColorLayers &value) noexcept
  {
    return std::ranges::any_of(value.Colors,
                               [](const auto &color) { return ContainsColorSchemeDependentColor(color); });
  }

  void Serialiser<ColorLayers>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                           const ColorLayers &value) noexcept
  {
    builder.Append(u8"color-layers(");

    if (value.BlendMode != BlendMode::Normal)
    {
      builder.Append(ToLower(boo::ToValueId(value.BlendMode)));
      builder.Append(u8", ");
    }

    auto first = true;
    for (const auto &color : value.Colors)
    {
      if (!first)
      {
        builder.Append(u8", ");
      }
      first = false;
      Serialise(builder, context, color);
    }

    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<ColorLayers>::operator()(ComputedStyleDependencies &dependencies,
                                                                   const ColorLayers &value) noexcept
  {
    CollectComputedStyleDependenciesOnRangeLike(dependencies, value.Colors);
  }

  IterationStatus
    ValueChildrenVisitor<ColorLayers>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                                  const ColorLayers &value) noexcept
  {
    return VisitValueChildrenOnRangeLike(func, value.Colors);
  }
}
