#include "Krystal.Booey/Style/Values/Masking/ClipPath.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  ClipPath CSSValueConversion<ClipPath>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    return ClipPath {
      ConvertCSSValueToStyle<RefPtr<PathOperation>>(state, value, SupportRayPathOperation::No)};
  }

  // MARK: - Blending

  bool Blending<ClipPath>::CanBlend(const ClipPath &a, const ClipPath &b) noexcept
  {
    if (a.IsNone() || b.IsNone())
    {
      return false;
    }

    return a._operation->CanBlend(*b._operation);
  }

  ClipPath Blending<ClipPath>::Blend(const ClipPath &a, const ClipPath &b,
                                     const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return ClipPath {a._operation->Blend(b._operation.get(), context)};
  }
}
