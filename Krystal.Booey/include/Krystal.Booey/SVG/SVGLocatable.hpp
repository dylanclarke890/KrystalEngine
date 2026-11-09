/*
 * Copyright (C) 2004, 2005 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) Research In Motion Limited 2010. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/FloatRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::svg
{
  class SVGElement;

  enum class CTMScope : bool
  {
    NearestViewportScope, // Used for getCTM()
    ScreenScope           // Used for getScreenCTM()
  };

  /// @brief The elements with a place in a coordinate system: their viewport elements, bounding box and
  /// transforms to their viewport and to the screen.
  class SVGLocatable
  {
  public:
    virtual ~SVGLocatable() noexcept = default;

    // 'SVGLocatable' functions
    KRYS_NODISCARD virtual SVGElement *NearestViewportElement() const noexcept = 0;
    KRYS_NODISCARD virtual SVGElement *FarthestViewportElement() const noexcept = 0;

    enum StyleUpdateStrategy : uint8
    {
      AllowStyleUpdate,
      DisallowStyleUpdate
    };

    KRYS_NODISCARD virtual FloatRect GetBBox(StyleUpdateStrategy) noexcept = 0;
    KRYS_NODISCARD virtual AffineTransform GetCTM(StyleUpdateStrategy) noexcept = 0;
    KRYS_NODISCARD virtual AffineTransform GetScreenCTM(StyleUpdateStrategy) noexcept = 0;

    KRYS_NODISCARD static SVGElement *NearestViewportElement(const SVGElement *element) noexcept;
    KRYS_NODISCARD static SVGElement *FarthestViewportElement(const SVGElement *element) noexcept;

  protected:
    KRYS_NODISCARD virtual AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept
    {
      return AffineTransform();
    }

    KRYS_NODISCARD static FloatRect GetBBox(SVGElement *element, StyleUpdateStrategy) noexcept;
    KRYS_NODISCARD static AffineTransform ComputeCTM(SVGElement *element, CTMScope,
                                                     StyleUpdateStrategy) noexcept;
  };
}
