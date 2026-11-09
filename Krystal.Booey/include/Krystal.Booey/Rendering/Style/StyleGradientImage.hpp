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
#include "Krystal.Booey/Style/Values/Images/Gradient.hpp"

namespace krys::boo
{
  /// @brief A gradient image.
  class StyleGradientImage final : public StyleGeneratedImage
  {
  private:
    style::Gradient _gradient;
    // FIXME: Decides whether the generated image may be cached per size, once it is.
    bool _knownCacheableBarringFilter {false};

  public:
    static constexpr bool IsFixedSize = false;

    KRYS_NODISCARD static Ref<StyleGradientImage> Create(style::Gradient gradient) noexcept
    {
      return AdoptRef(*new StyleGradientImage(krys::move(gradient)));
    }
    ~StyleGradientImage() noexcept override;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD bool Equals(const StyleGradientImage &) const noexcept;

  private:
    explicit StyleGradientImage(style::Gradient &&) noexcept;

    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final
    {
    }

    KRYS_NODISCARD RefPtr<Image> GetImage(const RenderElement *, const FloatSize &, const GraphicsContext &,
                                          bool isForFirstLine) const noexcept final;
    KRYS_NODISCARD bool KnownToBeOpaque(const RenderElement &) const noexcept final;
    KRYS_NODISCARD FloatSize FixedSize(const RenderElement &) const noexcept final;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StyleGradientImage, IsGradientImage)
