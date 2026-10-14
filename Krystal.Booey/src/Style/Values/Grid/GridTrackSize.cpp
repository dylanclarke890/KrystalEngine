#include "Krystal.Booey/Style/Values/Grid/GridTrackSize.hpp"

#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperCSSValueConversion.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesSerialisation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // MARK: - Conversion

  GridTrackSize CSSValueConversion<GridTrackSize>::operator()(BuilderState &state,
                                                              const css::Value &value) noexcept
  {
    if (auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      return ConvertCSSValueToStyle<GridTrackSize::Breadth>(state, *primitiveValue);
    }

    auto function = RequiredListDowncast<css::FunctionValue, css::PrimitiveValue>(state, value);
    if (!function)
    {
      return GridTrackSize::Breadth {0_css_px};
    }

    if (function->size() == 1)
    {
      return GridTrackSize::FitContent {
        .Parameters = ConvertCSSValueToStyle<GridTrackFitContentLength>(state, function->item(0)),
      };
    }

    return GridTrackSize::MinMax {.Parameters = {
                                    .Min = ConvertCSSValueToStyle<GridTrackBreadth>(state, function->item(0)),
                                    .Max = ConvertCSSValueToStyle<GridTrackBreadth>(state, function->item(1)),
                                  }};
  }

  // MARK: - Blending

  GridTrackSize Blending<GridTrackSize>::Blend(const GridTrackSize &from, const GridTrackSize &to,
                                               const BlendingContext &context) noexcept
  {
    if (from.GetType() != to.GetType())
    {
      return context.Progress < 0.5 ? from : to;
    }

    switch (from.GetType())
    {
      case GridTrackSize::Type::Breadth:
        return style::Blend(from.MinTrackBreadth(), to.MinTrackBreadth(), context);

      case GridTrackSize::Type::FitContent:
        return GridTrackSize::FitContent {
          .Parameters = style::Blend(from.FitContentTrackLength(), to.FitContentTrackLength(), context),
        };

      case GridTrackSize::Type::MinMax:
        return GridTrackSize::MinMax {
          .Parameters = {
            .Min = style::Blend(from.MinTrackBreadth(), to.MinTrackBreadth(), context),
            .Max = style::Blend(from.MaxTrackBreadth(), to.MaxTrackBreadth(), context),
          }};
    }

    krys_unreachable();
  }
}
