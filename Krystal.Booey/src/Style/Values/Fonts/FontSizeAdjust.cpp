#include "Krystal.Booey/Style/Values/Fonts/FontSizeAdjust.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Properties/Consumers/Font.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  constexpr auto DefaultMetric = FontSizeAdjust::Metric::ExHeight;

  Maybe<float> FontSizeAdjust::ResolvedMetricValue(const RenderStyle &style) const noexcept
  {
    if (_platform.ShouldResolveFromFont())
    {
      return _platform.Resolve(style.ComputedFontSize(), style.MetricsOfPrimaryFont());
    }
    return _platform.Value;
  }

  // MARK: - Conversion

  FontSizeAdjust CSSValueConversion<FontSizeAdjust>::operator()(BuilderState &state,
                                                                const css::Value &value) noexcept
  {
    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (auto valueID = primitiveValue->ValueId(); valueID)
      {
        case css::ValueId::None: return css::keywords::None {};

        case css::ValueId::FromFont:
          // We cannot determine the primary font here, so we defer resolving the
          // aspect value for from-font to when the primary font is created.
          // See FontCascadeFonts::primaryFont().
          return {DefaultMetric, css::keywords::FromFont {}};

        case css::ValueId::Invalid:
          return {DefaultMetric, ConvertCSSValueToStyle<FontSizeAdjust::Number>(state, *primitiveValue)};

        default:
          if (css::PropertyParserHelpers::IsSystemFontShorthand(valueID))
          {
            return css::keywords::None {};
          }

          state.SetCurrentPropertyInvalidAtComputedValueTime();
          return css::keywords::None {};
      }
    }

    auto pair = RequiredPairDowncast<css::PrimitiveValue>(state, value);
    if (!pair)
    {
      return css::keywords::None {};
    }

    auto metric = FromValueId<FontSizeAdjust::Metric>(pair->first->ValueId());

    if (pair->second->ValueId() == css::ValueId::FromFont)
    {
      // We cannot determine the primary font here, so we defer resolving the
      // aspect value for from-font to when the primary font is created.
      // See FontCascadeFonts::primaryFont().
      return {metric, css::keywords::FromFont {}};
    }

    return {metric, ConvertCSSValueToStyle<FontSizeAdjust::Number>(state, *pair->second)};
  }

  Ref<css::Value> CSSValueCreation<FontSizeAdjust>::operator()(css::ValuePool &pool, const RenderStyle &style,
                                                               const FontSizeAdjust &value) noexcept
  {
    if (value.IsNone())
    {
      return CreateCSSValue(pool, style, css::keywords::None {});
    }

    auto metric = value.GetMetric();
    auto metricValue = value.ResolvedMetricValue(style);

    if (!metricValue)
    {
      return CreateCSSValue(pool, style, css::keywords::None {});
    }

    if (metric == DefaultMetric)
    {
      return CreateCSSValue(pool, style, FontSizeAdjust::Number {*metricValue});
    }

    return CreateCSSValue(pool, style,
                          css::SpaceSeparatedTuple {metric, FontSizeAdjust::Number {*metricValue}});
  }

  // MARK: - Serialization

  void Serialiser<FontSizeAdjust>::operator()(css::CSSOMString &builder,
                                              const css::SerialisationContext &context,
                                              const RenderStyle &style, const FontSizeAdjust &value) noexcept
  {
    if (value.IsNone())
    {
      Serialise(builder, context, style, css::keywords::None {});
      return;
    }

    auto metric = value.GetMetric();
    auto metricValue = value.ResolvedMetricValue(style);

    if (!metricValue)
    {
      Serialise(builder, context, style, css::keywords::None {});
      return;
    }

    if (metric == DefaultMetric)
    {
      Serialise(builder, context, style, FontSizeAdjust::Number {*metricValue});
      return;
    }

    Serialise(builder, context, style,
              css::SpaceSeparatedTuple {metric, FontSizeAdjust::Number {*metricValue}});
  }

  // MARK: Blending

  bool Blending<FontSizeAdjust>::CanBlend(const FontSizeAdjust &a, const FontSizeAdjust &b) noexcept
  {
    return a.GetMetric() == b.GetMetric() && a.MetricValue() && b.MetricValue();
  }

  FontSizeAdjust Blending<FontSizeAdjust>::Blend(const FontSizeAdjust &a, const FontSizeAdjust &b,
                                                 const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      return !context.Progress ? a : b;
    }

    krys_debug_assert(CanBlend(a, b));
    return {a.GetMetric(), style::Blend(FontSizeAdjust::Number {*a.MetricValue()},
                                        FontSizeAdjust::Number {*b.MetricValue()}, context)};
  }
}
