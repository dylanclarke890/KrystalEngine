#include "Krystal.Booey/Style/Values/SVG/SVGPaint.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/UrlValue.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Style/BuilderChecking.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/ForVisitedLink.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD bool ContainsCurrentColor(const style::SVGPaint &paint) noexcept
  {
    if (auto color = paint.TryAnyColor())
    {
      return color->ContainsCurrentColor();
    }
    return false;
  }

  // MARK: - Conversion

  SVGPaint CSSValueConversion<SVGPaint>::operator()(BuilderState &state, const css::Value &value,
                                                    ForVisitedLink forVisitedLink) noexcept
  {
    if (const auto *list = DynamicDowncast<css::ValueList>(value))
    {
      const auto *firstValue = list->Item(0);
      auto *urlValue = RequiredDowncast<css::UrlValue>(state, *firstValue);
      if (!urlValue)
      {
        return css::keywords::None {};
      }

      auto url = ConvertToStyle(urlValue->URL(), state);

      if (list->size() == 1)
      {
        return url;
      }

      const auto *secondItem = list->Item(1);
      if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(secondItem))
      {
        switch (primitiveValue->ValueId())
        {
          case css::ValueId::None: return SVGPaint::URLNone {url, css::keywords::None {}};

          case css::ValueId::Currentcolor:
            state.Style().SetDisallowsFastPathInheritance();
            return SVGPaint::URLColor {url, Color::GetCurrentColor()};

          default: break;
        }
      }

      return SVGPaint::URLColor {url, ConvertCSSValueToStyle<Color>(state, *list->Item(1), forVisitedLink)};
    }

    if (const auto *urlValue = DynamicDowncast<css::UrlValue>(value))
    {
      return ConvertToStyle(urlValue->URL(), state);
    }

    if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(value))
    {
      switch (primitiveValue->ValueId())
      {
        case css::ValueId::None: return css::keywords::None {};

        case css::ValueId::Currentcolor:
          state.Style().SetDisallowsFastPathInheritance();
          return Color {Color::GetCurrentColor()};

        default: break;
      }
    }

    return ConvertCSSValueToStyle<Color>(state, value, forVisitedLink);
  }

  // MARK: - Blending

  bool Blending<SVGPaint>::Equals(const SVGPaint &a, const SVGPaint &b, const RenderStyle &aStyle,
                                  const RenderStyle &bStyle) noexcept
  {
    if (!a.HasSameType(b))
    {
      return false;
    }

    // We only support animations between SVGPaints that are pure Color values.
    // For everything else we must return true for this method, otherwise
    // we will try to animate between values forever.

    if (a.IsColor())
    {
      return EqualsForBlending(a.ColorDisregardingType(), b.ColorDisregardingType(), aStyle, bStyle);
    }

    return true;
  }

  bool Blending<SVGPaint>::CanBlend(const SVGPaint &a, const SVGPaint &b) noexcept
  {
    if (!a.IsColor() || !b.IsColor())
    {
      return false;
    }
    return style::CanBlend(a.ColorDisregardingType(), b.ColorDisregardingType());
  }

  SVGPaint Blending<SVGPaint>::Blend(const SVGPaint &a, const SVGPaint &b, const RenderStyle &aStyle,
                                     const RenderStyle &bStyle, const BlendingContext &context) noexcept
  {
    if (context.IsDiscrete)
    {
      krys_debug_assert(!context.Progress || context.Progress == 1);
      return context.Progress ? b : a;
    }
    return style::Blend(a.ColorDisregardingType(), b.ColorDisregardingType(), aStyle, bStyle, context);
  }
}
