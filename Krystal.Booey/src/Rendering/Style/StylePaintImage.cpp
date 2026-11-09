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

#include "Krystal.Booey/Rendering/Style/StylePaintImage.hpp"
#include "Krystal.Booey/CSS/Values/PaintImageValue.hpp"
#include "Krystal.Booey/CSS/Values/VariableData.hpp"

namespace krys::boo
{
  StylePaintImage::StylePaintImage(css::CSSOMString &&name, Ref<css::VariableData> &&arguments) noexcept
      : StyleGeneratedImage {Type::PaintImage, StylePaintImage::IsFixedSize}, _name {krys::move(name)},
        _arguments {krys::move(arguments)}
  {
  }

  StylePaintImage::~StylePaintImage() noexcept = default;

  bool StylePaintImage::Equals(const StyleImage &other) const noexcept
  {
    // FIXME: Should probably also compare arguments?
    const auto *otherPaintImage = DynamicDowncast<StylePaintImage>(other);
    return otherPaintImage && otherPaintImage->_name == _name;
  }

  Ref<css::Value> StylePaintImage::ComputedStyleValue(const RenderStyle &) const noexcept
  {
    return css::PaintImageValue::Create(_name, _arguments);
  }

  bool StylePaintImage::IsPending() const noexcept
  {
    return false;
  }
}
