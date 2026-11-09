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
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo
{
  StyleCrossfadeImage::StyleCrossfadeImage(RefPtr<StyleImage> &&from, RefPtr<StyleImage> &&to,
                                           double percentage) noexcept
      : StyleGeneratedImage {Type::CrossfadeImage, StyleCrossfadeImage::IsFixedSize},
        _from {krys::move(from)}, _to {krys::move(to)}, _percentage {percentage}
  {
  }

  StyleCrossfadeImage::~StyleCrossfadeImage() noexcept = default;

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

  RefPtr<StyleCrossfadeImage> StyleCrossfadeImage::Blend(KRYS_MAYBE_UNUSED const StyleCrossfadeImage &from,
                                                         const BlendingContext &) const noexcept
  {
    krys_debug_assert(EqualInputImages(from));

    // FIXME: Blend the percentages once both input images have loaded (their cached images), once the image
    // follows its inputs' cached images (see Load). Until then, they never have.
    return nullptr;
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
    // FIXME: Follow the input images' cached images as their client (imageChanged), and blend once they're
    // ready.
    if (_from && _from->IsPending())
    {
      _from->Load(loader, options);
    }

    if (_to && _to->IsPending())
    {
      _to->Load(loader, options);
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
}
