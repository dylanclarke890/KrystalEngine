#include "Krystal.Booey/Style/Values/Motion/OffsetAnchor.hpp"

#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  OffsetAnchor CSSValueConversion<OffsetAnchor>::operator()(BuilderState &state,
                                                            const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::Auto)
    {
      return OffsetAnchor {css::keywords::Auto {}};
    }

    auto *positionValue = RequiredDowncast<css::PositionValue>(state, value);
    if (!positionValue)
    {
      return OffsetAnchor {css::keywords::Auto {}};
    }
    return OffsetAnchor {ConvertToStyle(positionValue->GetPosition(), state)};
  }

  // MARK: - Blending

  bool Blending<OffsetAnchor>::CanBlend(const OffsetAnchor &a, const OffsetAnchor &b) noexcept
  {
    return a.IsPosition() && b.IsPosition();
  }

  bool Blending<OffsetAnchor>::RequiresInterpolationForAccumulativeIteration(const OffsetAnchor &a,
                                                                             const OffsetAnchor &b) noexcept
  {
    krys_debug_assert(CanBlend(a, b));
    return style::RequiresInterpolationForAccumulativeIteration(*a.TryPosition(), *b.TryPosition());
  }

  OffsetAnchor Blending<OffsetAnchor>::Blend(const OffsetAnchor &a, const OffsetAnchor &b,
                                             const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return OffsetAnchor {style::Blend(*a.TryPosition(), *b.TryPosition(), context)};
  }

  // MARK: - Evaluation
}
