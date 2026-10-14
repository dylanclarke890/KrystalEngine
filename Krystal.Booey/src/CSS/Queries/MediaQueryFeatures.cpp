#include "Krystal.Booey/CSS/Queries/MediaQueryFeatures.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

// FIXME: The schemas don't evaluate yet (Evaluate() returns Unknown). Each comes with its value function when
// media query evaluation is ported.

namespace krys::boo::css::mq::Features
{
  using enum FeatureSchema::Type;
  using ValueType = FeatureSchema::ValueType;
  using Dependency = MediaQueryDynamicDependency;

  const FeatureSchema &AnyHover() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"any-hover", Discrete, ValueType::Identifier,
                                                           Dependency::None,
                                                           FixedVector {ValueId::None, ValueId::Hover}};
    return schema;
  }

  const FeatureSchema &AnyPointer() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"any-pointer", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::None, ValueId::Fine, ValueId::Coarse}};
    return schema;
  }

  const FeatureSchema &AspectRatio() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"aspect-ratio", Range, ValueType::Ratio,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &Color() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"color", Range, ValueType::Integer,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &ColorGamut() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"color-gamut", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::SRGB, ValueId::P3, ValueId::Rec2020}};
    return schema;
  }

  const FeatureSchema &ColorIndex() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"color-index", Range, ValueType::Integer,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &DeviceAspectRatio() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"device-aspect-ratio", Range, ValueType::Ratio,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &DeviceHeight() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"device-height", Range, ValueType::Length,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &DeviceWidth() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"device-width", Range, ValueType::Length,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &DynamicRange() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"dynamic-range", Discrete, ValueType::Identifier,
                                                           Dependency::None,
                                                           FixedVector {ValueId::Standard, ValueId::High}};
    return schema;
  }

  const FeatureSchema &ForcedColors() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"forced-colors", Discrete, ValueType::Identifier,
                                                           Dependency::None,
                                                           FixedVector {ValueId::None, ValueId::Active}};
    return schema;
  }

  const FeatureSchema &Grid() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"grid", Discrete, ValueType::Integer,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &Height() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"height", Range, ValueType::Length,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &Hover() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"hover", Discrete, ValueType::Identifier,
                                                           Dependency::None,
                                                           FixedVector {ValueId::None, ValueId::Hover}};
    return schema;
  }

  const FeatureSchema &InvertedColors() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"inverted-colors", Discrete,
                                                           ValueType::Identifier, Dependency::Accessibility,
                                                           FixedVector {ValueId::None, ValueId::Inverted}};
    return schema;
  }

  const FeatureSchema &Monochrome() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"monochrome", Range, ValueType::Integer,
                                                           Dependency::Accessibility};
    return schema;
  }

  const FeatureSchema &Orientation() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"orientation", Discrete, ValueType::Identifier, Dependency::Viewport,
      FixedVector {ValueId::Landscape, ValueId::Portrait}};
    return schema;
  }

  const FeatureSchema &OverflowBlock() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"overflow-block", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::None, ValueId::Scroll, ValueId::Paged}};
    return schema;
  }

  const FeatureSchema &OverflowInline() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"overflow-inline", Discrete,
                                                           ValueType::Identifier, Dependency::None,
                                                           FixedVector {ValueId::None, ValueId::Scroll}};
    return schema;
  }

  const FeatureSchema &Pointer() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"pointer", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::None, ValueId::Fine, ValueId::Coarse}};
    return schema;
  }

  const FeatureSchema &PrefersContrast() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"prefers-contrast", Discrete, ValueType::Identifier, Dependency::Accessibility,
      FixedVector {ValueId::NoPreference, ValueId::More, ValueId::Less, ValueId::Custom}};
    return schema;
  }

  const FeatureSchema &PrefersReducedMotion() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"prefers-reduced-motion", Discrete, ValueType::Identifier, Dependency::Accessibility,
      FixedVector {ValueId::NoPreference, ValueId::Reduce}};
    return schema;
  }

  const FeatureSchema &Resolution() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"resolution", Range, ValueType::Resolution,
                                                           Dependency::None};
    return schema;
  }

  const FeatureSchema &Scan() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"scan", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::Interlace, ValueId::Progressive}};
    return schema;
  }

  const FeatureSchema &Scripting() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"scripting", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::None, ValueId::InitialOnly, ValueId::Enabled}};
    return schema;
  }

  const FeatureSchema &Update() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"update", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::None, ValueId::Slow, ValueId::Fast}};
    return schema;
  }

  const FeatureSchema &Width() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"width", Range, ValueType::Length,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &DisplayMode() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"display-mode", Discrete, ValueType::Identifier, Dependency::None,
      FixedVector {ValueId::Fullscreen, ValueId::Standalone, ValueId::MinimalUi, ValueId::Browser}};
    return schema;
  }

  const FeatureSchema &PrefersColorScheme() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"prefers-color-scheme", Discrete,
                                                           ValueType::Identifier, Dependency::Appearance,
                                                           FixedVector {ValueId::Light, ValueId::Dark}};
    return schema;
  }

  List<const FeatureSchema *> AllSchemas() noexcept
  {
    return {
      &AnyHover(),
      &AnyPointer(),
      &AspectRatio(),
      &Color(),
      &ColorGamut(),
      &ColorIndex(),
      &DeviceAspectRatio(),
      &DeviceHeight(),
      &DeviceWidth(),
      &DynamicRange(),
      &ForcedColors(),
      &Grid(),
      &Height(),
      &Hover(),
      &InvertedColors(),
      &Monochrome(),
      &OverflowBlock(),
      &OverflowInline(),
      &Orientation(),
      &Pointer(),
      &PrefersContrast(),
      &PrefersReducedMotion(),
      &Resolution(),
      &Scan(),
      &Scripting(),
      &Update(),
      &Width(),
      &DisplayMode(),
      &PrefersColorScheme(),
    };
  }
}
