/*
 Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies)

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

#include "Krystal.UI/Compositing/TextureMapperBackingStore.hpp"
#include "Krystal.UI/Compositing/TextureMapper.hpp"

namespace krys::ui
{
  void TextureMapperPlatformLayer::DrawBorder(TextureMapper &textureMapper, const boo::Color &color,
                                              float borderWidth, const RectF &targetRect,
                                              const TransformationMatrix &transform) noexcept
  {
    textureMapper.DrawBorder(color, borderWidth, targetRect, transform);
  }

  bool TextureMapperBackingStore::AllTileEdgesExposed(const RectF &totalRect, const RectF &tileRect) noexcept
  {
    return !tileRect.X() && !tileRect.Y() && tileRect.Width() + tileRect.X() >= totalRect.Width()
           && tileRect.Height() + tileRect.Y() >= totalRect.Height();
  }

  void TextureMapperSolidColorLayer::PaintToTextureMapper(TextureMapper &textureMapper,
                                                          const RectF &targetRect,
                                                          const TransformationMatrix &transform,
                                                          float opacity) noexcept
  {
    textureMapper.DrawSolidColor(targetRect, transform, _color.ColorWithAlphaMultipliedBy(opacity), true);
  }
}
