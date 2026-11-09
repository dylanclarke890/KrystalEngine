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

#include "Krystal.Booey/Rendering/Style/StyleGradientImage.hpp"
#include "Krystal.Booey/CSS/Values/GradientValue.hpp"
#include "Krystal.Booey/Graphics/Images/GradientImage.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo
{
  StyleGradientImage::StyleGradientImage(style::Gradient &&gradient) noexcept
      : StyleGeneratedImage {Type::GradientImage, StyleGradientImage::IsFixedSize},
        _gradient {krys::move(gradient)}, _knownCacheableBarringFilter {style::StopsAreCacheable(_gradient)}
  {
  }

  StyleGradientImage::~StyleGradientImage() noexcept = default;

  bool StyleGradientImage::Equals(const StyleImage &other) const noexcept
  {
    const auto *otherGradientImage = DynamicDowncast<StyleGradientImage>(other);
    return otherGradientImage && Equals(*otherGradientImage);
  }

  bool StyleGradientImage::Equals(const StyleGradientImage &other) const noexcept
  {
    return _gradient == other._gradient;
  }

  Ref<css::Value> StyleGradientImage::ComputedStyleValue(const RenderStyle &style) const noexcept
  {
    return css::GradientValue::Create(style::ConvertToCSS(_gradient, style));
  }

  bool StyleGradientImage::IsPending() const noexcept
  {
    return false;
  }

  RefPtr<Image> StyleGradientImage::GetImage(const RenderElement *renderer, const FloatSize &size,
                                             const GraphicsContext &, bool isForFirstLine) const noexcept
  {
    if (!renderer)
    {
      return ShareRefPtr(&Image::NullImage());
    }

    if (size.IsEmpty())
    {
      return nullptr;
    }

    const auto &style = isForFirstLine ? renderer->FirstLineStyle() : renderer->Style();
    auto gradient = style::CreatePlatformGradient(_gradient, size, style);
    return GradientImage::Create(*gradient, size);
  }

  bool StyleGradientImage::KnownToBeOpaque(const RenderElement &renderer) const noexcept
  {
    return style::IsOpaque(_gradient, renderer.Style());
  }

  FloatSize StyleGradientImage::FixedSize(const RenderElement &) const noexcept
  {
    return {};
  }
}
