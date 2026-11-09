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

namespace krys::boo
{
  struct BlendingContext;

  /// @brief A `cross-fade()` image.
  /// FIXME: Tracking the input images' cached images (`CachedImageClient`, `imageChanged`) and drawing
  /// (`CrossfadeGeneratedImage`).
  class StyleCrossfadeImage final : public StyleGeneratedImage
  {
  private:
    RefPtr<StyleImage> _from;
    RefPtr<StyleImage> _to;
    double _percentage;

  public:
    static constexpr bool IsFixedSize = true;

    KRYS_NODISCARD static Ref<StyleCrossfadeImage> Create(RefPtr<StyleImage> from, RefPtr<StyleImage> to,
                                                          double percentage) noexcept
    {
      return AdoptRef(*new StyleCrossfadeImage(krys::move(from), krys::move(to), percentage));
    }
    ~StyleCrossfadeImage() noexcept override;

    KRYS_NODISCARD RefPtr<StyleCrossfadeImage> Blend(const StyleCrossfadeImage &,
                                                     const BlendingContext &) const noexcept;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleCrossfadeImage &) const noexcept;
    KRYS_NODISCARD bool EqualInputImages(const StyleCrossfadeImage &) const noexcept;

  private:
    StyleCrossfadeImage(RefPtr<StyleImage> &&, RefPtr<StyleImage> &&, double) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleCrossfadeImage, IsCrossfadeImage)
