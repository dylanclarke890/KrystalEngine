/*
 * Copyright (C) 2013 Google Inc. All rights reserved.
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

#include "Krystal.Booey/SVG/Graphics/SVGImageForContainer.hpp"
#include "Krystal.Booey/Graphics/Geometry/IntSize.hpp"
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"

namespace krys::boo::svg
{
  SVGImageForContainer::SVGImageForContainer(SVGImage *image, const FloatSize &containerSize,
                                             float containerZoom, const url::URL &initialFragmentURL) noexcept
      : _image(CreateWeakPtr(image)), _containerSize(containerSize), _containerZoom(containerZoom),
        _initialFragmentURL(initialFragmentURL)
  {
  }

  FloatSize SVGImageForContainer::Size(ImageOrientation) const noexcept
  {
    FloatSize scaledContainerSize(_containerSize);
    scaledContainerSize.Scale(_containerZoom);
    return FloatSize(RoundedIntSize(scaledContainerSize));
  }

  ImageDrawResult SVGImageForContainer::Draw(GraphicsContext &context, const FloatRect &dstRect,
                                             const FloatRect &srcRect, ImagePaintingOptions options) noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    return image->DrawForContainer(context, _containerSize, _containerZoom, _initialFragmentURL, dstRect,
                                   srcRect, options);
  }

  void SVGImageForContainer::DrawPattern(GraphicsContext &context, const FloatRect &dstRect,
                                         const FloatRect &srcRect, const AffineTransform &patternTransform,
                                         const FloatPoint &phase, const FloatSize &spacing,
                                         ImagePaintingOptions options) noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    image->DrawPatternForContainer(context, _containerSize, _containerZoom, _initialFragmentURL, srcRect,
                                   patternTransform, phase, spacing, dstRect, options);
  }

  RefPtr<NativeImage> SVGImageForContainer::CurrentNativeImage() noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    return image->CurrentNativeImage();
  }
}
