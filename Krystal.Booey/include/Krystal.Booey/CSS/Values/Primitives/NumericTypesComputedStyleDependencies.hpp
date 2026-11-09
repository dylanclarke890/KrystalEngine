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

#include "Krystal.Booey/CSS/Values/Primitives/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

// What properties does this value rely on (eg, font-size for em units)?
namespace krys::boo::css
{
  // Most unit types have no dependencies.
  template <UnitEnum Unit>
  struct ComputedStyleDependenciesCollector<Unit>
  {
    constexpr void operator()(ComputedStyleDependencies &, Unit) noexcept
    {
      // Nothing to do.
    }
  };

  // Let composite units dispatch to their component parts.
  template <CompositeUnitEnum Unit>
  struct ComputedStyleDependenciesCollector<Unit>
  {
    constexpr void operator()(ComputedStyleDependencies &dependencies, Unit unit) noexcept
    {
      SwitchOnUnitType(unit, [&](auto unit) { CollectComputedStyleDependencies(dependencies, unit); });
    }
  };

  // The one leaf unit type that does need to do work is `LengthUnit`.
  template <>
  struct ComputedStyleDependenciesCollector<LengthUnit>
  {
    void operator()(ComputedStyleDependencies &, LengthUnit) noexcept;
  };

  // Dependencies are based only on the unit; primitives to dispatch to the unit type analysis.
  template <NumericRaw RawType>
  struct ComputedStyleDependenciesCollector<RawType>
  {
    constexpr void operator()(ComputedStyleDependencies &dependencies, const RawType &value) noexcept
    {
      CollectComputedStyleDependencies(dependencies, value.Unit);
    }
  };
}
