/*
    Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies)

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

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Core/Geometry/Rect.hpp"
#include "Krystal.Core/Geometry/TransformationMatrix.hpp"

namespace krys::ui
{
  class TextureMapper;

  /// @brief What a layer draws besides its contents: a backing store, a solid colour, an image.
  class TextureMapperPlatformLayer
  {
  public:
    class Client
    {
    public:
      virtual ~Client() noexcept = default;

      virtual void PlatformLayerWillBeDestroyed() noexcept = 0;

      virtual void SetPlatformLayerNeedsDisplay() noexcept = 0;
    };

  private:
    Client *_client {nullptr};

  public:
    TextureMapperPlatformLayer() noexcept = default;

    virtual ~TextureMapperPlatformLayer() noexcept = default;

    virtual void PaintToTextureMapper(TextureMapper &textureMapper, const RectF &targetRect,
                                      const TransformationMatrix &modelViewMatrix = {},
                                      float opacity = 1.0) noexcept = 0;

    virtual void DrawBorder(TextureMapper &textureMapper, const boo::Color &color, float borderWidth,
                            const RectF &targetRect, const TransformationMatrix &transform) noexcept;

    void SetClient(Client *client) noexcept
    {
      _client = client;
    }

  protected:
    KRYS_NODISCARD Client *GetClient() noexcept
    {
      return _client;
    }
  };
}
