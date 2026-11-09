/*
 * Copyright (C) Research In Motion Limited 2010-2011. All rights reserved.
 * Copyright (C) 2024 Apple Inc. All rights reserved.
 * Copyright (C) 2015 Google Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/SVG/SVGTextMetrics.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <cmath>
#include <limits>

namespace krys::boo
{
  class RenderSVGInlineText;

  /// @brief The x/y/dx/dy/rotate values a character of SVG text was given.
  struct SVGCharacterData
  {
    float X {std::numeric_limits<float>::quiet_NaN()};
    float Y {std::numeric_limits<float>::quiet_NaN()};
    float Dx {std::numeric_limits<float>::quiet_NaN()};
    float Dy {std::numeric_limits<float>::quiet_NaN()};
    float Rotate {std::numeric_limits<float>::quiet_NaN()};
  };

  /// @brief Character data keyed by one plus a byte offset into a text renderer's text (or, while building,
  /// one plus a character's index in the whole text).
  using SVGCharacterDataMap = HashMap<uint32, SVGCharacterData>;

  /// @brief The layout attributes of an SVG text renderer: its characters' positioning data, and its metrics.
  class SVGTextLayoutAttributes
  {
    KRYS_NON_COPYABLE(SVGTextLayoutAttributes);

  private:
    RenderSVGInlineText &_context;
    SVGCharacterDataMap _characterDataMap;
    List<SVGTextMetrics> _textMetricsValues;

  public:
    explicit SVGTextLayoutAttributes(RenderSVGInlineText &) noexcept;

    void Clear() noexcept;

    KRYS_NODISCARD static constexpr float EmptyValue() noexcept
    {
      return std::numeric_limits<float>::quiet_NaN();
    }
    KRYS_NODISCARD static bool IsEmptyValue(float value) noexcept
    {
      return std::isnan(value);
    }

    KRYS_NODISCARD RenderSVGInlineText &Context() noexcept
    {
      return _context;
    }
    KRYS_NODISCARD const RenderSVGInlineText &Context() const noexcept
    {
      return _context;
    }

    KRYS_NODISCARD SVGCharacterDataMap &CharacterDataMap() noexcept
    {
      return _characterDataMap;
    }
    KRYS_NODISCARD const SVGCharacterDataMap &CharacterDataMap() const noexcept
    {
      return _characterDataMap;
    }

    KRYS_NODISCARD List<SVGTextMetrics> &TextMetricsValues() noexcept
    {
      return _textMetricsValues;
    }
    KRYS_NODISCARD const List<SVGTextMetrics> &TextMetricsValues() const noexcept
    {
      return _textMetricsValues;
    }
  };
}
