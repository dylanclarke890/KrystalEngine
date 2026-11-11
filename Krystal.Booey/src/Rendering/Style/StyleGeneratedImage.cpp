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
#include "Krystal.Booey/Graphics/Images/GeneratedImage.hpp"
#include "Krystal.Booey/Platform/Timer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include <algorithm>
#include <chrono>

namespace krys::boo
{
  namespace
  {
    constexpr auto TimeToKeepCachedGeneratedImages = std::chrono::seconds(3);
  }

  /// @brief A generated image kept for its size, until it hasn't been asked for in a few seconds.
  class StyleGeneratedImage::CachedGeneratedImage
  {
    KRYS_NON_COPY_MOVABLE(CachedGeneratedImage);

  private:
    StyleGeneratedImage &_owner;
    const FloatSize _size;
    const Ref<GeneratedImage> _image;
    Timer _evictionTimer;

  public:
    CachedGeneratedImage(StyleGeneratedImage &owner, FloatSize size, GeneratedImage &image) noexcept
        : _owner(owner), _size(size), _image(ShareRef(image)),
          _evictionTimer(*this, &CachedGeneratedImage::EvictionTimerFired)
    {
      PuntEvictionTimer();
    }

    KRYS_NODISCARD FloatSize Size() const noexcept
    {
      return _size;
    }
    KRYS_NODISCARD GeneratedImage &Image() const noexcept
    {
      return *_image;
    }
    void PuntEvictionTimer() noexcept
    {
      _evictionTimer.StartOneShot(TimeToKeepCachedGeneratedImages);
    }

  private:
    void EvictionTimerFired() noexcept
    {
      // This destroys this entry.
      _owner.EvictCachedGeneratedImage(*this);
    }
  };

  GeneratedImage *StyleGeneratedImage::CachedImageForSize(FloatSize size) noexcept
  {
    if (size.IsEmpty())
    {
      return nullptr;
    }

    auto it = std::ranges::find_if(_images, [&](auto &entry) { return entry->Size() == size; });
    if (it == _images.end())
    {
      return nullptr;
    }
    (*it)->PuntEvictionTimer();
    return &(*it)->Image();
  }

  void StyleGeneratedImage::SaveCachedImageForSize(FloatSize size, GeneratedImage &image) noexcept
  {
    krys_debug_assert(std::ranges::none_of(_images, [&](auto &entry) { return entry->Size() == size; }));
    _images.push_back(CreateUnique<CachedGeneratedImage>(*this, size, image));
  }

  void StyleGeneratedImage::EvictCachedGeneratedImage(const CachedGeneratedImage &entry) noexcept
  {
    auto it = std::ranges::find_if(_images, [&](auto &cached) { return cached.get() == &entry; });
    krys_debug_assert(it != _images.end());
    _images.erase(it);
  }

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
