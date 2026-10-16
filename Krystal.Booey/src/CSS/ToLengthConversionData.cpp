#include "Krystal.Booey/CSS/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Graphics/FloatSize.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/BuilderState.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo::css
{
  ToLengthConversionData::ToLengthConversionData() noexcept = default;
  ToLengthConversionData::ToLengthConversionData(const ToLengthConversionData &) noexcept = default;
  ToLengthConversionData::ToLengthConversionData(ToLengthConversionData &&) noexcept = default;
  ToLengthConversionData &
    ToLengthConversionData::operator=(const ToLengthConversionData &) noexcept = default;
  ToLengthConversionData &ToLengthConversionData::operator=(ToLengthConversionData &&) noexcept = default;

  ToLengthConversionData::ToLengthConversionData(const RenderStyle &style,
                                                 style::BuilderState &builderState) noexcept
      : _style(&style), _rootStyle(builderState.RootElementStyle()),
        _parentStyle(&builderState.ParentStyle()), _renderView(RenderViewOf(builderState.GetDocument())),
        _elementForContainerUnitResolution(ShareRefPtr(builderState.GetElement())),
        _styleBuilderState(ShareCheckedPtr(&builderState))
  {
  }

  ToLengthConversionData::ToLengthConversionData(
    const RenderStyle &style, const RenderStyle *rootStyle, const RenderStyle *parentStyle,
    const boo::RenderView *renderView, const dom::Element *elementForContainerUnitResolution) noexcept
      : _style(&style), _rootStyle(rootStyle), _parentStyle(parentStyle), _renderView(renderView),
        _elementForContainerUnitResolution(ShareRefPtr(elementForContainerUnitResolution)), _zoom(1.f)
  {
  }

  ToLengthConversionData::~ToLengthConversionData() noexcept = default;

  const FontCascade &ToLengthConversionData::FontCascadeForFontUnits() const noexcept
  {
    if (ComputingFontSize())
    {
      krys_debug_assert(ParentStyle());
      return ParentStyle()->GetFontCascade();
    }
    krys_debug_assert(Style());
    return Style()->GetFontCascade();
  }

  float ToLengthConversionData::ComputedLineHeightForFontUnits() const noexcept
  {
    if (ComputingFontSize())
    {
      krys_debug_assert(ParentStyle());
      return ParentStyle()->ComputedLineHeight();
    }
    krys_debug_assert(Style());
    return Style()->ComputedLineHeight();
  }

  float ToLengthConversionData::Zoom() const noexcept
  {
    return _zoom.value_or(_style ? _style->UsedZoom() : 1.f);
  }

  FloatSize ToLengthConversionData::DefaultViewportFactor() const noexcept
  {
    if (_styleBuilderState)
    {
      _styleBuilderState->SetUsesViewportUnits();
    }

    if (!_renderView)
    {
      return {};
    }

    return _renderView->SizeForCSSDefaultViewportUnits() / 100.0f;
  }

  FloatSize ToLengthConversionData::SmallViewportFactor() const noexcept
  {
    if (_styleBuilderState)
    {
      _styleBuilderState->SetUsesViewportUnits();
    }

    if (!_renderView)
    {
      return {};
    }

    return _renderView->SizeForCSSSmallViewportUnits() / 100.0f;
  }

  FloatSize ToLengthConversionData::LargeViewportFactor() const noexcept
  {
    if (_styleBuilderState)
    {
      _styleBuilderState->SetUsesViewportUnits();
    }

    if (!_renderView)
    {
      return {};
    }

    return _renderView->SizeForCSSLargeViewportUnits() / 100.0f;
  }

  FloatSize ToLengthConversionData::DynamicViewportFactor() const noexcept
  {
    if (_styleBuilderState)
    {
      _styleBuilderState->SetUsesViewportUnits();
    }

    if (!_renderView)
    {
      return {};
    }

    return _renderView->SizeForCSSDynamicViewportUnits() / 100.0f;
  }

  void ToLengthConversionData::SetUsesContainerUnits() const noexcept
  {
    if (_styleBuilderState)
    {
      _styleBuilderState->SetUsesContainerUnits();
    }
  }

  style::BuilderState *ToLengthConversionData::StyleBuilderState() const noexcept
  {
    return _styleBuilderState.get();
  }

  CheckedPtr<style::BuilderState> ToLengthConversionData::ProtectedStyleBuilderState() const noexcept
  {
    return _styleBuilderState;
  }

  bool ToLengthConversionData::EvaluationTimeZoomEnabled() const noexcept
  {
    krys_debug_assert(_style);
    return _style->EvaluationTimeZoomEnabled();
  }
}
