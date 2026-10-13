#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayersResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"

namespace krys::boo::css
{
  namespace
  {
    // FIXME: Use the `BlendMode` mapping in `PrimitiveValueMappings.hpp` once it's reviewed.
    KRYS_NODISCARD constexpr ValueId ToValueId(BlendMode value) noexcept
    {
      switch (value)
      {
        case BlendMode::Normal:      return ValueId::Normal;
        case BlendMode::Multiply:    return ValueId::Multiply;
        case BlendMode::Screen:      return ValueId::Screen;
        case BlendMode::Darken:      return ValueId::Darken;
        case BlendMode::Lighten:     return ValueId::Lighten;
        case BlendMode::Overlay:     return ValueId::Overlay;
        case BlendMode::ColorDodge:  return ValueId::ColorDodge;
        case BlendMode::ColorBurn:   return ValueId::ColorBurn;
        case BlendMode::HardLight:   return ValueId::HardLight;
        case BlendMode::SoftLight:   return ValueId::SoftLight;
        case BlendMode::Difference:  return ValueId::Difference;
        case BlendMode::Exclusion:   return ValueId::Exclusion;
        case BlendMode::Hue:         return ValueId::Hue;
        case BlendMode::Saturation:  return ValueId::Saturation;
        case BlendMode::Color:       return ValueId::Color;
        case BlendMode::Luminosity:  return ValueId::Luminosity;
        case BlendMode::PlusDarker:  return ValueId::PlusDarker;
        case BlendMode::PlusLighter: return ValueId::PlusLighter;
      }
      krys_unreachable();
    }
  }

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

  void Serialiser<ColorLayers>::operator()(CSSOMString &builder, const SerialisationContext &context,
                                           const ColorLayers &value) noexcept
  {
    builder.append(u8"color-layers(");

    if (value.BlendMode != BlendMode::Normal)
    {
      builder.append(ToLower(ToValueId(value.BlendMode)));
      builder.append(u8", ");
    }

    auto first = true;
    for (const auto &color : value.Colors)
    {
      if (!first)
      {
        builder.append(u8", ");
      }
      first = false;
      Serialise(builder, context, color);
    }

    builder.push_back(u8')');
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
