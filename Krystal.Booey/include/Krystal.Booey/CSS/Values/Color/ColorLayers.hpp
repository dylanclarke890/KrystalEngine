#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo
{
  class Color;

  enum class BlendMode : uint8;
}

namespace krys::boo::css
{
  struct PlatformColorResolutionState;

  struct ColorLayers
  {
    BlendMode BlendMode;
    CommaSeparatedList<Color> Colors;

    bool operator==(const ColorLayers &) const noexcept = default;
  };

  KRYS_NODISCARD boo::Color CreateColor(const ColorLayers &, PlatformColorResolutionState &) noexcept;

  KRYS_NODISCARD bool ContainsCurrentColor(const ColorLayers &) noexcept;

  KRYS_NODISCARD bool ContainsColorSchemeDependentColor(const ColorLayers &) noexcept;

  template <>
  struct Serialiser<ColorLayers>
  {
    void operator()(CSSOMString &builder, const SerialisationContext &context,
                    const ColorLayers &value) noexcept;
  };

  template <>
  struct ComputedStyleDependenciesCollector<ColorLayers>
  {
    void operator()(ComputedStyleDependencies &, const ColorLayers &) noexcept;
  };

  template <>
  struct ValueChildrenVisitor<ColorLayers>
  {
    IterationStatus operator()(const Func<IterationStatus(css::Value &)> &, const ColorLayers &) noexcept;
  };
}
