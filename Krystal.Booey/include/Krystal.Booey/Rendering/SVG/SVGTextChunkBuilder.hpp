/*
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2015 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/SVG/SVGTextChunk.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class RenderSVGInlineText;
  class SVGInlineTextBox;
  struct SVGTextFragment;

  using SVGTextChunkStarts = HashSet<inline_iterator::SVGTextBox::Key, inline_iterator::SVGTextBoxKeyHash>;

  /// @brief Performs the third layout phase for SVG text.
  ///
  /// Phase one built the layout information from the SVG DOM stored in the RenderSVGInlineText objects
  /// (SVGTextLayoutAttributes). Phase two performed the actual per-character layout, computing the final
  /// positions for each character, stored in the SVGInlineTextBox objects (SVGTextFragment). Phase three
  /// performs all modifications that have to be applied to each individual text chunk (text-anchor &
  /// textLength).
  class SVGTextChunkBuilder
  {
  private:
    List<SVGTextChunk> _textChunks;
    SVGChunkTransformMap _textBoxTransformations;

  public:
    SVGTextChunkBuilder() noexcept = default;
    SVGTextChunkBuilder(SVGTextChunkBuilder &&) noexcept = default;
    SVGTextChunkBuilder(const SVGTextChunkBuilder &) = delete;

    KRYS_NODISCARD const List<SVGTextChunk> &TextChunks() const noexcept
    {
      return _textChunks;
    }
    /// @brief In characters (code points).
    KRYS_NODISCARD uint32 TotalCharacters() const noexcept;
    KRYS_NODISCARD float TotalLength() const noexcept;
    KRYS_NODISCARD float TotalAnchorShift() const noexcept;

    KRYS_NODISCARD
    AffineTransform TransformationForTextBox(inline_iterator::SVGTextBoxIterator) const noexcept;

    void BuildTextChunks(const List<inline_iterator::SVGTextBoxIterator> &lineLayoutBoxes,
                         const SVGTextChunkStarts &chunkStarts, SVGTextFragmentMap &) noexcept;
    void LayoutTextChunks(const List<inline_iterator::SVGTextBoxIterator> &lineLayoutBoxes,
                          const SVGTextChunkStarts &chunkStarts, SVGTextFragmentMap &) noexcept;
  };
}
