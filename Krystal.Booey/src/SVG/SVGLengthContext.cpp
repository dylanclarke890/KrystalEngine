#include "Krystal.Booey/SVG/SVGLengthContext.hpp"
#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Booey/Graphics/FontMetrics.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Values/FillStroke/StrokeWidth.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthResolution.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"
#include "Krystal.Booey/Style/Values/Sizing/PreferredSize.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGCenterCoordinateComponent.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGCoordinateComponent.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGRadius.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGRadiusComponent.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDasharray.hpp"
#include "Krystal.Booey/Style/Values/SVG/SVGStrokeDashoffset.hpp"
#include "Krystal.Booey/Style/Values/Viewport/ZoomPrimitives.hpp"
#include "Krystal.Booey/SVG/SVGElement.hpp"
#include "Krystal.Booey/SVG/SVGSvgElement.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <cmath>
#include <numbers>

namespace krys::boo::svg
{
  SVGLengthContext::SVGLengthContext(const SVGElement *context, const Maybe<FloatSize> &viewportSize) noexcept
      : _context(CreateWeakPtr(const_cast<SVGElement *>(context))),
        _viewportSize(!_context ? viewportSize : Maybe<FloatSize> {})
  {
  }

  SVGLengthContext::~SVGLengthContext() noexcept = default;

  FloatRect SVGLengthContext::ResolveRectangle(const SVGElement *context, SVGUnitTypes::SVGUnitType type,
                                               const FloatRect &viewport, const SVGLengthValue &x,
                                               const SVGLengthValue &y, const SVGLengthValue &width,
                                               const SVGLengthValue &height) noexcept
  {
    krys_debug_assert(type != SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    if (type != SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)
    {
      auto viewportSize = viewport.Size();
      return FloatRect(
        ConvertValueFromPercentageToUserUnits(x.ValueAsPercentage(), x.LengthMode(), viewportSize)
          + viewport.X(),
        ConvertValueFromPercentageToUserUnits(y.ValueAsPercentage(), y.LengthMode(), viewportSize)
          + viewport.Y(),
        ConvertValueFromPercentageToUserUnits(width.ValueAsPercentage(), width.LengthMode(), viewportSize),
        ConvertValueFromPercentageToUserUnits(height.ValueAsPercentage(), height.LengthMode(), viewportSize));
    }

    SVGLengthContext lengthContext(context, viewport.Size());
    return FloatRect(x.Value(lengthContext), y.Value(lengthContext), width.Value(lengthContext),
                     height.Value(lengthContext));
  }

  FloatPoint SVGLengthContext::ResolvePoint(const SVGElement *context, SVGUnitTypes::SVGUnitType type,
                                            const SVGLengthValue &x, const SVGLengthValue &y) noexcept
  {
    krys_debug_assert(type != SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    if (type == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)
    {
      SVGLengthContext lengthContext(context);
      return FloatPoint(x.Value(lengthContext), y.Value(lengthContext));
    }

    // FIXME: valueAsPercentage() won't be correct for eg. cm units. They need to be resolved in user space
    // and then be considered in objectBoundingBox space.
    return FloatPoint(x.ValueAsPercentage(), y.ValueAsPercentage());
  }

  float SVGLengthContext::ResolveLength(const SVGElement *context, SVGUnitTypes::SVGUnitType type,
                                        const SVGLengthValue &x) noexcept
  {
    krys_debug_assert(type != SVGUnitTypes::SVG_UNIT_TYPE_UNKNOWN);
    if (type == SVGUnitTypes::SVG_UNIT_TYPE_USERSPACEONUSE)
    {
      SVGLengthContext lengthContext(context);
      return x.Value(lengthContext);
    }

    // FIXME: valueAsPercentage() won't be correct for eg. cm units. They need to be resolved in user space
    // and then be considered in objectBoundingBox space.
    return x.ValueAsPercentage();
  }

  namespace
  {
    KRYS_NODISCARD float DimensionForLengthMode(SVGLengthMode mode, FloatSize viewportSize) noexcept
    {
      switch (mode)
      {
        case SVGLengthMode::Width:
        {
          return viewportSize.Width();
        }
        case SVGLengthMode::Height:
        {
          return viewportSize.Height();
        }
        case SVGLengthMode::Other:
        {
          return viewportSize.DiagonalLength() / std::numbers::sqrt2_v<float>;
        }
      }
      krys_assert_msg(false, "Not reached");
      return 0;
    }
  }

  template <typename TSize>
  float SVGLengthContext::ValueForSizeType(const TSize &size, style::ZoomFactor usedZoom,
                                           SVGLengthMode lengthMode) noexcept
  requires(TSize::Fixed::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed
           || TSize::Calc::Range.ZoomOptions == css::RangeZoomOptions::Unzoomed)
  {
    return size.SwitchOn(
      [&](const typename TSize::Fixed &fixed) -> float { return style::Evaluate<float>(fixed, usedZoom); },
      [&](const typename TSize::Percentage &percentage) -> float
      {
        auto result = ConvertValueFromPercentageToUserUnits(percentage.Value / 100, lengthMode);
        if (result.HasException())
        {
          return 0;
        }
        return result.ReleaseValue();
      },
      [&](const typename TSize::Calc &calc) -> float
      {
        auto viewportSize = this->ViewportSize().value_or(FloatSize {});
        return style::Evaluate<float>(calc, DimensionForLengthMode(lengthMode, viewportSize), usedZoom);
      },
      [&](const auto &) -> float { return 0; });
  }

  template <typename TSize>
  float SVGLengthContext::ValueForSizeType(const TSize &size, style::ZoomNeeded zoomNeeded,
                                           SVGLengthMode lengthMode) noexcept
  {
    return size.SwitchOn(
      [&](const typename TSize::Fixed &fixed) -> float { return style::Evaluate<float>(fixed, zoomNeeded); },
      [&](const typename TSize::Percentage &percentage) -> float
      {
        auto result = ConvertValueFromPercentageToUserUnits(percentage.Value / 100, lengthMode);
        if (result.HasException())
        {
          return 0;
        }
        return result.ReleaseValue();
      },
      [&](const typename TSize::Calc &calc) -> float
      {
        auto viewportSize = this->ViewportSize().value_or(FloatSize {});
        return style::Evaluate<float>(calc, DimensionForLengthMode(lengthMode, viewportSize), zoomNeeded);
      },
      [&](const auto &) -> float { return 0; });
  }

  float SVGLengthContext::ValueForLength(const style::PreferredSize &size, style::ZoomFactor usedZoom,
                                         SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, usedZoom, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::SVGCenterCoordinateComponent &size,
                                         style::ZoomNeeded zoomNeeded, SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::SVGCoordinateComponent &size,
                                         style::ZoomNeeded zoomNeeded, SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::SVGRadius &size, style::ZoomNeeded zoomNeeded,
                                         SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::SVGRadiusComponent &size, style::ZoomNeeded zoomNeeded,
                                         SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::SVGStrokeDasharrayValue &size,
                                         style::ZoomNeeded zoomNeeded, SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::SVGStrokeDashoffset &size, style::ZoomNeeded zoomNeeded,
                                         SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ValueForLength(const style::StrokeWidth &size, style::ZoomNeeded zoomNeeded,
                                         SVGLengthMode lengthMode) noexcept
  {
    return ValueForSizeType(size, zoomNeeded, lengthMode);
  }

  float SVGLengthContext::ComputeNonCalcLength(float inputValue, css::LengthUnit unit) const noexcept
  {
    if (!css::ConversionToCanonicalUnitRequiresConversionData(unit))
    {
      return ClampTo<float>(style::ComputeNonCalcLengthDouble(inputValue, unit, {}));
    }

    auto conversionData = CssConversionData();
    if (!conversionData)
    {
      return 0.0f;
    }

    auto resolvedValue = ClampTo<float>(style::ComputeNonCalcLengthDouble(inputValue, unit, *conversionData));

    // "Font dependent" or "Root font dependent" resolve against computed font sizes, which may include
    // CSS zoom scaling. However, lengths within the SVG subtree shall be resolved
    // excluding zoom, because the (anonymous) RenderSVGViewportContainer applies zooming
    // for the whole SVG subtree as an affine transform. Therefore any font-relative length
    // within the SVG subtree needs to exclude the 'zoom' information.
    if (css::IsFontOrRootFontRelativeLength(unit))
    {
      resolvedValue = RemoveZoomFromFontOrRootFontRelativeLength(resolvedValue, unit);
    }

    return resolvedValue;
  }

  float SVGLengthContext::RemoveZoomFromFontOrRootFontRelativeLength(float value,
                                                                     css::LengthUnit unit) const noexcept
  {
    RefPtr<const SVGSvgElement> svgElement = ShareRefPtr(
      _context->IsOutermostSVGSVGElement() ? Downcast<SVGSvgElement>(_context.get())
                                           : DynamicDowncast<SVGSvgElement>(_context->ViewportElement()));

    if (!svgElement || !svgElement->Renderer())
    {
      return value;
    }

    float usedZoom = 1.0f;

    if (css::IsFontRelativeLength(unit))
    {
      usedZoom = svgElement->Renderer()->Style().UsedZoom();
    }
    else if (css::IsRootFontRelativeLength(unit))
    {
      if (auto *rootRenderer = svgElement->NodeDocument().DocumentElement()->Renderer())
      {
        usedZoom = rootRenderer->Style().UsedZoom();
      }
    }

    return (usedZoom != 1.0f) ? value / usedZoom : value;
  }

  dom::ExceptionOr<float>
    SVGLengthContext::ResolveValueToUserUnits(float value, const css::LengthPercentageUnit &targetUnit,
                                              SVGLengthMode lengthMode) const noexcept
  {
    switch (targetUnit)
    {
      case css::LengthPercentageUnit::Percentage:
      {
        return ConvertValueFromPercentageToUserUnits(static_cast<float>(value / 100.0), lengthMode);
      }

      case css::LengthPercentageUnit::Ex:
      {
        // FIXME: Legacy quirk. Using the computeNonCalcLengthDouble conversion here causes test failures
        // (e.g. coords-units-03-b.svg drifting from 150 > ~139). Needs deeper investigation before unifying.
        return ConvertValueFromEXSToUserUnits(value);
      }

      default:
      {
        auto cssUnit = css::ToUnitType(targetUnit);
        auto lengthUnit = css::ToLengthUnit(cssUnit);

        if (!lengthUnit)
        {
          return dom::Exception {dom::ExceptionCode::NotSupportedError};
        }

        return ComputeNonCalcLength(value, *lengthUnit);
      }
    }
  }

  dom::ExceptionOr<css::LengthPercentage<>>
    SVGLengthContext::ResolveValueFromUserUnits(float value, const css::LengthPercentageUnit &targetUnit,
                                                SVGLengthMode lengthMode) const noexcept
  {
    switch (targetUnit)
    {
      case css::LengthPercentageUnit::Percentage:
      {
        auto percent = ConvertValueFromUserUnitsToPercentage(value, lengthMode);
        if (percent.HasException())
        {
          return percent.ReleaseException();
        }
        return css::LengthPercentage<>(targetUnit, percent.ReleaseValue());
      }

      case css::LengthPercentageUnit::Ex:
      {
        auto exVal = ConvertValueFromUserUnitsToEXS(value);
        if (exVal.HasException())
        {
          return exVal.ReleaseException();
        }
        return css::LengthPercentage<>(targetUnit, exVal.ReleaseValue());
      }

      default:
      {
        auto cssUnit = css::ToUnitType(targetUnit);
        auto lengthUnit = css::ToLengthUnit(cssUnit);

        if (!lengthUnit)
        {
          return dom::Exception {dom::ExceptionCode::NotSupportedError};
        }

        auto pxPerUnit = ComputeNonCalcLength(1.0, *lengthUnit);
        if (!pxPerUnit)
        {
          return dom::Exception {dom::ExceptionCode::NotSupportedError};
        }

        return css::LengthPercentage<>(targetUnit, value / pxPerUnit);
      }
    }
  }

  dom::ExceptionOr<float>
    SVGLengthContext::ConvertValueFromUserUnitsToPercentage(float value,
                                                            SVGLengthMode lengthMode) const noexcept
  {
    auto viewportSize = this->ViewportSize();
    if (!viewportSize)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    if (auto divisor = DimensionForLengthMode(lengthMode, *viewportSize))
    {
      return value / divisor * 100;
    }

    return value;
  }

  dom::ExceptionOr<float>
    SVGLengthContext::ConvertValueFromPercentageToUserUnits(float value,
                                                            SVGLengthMode lengthMode) const noexcept
  {
    auto viewportSize = this->ViewportSize();
    if (!viewportSize)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    return ConvertValueFromPercentageToUserUnits(value, lengthMode, *viewportSize);
  }

  float SVGLengthContext::ConvertValueFromPercentageToUserUnits(float value, SVGLengthMode lengthMode,
                                                                FloatSize viewportSize) noexcept
  {
    return value * DimensionForLengthMode(lengthMode, viewportSize);
  }

  namespace
  {
    KRYS_NODISCARD const RenderStyle *RenderStyleForLengthResolving(const SVGElement *context) noexcept
    {
      if (!context)
      {
        return nullptr;
      }

      const dom::Node *currentContext = context;
      do
      {
        if (currentContext->Renderer())
        {
          return &currentContext->Renderer()->Style();
        }
        currentContext = currentContext->ParentNode();
      } while (currentContext);

      return nullptr;
    }

    KRYS_NODISCARD const RenderStyle *RootRenderStyleForLengthResolving(const SVGElement *svgElement) noexcept
    {
      if (!svgElement)
      {
        return nullptr;
      }

      RefPtr<const dom::Element> rootElement = svgElement->NodeDocument().DocumentElement();
      if (!rootElement || !rootElement->Renderer())
      {
        return nullptr;
      }

      return &rootElement->Renderer()->Style();
    }
  }

  Maybe<css::ToLengthConversionData> SVGLengthContext::CssConversionData() const noexcept
  {
    auto element = ProtectedContext();
    if (!element)
    {
      return {};
    }

    auto *currentStyle = RenderStyleForLengthResolving(element.get());
    if (!currentStyle)
    {
      return {};
    }

    auto *rootStyle = RootRenderStyleForLengthResolving(element.get());

    const RenderStyle *parentStyle = nullptr;
    if (auto *renderer = element->Renderer())
    {
      parentStyle = renderer->ParentStyle();
    }

    return css::ToLengthConversionData {
      *currentStyle, rootStyle, parentStyle, element->NodeDocument().GetRenderView(), element.get(),
    };
  }

  RefPtr<const SVGElement> SVGLengthContext::ProtectedContext() const noexcept
  {
    return ShareRefPtr(static_cast<const SVGElement *>(_context.get()));
  }

  dom::ExceptionOr<float> SVGLengthContext::ConvertValueFromUserUnitsToEXS(float value) const noexcept
  {
    auto *style = RenderStyleForLengthResolving(ProtectedContext().get());
    if (!style)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // Use of ceil allows a pixel match to the W3Cs expected output of coords-units-03-b.svg
    // if this causes problems in real world cases maybe it would be best to remove this
    float xHeight = std::ceil(style->MetricsOfPrimaryFont().XHeight().value_or(0));
    if (!xHeight)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    return value / xHeight;
  }

  dom::ExceptionOr<float> SVGLengthContext::ConvertValueFromEXSToUserUnits(float value) const noexcept
  {
    auto *style = RenderStyleForLengthResolving(ProtectedContext().get());
    if (!style)
    {
      return dom::Exception {dom::ExceptionCode::NotSupportedError};
    }

    // Use of ceil allows a pixel match to the W3Cs expected output of coords-units-03-b.svg
    // if this causes problems in real world cases maybe it would be best to remove this
    return value * std::ceil(style->MetricsOfPrimaryFont().XHeight().value_or(0));
  }

  Maybe<FloatSize> SVGLengthContext::ViewportSize() const noexcept
  {
    if (!_context)
    {
      return _viewportSize;
    }

    if (!_viewportSize)
    {
      _viewportSize = ComputeViewportSize();
    }

    return _viewportSize;
  }

  Maybe<FloatSize> SVGLengthContext::ComputeViewportSize() const noexcept
  {
    krys_debug_assert(_context);

    // Root <svg> element lengths are resolved against the top level viewport,
    // however excluding 'zoom' induced scaling. Length within the <svg> subtree
    // shall be resolved against the 'vanilla' viewport size, excluding zoom, because
    // the (anonymous) RenderSVGViewportContainer (first and only child of RenderSVGRoot)
    // applies zooming/panning for the whole SVG subtree as affine transform. Therefore
    // any length within the SVG subtree needs to exclude the 'zoom' information.
    if (_context->IsOutermostSVGSVGElement())
    {
      return Downcast<SVGSvgElement>(*ProtectedContext()).CurrentViewportSizeExcludingZoom();
    }

    // Take size from nearest SVGSVGElement, skipping over <symbol> elements.
    RefPtr<const SVGSvgElement> svg = ShareRefPtr(
      DynamicDowncast<SVGSvgElement>(_context->ViewportElement(SVGElement::ViewportElementType::SVGSVGOnly)));
    if (!svg)
    {
      return {};
    }

    auto viewportSize = svg->CurrentViewBoxRect().Size();
    if (viewportSize.IsEmpty())
    {
      viewportSize = svg->CurrentViewportSizeExcludingZoom();
    }

    return viewportSize;
  }
}
