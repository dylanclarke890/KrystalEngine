/*
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ColorLayers
  {
    boo::BlendMode BlendMode;
    CommaSeparatedList<Color> Colors;

    KRYS_NODISCARD bool operator==(const ColorLayers &) const noexcept = default;
  };

  KRYS_NODISCARD boo::Color CreateColor(const ColorLayers &value,
                                        PlatformColorResolutionState &state) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ColorLayers &value) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const ColorLayers &value) noexcept;

  template <>
  struct Serialiser<ColorLayers>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ColorLayers &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ColorLayers>
  {
    void operator()(ComputedStyleDependencies &dependencies, const ColorLayers &value) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<ColorLayers>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const ColorLayers &value) noexcept;
  };
}
