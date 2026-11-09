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

#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Layout/Integration/Inline/InlineIteratorSVGTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo
{
  class SVGInlineTextBox;

  using SVGChunkTransformMap =
    HashMap<inline_iterator::SVGTextBox::Key, AffineTransform, inline_iterator::SVGTextBoxKeyHash>;
  using SVGTextFragmentMap =
    HashMap<inline_iterator::SVGTextBox::Key, List<SVGTextFragment>, inline_iterator::SVGTextBoxKeyHash>;

  /// @brief A range of text fragments: the SVG spec's "text chunk".
  class SVGTextChunk
  {
  public:
    enum ChunkStyle : uint8
    {
      DefaultStyle = 1 << 0,
      MiddleAnchor = 1 << 1,
      EndAnchor = 1 << 2,
      RightToLeftText = 1 << 3,
      VerticalText = 1 << 4,
      LengthAdjustSpacing = 1 << 5,
      LengthAdjustSpacingAndGlyphs = 1 << 6
    };

  private:
    // Contains all SVGInlineTextBoxes this chunk spans.
    struct BoxAndFragments
    {
      inline_iterator::SVGTextBoxIterator Box;
      List<SVGTextFragment> *Fragments;
    };

    List<BoxAndFragments> _boxes;
    uint32 _chunkStyle {DefaultStyle};
    float _desiredTextLength {0};

  public:
    SVGTextChunk(const List<inline_iterator::SVGTextBoxIterator> &, uint32 first, uint32 limit,
                 SVGTextFragmentMap &) noexcept;

    /// @brief In characters (code points).
    KRYS_NODISCARD uint32 TotalCharacters() const noexcept;
    KRYS_NODISCARD float TotalLength() const noexcept;
    KRYS_NODISCARD float TotalAnchorShift() const noexcept;
    void Layout(SVGChunkTransformMap &) const noexcept;

  private:
    void ProcessTextAnchorCorrection() const noexcept;
    void BuildBoxTransformations(SVGChunkTransformMap &) const noexcept;
    void ProcessTextLengthSpacingCorrection() const noexcept;

    KRYS_NODISCARD bool IsVerticalText() const noexcept
    {
      return _chunkStyle & VerticalText;
    }
    KRYS_NODISCARD float DesiredTextLength() const noexcept
    {
      return _desiredTextLength;
    }

    KRYS_NODISCARD bool HasDesiredTextLength() const noexcept
    {
      return _desiredTextLength > 0
             && ((_chunkStyle & LengthAdjustSpacing) || (_chunkStyle & LengthAdjustSpacingAndGlyphs));
    }
    KRYS_NODISCARD bool HasTextAnchor() const noexcept
    {
      return _chunkStyle & RightToLeftText ? !(_chunkStyle & EndAnchor)
                                           : (_chunkStyle & (MiddleAnchor | EndAnchor));
    }
    KRYS_NODISCARD bool HasLengthAdjustSpacing() const noexcept
    {
      return _chunkStyle & LengthAdjustSpacing;
    }
    KRYS_NODISCARD bool HasLengthAdjustSpacingAndGlyphs() const noexcept
    {
      return _chunkStyle & LengthAdjustSpacingAndGlyphs;
    }

    bool BoxSpacingAndGlyphsTransform(const List<SVGTextFragment> &, AffineTransform &) const noexcept;
  };
}
