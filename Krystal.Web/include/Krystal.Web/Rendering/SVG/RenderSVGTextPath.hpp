/*
 * Copyright (C) 2007 Nikolas Zimmermann <zimmermann@kde.org>
 * Copyright (C) 2009 Apple Inc. All rights reserved.
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

#include "Krystal.Web/Graphics/Paths/Path.hpp"
#include "Krystal.Web/Rendering/SVG/RenderSVGInline.hpp"

namespace krys::web
{
  class SVGGeometryElement;
  class SVGLengthValue;
  class SVGTextPathElement;

  /// @brief The renderer of a `<textPath>`: its text is laid out along the referenced shape's path.
  class RenderSVGTextPath final : public RenderSVGInline
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(RenderSVGTextPath);

  public:
    RenderSVGTextPath(SVGTextPathElement &, RenderStyle &&) noexcept;

    ~RenderSVGTextPath() noexcept override;

    KRYS_NODISCARD SVGTextPathElement &TextPathElement() const noexcept;

    KRYS_NODISCARD SVGGeometryElement *TargetElement() const noexcept;

    KRYS_NODISCARD Path LayoutPath() const noexcept;

    KRYS_NODISCARD const SVGLengthValue &StartOffset() const noexcept;

    void GraphicsElement() const = delete;

  private:
    KRYS_NODISCARD utf8_stringview RenderName() const noexcept override
    {
      return u8"RenderSVGTextPath";
    }
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::RenderSVGTextPath)
  KRYS_NODISCARD static bool IsType(const krys::web::RenderObject &renderer) noexcept
  {
    return renderer.IsRenderSVGTextPath();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
