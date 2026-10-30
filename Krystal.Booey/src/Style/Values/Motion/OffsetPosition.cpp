#include "Krystal.Booey/Style/Values/Motion/OffsetPosition.hpp"

#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  OffsetPosition CSSValueConversion<OffsetPosition>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (value.ValueId() == css::ValueId::Auto)
    {
      return OffsetPosition {css::keywords::Auto {}};
    }
    if (value.ValueId() == css::ValueId::Normal)
    {
      return OffsetPosition {css::keywords::Normal {}};
    }

    auto *positionValue = RequiredDowncast<css::PositionValue>(state, value);
    if (!positionValue)
    {
      return OffsetPosition {css::keywords::Normal {}};
    }
    return OffsetPosition {ConvertToStyle(positionValue->GetPosition(), state)};
  }

  // MARK: - Blending

  bool Blending<OffsetPosition>::CanBlend(const OffsetPosition &a, const OffsetPosition &b) noexcept
  {
    return a.IsPosition() && b.IsPosition();
  }

  bool
    Blending<OffsetPosition>::RequiresInterpolationForAccumulativeIteration(const OffsetPosition &a,
                                                                            const OffsetPosition &b) noexcept
  {
    krys_debug_assert(CanBlend(a, b));
    return style::RequiresInterpolationForAccumulativeIteration(*a.TryPosition(), *b.TryPosition());
  }

  OffsetPosition Blending<OffsetPosition>::Blend(const OffsetPosition &a, const OffsetPosition &b,
                                                 const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return OffsetPosition {style::Blend(*a.TryPosition(), *b.TryPosition(), context)};
  }

  // MARK: - Evaluation
}
