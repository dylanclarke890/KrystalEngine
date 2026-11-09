/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 * copyright notice, this list of conditions and the following disclaimer
 * in the documentation and/or other materials provided with the
 * distribution.
 *     * Neither the name of Google Inc. nor the names of its
 * contributors may be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
  class RenderStyle;
  class RenderView;
}

namespace krys::boo::style
{
  class BuilderState;
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
    const RenderStyle *_style {nullptr};
    const RenderStyle *_rootStyle {nullptr};
    const RenderStyle *_parentStyle {nullptr};
    const boo::RenderView *_renderView {nullptr};
    RefPtr<const dom::Element> _elementForContainerUnitResolution;
    Maybe<float> _zoom;
    Maybe<PropertyId> _propertyToCompute;
    CheckedPtr<style::BuilderState> _styleBuilderState;
    RangeZoomOptions _rangeZoomOption {RangeZoomOptions::Default};

  public:
    ToLengthConversionData() noexcept;

    ToLengthConversionData(const ToLengthConversionData &) noexcept;

    ToLengthConversionData(ToLengthConversionData &&) noexcept;
    ToLengthConversionData &operator=(const ToLengthConversionData &) noexcept;
    ToLengthConversionData &operator=(ToLengthConversionData &&) noexcept;

    /// @brief This is used during style building. The 'zoom' property is taken into account.
    ToLengthConversionData(const RenderStyle &, style::BuilderState &) noexcept;

    /// @brief This constructor ignores the `zoom` property.
    ToLengthConversionData(const RenderStyle &, const RenderStyle *rootStyle, const RenderStyle *parentStyle,
                           const boo::RenderView *,
                           const dom::Element *elementForContainerUnitResolution = nullptr) noexcept;

    ~ToLengthConversionData() noexcept;

    KRYS_NODISCARD const RenderStyle *Style() const noexcept
    {
      return _style;
    }

    KRYS_NODISCARD const RenderStyle *RootStyle() const noexcept
    {
      return _rootStyle;
    }

    KRYS_NODISCARD const RenderStyle *ParentStyle() const noexcept
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

    KRYS_NODISCARD const boo::RenderView *GetRenderView() const noexcept
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
    }

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

    KRYS_NODISCARD style::BuilderState *StyleBuilderState() const noexcept;

    KRYS_NODISCARD CheckedPtr<style::BuilderState> ProtectedStyleBuilderState() const noexcept;
  };
}
