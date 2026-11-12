/*
 * Copyright (C) 2022-2023 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Style/StyleFilterImage.hpp"
#include "Krystal.Booey/CSS/Values/FilterImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/Graphics/Images/FilterGeneratedImage.hpp"
#include "Krystal.Booey/Graphics/NullGraphicsContext.hpp"
#include "Krystal.Booey/Loader/Cache/CachedImage.hpp"
#include "Krystal.Booey/Page/Page.hpp"
#include "Krystal.Booey/Rendering/Painting/CSSFilterRenderer.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleFilterImage::StyleFilterImage(RefPtr<StyleImage> &&image, style::Filter &&filter) noexcept
      : StyleGeneratedImage {Type::FilterImage, StyleFilterImage::IsFixedSize}, _image {krys::move(image)},
        _filter {krys::move(filter)}
  {
  }

  StyleFilterImage::~StyleFilterImage() noexcept
  {
    if (_cachedImage)
    {
      _cachedImage->RemoveClient(*this);
    }
  }

  bool StyleFilterImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherFilterImage = DynamicDowncast<StyleFilterImage>(other);
    return otherFilterImage && Equals(*otherFilterImage);
  }

  bool StyleFilterImage::Equals(const StyleFilterImage &other) const noexcept
  {
    return EqualInputImages(other) && _filter == other._filter;
  }

  bool StyleFilterImage::EqualInputImages(const StyleFilterImage &other) const noexcept
  {
    return ArePointingToEqualData(_image, other._image);
  }

  Ref<css::Value> StyleFilterImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    return css::FilterImageValue::Create(
      _image ? _image->ComputedStyleValue(style)
             : Ref<css::Value> {css::PrimitiveValue::Create(css::ValueId::None)},
      style::ConvertToCSS(_filter, style));
  }

  void StyleFilterImage::Load(CachedResourceLoader &cachedResourceLoader,
                              const ResourceLoaderOptions &options) noexcept
  {
    auto oldCachedImage = _cachedImage;

    if (_image)
    {
      if (_image->IsPending())
      {
        _image->Load(cachedResourceLoader, options);
      }
      _cachedImage = _image->GetCachedImage();
    }
    else
    {
      _cachedImage = nullptr;
    }

    if (_cachedImage != oldCachedImage)
    {
      if (oldCachedImage)
      {
        oldCachedImage->RemoveClient(*this);
      }
      if (_cachedImage)
      {
        _cachedImage->AddClient(*this);
      }
    }

    // FIXME: Load the reference filters' external documents (loadExternalDocumentIfNeeded).
    _inputImageIsReady = true;
  }

  RefPtr<Image> StyleFilterImage::GetImage(const RenderElement *renderer, const FloatSize &size,
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

    if (!_image)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    auto image = _image->GetImage(renderer, size, destinationContext, isForFirstLine);
    if (!image || image->IsNull())
    {
      return ShareRefPtr(&Image::NullImage());
    }

    auto *page = renderer->GetDocument().GetPage();
    if (!page)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    auto preferredFilterRenderingModes = page->PreferredFilterRenderingModes(destinationContext);
    auto sourceImageRect = FloatRect {FloatPoint(), size};

    auto cssFilter = CSSFilterRenderer::Create(const_cast<RenderElement &>(*renderer), _filter,
                                               {
                                                 .ReferenceBox = sourceImageRect,
                                                 .FilterRegion = sourceImageRect,
                                                 .Scale = {1, 1},
                                               },
                                               preferredFilterRenderingModes, NullGraphicsContext());
    if (!cssFilter)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    cssFilter->SetFilterRegion(sourceImageRect);

    // Drawn through the filter as it's painted, rather than into a filtered bitmap here.
    return FilterGeneratedImage::Create(*image, *cssFilter, size);
  }

  void StyleFilterImage::ImageChanged(CachedImage *, const IntRect *) noexcept
  {
    if (!_inputImageIsReady)
    {
      return;
    }
    for (const auto &[client, count] : Clients())
    {
      client->ImageChanged(static_cast<WrappedImagePtr>(this));
    }
  }

  bool StyleFilterImage::IsPending() const noexcept
  {
    return _image && _image->IsPending();
  }

  FloatSize StyleFilterImage::FixedSize(const RenderElement &renderer) const noexcept
  {
    if (_image)
    {
      return _image->ImageSize(&renderer, 1);
    }
    return {};
  }
}
