/*
 * Copyright (C) 1999 Lars Knoll <knoll@kde.org>
 * Copyright (C) 1999 Antti Koivisto <koivisto@kde.org>
 * Copyright (C) 2000 Dirk Mueller <mueller@kde.org>
 * Copyright (C) 2006 Allan Sandfeld Jensen <kde@carewolf.com>
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Google Inc. All rights reserved.
 * Copyright (C) 2010 Patrick Gansterer <paroga@paroga.com>
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

#include "Krystal.Booey/Rendering/Replaced/RenderImageResourceStyleImage.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderImageResourceStyleImage::RenderImageResourceStyleImage(StyleImage &styleImage) noexcept
      : _styleImage(ShareRef(styleImage))
  {
  }

  void RenderImageResourceStyleImage::Initialize(RenderElement &renderer) noexcept
  {
    RenderImageResource::Initialize(renderer,
                                    _styleImage->HasCachedImage() ? _styleImage->GetCachedImage() : nullptr);
    _styleImage->AddClient(renderer);
  }

  void RenderImageResourceStyleImage::Shutdown() noexcept
  {
    RenderImageResource::Shutdown();
    if (auto *renderer = Renderer())
    {
      _styleImage->RemoveClient(*renderer);
    }
  }

  RefPtr<Image> RenderImageResourceStyleImage::GetImage(const IntSize &size) const noexcept
  {
    // Generated content may trigger calls to image() while we're still pending, don't assert but gracefully
    // exit.
    if (_styleImage->IsPending())
    {
      return ShareRefPtr(&Image::NullImage());
    }
    if (auto image = _styleImage->GetImage(Renderer(), FloatSize(size), NullGraphicsContext()))
    {
      return image;
    }
    return ShareRefPtr(&Image::NullImage());
  }

  void RenderImageResourceStyleImage::SetContainerContext(const IntSize &size, const url::URL &) noexcept
  {
    if (auto *renderer = Renderer())
    {
      _styleImage->SetContainerContextForRenderer(*renderer, FloatSize(size), renderer->Style().UsedZoom());
    }
  }
}
