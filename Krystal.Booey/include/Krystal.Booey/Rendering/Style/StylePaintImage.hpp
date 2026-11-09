/*
 * Copyright (C) 2022-2025 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Rendering/Style/StyleGeneratedImage.hpp"

namespace krys::boo::css
{
  class VariableData;
}

namespace krys::boo
{
  /// @brief A `paint()` image, drawn by a paint worklet.
  /// FIXME: Drawing (`CustomPaintImage` and the paint worklet lookup) comes with worklets.
  class StylePaintImage final : public StyleGeneratedImage
  {
  private:
    css::CSSOMString _name;
    const Ref<css::VariableData> _arguments;

  public:
    static constexpr bool IsFixedSize = false;

    KRYS_NODISCARD static Ref<StylePaintImage> Create(css::CSSOMString name,
                                                      Ref<css::VariableData> arguments) noexcept
    {
      return AdoptRef(*new StylePaintImage(krys::move(name), krys::move(arguments)));
    }
    ~StylePaintImage() noexcept override;

  private:
    StylePaintImage(css::CSSOMString &&, Ref<css::VariableData> &&) noexcept;

    KRYS_NODISCARD bool Equals(const StyleImage &) const noexcept final;
    KRYS_NODISCARD Ref<css::Value> ComputedStyleValue(const RenderStyle &) const noexcept final;
    KRYS_NODISCARD bool IsPending() const noexcept final;
    void Load(CachedResourceLoader &, const ResourceLoaderOptions &) noexcept final
    {
    }
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_IMAGE(StylePaintImage, IsPaintImage)
