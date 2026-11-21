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

#include "Krystal.UI/Compositing/TextureMapperTile.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  TextureMapperTile::TextureMapperTile(const RectF &rect) noexcept : _rect(rect)
  {
  }

  TextureMapperTile::~TextureMapperTile() noexcept = default;

  RefPtr<BitmapTexture> TextureMapperTile::Texture() const noexcept
  {
    return _texture;
  }

  void TextureMapperTile::SetTexture(BitmapTexture *texture) noexcept
  {
    _texture = ShareRefPtr(texture);
  }

  void TextureMapperTile::UpdateContents(TextureMapper &textureMapper, boo::Image &image,
                                         const RectI &dirtyRect) noexcept
  {
    RectI targetRect = EnclosingRectI(_rect);
    targetRect.Intersect(dirtyRect);
    if (targetRect.IsEmpty())
    {
      return;
    }
    PointI sourceOffset = targetRect.Location();

    // Normalize sourceRect to the buffer's coordinates.
    sourceOffset.Move(-dirtyRect.X(), -dirtyRect.Y());

    // Normalize targetRect to the texture's coordinates.
    targetRect.Move(-static_cast<int32>(_rect.X()), -static_cast<int32>(_rect.Y()));
    if (!_texture)
    {
      EnumSet<BitmapTexture::Flags> flags;
      if (!image.CurrentFrameKnownToBeOpaque())
      {
        flags.Add(BitmapTexture::Flags::SupportsAlpha);
      }
      _texture = BitmapTexture::Create(textureMapper.Device(), EnclosingRectI(_rect).Size(), flags);
    }

    textureMapper.UpdateContents(*_texture, image, targetRect, sourceOffset);
  }

  void TextureMapperTile::UpdateContents(TextureMapper &textureMapper, const boo::GraphicsLayer &sourceLayer,
                                         const RectI &dirtyRect, float scale) noexcept
  {
    RectI targetRect = EnclosingRectI(_rect);
    targetRect.Intersect(dirtyRect);
    _dirtyRect = {};
    if (targetRect.IsEmpty())
    {
      return;
    }
    PointI sourceOffset = targetRect.Location();

    // Normalize targetRect to the texture's coordinates.
    targetRect.Move(-static_cast<int32>(_rect.X()), -static_cast<int32>(_rect.Y()));
    if (!_texture)
    {
      _texture = BitmapTexture::Create(textureMapper.Device(), EnclosingRectI(_rect).Size(),
                                       {BitmapTexture::Flags::SupportsAlpha});
    }

    textureMapper.UpdateContents(*_texture, sourceLayer, targetRect, sourceOffset, scale);
  }

  void TextureMapperTile::Paint(TextureMapper &textureMapper, const TransformationMatrix &transform,
                                float opacity, bool allEdgesExposed) noexcept
  {
    if (auto texture = Texture())
    {
      textureMapper.DrawTexture(*texture, Rect(), transform, opacity,
                                allEdgesExposed ? TextureMapper::AllEdgesExposed::Yes
                                                : TextureMapper::AllEdgesExposed::No);
    }
  }
}
