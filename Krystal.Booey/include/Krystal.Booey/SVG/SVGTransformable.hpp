/*
 * Copyright (C) 2004, 2005, 2008 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2004, 2005, 2006, 2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2007 Eric Seidel <eric@webkit.org>
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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/SVG/SVGLocatable.hpp"
#include "Krystal.Booey/SVG/SVGTransformValue.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::svg
{
  class SVGTransform;

  /// @brief The elements with a `transform` attribute, whose local coordinate space is their animated
  /// transform; parses the transforms of a `transform` attribute.
  class SVGTransformable : public SVGLocatable
  {
  public:
    virtual ~SVGTransformable() noexcept;

    static bool ParseAndReplaceTransform(SVGTransformValue::SVGTransformType, dom::DOMStringView &buffer,
                                         SVGTransform &) noexcept;
    KRYS_NODISCARD static RefPtr<SVGTransform> ParseTransform(SVGTransformValue::SVGTransformType,
                                                              dom::DOMStringView &buffer) noexcept;

    KRYS_NODISCARD static Maybe<SVGTransformValue::SVGTransformType>
      ParseTransformTypeFromString(dom::DOMStringView) noexcept;
    KRYS_NODISCARD static Maybe<SVGTransformValue::SVGTransformType>
      ParseTransformType(dom::DOMStringView &buffer) noexcept;

    KRYS_NODISCARD AffineTransform LocalCoordinateSpaceTransform(CTMScope) const noexcept override
    {
      return AnimatedLocalTransform();
    }

    KRYS_NODISCARD virtual AffineTransform AnimatedLocalTransform() const noexcept = 0;
  };
}
