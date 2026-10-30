#include "Krystal.Booey/Style/Values/Break/Widows.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Blending

  Widows Blending<Widows>::Blend(const Widows &a, const Widows &b, const BlendingContext &context) noexcept
  {
    return style::Blend(a.TryValue().value_or(2), b.TryValue().value_or(2), context);
  }
}
