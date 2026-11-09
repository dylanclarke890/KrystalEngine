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

#include "Krystal.Booey/Rendering/SVG/SVGTextChunkBuilder.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/SVG/SVGTextFragment.hpp"

namespace krys::boo
{
  uint32 SVGTextChunkBuilder::TotalCharacters() const noexcept
  {
    uint32 characters = 0;
    for (const auto &chunk : _textChunks)
    {
      characters += chunk.TotalCharacters();
    }
    return characters;
  }

  float SVGTextChunkBuilder::TotalLength() const noexcept
  {
    float length = 0;
    for (const auto &chunk : _textChunks)
    {
      length += chunk.TotalLength();
    }
    return length;
  }

  float SVGTextChunkBuilder::TotalAnchorShift() const noexcept
  {
    float anchorShift = 0;
    for (const auto &chunk : _textChunks)
    {
      anchorShift += chunk.TotalAnchorShift();
    }
    return anchorShift;
  }

  AffineTransform
    SVGTextChunkBuilder::TransformationForTextBox(inline_iterator::SVGTextBoxIterator textBox) const noexcept
  {
    auto it = _textBoxTransformations.find(inline_iterator::MakeKey(*textBox));
    return it == _textBoxTransformations.end() ? AffineTransform() : it->second;
  }

  void SVGTextChunkBuilder::BuildTextChunks(const List<inline_iterator::SVGTextBoxIterator> &lineLayoutBoxes,
                                            const SVGTextChunkStarts &chunkStarts,
                                            SVGTextFragmentMap &fragmentMap) noexcept
  {
    if (lineLayoutBoxes.empty())
    {
      return;
    }

    auto limit = static_cast<uint32>(lineLayoutBoxes.size());
    uint32 first = limit;

    for (uint32 i = 0; i < limit; ++i)
    {
      if (!chunkStarts.contains(inline_iterator::MakeKey(*lineLayoutBoxes[i])))
      {
        continue;
      }

      if (first == limit)
      {
        first = i;
      }
      else
      {
        krys_assert(first != i);
        _textChunks.push_back(SVGTextChunk(lineLayoutBoxes, first, i, fragmentMap));
        first = i;
      }
    }

    if (first != limit)
    {
      _textChunks.push_back(SVGTextChunk(lineLayoutBoxes, first, limit, fragmentMap));
    }
  }

  void SVGTextChunkBuilder::LayoutTextChunks(const List<inline_iterator::SVGTextBoxIterator> &lineLayoutBoxes,
                                             const SVGTextChunkStarts &chunkStarts,
                                             SVGTextFragmentMap &fragmentMap) noexcept
  {
    BuildTextChunks(lineLayoutBoxes, chunkStarts, fragmentMap);
    if (_textChunks.empty())
    {
      return;
    }

    for (const auto &chunk : _textChunks)
    {
      chunk.Layout(_textBoxTransformations);
    }

    _textChunks.clear();
  }
}
