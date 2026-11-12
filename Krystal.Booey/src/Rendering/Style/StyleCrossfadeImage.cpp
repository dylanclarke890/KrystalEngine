/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY,
 * OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
 * TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Style/StyleCrossfadeImage.hpp"
#include "Krystal.Booey/CSS/Values/CrossFadeValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Images/CrossfadeGeneratedImage.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImage.hpp"
#include "Krystal.Booey/SVG/Graphics/SVGImageForContainer.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleCrossfadeImage::StyleCrossfadeImage(RefPtr<StyleImage> &&from, RefPtr<StyleImage> &&to,
                                           double percentage) noexcept
      : StyleGeneratedImage {Type::CrossfadeImage, StyleCrossfadeImage::IsFixedSize},
        _from {krys::move(from)}, _to {krys::move(to)}, _percentage {percentage}
  {
  }

  StyleCrossfadeImage::~StyleCrossfadeImage() noexcept
  {
    if (_cachedFromImage)
    {
      _cachedFromImage->RemoveClient(*this);
    }
    if (_cachedToImage)
    {
      _cachedToImage->RemoveClient(*this);
    }
  }

  bool StyleCrossfadeImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherCrossfadeImage = DynamicDowncast<StyleCrossfadeImage>(other);
    return otherCrossfadeImage && Equals(*otherCrossfadeImage);
  }

  bool StyleCrossfadeImage::Equals(const StyleCrossfadeImage &other) const noexcept
  {
    return EqualInputImages(other) && _percentage == other._percentage;
  }

  bool StyleCrossfadeImage::EqualInputImages(const StyleCrossfadeImage &other) const noexcept
  {
    return ArePointingToEqualData(_from, other._from) && ArePointingToEqualData(_to, other._to);
  }

  RefPtr<StyleCrossfadeImage> StyleCrossfadeImage::Blend(const StyleCrossfadeImage &from,
                                                         const BlendingContext &context) const noexcept
  {
    krys_debug_assert(EqualInputImages(from));

    if (!_cachedToImage || !_cachedFromImage)
    {
      return nullptr;
    }

    auto newPercentage = boo::Blend(from._percentage, _percentage, context);
    return StyleCrossfadeImage::Create(_from, _to, newPercentage);
  }

  Ref<css::Value> StyleCrossfadeImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    auto fromComputedValue = _from ? _from->ComputedStyleValue(style)
                                   : Ref<css::Value> {css::PrimitiveValue::Create(css::ValueId::None)};
    auto toComputedValue = _to ? _to->ComputedStyleValue(style)
                               : Ref<css::Value> {css::PrimitiveValue::Create(css::ValueId::None)};
    return css::CrossFadeValue::Create(krys::move(fromComputedValue), krys::move(toComputedValue),
                                       css::PrimitiveValue::Create(_percentage));
  }

  void StyleCrossfadeImage::Load(CachedResourceLoader &loader, const ResourceLoaderOptions &options) noexcept
  {
    auto oldCachedFromImage = _cachedFromImage;
    auto oldCachedToImage = _cachedToImage;

    if (_from)
    {
      if (_from->IsPending())
      {
        _from->Load(loader, options);
      }
      _cachedFromImage = _from->GetCachedImage();
    }
    else
    {
      _cachedFromImage = nullptr;
    }

    if (_to)
    {
      if (_to->IsPending())
      {
        _to->Load(loader, options);
      }
      _cachedToImage = _to->GetCachedImage();
    }
    else
    {
      _cachedToImage = nullptr;
    }

    if (_cachedFromImage != oldCachedFromImage)
    {
      if (oldCachedFromImage)
      {
        oldCachedFromImage->RemoveClient(*this);
      }
      if (_cachedFromImage)
      {
        _cachedFromImage->AddClient(*this);
      }
    }

    if (_cachedToImage != oldCachedToImage)
    {
      if (oldCachedToImage)
      {
        oldCachedToImage->RemoveClient(*this);
      }
      if (_cachedToImage)
      {
        _cachedToImage->AddClient(*this);
      }
    }

    _inputImagesAreReady = true;
  }

  RefPtr<Image> StyleCrossfadeImage::GetImage(const RenderElement *renderer, const FloatSize &size,
                                              const GraphicsContext &destinationContext,
                                              bool isForFirstLine) const noexcept
  {
    if (!renderer)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    if (size.IsEmpty())
    {
      return nullptr;
    }

    if (!_from || !_to)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    RefPtr fromImage = _from->GetImage(renderer, size, destinationContext, isForFirstLine);
    RefPtr toImage = _to->GetImage(renderer, size, destinationContext, isForFirstLine);

    if (!fromImage || !toImage)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    if (auto *fromSVGImage = DynamicDowncast<svg::SVGImage>(fromImage.get()))
    {
      auto fromURL = _cachedFromImage ? _cachedFromImage->URL() : url::URL();
      fromImage = svg::SVGImageForContainer::Create(fromSVGImage, size, 1, fromURL);
    }
    if (auto *toSVGImage = DynamicDowncast<svg::SVGImage>(toImage.get()))
    {
      auto toURL = _cachedToImage ? _cachedToImage->URL() : url::URL();
      toImage = svg::SVGImageForContainer::Create(toSVGImage, size, 1, toURL);
    }

    return CrossfadeGeneratedImage::Create(*fromImage, *toImage, static_cast<float>(_percentage),
                                           FixedSize(*renderer), size);
  }

  bool StyleCrossfadeImage::KnownToBeOpaque(const RenderElement &renderer) const noexcept
  {
    if (_from && !_from->KnownToBeOpaque(renderer))
    {
      return false;
    }
    if (_to && !_to->KnownToBeOpaque(renderer))
    {
      return false;
    }
    return true;
  }

  void StyleCrossfadeImage::ImageChanged(CachedImage *, const IntRect *) noexcept
  {
    if (!_inputImagesAreReady)
    {
      return;
    }
    for (const auto &[client, count] : Clients())
    {
      client->ImageChanged(static_cast<WrappedImagePtr>(this));
    }
  }

  bool StyleCrossfadeImage::IsPending() const noexcept
  {
    if (_from && _from->IsPending())
    {
      return true;
    }

    if (_to && _to->IsPending())
    {
      return true;
    }

    return false;
  }

  FloatSize StyleCrossfadeImage::FixedSize(const RenderElement &renderer) const noexcept
  {
    if (!_from || !_to)
    {
      return {};
    }

    auto fromImageSize = _from->ImageSize(&renderer, 1);
    auto toImageSize = _to->ImageSize(&renderer, 1);

    // Rounding issues can cause transitions between images of equal size to return a different fixed size;
    // avoid performing the interpolation if the images are the same size.
    if (fromImageSize == toImageSize)
    {
      return fromImageSize;
    }

    auto percentage = static_cast<float>(_percentage);
    auto inversePercentage = 1 - percentage;

    return fromImageSize * inversePercentage + toImageSize * percentage;
  }
}
