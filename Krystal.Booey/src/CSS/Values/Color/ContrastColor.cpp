/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  bool ContrastColor::operator==(const ContrastColor &) const noexcept = default;

  boo::Color CreateColor(const ContrastColor &unresolved, PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};
    return Resolve(ContrastColorResolver {CreateColor(unresolved.Color, state)});
  }

  bool ContainsCurrentColor(const ContrastColor &unresolved) noexcept
  {
    return ContainsCurrentColor(unresolved.Color);
  }

  bool ContainsColorSchemeDependentColor(const ContrastColor &unresolved) noexcept
  {
    return ContainsColorSchemeDependentColor(unresolved.Color);
  }

  void Serialiser<ContrastColor>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                             const ContrastColor &value) noexcept
  {
    builder.Append(u8"contrast-color(");
    Serialise(builder, context, value.Color);
    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<ContrastColor>::operator()(ComputedStyleDependencies &dependencies,
                                                                     const ContrastColor &value) noexcept
  {
    CollectComputedStyleDependencies(dependencies, value.Color);
  }

  IterationStatus
    ValueChildrenVisitor<ContrastColor>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                                    const ContrastColor &value) noexcept
  {
    return VisitValueChildren(func, value.Color);
  }
}
