#include "Krystal.Booey/Style/Values/Backgrounds/BorderImageOutset.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/QuadValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  KRYS_NODISCARD static BorderImageOutsetValue ConvertBorderImageOutsetValue(BuilderState &state,
                                                                             const css::Value &value) noexcept
  {
    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return BorderImageOutsetValue {0_css_number};
    }

    if (primitiveValue->IsNumber())
    {
      return ConvertCSSValueToStyle<BorderImageOutsetValue::Number>(state, *primitiveValue);
    }
    return ConvertCSSValueToStyle<BorderImageOutsetValue::Length>(state, *primitiveValue);
  }

  BorderImageOutset CSSValueConversion<BorderImageOutset>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    if (auto *quadValue = DynamicDowncast<css::QuadValue>(value))
    {
      auto &quad = quadValue->GetQuad();
      return BorderImageOutset {
        ConvertBorderImageOutsetValue(state, quad.Top()),
        ConvertBorderImageOutsetValue(state, quad.Right()),
        ConvertBorderImageOutsetValue(state, quad.Bottom()),
        ConvertBorderImageOutsetValue(state, quad.Left()),
      };
    }

    // Values coming from CSS Typed OM may not have been converted to a Quad.
    return ConvertBorderImageOutsetValue(state, value);
  }

  Ref<css::Value> CSSValueCreation<BorderImageOutset>::operator()(css::ValuePool &pool,
                                                                  const RenderStyle &style,
                                                                  const BorderImageOutset &value) noexcept
  {
    return css::QuadValue::Create({
      CreateCSSValue(pool, style, value.Values.Top()),
      CreateCSSValue(pool, style, value.Values.Right()),
      CreateCSSValue(pool, style, value.Values.Bottom()),
      CreateCSSValue(pool, style, value.Values.Left()),
    });
  }

  // MARK: - Blending

  inline bool Blending<BorderImageOutsetValue>::CanBlend(const BorderImageOutsetValue &a,
                                                         const BorderImageOutsetValue &b) noexcept
  {
    return a.HasSameType(b);
  }

  inline bool Blending<BorderImageOutsetValue>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageOutsetValue &a, const BorderImageOutsetValue &b) noexcept
  {
    return !a.HasSameType(b);
  }

  inline BorderImageOutsetValue
    Blending<BorderImageOutsetValue>::Blend(const BorderImageOutsetValue &a, const BorderImageOutsetValue &b,
                                            const BlendingContext &context) noexcept
  {
    if (!a.HasSameType(b))
    {
      return BorderImageOutsetValue {0_css_px};
    }

    if (!context.Progress && context.IsReplace())
    {
      return a;
    }
    if (context.Progress == 1 && context.IsReplace())
    {
      return b;
    }

    return std::visit(krys::CreateVisitor([&]<typename T>(const T &a, const T &b) -> BorderImageOutsetValue
                                          { return style::Blend(a, b, context); },
                                          [&](const auto &, const auto &) -> BorderImageOutsetValue
                                          { krys_unreachable(); }),
                      a._value, b._value);
  }

  bool Blending<BorderImageOutset>::CanBlend(const BorderImageOutset &a, const BorderImageOutset &b) noexcept
  {
    return style::CanBlend(a.Values.Top(), b.Values.Top())
           && style::CanBlend(a.Values.Right(), b.Values.Right())
           && style::CanBlend(a.Values.Bottom(), b.Values.Bottom())
           && style::CanBlend(a.Values.Left(), b.Values.Left());
  }

  bool Blending<BorderImageOutset>::RequiresInterpolationForAccumulativeIteration(
    const BorderImageOutset &a, const BorderImageOutset &b) noexcept
  {
    return style::RequiresInterpolationForAccumulativeIteration(a.Values.Top(), b.Values.Top())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Right(), b.Values.Right())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Bottom(), b.Values.Bottom())
           && style::RequiresInterpolationForAccumulativeIteration(a.Values.Left(), b.Values.Left());
  }

  BorderImageOutset Blending<BorderImageOutset>::Blend(const BorderImageOutset &a, const BorderImageOutset &b,
                                                       const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }

    return BorderImageOutset {
      style::Blend(a.Values.Top(), b.Values.Top(), context),
      style::Blend(a.Values.Right(), b.Values.Right(), context),
      style::Blend(a.Values.Bottom(), b.Values.Bottom(), context),
      style::Blend(a.Values.Left(), b.Values.Left(), context),
    };
  }
}
