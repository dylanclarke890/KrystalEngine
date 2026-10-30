#include "Krystal.Booey/Style/Values/Break/Orphans.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Blending

  Orphans Blending<Orphans>::Blend(const Orphans &a, const Orphans &b,
                                   const BlendingContext &context) noexcept
  {
    return style::Blend(a.TryValue().value_or(2), b.TryValue().value_or(2), context);
  }
}
