#include "Krystal.Booey/Style/Values/Multicol/ColumnWidth.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Blending

  bool Blending<ColumnWidth>::CanBlend(const ColumnWidth &a, const ColumnWidth &b) noexcept
  {
    return !a.IsAuto() && !b.IsAuto();
  }

  ColumnWidth Blending<ColumnWidth>::Blend(const ColumnWidth &a, const ColumnWidth &b,
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
