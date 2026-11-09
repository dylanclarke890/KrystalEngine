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

#include "Krystal.Booey/Graphics/Paths/Path.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextChunkBuilder.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextLayoutAttributes.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderObject;
  class RenderStyle;
  class RenderSVGInlineText;
  class RenderSVGTextPath;
  class SVGInlineTextBox;

  /// @brief Performs the second layout phase for SVG text.
  ///
  /// The inline box tree was created, containing the text chunk information, necessary to apply certain SVG
  /// specific text layout properties (text-length adjustments and text-anchor). The second layout phase uses
  /// the SVGTextLayoutAttributes stored in the individual RenderSVGInlineText renderers to compute the final
  /// positions for each character, which are stored as the text boxes' text fragments.
  class SVGTextLayoutEngine
  {
  private:
    List<SVGTextLayoutAttributes *> &_layoutAttributes;

    List<inline_iterator::SVGTextBoxIterator> _lineLayoutBoxes;
    List<inline_iterator::SVGTextBoxIterator> _pathLayoutBoxes;

    // Output.
    SVGTextFragmentMap _fragmentMap;

    SVGTextChunkBuilder _chunkLayoutBuilder;
    SVGTextChunkStarts _lineLayoutChunkStarts;

    SVGTextFragment _currentTextFragment;
    uint32 _layoutAttributesPosition {0};
    /// @brief Byte offsets into the text renderers' text.
    uint32 _logicalCharacterOffset {0};
    uint32 _logicalMetricsListOffset {0};
    uint32 _visualCharacterOffset {0};
    uint32 _visualMetricsListOffset {0};
    float _x {0.0f};
    float _y {0.0f};
    float _dx {0.0f};
    float _dy {0.0f};
    float _lastChunkStartPosition {0.0f};
    bool _lastChunkHasTextLength {false};
    bool _lastChunkIsVerticalText {false};
    bool _isVerticalText {false};
    bool _inPathLayout {false};

    // Text on path layout
    Path _textPath;
    float _textPathLength {0.0f};
    float _textPathStartOffset {0.0f};
    float _textPathCurrentOffset {0.0f};
    float _textPathSpacing {0.0f};
    float _textPathScaling {1.0f};

  public:
    explicit SVGTextLayoutEngine(List<SVGTextLayoutAttributes *> &) noexcept;
    SVGTextLayoutEngine(SVGTextLayoutEngine &&) noexcept = default;
    SVGTextLayoutEngine(const SVGTextLayoutEngine &) = delete;

    KRYS_NODISCARD List<SVGTextLayoutAttributes *> &LayoutAttributes() noexcept
    {
      return _layoutAttributes;
    }

    void BeginTextPathLayout(const RenderSVGTextPath &, SVGTextLayoutEngine &lineLayout) noexcept;
    void EndTextPathLayout() noexcept;

    void LayoutInlineTextBox(inline_iterator::SVGTextBoxIterator) noexcept;
    KRYS_NODISCARD SVGTextFragmentMap FinishLayout() noexcept;

  private:
    void UpdateCharacterPositionIfNeeded(float &x, float &y) noexcept;
    void UpdateCurrentTextPosition(float x, float y, float glyphAdvance) noexcept;
    void UpdateRelativePositionAdjustmentsIfNeeded(float dx, float dy) noexcept;

    void RecordTextFragment(inline_iterator::SVGTextBoxIterator, const List<SVGTextMetrics> &) noexcept;
    KRYS_NODISCARD bool ParentDefinesTextLength(RenderObject *) const noexcept;

    void LayoutTextOnLineOrPath(inline_iterator::SVGTextBoxIterator, const RenderSVGInlineText &,
                                const RenderStyle &) noexcept;
    void FinalizeTransformMatrices(List<inline_iterator::SVGTextBoxIterator> &) noexcept;

    bool CurrentLogicalCharacterAttributes(SVGTextLayoutAttributes *&) noexcept;
    bool CurrentLogicalCharacterMetrics(SVGTextLayoutAttributes *&, SVGTextMetrics &) noexcept;
    bool CurrentVisualCharacterMetrics(const inline_iterator::SVGTextBox &, const List<SVGTextMetrics> &,
                                       SVGTextMetrics &) noexcept;

    void AdvanceToNextLogicalCharacter(const SVGTextMetrics &) noexcept;
    void AdvanceToNextVisualCharacter(const SVGTextMetrics &) noexcept;
  };
}
