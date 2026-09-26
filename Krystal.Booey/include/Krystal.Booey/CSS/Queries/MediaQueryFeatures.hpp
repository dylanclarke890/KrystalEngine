#pragma once

#include "Krystal.Booey/CSS/Queries/MediaQuery.hpp"

namespace krys::boo::css
{
  enum class Category : uint8;

  struct ComputedStyleDependencies;
}

namespace krys::boo::css::mq
{
  namespace Features
  {
    KRYS_NODISCARD const FeatureSchema &Animation() noexcept;
    KRYS_NODISCARD const FeatureSchema &AnyHover() noexcept;
    KRYS_NODISCARD const FeatureSchema &AnyPointer() noexcept;
    KRYS_NODISCARD const FeatureSchema &AspectRatio() noexcept;
    KRYS_NODISCARD const FeatureSchema &Color() noexcept;
    KRYS_NODISCARD const FeatureSchema &ColorGamut() noexcept;
    KRYS_NODISCARD const FeatureSchema &ColorIndex() noexcept;
    KRYS_NODISCARD const FeatureSchema &DeviceAspectRatio() noexcept;
    KRYS_NODISCARD const FeatureSchema &DeviceHeight() noexcept;
    KRYS_NODISCARD const FeatureSchema &DevicePixelRatio() noexcept;
    KRYS_NODISCARD const FeatureSchema &DeviceWidth() noexcept;
    KRYS_NODISCARD const FeatureSchema &DynamicRange() noexcept;
    KRYS_NODISCARD const FeatureSchema &ForcedColors() noexcept;
    KRYS_NODISCARD const FeatureSchema &Grid() noexcept;
    KRYS_NODISCARD const FeatureSchema &Height() noexcept;
    KRYS_NODISCARD const FeatureSchema &Hover() noexcept;
    KRYS_NODISCARD const FeatureSchema &InvertedColors() noexcept;
    KRYS_NODISCARD const FeatureSchema &Monochrome() noexcept;
    KRYS_NODISCARD const FeatureSchema &Orientation() noexcept;
    KRYS_NODISCARD const FeatureSchema &OverflowBlock() noexcept;
    KRYS_NODISCARD const FeatureSchema &OverflowInline() noexcept;
    KRYS_NODISCARD const FeatureSchema &Pointer() noexcept;
    KRYS_NODISCARD const FeatureSchema &PrefersContrast() noexcept;
    KRYS_NODISCARD const FeatureSchema &PrefersDarkInterface() noexcept;
    KRYS_NODISCARD const FeatureSchema &PrefersReducedMotion() noexcept;
    KRYS_NODISCARD const FeatureSchema &Resolution() noexcept;
    KRYS_NODISCARD const FeatureSchema &Scan() noexcept;
    KRYS_NODISCARD const FeatureSchema &Scripting() noexcept;
    KRYS_NODISCARD const FeatureSchema &Transform2d() noexcept;
    KRYS_NODISCARD const FeatureSchema &Transform3d() noexcept;
    KRYS_NODISCARD const FeatureSchema &Transition() noexcept;
    KRYS_NODISCARD const FeatureSchema &Update() noexcept;
    KRYS_NODISCARD const FeatureSchema &VideoPlayableInline() noexcept;
    KRYS_NODISCARD const FeatureSchema &Width() noexcept;
    KRYS_NODISCARD const FeatureSchema &PrefersColorScheme() noexcept;

    KRYS_NODISCARD SmallList<const FeatureSchema *> AllSchemas() noexcept;
  }
}
