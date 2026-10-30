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
