/**
 * Copyright (C) 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
 * Copyright (C) 2023-2024 Apple Inc. All rights reserved.
 * Copyright (C) 2014-2016 Google Inc. All rights reserved.
 * Copyright (C) 2023, 2024 Igalia S.L.
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

#include "Krystal.Booey/Rendering/SVG/SVGInlineTextBox.hpp"

#include "Krystal.Booey/Graphics/Fonts/FontCascade.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"
#include <algorithm>

namespace krys::boo
{
  SVGInlineTextBox::SVGInlineTextBox(RenderSVGInlineText &renderer) noexcept : LegacyInlineTextBox(renderer)
  {
  }

  SVGInlineTextBox *SVGInlineTextBox::NextTextBox() const noexcept
  {
    return Downcast<SVGInlineTextBox>(LegacyInlineTextBox::NextTextBox());
  }

  void SVGInlineTextBox::DirtyOwnLineBoxes() noexcept
  {
    LegacyInlineTextBox::DirtyLineBoxes();

    _textFragments = {};
  }

  void SVGInlineTextBox::DirtyLineBoxes() noexcept
  {
    DirtyOwnLineBoxes();

    // And clear any following text fragments as the text on which they
    // depend may now no longer exist, or glyph positions may be wrong
    for (auto *nextBox = NextTextBox(); nextBox; nextBox = nextBox->NextTextBox())
    {
      nextBox->DirtyOwnLineBoxes();
    }
  }

  bool SVGInlineTextBox::MapStartEndPositionsIntoFragmentCoordinates(const SVGTextFragment &fragment,
                                                                     uint32 &startPosition,
                                                                     uint32 &endPosition) const noexcept
  {
    uint32 startFragment = fragment.CharacterOffset - Start();
    uint32 endFragment = startFragment + fragment.Length;

    // Find intersection between the intervals: [startFragment..endFragment) and [startPosition..endPosition)
    startPosition = std::max(startFragment, startPosition);
    endPosition = std::min(endFragment, endPosition);

    if (startPosition >= endPosition)
    {
      return false;
    }

    startPosition -= startFragment;
    endPosition -= startFragment;

    return true;
  }

  FloatRect SVGInlineTextBox::CalculateBoundaries() const noexcept
  {
    FloatRect textRect;

    float scalingFactor = Renderer().ScalingFactor();
    krys_debug_assert(scalingFactor);

    float baseline = Renderer().ScaledFont().MetricsOfPrimaryFont().Ascent() / scalingFactor;

    AffineTransform fragmentTransform;
    for (const auto &fragment : _textFragments)
    {
      FloatRect fragmentRect(fragment.X, fragment.Y - baseline, fragment.Width, fragment.Height);
      fragment.BuildFragmentTransform(fragmentTransform);
      if (!fragmentTransform.IsIdentity())
      {
        fragmentRect = fragmentTransform.MapRect(fragmentRect);
      }

      textRect.Unite(fragmentRect);
    }

    return textRect;
  }

  void SVGInlineTextBox::SetTextFragments(List<SVGTextFragment> &&fragments) noexcept
  {
    _textFragments = krys::move(fragments);
  }
}
