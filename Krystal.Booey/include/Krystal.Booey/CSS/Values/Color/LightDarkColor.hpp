#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  enum class LightDarkColorAppearance : bool
  {
    Light,
    Dark
  };

  struct LightDarkColor
  {
    Color LightColor;
    Color DarkColor;

    KRYS_NODISCARD bool operator==(const LightDarkColor &) const noexcept;
  };

  KRYS_NODISCARD boo::Color CreateColor(const LightDarkColor &unresolved,
                                        PlatformColorResolutionState &state) noexcept;
  KRYS_NODISCARD bool ContainsCurrentColor(const LightDarkColor &unresolved) noexcept;

  KRYS_NODISCARD constexpr bool ContainsColorSchemeDependentColor(const LightDarkColor &) noexcept
  {
    return true;
  }

  template <>
  struct Serialiser<LightDarkColor>
  {
    void operator()(StringBuilder &builder, const SerialisationContext &context,
                    const LightDarkColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<LightDarkColor>
  {
    void operator()(ComputedStyleDependencies &dependencies, const LightDarkColor &value) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<LightDarkColor>
  {
    IterationStatus operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                               const LightDarkColor &value) noexcept;
  };
}
