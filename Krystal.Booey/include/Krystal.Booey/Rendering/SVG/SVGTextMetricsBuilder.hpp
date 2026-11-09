/*
 * Copyright (C) Research In Motion Limited 2010-2012. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Booey/Graphics/TextRuns/TextRun.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributes.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextMetrics.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"
#include <tuple>

namespace krys::boo
{
  class ComplexTextController;
  class RenderElement;
  class RenderSVGInlineText;
  class RenderSVGText;
  struct MeasureTextData;
  struct WidthIterator;

  /// @brief Measures the characters of SVG text renderers, and maps the positioning data of the whole text
  /// onto them.
  class SVGTextMetricsBuilder
  {
    KRYS_NON_COPYABLE(SVGTextMetricsBuilder);

  private:
    WeakPtr<RenderSVGInlineText> _text;
    TextRun _run;
    /// @brief A byte offset into the run.
    uint32 _textPosition {0};
    bool _isComplexText {false};
    bool _canUseSimplifiedTextMeasuring {false};
    SVGTextMetrics _currentMetrics;
    float _totalWidth {0};

    // Complex text only.
    SVGTextMetrics _complexStartToCurrentMetrics;

  public:
    SVGTextMetricsBuilder() noexcept;

    void MeasureTextRenderer(RenderSVGText &, RenderSVGInlineText *stopAtLeaf) noexcept;
    void BuildMetricsAndLayoutAttributes(RenderSVGText &, RenderSVGInlineText *stopAtLeaf,
                                         SVGCharacterDataMap &allCharactersMap) noexcept;

  private:
    template <typename TIterator>
    bool Advance(TIterator &) noexcept;
    void AdvanceIterator(WidthIterator &) noexcept;
    void AdvanceIterator(ComplexTextController &) noexcept;
    /// @brief The length in bytes of the character at the current position.
    KRYS_NODISCARD uint32 CurrentCharacterLength() const noexcept;

    void InitializeMeasurementWithTextRenderer(RenderSVGInlineText &) noexcept;
    void WalkTree(RenderElement &, RenderSVGInlineText *stopAtLeaf, MeasureTextData &) noexcept;
    std::tuple<uint32, char32> MeasureTextRenderer(RenderSVGInlineText &, const MeasureTextData &,
                                                   std::tuple<uint32, char32>) noexcept;

    template <typename TIterator>
    std::tuple<uint32, char32> MeasureTextRendererWithIterator(TIterator &, RenderSVGInlineText &,
                                                               const MeasureTextData &,
                                                               std::tuple<uint32, char32>) noexcept;
  };
}
