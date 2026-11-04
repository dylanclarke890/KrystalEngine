#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo::style::interpolation
{
  class Client;

  /// @brief The blending context of one property's interpolation, with the animation it's for.
  struct Context : BlendingContext
  {
    const interpolation::Client &Client;
    AnimatableCSSProperty Property;

    Context(const AnimatableCSSProperty &property, double progress, bool isDiscrete,
            boo::CompositeOperation compositeOperation,
            boo::IterationCompositeOperation iterationCompositeOperation, double currentIteration,
            boo::Color fromColor, boo::Color toColor, const interpolation::Client &client) noexcept
        : BlendingContext(progress, isDiscrete, compositeOperation, iterationCompositeOperation,
                          currentIteration, krys::move(fromColor), krys::move(toColor)),
          Client(client), Property(property)
    {
    }
  };
}
