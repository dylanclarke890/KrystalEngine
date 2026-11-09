/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"

namespace krys::boo::style
{
  // <'mask-border-source'> = none | <image>
  // https://drafts.fxtf.org/css-masking-1/#propdef-mask-border-source
  struct MaskBorderSource
  {
    MaskBorderSource(css::keywords::None) noexcept
    {
    }

    MaskBorderSource(ImageWrapper &&image) noexcept : _image {krys::move(image.Value)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_image;
    }
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return !!_image;
    }

    KRYS_NODISCARD Maybe<ImageWrapper> TryImage() const noexcept
    {
      return _image ? Maybe<ImageWrapper> {ImageWrapper {ShareRef(*_image)}} : std::nullopt;
    }
    KRYS_NODISCARD RefPtr<StyleImage> TryStyleImage() const noexcept
    {
      return _image;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(ImageWrapper {ShareRef(*_image)});
    }

    bool operator==(const MaskBorderSource &other) const noexcept
    {
      return ArePointingToEqualData(_image, other._image);
    }

  private:
    RefPtr<StyleImage> _image {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<MaskBorderSource>
  {
    KRYS_NODISCARD MaskBorderSource operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<MaskBorderSource>
  {
    KRYS_NODISCARD bool CanBlend(const MaskBorderSource &, const MaskBorderSource &) noexcept;
    KRYS_NODISCARD MaskBorderSource Blend(const MaskBorderSource &, const MaskBorderSource &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::MaskBorderSource)
