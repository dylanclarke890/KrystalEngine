#include "Krystal.Booey/Style/Values/SVG/SVGPathData.hpp"

#include "Krystal.Booey/CSS/Values/PathValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  // MARK: - Conversion

  SVGPathData CSSValueConversion<SVGPathData>::operator()(BuilderState &state,
                                                          const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      if (primitiveValue->ValueId() == css::ValueId::None)
      {
        return css::keywords::None {};
      }

      state.SetCurrentPropertyInvalidAtComputedValueTime();
      return css::keywords::None {};
    }

    auto *pathValue = RequiredDowncast<css::PathValue>(state, value);
    if (!pathValue)
    {
      return css::keywords::None {};
    }

    return ConvertToStyle(pathValue->GetPath(), state);
  }

  Ref<css::Value> CSSValueCreation<SVGPathData>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                            const SVGPathData &value) noexcept
  {
    if (value.IsNone())
    {
      return CreateCSSValue(pool, style, css::keywords::None {});
    }
    return css::PathValue::Create(ConvertToCSS(*value.TryPath(), style, PathConversion::ForceAbsolute));
  }

  // MARK: - Serialization

  void Serialiser<SVGPathData>::operator()(css::CSSOMString &builder,
                                           const css::SerialisationContext &context, const RenderStyle &style,
                                           const SVGPathData &value) noexcept
  {
    if (value.IsNone())
    {
      Serialise(builder, context, style, css::keywords::None {});
      return;
    }
    Serialise(builder, context, style, *value.TryPath(), PathConversion::ForceAbsolute);
  }

  // MARK: - Blending

  bool Blending<SVGPathData>::CanBlend(const SVGPathData &a, const SVGPathData &b) noexcept
  {
    auto aPath = a.TryPath();
    auto bPath = b.TryPath();
    return aPath && bPath && style::CanBlend(*aPath, *bPath);
  }

  SVGPathData Blending<SVGPathData>::Blend(const SVGPathData &a, const SVGPathData &b,
                                           const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      return context.Progress < 0.5 ? a : b;
    }

    krys_debug_assert(style::CanBlend(a, b));
    return style::Blend(*a.TryPath(), *b.TryPath(), context);
  }
}
