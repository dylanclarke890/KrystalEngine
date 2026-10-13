#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"

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
    void operator()(CSSOMString &builder, const SerialisationContext &context,
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
