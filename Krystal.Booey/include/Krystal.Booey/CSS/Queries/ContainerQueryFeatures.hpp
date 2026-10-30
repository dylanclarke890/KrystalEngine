#pragma once

#include "Krystal.Booey/CSS/Queries/GenericMediaQueryTypes.hpp"

namespace krys::boo::css::cq::Features
{
  KRYS_NODISCARD const mq::FeatureSchema &Width() noexcept;
  KRYS_NODISCARD const mq::FeatureSchema &Height() noexcept;
  KRYS_NODISCARD const mq::FeatureSchema &InlineSize() noexcept;
  KRYS_NODISCARD const mq::FeatureSchema &BlockSize() noexcept;
  KRYS_NODISCARD const mq::FeatureSchema &AspectRatio() noexcept;
  KRYS_NODISCARD const mq::FeatureSchema &Orientation() noexcept;
  KRYS_NODISCARD const mq::FeatureSchema &Style() noexcept;

  /// @brief The size features. `style()` isn't looked up by name, so it isn't included.
  KRYS_NODISCARD List<const mq::FeatureSchema *> AllSchemas() noexcept;
}
