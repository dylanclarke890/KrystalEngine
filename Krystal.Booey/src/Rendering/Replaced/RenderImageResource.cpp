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

#include "Krystal.Booey/Rendering/Replaced/RenderImageResource.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/Replaced/RenderImage.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  RenderImageResource::RenderImageResource() noexcept = default;

  RenderImageResource::~RenderImageResource() noexcept = default;

  void RenderImageResource::Initialize(RenderElement &renderer, CachedImage *styleCachedImage) noexcept
  {
    krys_debug_assert(!_renderer);
    krys_debug_assert(!_cachedImage);
    _renderer = &renderer;
    _cachedImage = styleCachedImage;
    _cachedImageRemoveClientIsNeeded = !styleCachedImage;
  }

  void RenderImageResource::Shutdown() noexcept
  {
    GetImage()->StopAnimation();
    SetCachedImage(nullptr);
  }

  void RenderImageResource::SetCachedImage(CachedResourceHandle<CachedImage> &&newImage) noexcept
  {
    if (_cachedImage == newImage)
    {
      return;
    }

    if (_cachedImage && _renderer && _cachedImageRemoveClientIsNeeded)
    {
      _cachedImage->RemoveClient(_renderer->GetCachedImageClient());
    }
    if (!_renderer)
    {
      return;
    }
    _cachedImage = krys::move(newImage);
    _cachedImageRemoveClientIsNeeded = true;
    if (!_cachedImage)
    {
      return;
    }

    _cachedImage->AddClient(_renderer->GetCachedImageClient());
    if (_cachedImage->ErrorOccurred())
    {
      _renderer->ImageChanged(_cachedImage.get());
    }
  }

  void RenderImageResource::ResetAnimation() noexcept
  {
    if (!_cachedImage)
    {
      return;
    }

    GetImage()->ResetAnimation();

    if (_renderer && !_renderer->NeedsLayout())
    {
      _renderer->Repaint();
    }
  }

  RefPtr<Image> RenderImageResource::GetImage(const IntSize &) const noexcept
  {
    if (!_cachedImage)
    {
      return ShareRefPtr(&Image::NullImage());
    }
    if (auto *image = _cachedImage->ImageForRenderer(_renderer))
    {
      return ShareRefPtr(image);
    }
    return ShareRefPtr(&Image::NullImage());
  }

  bool RenderImageResource::CurrentFrameIsComplete() const noexcept
  {
    if (!_cachedImage)
    {
      return false;
    }

    return _cachedImage->CurrentFrameIsComplete(_renderer);
  }

  void RenderImageResource::SetContainerContext(const IntSize &imageContainerSize,
                                                const url::URL &imageURL) noexcept
  {
    if (!_cachedImage || !_renderer)
    {
      return;
    }
    _cachedImage->SetContainerContextForClient(_renderer->GetCachedImageClient(),
                                               LayoutSize(imageContainerSize), _renderer->Style().UsedZoom(),
                                               imageURL);
  }

  LayoutSize RenderImageResource::ImageSize(float multiplier, CachedImage::SizeType type) const noexcept
  {
    if (!_cachedImage)
    {
      return {};
    }
    LayoutSize size = _cachedImage->ImageSizeForRenderer(_renderer, multiplier, type);
    if (auto *renderImage = DynamicDowncast<RenderImage>(_renderer))
    {
      size.Scale(renderImage->ImageDevicePixelRatio());
    }
    return size;
  }
}
