#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"

namespace krys::boo
{
  class Color;
  struct PlatformColorResolutionState;
}

namespace krys::boo::css
{
  struct ContrastColor
  {
    Color Color;

    bool operator==(const ContrastColor &) const noexcept;
  };

  KRYS_NODISCARD boo::Color CreateColor(const ContrastColor &, PlatformColorResolutionState &) noexcept;

  KRYS_NODISCARD bool ContainsCurrentColor(const ContrastColor &) noexcept;

  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const ContrastColor &) noexcept;

  template <>
  struct Serialiser<ContrastColor>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const ContrastColor &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ContrastColor>
  {
    void operator()(ComputedStyleDependencies &, const ContrastColor &);
  };

  template <>
  struct ValueChildrenVisitor<ContrastColor>
  {
    IterationStatus operator()(const Func<IterationStatus(css::Value &)> &, const ContrastColor &) noexcept;
  };
}