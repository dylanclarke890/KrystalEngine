/*
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ContrastColor
  {
    css::Color Color;

    KRYS_NODISCARD bool operator==(const ContrastColor &) const noexcept;
  };

  KRYS_NODISCARD boo::Color CreateColor(const ContrastColor &unresolved,
                                        PlatformColorResolutionState &state) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const ContrastColor &unresolved) noexcept;
  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const ContrastColor &unresolved) noexcept;

  template <>
  struct Serialiser<ContrastColor>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const ContrastColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ContrastColor>
  {
    void operator()(ComputedStyleDependencies &dependencies, const ContrastColor &value) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<ContrastColor>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const ContrastColor &value) noexcept;
  };
}
