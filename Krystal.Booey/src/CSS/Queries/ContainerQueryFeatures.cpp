#include "Krystal.Booey/CSS/Queries/ContainerQueryFeatures.hpp"
#include "Krystal.Core/Utils/NeverDestroyed.hpp"

// FIXME: The schemas don't evaluate yet (Evaluate() returns Unknown). The size features evaluate against the
// container's principal box and style() against its computed custom properties; they come with container
// query evaluation.

namespace krys::boo::css::cq::Features
{
  using mq::FeatureSchema;
  using enum mq::FeatureSchema::Type;
  using ValueType = mq::FeatureSchema::ValueType;
  using Dependency = mq::MediaQueryDynamicDependency;

  const FeatureSchema &Width() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"width", Range, ValueType::Length,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &Height() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"height", Range, ValueType::Length,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &InlineSize() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"inline-size", Range, ValueType::Length,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &BlockSize() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"block-size", Range, ValueType::Length,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &AspectRatio() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"aspect-ratio", Range, ValueType::Ratio,
                                                           Dependency::Viewport};
    return schema;
  }

  const FeatureSchema &Orientation() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {
      u8"orientation", Discrete, ValueType::Identifier, Dependency::Viewport,
      FixedVector {ValueId::Portrait, ValueId::Landscape}};
    return schema;
  }

  const FeatureSchema &Style() noexcept
  {
    static MainThreadNeverDestroyed<FeatureSchema> schema {u8"style", Discrete, ValueType::CustomProperty,
                                                           Dependency::None};
    return schema;
  }

  List<const FeatureSchema *> AllSchemas() noexcept
  {
    return {
      &Width(), &Height(), &InlineSize(), &BlockSize(), &AspectRatio(), &Orientation(),
    };
  }
}
