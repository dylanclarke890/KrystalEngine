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
#include "Krystal.Booey/Graphics/Images/NativeImage.hpp"
#include "Krystal.Core/Geometry/Size.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo
{
  SVGImageForContainer::SVGImageForContainer(SVGImage *image, const SizeF &containerSize, float containerZoom,
                                             net::Url initialFragmentUrl) noexcept
      : _image(CreateWeakPtr(image)), _containerSize(containerSize), _containerZoom(containerZoom),
        _initialFragmentUrl(krys::move(initialFragmentUrl))
  {
  }

  SizeF SVGImageForContainer::Size(ImageOrientation) const noexcept
  {
    SizeF scaledContainerSize(_containerSize);
    scaledContainerSize.Scale(_containerZoom);
    return SizeF(RoundedSizeI(scaledContainerSize));
  }

  ImageDrawResult SVGImageForContainer::Draw(GraphicsContext &context, const RectF &dstRect,
                                             const RectF &srcRect, ImagePaintingOptions options) noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    return image->DrawForContainer(context, _containerSize, _containerZoom, _initialFragmentUrl, dstRect,
                                   srcRect, options);
  }

  void SVGImageForContainer::DrawPattern(GraphicsContext &context, const RectF &dstRect, const RectF &srcRect,
                                         const AffineTransform &patternTransform, const PointF &phase,
                                         const SizeF &spacing, ImagePaintingOptions options) noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    image->DrawPatternForContainer(context, _containerSize, _containerZoom, _initialFragmentUrl, srcRect,
                                   patternTransform, phase, spacing, dstRect, options);
  }

  RefPtr<NativeImage> SVGImageForContainer::CurrentNativeImage() noexcept
  {
    RefPtr<SVGImage> image = ShareRefPtr(_image.get());
    return image->CurrentNativeImage();
  }
}
