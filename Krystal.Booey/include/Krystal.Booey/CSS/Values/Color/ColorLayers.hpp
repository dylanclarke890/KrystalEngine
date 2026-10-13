#pragma once

#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"

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
    void operator()(CSSOMString &builder, const SerialisationContext &context,
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
