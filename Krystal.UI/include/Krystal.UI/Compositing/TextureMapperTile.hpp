/*
 Copyright (C) 2013 Nokia Corporation and/or its subsidiary(-ies)

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Library General Public
 License as published by the Free Software Foundation; either
 version 2 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Library General Public License for more details.

 You should have received a copy of the GNU Library General Public License
 along with this library; see the file COPYING.LIB.  If not, write to
 the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 Boston, MA 02110-1301, USA.
 */

#pragma once

#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"
#include "Krystal.UI/Compositing/BitmapTexture.hpp"

namespace krys::boo
{
  class GraphicsLayer;
  class Image;
}

namespace krys::ui
{
  class TextureMapper;

  /// @brief A part of a backing store, painted into a texture of its own.
  class TextureMapperTile
  {
  protected:
    RefPtr<BitmapTexture> _texture;

  private:
    RectF _rect;
    /// @brief What needs painting again, in the backing store's coordinates.
    RectI _dirtyRect;

  public:
    explicit TextureMapperTile(const RectF &rect) noexcept;

    virtual ~TextureMapperTile() noexcept;

    TextureMapperTile(TextureMapperTile &&) noexcept = default;

    TextureMapperTile &operator=(TextureMapperTile &&) noexcept = default;

    KRYS_NODISCARD RefPtr<BitmapTexture> Texture() const noexcept;

    KRYS_NODISCARD RectF Rect() const noexcept
    {
      return _rect;
    }

    void SetTexture(BitmapTexture *texture) noexcept;

    void SetRect(const RectF &rect) noexcept
    {
      _rect = rect;
    }

    KRYS_NODISCARD const RectI &DirtyRect() const noexcept
    {
      return _dirtyRect;
    }

    void AddDirtyRect(const RectI &rect) noexcept
    {
      _dirtyRect.Unite(rect);
    }

    void UpdateContents(TextureMapper &textureMapper, boo::Image &image, const RectI &dirtyRect) noexcept;

    void UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                        const RectI &dirtyRect, float scale = 1) noexcept;

    virtual void Paint(TextureMapper &textureMapper, const TransformationMatrix &transform, float opacity,
                       bool allEdgesExposed) noexcept;
  };
}
