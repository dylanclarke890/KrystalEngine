#include "Krystal.Booey/Style/Values/Motion/OffsetPath.hpp"

#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  OffsetPath CSSValueConversion<OffsetPath>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    return OffsetPath {
      ConvertCSSValueToStyle<RefPtr<PathOperation>>(state, value, SupportRayPathOperation::Yes)};
  }

  Ref<css::Value> CSSValueCreation<OffsetPath>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                           const OffsetPath &value) noexcept
  {
    return krys::SwitchOn(
      value, [&](const BasicShapePath &path)
      { return CreateCSSValue(pool, style, path, PathConversion::ForceAbsolute); },
      [&](const auto &path) { return CreateCSSValue(pool, style, path); });
  }

  // MARK: - Serialization

  void Serialiser<OffsetPath>::operator()(css::CSSOMString &builder, const css::SerialisationContext &context,
                                          const RenderStyle &style, const OffsetPath &value) noexcept
  {
    return krys::SwitchOn(
      value, [&](const BasicShapePath &path)
      { Serialise(builder, context, style, path, PathConversion::ForceAbsolute); },
      [&](const auto &path) { Serialise(builder, context, style, path); });
  }

  // MARK: - Blending

  bool Blending<OffsetPath>::CanBlend(const OffsetPath &a, const OffsetPath &b) noexcept
  {
    if (a.IsNone() || b.IsNone())
    {
      return false;
    }

    return a._operation->CanBlend(*b._operation);
  }

  OffsetPath Blending<OffsetPath>::Blend(const OffsetPath &a, const OffsetPath &b,
                                         const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1.0);
      return context.Progress ? b : a;
    }

    krys_debug_assert(CanBlend(a, b));
    return OffsetPath {a._operation->Blend(b._operation.get(), context)};
  }

  // MARK: - Platform

  RefPtr<PathOperation> ToPlatform<OffsetPath>::operator()(const OffsetPath &value) noexcept
  {
    return value._operation;
  }
}
