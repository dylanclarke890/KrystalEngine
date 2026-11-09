/*
 * Copyright (C) 2011 Research In Motion Limited. All rights reserved.
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

#pragma once

#include "Krystal.Booey/Graphics/Geometry/FloatSize.hpp"
#include "Krystal.Booey/Graphics/Images/Image.hpp"
#include "Krystal.Booey/URL/URL.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo
{
  class CachedImageClient;
  class LayoutSize;
  class RenderObject;
}

namespace krys::boo::svg
{
  class SVGImage;
  class SVGImageForContainer;

  /// @brief The images a cached SVG image is drawn as, one per client, each at its client's container size.
  class SVGImageCache
  {
  private:
    using ImageForContainerMap = HashMap<const CachedImageClient *, RefPtr<SVGImageForContainer>>;

    WeakPtr<SVGImage> _svgImage;
    ImageForContainerMap _imageForContainerMap;

  public:
    explicit SVGImageCache(SVGImage *) noexcept;
    ~SVGImageCache() noexcept;

    void RemoveClientFromCache(const CachedImageClient *) noexcept;

    void SetContainerContextForClient(const CachedImageClient &, const LayoutSize &, float,
                                      const url::URL &) noexcept;
    KRYS_NODISCARD FloatSize ImageSizeForRenderer(const RenderObject *) const noexcept;

    KRYS_NODISCARD Image *ImageForRenderer(const RenderObject *) const noexcept;

  private:
    KRYS_NODISCARD Image *FindImageForRenderer(const RenderObject *) const noexcept;
  };
}
