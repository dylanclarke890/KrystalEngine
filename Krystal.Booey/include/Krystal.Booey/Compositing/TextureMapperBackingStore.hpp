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

#pragma once

#include "Krystal.Booey/Compositing/TextureMapperPlatformLayer.hpp"

namespace krys::boo
{
  /// @brief A layer's painted contents, drawn by the compositor.
  class TextureMapperBackingStore : public TextureMapperPlatformLayer
  {
  public:
    ~TextureMapperBackingStore() noexcept override = default;

    void PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                              const TransformationMatrix &modelViewMatrix,
                              float opacity) noexcept override = 0;

    virtual void DrawRepaintCounter(TextureMapper &, int32, const web::Color &, const RectF &,
                                    const TransformationMatrix &) noexcept
    {
    }

  protected:
    KRYS_NODISCARD static bool AllTileEdgesExposed(const RectF &totalRect, const RectF &tileRect) noexcept;
  };

  /// @brief A backing store of one colour.
  class TextureMapperSolidColorLayer : public TextureMapperBackingStore
  {
  private:
    web::Color _color;

  public:
    void SetColor(web::Color color) noexcept
    {
      _color = krys::move(color);
    }

    void PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                              const TransformationMatrix &transform, float opacity) noexcept override;
  };
}
