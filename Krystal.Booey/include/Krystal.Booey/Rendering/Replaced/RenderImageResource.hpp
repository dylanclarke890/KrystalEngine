/*
 * Copyright (C) 1999 Lars Knoll <knoll@kde.org>
 * Copyright (C) 1999 Antti Koivisto <koivisto@kde.org>
 * Copyright (C) 2006 Allan Sandfeld Jensen <kde@carewolf.com>
 * Copyright (C) 2006 Samuel Weinig <sam.weinig@gmail.com>
 * Copyright (C) 2004, 2005, 2006, 2007, 2009, 2010 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Graphics/Images/ImageTypes.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedResourceHandle.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  class Image;
  class RenderElement;

  /// @brief The image a `RenderImage` shows: a cached image its element loaded, or (in a subclass) a style
  /// image from generated content.
  class RenderImageResource
  {
    KRYS_NON_COPY_MOVABLE(RenderImageResource);

  private:
    RenderElement *_renderer {nullptr};
    CachedResourceHandle<CachedImage> _cachedImage;
    bool _cachedImageRemoveClientIsNeeded {true};

  public:
    RenderImageResource() noexcept;
    virtual ~RenderImageResource() noexcept;

    virtual void Initialize(RenderElement &renderer) noexcept
    {
      Initialize(renderer, nullptr);
    }
    virtual void Shutdown() noexcept;

    void SetCachedImage(CachedResourceHandle<CachedImage> &&newImage) noexcept;
    KRYS_NODISCARD CachedImage *GetCachedImage() const noexcept
    {
      return _cachedImage.get();
    }

    void ResetAnimation() noexcept;

    KRYS_NODISCARD virtual RefPtr<Image> GetImage(const IntSize &size = {}) const noexcept;
    KRYS_NODISCARD virtual bool CurrentFrameIsComplete() const noexcept;
    KRYS_NODISCARD virtual bool ErrorOccurred() const noexcept
    {
      return _cachedImage && _cachedImage->ErrorOccurred();
    }

    virtual void SetContainerContext(const IntSize &, const url::URL &) noexcept;

    KRYS_NODISCARD virtual bool ImageHasRelativeWidth() const noexcept
    {
      return _cachedImage && _cachedImage->ImageHasRelativeWidth();
    }
    KRYS_NODISCARD virtual bool ImageHasRelativeHeight() const noexcept
    {
      return _cachedImage && _cachedImage->ImageHasRelativeHeight();
    }

    KRYS_NODISCARD LayoutSize ImageSize(float multiplier) const noexcept
    {
      return ImageSize(multiplier, CachedImage::UsedSize);
    }
    KRYS_NODISCARD LayoutSize IntrinsicSize(float multiplier) const noexcept
    {
      return ImageSize(multiplier, CachedImage::IntrinsicSize);
    }

    KRYS_NODISCARD virtual WrappedImagePtr ImagePtr() const noexcept
    {
      return _cachedImage.get();
    }

  protected:
    KRYS_NODISCARD RenderElement *Renderer() const noexcept
    {
      return _renderer;
    }
    void Initialize(RenderElement &renderer, CachedImage *styleCachedImage) noexcept;

  private:
    KRYS_NODISCARD virtual LayoutSize ImageSize(float multiplier, CachedImage::SizeType type) const noexcept;
  };
}
