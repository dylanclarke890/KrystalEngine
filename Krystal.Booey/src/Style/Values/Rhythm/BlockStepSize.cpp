#include "Krystal.Booey/Style/Values/Rhythm/BlockStepSize.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Blending

  bool Blending<BlockStepSize>::CanBlend(const BlockStepSize &a, const BlockStepSize &b) noexcept
  {
    return !a.IsNone() && !b.IsNone();
  }

  bool Blending<BlockStepSize>::RequiresInterpolationForAccumulativeIteration(const BlockStepSize &a,
                                                                              const BlockStepSize &b) noexcept
  {
    return a.IsNone() != b.IsNone();
  }

  BlockStepSize Blending<BlockStepSize>::Blend(const BlockStepSize &a, const BlockStepSize &b,
                                               const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(!a.IsNone() && !b.IsNone());
    return style::Blend(*a.TryValue(), *b.TryValue(), context);
  }
}
