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

#pragma once

#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"
#include "Krystal.Booey/Style/Values/FilterEffects/Filter.hpp"

namespace krys::boo
{
  /// @brief A `filter()` image: an image with filters applied.
  /// FIXME: Tracking the input image's cached image (`CachedImageClient`, `imageChanged`), loading reference
  /// filters' documents and drawing.
  class StyleFilterImage final : public StyleGeneratedImage
  {
  private:
    RefPtr<StyleImage> _image;
    style::Filter _filter;

  public:
    static constexpr bool IsFixedSize = true;

    KRYS_NODISCARD static Ref<StyleFilterImage> Create(RefPtr<StyleImage> image,
                                                       style::Filter filter) noexcept
    {
      return AdoptRef(*new StyleFilterImage(krys::move(image), krys::move(filter)));
    }
    ~StyleFilterImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &other) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleFilterImage &) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const StyleFilterImage &) const noexcept;

    KRYS_NODISCARD RefPtr<StyleImage> InputImage() const noexcept
    {
      return _image;
    }
    KRYS_NODISCARD const style::Filter &GetFilter() const noexcept
    {
      return _filter;
    }

  private:
    StyleFilterImage(RefPtr<StyleImage> &&, style::Filter &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleFilterImage, IsFilterImage)
