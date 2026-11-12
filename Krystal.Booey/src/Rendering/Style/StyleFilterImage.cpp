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
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleFilterImage::StyleFilterImage(RefPtr<StyleImage> &&image, style::Filter &&filter) noexcept
      : StyleGeneratedImage {Type::FilterImage, StyleFilterImage::IsFixedSize}, _image {krys::move(image)},
        _filter {krys::move(filter)}
  {
  }

  StyleFilterImage::~StyleFilterImage() noexcept = default;

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
    // FIXME: Follow the input image's cached image as its client (imageChanged), and load the reference
    // filters' external documents.
    if (_image && _image->IsPending())
    {
      _image->Load(cachedResourceLoader, options);
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
