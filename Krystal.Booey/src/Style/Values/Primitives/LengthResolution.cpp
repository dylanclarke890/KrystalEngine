#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Booey/Core/BoxSides.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Units.hpp"
#include "Krystal.Booey/CSS/Values/Units.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/ContainerQueryEvaluator.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include "Krystal.Booey/Style/SeamRenderers.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include <algorithm>

namespace krys::boo::style
{
  static double AdjustValueForPageZoom(double dimension,
                                       const css::ToLengthConversionData &conversionData) noexcept
  {
    if (conversionData.RangeZoomOption() != css::RangeZoomOptions::Unzoomed)
    {
      return dimension;
    }

    auto *style = conversionData.Style();
    if (!style || !EvaluationTimeZoomEnabled(*style))
    {
      return dimension;
    }

    return dimension / conversionData.GetRenderView()->ZoomFactor();
  }

  static double LengthOfViewportPhysicalAxisForLogicalAxis(LogicalBoxAxis logicalAxis, const FloatSize &size,
                                                           const RenderStyle *style) noexcept
  {
    if (!style)
    {
      return 0;
    }

    switch (MapAxisLogicalToPhysical(style->GetWritingMode(), logicalAxis))
    {
      case BoxAxis::Horizontal: return size.Width();
      case BoxAxis::Vertical:   return size.Height();
    }

    krys_unreachable();
  }

  static double LengthOfViewportPhysicalAxisForLogicalAxis(LogicalBoxAxis logicalAxis, const FloatSize &size,
                                                           const RenderView &renderView) noexcept
  {
    auto rootElement = renderView.GetDocument().DocumentElement();
    if (!rootElement)
    {
      return 0;
    }

    return LengthOfViewportPhysicalAxisForLogicalAxis(logicalAxis, size, rootElement->GetRenderStyle());
  }

  double ComputeUnzoomedNonCalcLengthDouble(double value, css::LengthUnit lengthUnit,
                                            css::PropertyId propertyToCompute,
                                            const FontCascade *fontCascadeForUnit,
                                            css::RangeZoomOptions rangeZoomOption,
                                            const RenderView *renderView) noexcept
  {
    using enum css::LengthUnit;

    switch (lengthUnit)
    {
      case Px: return value;
      case Cm: return css::pxPerCm * value;
      case Mm: return css::pxPerMm * value;
      case Q:  return css::pxPerQ * value;
      case In: return css::pxPerInch * value;
      case Pt: return css::pxPerPt * value;
      case Pc:
        return css::pxPerPc * value;

        // MARK: "font dependent" and "root font dependent" resolution

      case Em:
      case QuirkyEm:
      case Rem:
      {
        krys_debug_assert(fontCascadeForUnit);
        auto &fontDescription = fontCascadeForUnit->GetFontDescription();
        return ((propertyToCompute == css::PropertyId::FontSize)
                  ? fontDescription.SpecifiedSize()
                  : fontDescription.ComputedSizeForRangeZoomOption(rangeZoomOption))
               * value;
      }
      case Ex:
      case Rex:
      {
        krys_debug_assert(fontCascadeForUnit);
        auto &fontMetrics = fontCascadeForUnit->MetricsOfPrimaryFont();
        if (fontMetrics.XHeight())
        {
          return fontMetrics.XHeight().value() * value;
        }
        auto &fontDescription = fontCascadeForUnit->GetFontDescription();
        return ((propertyToCompute == css::PropertyId::FontSize)
                  ? fontDescription.SpecifiedSize()
                  : fontDescription.ComputedSizeForRangeZoomOption(rangeZoomOption))
               / 2.0 * value;
      }
      case Cap:
      case Rcap:
      {
        krys_debug_assert(fontCascadeForUnit);
        auto &fontMetrics = fontCascadeForUnit->MetricsOfPrimaryFont();
        if (fontMetrics.CapHeight())
        {
          return fontMetrics.CapHeight().value() * value;
        }
        return fontMetrics.IntAscent() * value;
      }
      case Ch:
      case Rch:
      {
        krys_debug_assert(fontCascadeForUnit);
        return fontCascadeForUnit->ZeroWidth() * value;
      }
      case Ic:
      case Ric:
      {
        krys_debug_assert(fontCascadeForUnit);
        return fontCascadeForUnit->MetricsOfPrimaryFont().IdeogramWidth().value_or(0) * value;
      }

        // MARK: "viewport percentage" resolution

      case Vh: return renderView ? renderView->SizeForCSSDefaultViewportUnits().Height() / 100.0 * value : 0;
      case Vw: return renderView ? renderView->SizeForCSSDefaultViewportUnits().Width() / 100.0 * value : 0;
      case Vmax:
        return renderView ? renderView->SizeForCSSDefaultViewportUnits().MaxDimension() / 100.0 * value
                          : value;
      case Vmin:
        return renderView ? renderView->SizeForCSSDefaultViewportUnits().MinDimension() / 100.0 * value
                          : value;
      case Vb:
        return renderView
                 ? LengthOfViewportPhysicalAxisForLogicalAxis(
                     LogicalBoxAxis::Block, renderView->SizeForCSSDefaultViewportUnits(), *renderView)
                     / 100.0 * value
                 : 0;
      case Vi:
        return renderView
                 ? LengthOfViewportPhysicalAxisForLogicalAxis(
                     LogicalBoxAxis::Inline, renderView->SizeForCSSDefaultViewportUnits(), *renderView)
                     / 100.0 * value
                 : 0;
      case Svh: return renderView ? renderView->SizeForCSSSmallViewportUnits().Height() / 100.0 * value : 0;
      case Svw: return renderView ? renderView->SizeForCSSSmallViewportUnits().Width() / 100.0 * value : 0;
      case Svmax:
        return renderView ? renderView->SizeForCSSSmallViewportUnits().MaxDimension() / 100.0 * value : value;
      case Svmin:
        return renderView ? renderView->SizeForCSSSmallViewportUnits().MinDimension() / 100.0 * value : value;
      case Svb:
        return renderView ? LengthOfViewportPhysicalAxisForLogicalAxis(
                              LogicalBoxAxis::Block, renderView->SizeForCSSSmallViewportUnits(), *renderView)
                              / 100.0 * value
                          : 0;
      case Svi:
        return renderView ? LengthOfViewportPhysicalAxisForLogicalAxis(
                              LogicalBoxAxis::Inline, renderView->SizeForCSSSmallViewportUnits(), *renderView)
                              / 100.0 * value
                          : 0;
      case Lvh: return renderView ? renderView->SizeForCSSLargeViewportUnits().Height() / 100.0 * value : 0;
      case Lvw: return renderView ? renderView->SizeForCSSLargeViewportUnits().Width() / 100.0 * value : 0;
      case Lvmax:
        return renderView ? renderView->SizeForCSSLargeViewportUnits().MaxDimension() / 100.0 * value : value;
      case Lvmin:
        return renderView ? renderView->SizeForCSSLargeViewportUnits().MinDimension() / 100.0 * value : value;
      case Lvb:
        return renderView ? LengthOfViewportPhysicalAxisForLogicalAxis(
                              LogicalBoxAxis::Block, renderView->SizeForCSSLargeViewportUnits(), *renderView)
                              / 100.0 * value
                          : 0;
      case Lvi:
        return renderView ? LengthOfViewportPhysicalAxisForLogicalAxis(
                              LogicalBoxAxis::Inline, renderView->SizeForCSSLargeViewportUnits(), *renderView)
                              / 100.0 * value
                          : 0;
      case Dvh: return renderView ? renderView->SizeForCSSDynamicViewportUnits().Height() / 100.0 * value : 0;
      case Dvw: return renderView ? renderView->SizeForCSSDynamicViewportUnits().Width() / 100.0 * value : 0;
      case Dvmax:
        return renderView ? renderView->SizeForCSSDynamicViewportUnits().MaxDimension() / 100.0 * value
                          : value;
      case Dvmin:
        return renderView ? renderView->SizeForCSSDynamicViewportUnits().MinDimension() / 100.0 * value
                          : value;
      case Dvb:
        return renderView
                 ? LengthOfViewportPhysicalAxisForLogicalAxis(
                     LogicalBoxAxis::Block, renderView->SizeForCSSDynamicViewportUnits(), *renderView)
                     / 100.0 * value
                 : 0;
      case Dvi:
        return renderView
                 ? LengthOfViewportPhysicalAxisForLogicalAxis(
                     LogicalBoxAxis::Inline, renderView->SizeForCSSDynamicViewportUnits(), *renderView)
                     / 100.0 * value
                 : 0;

      case Lh:
      case Rlh:
      case Cqw:
      case Cqh:
      case Cqi:
      case Cqb:
      case Cqmin:
      case Cqmax:
      {
        krys_debug_assert_msg(false, "Line height and container-percentage units can't be resolved here.");
        return -1.0;
      }
    }

    krys_unreachable();
  }

  static double
    AdjustZoomStateForFontRelativeUnitsIfNeeded(double value,
                                                const css::ToLengthConversionData &conversionData) noexcept
  {
    // Apply text zoom for font-relative units when evaluationTimeZoomEnabled with unzoomed range option.
    // computedSizeForRangeZoomOption returns unzoomed font size when rangeZoomOption is Unzoomed,
    // so we need to multiply by text zoom to get the correct final value.
    // We explicitly use zoomWithTextZoomFactor() here instead of conversionData.zoom() because
    // zoomWithTextZoomFactor() is guaranteed to return only the text zoom factor (not page zoom or other zoom
    // types) when evaluationTimeZoomEnabled is true. This ensures font-relative units scale correctly with
    // text zoom while remaining independent of other zoom mechanisms.
    if (conversionData.EvaluationTimeZoomEnabled()
        && conversionData.RangeZoomOption() == css::RangeZoomOptions::Unzoomed)
    {
      if (auto builderState = conversionData.ProtectedStyleBuilderState())
      {
        auto textZoom = builderState->ZoomWithTextZoomFactor();
        return value * textZoom;
      }
    }
    return value;
  }

  double ComputeCanonicalNonCalcLengthDouble(double value, css::LengthUnit lengthUnit,
                                             const css::ToLengthConversionData &conversionData) noexcept
  {
    // We are only interested in canonicalizing to `px`, not adjusting for zoom, which will be handled later.
    // When computing font-size, zoom is not applied in the same way, so must be special cased here.
    auto computedValue = ComputeNonCalcLengthDouble(value, lengthUnit, conversionData);
    if (conversionData.ComputingFontSize()
        || (conversionData.EvaluationTimeZoomEnabled()
            && conversionData.RangeZoomOption() == css::RangeZoomOptions::Unzoomed))
    {
      return computedValue;
    }

    return computedValue / conversionData.Style()->UsedZoom();
  }

  double ComputeNonCalcLengthDouble(double value, css::LengthUnit lengthUnit,
                                    const css::ToLengthConversionData &conversionData) noexcept
  {
    using enum css::LengthUnit;

    auto resolveContainerUnit = [&](css::cq::Axis physicalAxis) -> Maybe<double>
    {
      krys_debug_assert(physicalAxis == css::cq::Axis::Width || physicalAxis == css::cq::Axis::Height);

      conversionData.SetUsesContainerUnits();

      auto element = ShareRefPtr(conversionData.ElementForContainerUnitResolution());
      if (!element)
      {
        return null;
      }

      auto mode = !conversionData.Style()->GetPseudoElementType()
                    ? ContainerQueryEvaluator::SelectionMode::Element
                    : ContainerQueryEvaluator::SelectionMode::PseudoElement;

      // "The query container for each axis is the nearest ancestor container that accepts container size
      // queries on that axis."
      while ((element = ContainerQueryEvaluator::SelectContainer(physicalAxis, {}, *element, mode)))
      {
        auto *containerRenderer = RenderBoxOf(*element);
        if (containerRenderer && containerRenderer->HasEligibleContainmentForSizeQuery())
        {
          auto widthOrHeight = physicalAxis == css::cq::Axis::Width ? containerRenderer->ContentBoxWidth()
                                                                    : containerRenderer->ContentBoxHeight();
          return widthOrHeight * value / 100;
        }
        // For pseudo-elements the element itself can be the container. Avoid looping forever.
        mode = ContainerQueryEvaluator::SelectionMode::Element;
      }
      return null;
    };

    switch (lengthUnit)
    {
      case Px: break;
      case Cm: value = css::pxPerCm * value; break;
      case Mm: value = css::pxPerMm * value; break;
      case Q:  value = css::pxPerQ * value; break;
      case In: value = css::pxPerInch * value; break;
      case Pt: value = css::pxPerPt * value; break;
      case Pc:
        value = css::pxPerPc * value;
        break;

        // MARK: "font dependent" resolution

      case Em:
      case QuirkyEm:
      case Ex:
      case Cap:
      case Ch:
      case Ic:
      {
        // FIXME: We have a bug right now where the zoom will be applied twice to EX units.
        // We really need to compute EX using fontMetrics for the original specifiedSize and not use
        // our actual constructed rendering font.
        value = ComputeUnzoomedNonCalcLengthDouble(value, lengthUnit, conversionData.PropertyToCompute(),
                                                   &conversionData.FontCascadeForFontUnits(),
                                                   conversionData.RangeZoomOption());
        value = AdjustZoomStateForFontRelativeUnitsIfNeeded(value, conversionData);
        break;
      }

      case Lh:
      {
        if (conversionData.ComputingLineHeight() || conversionData.ComputingFontSize())
        {
          // Try to get the parent's computed line-height, or fall back to the initial line-height of this
          // element's font spacing.
          value *= conversionData.ParentStyle()
                     ? conversionData.ParentStyle()->ComputedLineHeight()
                     : static_cast<float>(
                         conversionData.FontCascadeForFontUnits().MetricsOfPrimaryFont().IntLineSpacing());
        }
        else if (auto fixedLineHeight = conversionData.Style()->GetLineHeight().TryFixed())
        {
          // We can't use computedLineHeightForFontUnits if the line height is fixed since
          // that will apply the usedZoomFactor. We probably should refactor it so that
          // does not happen and we don't have to special case this scenario.
          value *=
            Evaluate<LayoutUnit>(*fixedLineHeight, ZoomFactor {conversionData.Zoom(),
                                                               conversionData.Style()->DeviceScaleFactor()})
              .ToFloat();
        }
        else
        {
          value *= conversionData.ComputedLineHeightForFontUnits();
        }
        break;
      }

        // MARK: "root font dependent" resolution

      case Rcap:
      case Rch:
      case Rem:
      case Rex:
      case Ric:
      {
        value = ComputeUnzoomedNonCalcLengthDouble(value, lengthUnit, conversionData.PropertyToCompute(),
                                                   conversionData.RootStyle()
                                                     ? &conversionData.RootStyle()->GetFontCascade()
                                                     : &conversionData.FontCascadeForFontUnits(),
                                                   conversionData.RangeZoomOption());
        value = AdjustZoomStateForFontRelativeUnitsIfNeeded(value, conversionData);
        break;
      }

      case Rlh:
      {
        if (conversionData.RootStyle())
        {
          if (conversionData.ComputingLineHeight() || conversionData.ComputingFontSize())
          {
            value *= conversionData.RootStyle()->ComputeLineHeight(
              conversionData.RootStyle()->SpecifiedLineHeight());
          }
          else
          {
            value *= conversionData.RootStyle()->ComputedLineHeight();
          }
        }
        break;
      }

        // MARK: "viewport-percentage" resolution

      case Vh:
        return value
               * AdjustValueForPageZoom(conversionData.DefaultViewportFactor().Height(), conversionData);

      case Vw:
        return value * AdjustValueForPageZoom(conversionData.DefaultViewportFactor().Width(), conversionData);

      case Vmax:
        return value
               * AdjustValueForPageZoom(conversionData.DefaultViewportFactor().MaxDimension(),
                                        conversionData);

      case Vmin:
        return value
               * AdjustValueForPageZoom(conversionData.DefaultViewportFactor().MinDimension(),
                                        conversionData);

      case Vb:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Block, conversionData.DefaultViewportFactor(), conversionData.Style()),
                 conversionData);

      case Vi:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Inline, conversionData.DefaultViewportFactor(), conversionData.Style()),
                 conversionData);

      case Svh:
        return value * AdjustValueForPageZoom(conversionData.SmallViewportFactor().Height(), conversionData);

      case Svw:
        return value * AdjustValueForPageZoom(conversionData.SmallViewportFactor().Width(), conversionData);

      case Svmax:
        return value
               * AdjustValueForPageZoom(conversionData.SmallViewportFactor().MaxDimension(), conversionData);

      case Svmin:
        return value
               * AdjustValueForPageZoom(conversionData.SmallViewportFactor().MinDimension(), conversionData);

      case Svb:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Block, conversionData.SmallViewportFactor(), conversionData.Style()),
                 conversionData);

      case Svi:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Inline, conversionData.SmallViewportFactor(), conversionData.Style()),
                 conversionData);

      case Lvh:
        return value * AdjustValueForPageZoom(conversionData.LargeViewportFactor().Height(), conversionData);

      case Lvw:
        return value * AdjustValueForPageZoom(conversionData.LargeViewportFactor().Width(), conversionData);

      case Lvmax:
        return value
               * AdjustValueForPageZoom(conversionData.LargeViewportFactor().MaxDimension(), conversionData);

      case Lvmin:
        return value
               * AdjustValueForPageZoom(conversionData.LargeViewportFactor().MinDimension(), conversionData);

      case Lvb:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Block, conversionData.LargeViewportFactor(), conversionData.Style()),
                 conversionData);

      case Lvi:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Inline, conversionData.LargeViewportFactor(), conversionData.Style()),
                 conversionData);

      case Dvh:
        return value
               * AdjustValueForPageZoom(conversionData.DynamicViewportFactor().Height(), conversionData);

      case Dvw:
        return value * AdjustValueForPageZoom(conversionData.DynamicViewportFactor().Width(), conversionData);

      case Dvmax:
        return value
               * AdjustValueForPageZoom(conversionData.DynamicViewportFactor().MaxDimension(),
                                        conversionData);

      case Dvmin:
        return value
               * AdjustValueForPageZoom(conversionData.DynamicViewportFactor().MinDimension(),
                                        conversionData);

      case Dvb:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Block, conversionData.DynamicViewportFactor(), conversionData.Style()),
                 conversionData);

      case Dvi:
        return value
               * AdjustValueForPageZoom(
                 LengthOfViewportPhysicalAxisForLogicalAxis(
                   LogicalBoxAxis::Inline, conversionData.DynamicViewportFactor(), conversionData.Style()),
                 conversionData);

        // MARK: "container-percentage" resolution

      case Cqw:
      {
        if (auto resolvedValue = resolveContainerUnit(css::cq::Axis::Width))
        {
          return *resolvedValue;
        }
        return ComputeNonCalcLengthDouble(value, Svw, conversionData);
      }

      case Cqh:
      {
        if (auto resolvedValue = resolveContainerUnit(css::cq::Axis::Height))
        {
          return *resolvedValue;
        }
        return ComputeNonCalcLengthDouble(value, Svh, conversionData);
      }

      case Cqi:
      {
        if (auto resolvedValue = resolveContainerUnit(conversionData.Style()->GetWritingMode().IsHorizontal()
                                                        ? css::cq::Axis::Width
                                                        : css::cq::Axis::Height))
        {
          return *resolvedValue;
        }
        return ComputeNonCalcLengthDouble(value, Svi, conversionData);
      }

      case Cqb:
      {
        if (auto resolvedValue = resolveContainerUnit(conversionData.Style()->GetWritingMode().IsHorizontal()
                                                        ? css::cq::Axis::Height
                                                        : css::cq::Axis::Width))
        {
          return *resolvedValue;
        }
        return ComputeNonCalcLengthDouble(value, Svb, conversionData);
      }

      case Cqmax:
      {
        if (value < 0)
        {
          return std::min(ComputeNonCalcLengthDouble(value, Cqb, conversionData),
                          ComputeNonCalcLengthDouble(value, Cqi, conversionData));
        }
        return std::max(ComputeNonCalcLengthDouble(value, Cqb, conversionData),
                        ComputeNonCalcLengthDouble(value, Cqi, conversionData));
      }

      case Cqmin:
      {
        if (value < 0)
        {
          return std::max(ComputeNonCalcLengthDouble(value, Cqb, conversionData),
                          ComputeNonCalcLengthDouble(value, Cqi, conversionData));
        }
        return std::min(ComputeNonCalcLengthDouble(value, Cqb, conversionData),
                        ComputeNonCalcLengthDouble(value, Cqi, conversionData));
      }
    }

    // We do not apply the zoom factor when we are computing the value of the font-size property. The zooming
    // for font sizes is much more complicated, since we have to worry about enforcing the minimum font size
    // preference as well as enforcing the implicit "smart minimum."
    if (conversionData.ComputingFontSize() || css::IsFontOrRootFontRelativeLength(lengthUnit))
    {
      return value;
    }

    return value * conversionData.Zoom();
  }

  bool EqualForLengthResolution(const RenderStyle &styleA, const RenderStyle &styleB) noexcept
  {
    // These properties affect results of `ComputeNonCalcLengthDouble` above.

    if (styleA.GetFontDescription().ComputedSize() != styleB.GetFontDescription().ComputedSize())
    {
      return false;
    }
    if (styleA.GetFontDescription().SpecifiedSize() != styleB.GetFontDescription().SpecifiedSize())
    {
      return false;
    }

    if (styleA.MetricsOfPrimaryFont().XHeight() != styleB.MetricsOfPrimaryFont().XHeight())
    {
      return false;
    }
    if (styleA.MetricsOfPrimaryFont().ZeroWidth() != styleB.MetricsOfPrimaryFont().ZeroWidth())
    {
      return false;
    }

    if (styleA.GetZoom() != styleB.GetZoom())
    {
      return false;
    }

    return true;
  }
}
