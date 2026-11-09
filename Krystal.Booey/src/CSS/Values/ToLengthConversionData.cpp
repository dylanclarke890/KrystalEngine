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

#include "Krystal.Booey/CSS/Values/ToLengthConversionData.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"

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
        _parentStyle(&builderState.ParentStyle()), _renderView(builderState.GetDocument().GetRenderView()),
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
