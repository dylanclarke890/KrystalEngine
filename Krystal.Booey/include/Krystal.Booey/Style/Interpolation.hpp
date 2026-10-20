#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/IterationCompositeOperation.hpp"

namespace krys::boo
{
  class RenderStyle;
  struct Settings;

  namespace dom
  {
    class Document;
  }
}

namespace krys::boo::style::interpolation
{
  class Client;

  // MARK: - RenderStyle independent

  /// @brief Whether the property can ever be interpolated.
  /// @note For properties other than custom properties, the "animation-type" key in the properties' JSON
  /// decides.
  KRYS_NODISCARD bool CanInterpolate(const AnimatableCSSProperty &property) noexcept;

  /// @brief Whether the property supports additive or cumulative interpolation.
  /// @note For properties other than custom properties, the
  /// "animation-wrapper-requires-non-additive-or-cumulative-interpolation" key decides.
  KRYS_NODISCARD bool IsAdditiveOrCumulative(const AnimatableCSSProperty &property) noexcept;

  /// @brief Whether the property supports accelerated animation.
  /// @note For properties other than custom properties, the "animation-wrapper-acceleration" key decides.
  KRYS_NODISCARD bool IsAccelerated(const AnimatableCSSProperty &property, const Settings &settings) noexcept;

  // MARK: - RenderStyle dependent

  /// @brief Whether the property's values in @p a and @p b are equal.
  KRYS_NODISCARD bool Equals(const AnimatableCSSProperty &property, const RenderStyle &a,
                             const RenderStyle &b, const dom::Document &document) noexcept;

  /// @brief Whether the property can be interpolated between its values in @p a and @p b.
  KRYS_NODISCARD bool CanInterpolate(const AnimatableCSSProperty &property, const RenderStyle &a,
                                     const RenderStyle &b, const dom::Document &document) noexcept;

  /// @brief Interpolates the property from its value in @p from to its value in @p to into @p destination.
  void Interpolate(const AnimatableCSSProperty &property, RenderStyle &destination, const RenderStyle &from,
                   const RenderStyle &to, double progress, CompositeOperation compositeOperation,
                   IterationCompositeOperation iterationCompositeOperation, double currentIteration,
                   const Client &client) noexcept;
  void Interpolate(const AnimatableCSSProperty &property, RenderStyle &destination, const RenderStyle &from,
                   const RenderStyle &to, double progress, CompositeOperation compositeOperation,
                   const Client &client) noexcept;

  /// @brief Whether the property needs interpolating for accumulative iteration between its values in @p a
  /// and @p b.
  KRYS_NODISCARD bool RequiresInterpolationForAccumulativeIteration(const AnimatableCSSProperty &property,
                                                                    const RenderStyle &a,
                                                                    const RenderStyle &b,
                                                                    const Client &client) noexcept;
}
