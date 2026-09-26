#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"

namespace krys::boo::css
{
  class ToLengthConversionData;
  enum class Category : uint8;

  namespace cq::Features
  {
    const mq::FeatureSchema &Width() noexcept;
    const mq::FeatureSchema &Height() noexcept;
    const mq::FeatureSchema &InlineSize() noexcept;
    const mq::FeatureSchema &BlockSize() noexcept;
    const mq::FeatureSchema &AspectRatio() noexcept;
    const mq::FeatureSchema &Orientation() noexcept;
    const mq::FeatureSchema &Style() noexcept;

    SmallList<const mq::FeatureSchema *> AllSchemas() noexcept;
  }
}
