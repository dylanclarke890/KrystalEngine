/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 *
 */

#pragma once

#include "Krystal.Booey/Graphics/Images/ImagePaintingOptions.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class GraphicsContext;
  class LayoutRect;
  class LayoutSize;
  class RenderElement;
  class RenderStyle;

  namespace style
  {
    struct BorderImage;
    struct MaskBorder;
  }

  /// @brief Paints a nine-piece image (`border-image` or `mask-border`): the image's corners, sides and
  /// middle sliced, scaled and tiled into the border image area.
  class NinePieceImagePainter
  {
  public:
    static void Paint(const style::BorderImage &, GraphicsContext &, const RenderElement *,
                      const RenderStyle &, const LayoutRect &destination, const LayoutSize &source,
                      float deviceScaleFactor, ImagePaintingOptions) noexcept;
    static void Paint(const style::MaskBorder &, GraphicsContext &, const RenderElement *,
                      const RenderStyle &, const LayoutRect &destination, const LayoutSize &source,
                      float deviceScaleFactor, ImagePaintingOptions) noexcept;
  };
}
