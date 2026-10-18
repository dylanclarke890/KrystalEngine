#pragma once

#include "Krystal.Booey/CSS/Values/Types.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct HexColor
  {
    SRGBA<uint8> Value;

    KRYS_NODISCARD constexpr bool operator==(const HexColor &) const noexcept = default;
  };

  KRYS_NODISCARD inline boo::Color CreateColor(const HexColor &unresolved,
                                               PlatformColorResolutionState &) noexcept
  {
    return boo::Color {unresolved.Value};
  }

  KRYS_NODISCARD constexpr bool ContainsCurrentColor(const HexColor &) noexcept
  {
    return false;
  }

  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const HexColor &) noexcept
  {
    return false;
  }

  template <>
  struct Serialiser<HexColor>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const HexColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<HexColor>
  {
    constexpr void operator()(ComputedStyleDependencies &, const HexColor &) noexcept
    {
    }
  };

  template <>
  struct ValueChildrenVisitor<HexColor>
  {
    constexpr IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &,
                                         const HexColor &) noexcept
    {
      return IterationStatus::Continue;
    }
  };
}
