#include "Krystal.Booey/Style/Values/Multicol/ColumnCount.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Blending

  bool Blending<ColumnCount>::CanBlend(const ColumnCount &a, const ColumnCount &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto();
  }

  ColumnCount Blending<ColumnCount>::Blend(const ColumnCount &a, const ColumnCount &b,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    krys_debug_assert(!a.IsAuto());
    krys_debug_assert(!b.IsAuto());
    return {krys::boo::style::Blend(*a.TryValue(), *b.TryValue(), context)};
  }
}
