/*
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
 * Copyright (C) 2013-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Graphics/SVGImageCache.hpp"
#include "Krystal.Booey/Graphics/Geometry/LayoutSize.hpp"
#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImageForContainer.hpp"

namespace krys::boo::svg
{
  SVGImageCache::SVGImageCache(SVGImage *svgImage) noexcept : _svgImage(CreateWeakPtr(svgImage))
  {
    krys_debug_assert(_svgImage);
  }

  SVGImageCache::~SVGImageCache() noexcept
  {
    _imageForContainerMap.clear();
  }

  void SVGImageCache::RemoveClientFromCache(const CachedImageClient *client) noexcept
  {
    krys_debug_assert(client);

    _imageForContainerMap.erase(client);
  }

  void SVGImageCache::SetContainerContextForClient(const CachedImageClient &client,
                                                   const LayoutSize &containerSize, float containerZoom,
                                                   const url::URL &imageURL) noexcept
  {
    krys_debug_assert(!containerSize.IsEmpty());
    krys_debug_assert(containerZoom);

    // SVG container has width or height less than 1 pixel.
    if (FlooredIntSize(containerSize).IsEmpty())
    {
      return;
    }

    FloatSize containerSizeWithoutZoom(containerSize);
    containerSizeWithoutZoom.Scale(1 / containerZoom);

    _imageForContainerMap.insert_or_assign(
      &client,
      SVGImageForContainer::Create(_svgImage.get(), containerSizeWithoutZoom, containerZoom, imageURL));
  }

  Image *SVGImageCache::FindImageForRenderer(const RenderObject *renderer) const noexcept
  {
    if (!renderer)
    {
      return nullptr;
    }
    auto it = _imageForContainerMap.find(&renderer->GetCachedImageClient());
    return it != _imageForContainerMap.end() ? it->second.get() : nullptr;
  }

  FloatSize SVGImageCache::ImageSizeForRenderer(const RenderObject *renderer) const noexcept
  {
    auto *image = FindImageForRenderer(renderer);
    return image ? image->Size() : _svgImage->Size();
  }

  // FIXME: This doesn't take into account the animation timeline so animations will not
  // restart on page load, nor will two animations in different pages have different timelines.
  Image *SVGImageCache::ImageForRenderer(const RenderObject *renderer) const noexcept
  {
    auto *image = FindImageForRenderer(renderer);
    if (!image)
    {
      return &Image::NullImage();
    }
    krys_debug_assert(!image->Size().IsEmpty());
    return image;
  }
}
