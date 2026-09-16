#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Range.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class FloatSize;
  class FontCascade;
}

namespace krys::boo::style
{
  class BuilderState;
  class RenderStyle;
  class RenderView;
}

namespace krys::boo::dom
{
  class Element;
}

namespace krys::boo::css
{
  class ToLengthConversionData
  {
  private:
    const style::RenderStyle *_style {nullptr};
    const style::RenderStyle *_rootStyle {nullptr};
    const style::RenderStyle *_parentStyle {nullptr};
    const style::RenderView *_renderView {nullptr};
    RefPtr<const dom::Element> _elementForContainerUnitResolution;
    Maybe<float> _zoom;
    Maybe<PropertyId> _propertyToCompute;
    CheckedPtr<style::BuilderState> _styleBuilderState;
    RangeZoomOptions _rangeZoomOption {RangeZoomOptions::Default};

  public:
    ToLengthConversionData() noexcept;

    ToLengthConversionData(const ToLengthConversionData &other) noexcept;

    ToLengthConversionData(ToLengthConversionData &&other) noexcept;

    /// @brief This is used during style building. The 'zoom' property is taken into account.
    ToLengthConversionData(const style::RenderStyle &, style::BuilderState &) noexcept;

    /// @brief This constructor ignores the `zoom` property.
    ToLengthConversionData(const style::RenderStyle &, const style::RenderStyle *rootStyle,
                           const style::RenderStyle *parentStyle, const style::RenderView *,
                           const dom::Element *elementForContainerUnitResolution = nullptr) noexcept;

    ~ToLengthConversionData() noexcept;

    KRYS_NODISCARD const style::RenderStyle *Style() const noexcept
    {
      return _style;
    }

    KRYS_NODISCARD const style::RenderStyle *RootStyle() const noexcept
    {
      return _rootStyle;
    }

    KRYS_NODISCARD const style::RenderStyle *ParentStyle() const noexcept
    {
      return _parentStyle;
    }

    KRYS_NODISCARD float Zoom() const noexcept;

    KRYS_NODISCARD RangeZoomOptions RangeZoomOption() const noexcept
    {
      return _rangeZoomOption;
    }

    KRYS_NODISCARD bool ComputingFontSize() const noexcept
    {
      return _propertyToCompute == PropertyId::FontSize;
    }

    KRYS_NODISCARD bool ComputingLineHeight() const noexcept
    {
      return _propertyToCompute == PropertyId::LineHeight;
    }

    KRYS_NODISCARD PropertyId PropertyToCompute() const noexcept
    {
      return _propertyToCompute.value_or(PropertyId::Invalid);
    }

    KRYS_NODISCARD bool EvaluationTimeZoomEnabled() const noexcept;

    KRYS_NODISCARD const style::RenderView *RenderView() const noexcept
    {
      return _renderView;
    }

    KRYS_NODISCARD const dom::Element *ElementForContainerUnitResolution() const noexcept
    {
      return _elementForContainerUnitResolution.get();
    }

    KRYS_NODISCARD const FontCascade &FontCascadeForFontUnits() const noexcept;

    KRYS_NODISCARD float ComputedLineHeightForFontUnits() const noexcept;

    KRYS_NODISCARD FloatSize DefaultViewportFactor() const noexcept;

    KRYS_NODISCARD FloatSize SmallViewportFactor() const noexcept;

    KRYS_NODISCARD FloatSize LargeViewportFactor() const noexcept;

    KRYS_NODISCARD FloatSize DynamicViewportFactor() const noexcept;

    KRYS_NODISCARD ToLengthConversionData CopyForFontSize() const noexcept
    {
      ToLengthConversionData copy(*this);
      copy._zoom = 1.f;
      copy._propertyToCompute = PropertyId::FontSize;
      return copy;
    };

    KRYS_NODISCARD ToLengthConversionData CopyWithAdjustedZoom(
      float zoom, RangeZoomOptions rangeZoomOption = RangeZoomOptions::Default) const noexcept
    {
      ToLengthConversionData copy(*this);
      copy._zoom = zoom;
      copy._rangeZoomOption = rangeZoomOption;
      return copy;
    }

    KRYS_NODISCARD ToLengthConversionData CopyForLineHeight(float zoom) const noexcept
    {
      ToLengthConversionData copy(*this);
      copy._zoom = zoom;
      copy._propertyToCompute = PropertyId::LineHeight;
      copy._rangeZoomOption = RangeZoomOptions::Unzoomed;
      return copy;
    }

    void SetUsesContainerUnits() const noexcept;

    KRYS_NODISCARD style::BuilderState *StyleBuilderState() const noexcept
    {
      return _styleBuilderState.get();
    }
  };
}