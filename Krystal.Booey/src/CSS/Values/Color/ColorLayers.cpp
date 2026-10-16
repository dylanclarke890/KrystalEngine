#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::css
{
  boo::Color CreateColor(const ColorLayers &value, PlatformColorResolutionState &state) noexcept
  {
    PlatformColorResolutionStateNester nester {state};

    // FIXME: This should be made into a lazy transformed range to avoid the unnecessary temporary allocation.
    List<boo::Color> colors;
    colors.reserve(value.Colors.size());
    for (const auto &color : value.Colors)
    {
      colors.push_back(CreateColor(color, state));
    }

    auto resolver = ColorLayersResolver {.BlendMode = value.BlendMode, .Colors = krys::move(colors)};

    return BlendSourceOver(krys::move(resolver));
  }

  bool ContainsCurrentColor(const ColorLayers &value) noexcept
  {
    return std::ranges::any_of(value.Colors, [](const auto &color) { return ContainsCurrentColor(color); });
  }

  bool ContainsColorSchemeDependentColor(const ColorLayers &value) noexcept
  {
    return std::ranges::any_of(value.Colors,
                               [](const auto &color) { return ContainsColorSchemeDependentColor(color); });
  }

  void Serialiser<ColorLayers>::operator()(StringBuilder &builder, const SerialisationContext &context,
                                           const ColorLayers &value) noexcept
  {
    builder.Append(u8"color-layers(");

    if (value.BlendMode != BlendMode::Normal)
    {
      builder.Append(ToLower(ToValueId(value.BlendMode)));
      builder.Append(u8", ");
    }

    auto first = true;
    for (const auto &color : value.Colors)
    {
      if (!first)
      {
        builder.Append(u8", ");
      }
      first = false;
      Serialise(builder, context, color);
    }

    builder.Append(u8')');
  }

  void ComputedStyleDependenciesCollector<ColorLayers>::operator()(ComputedStyleDependencies &dependencies,
                                                                   const ColorLayers &value) noexcept
  {
    CollectComputedStyleDependenciesOnRangeLike(dependencies, value.Colors);
  }

  IterationStatus
    ValueChildrenVisitor<ColorLayers>::operator()(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func,
                                                  const ColorLayers &value) noexcept
  {
    return VisitValueChildrenOnRangeLike(func, value.Colors);
  }
}
