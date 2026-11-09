/*
 * Copyright (C) 2000 Lars Knoll (knoll@kde.org)
 *           (C) 2000 Antti Koivisto (koivisto@kde.org)
 *           (C) 2000 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2021 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo
{
  StyleGeneratedImage::StyleGeneratedImage(StyleImage::Type type, bool fixedSize) noexcept
      : StyleImage {type}, _fixedSize {fixedSize}
  {
  }

  StyleGeneratedImage::~StyleGeneratedImage() noexcept = default;

  // MARK: Client support.

  void StyleGeneratedImage::AddClient(RenderElement &renderer) noexcept
  {
    ++_clients[&renderer];

    DidAddClient(renderer);
  }

  void StyleGeneratedImage::RemoveClient(RenderElement &renderer) noexcept
  {
    auto found = _clients.find(&renderer);
    krys_debug_assert(found != _clients.end());
    if (found == _clients.end())
    {
      return;
    }
    if (!--found->second)
    {
      _clients.erase(found);
    }

    DidRemoveClient(renderer);
  }

  bool StyleGeneratedImage::HasClient(RenderElement &renderer) const noexcept
  {
    return _clients.contains(&renderer);
  }

  FloatSize StyleGeneratedImage::ImageSize(const RenderElement *renderer, float multiplier) const noexcept
  {
    if (!_fixedSize)
    {
      return _containerSize;
    }

    if (!renderer)
    {
      return {};
    }

    FloatSize fixedSize = FixedSize(*renderer);
    if (multiplier == 1.0f)
    {
      return fixedSize;
    }

    float width = fixedSize.Width() * multiplier;
    float height = fixedSize.Height() * multiplier;

    // Don't let images that have a width/height >= 1 shrink below 1 device pixel when zoomed.
    float deviceScaleFactor = renderer->GetDocument().DeviceScaleFactor();
    if (fixedSize.Width() > 0)
    {
      width = std::max<float>(1 / deviceScaleFactor, width);
    }
    if (fixedSize.Height() > 0)
    {
      height = std::max<float>(1 / deviceScaleFactor, height);
    }

    return {width, height};
  }

  void StyleGeneratedImage::ComputeIntrinsicDimensions(const RenderElement *renderer, float &intrinsicWidth,
                                                       float &intrinsicHeight,
                                                       FloatSize &intrinsicRatio) noexcept
  {
    // At a zoom level of 1 the image is guaranteed to have a device pixel size.
    FloatSize size = FloorSizeToDevicePixels(LayoutSize(ImageSize(renderer, 1)),
                                             renderer ? renderer->GetDocument().DeviceScaleFactor() : 1);
    intrinsicWidth = size.Width();
    intrinsicHeight = size.Height();
    intrinsicRatio = size;
  }
}
