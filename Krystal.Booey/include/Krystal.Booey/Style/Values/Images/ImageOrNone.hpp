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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Style/Values/Images/ImageWrapper.hpp"

namespace krys::boo::style
{
  struct ImageOrNone
  {
    ImageOrNone(css::keywords::None) noexcept
    {
    }

    ImageOrNone(ImageWrapper &&image) noexcept : _value {krys::move(image.Value)}
    {
    }

    ImageOrNone(RefPtr<StyleImage> &&image) noexcept : _value {krys::move(image)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_value;
    }
    KRYS_NODISCARD bool IsImage() const noexcept
    {
      return !!_value;
    }

    KRYS_NODISCARD Maybe<ImageWrapper> TryImage() const noexcept
    {
      return _value ? Maybe<ImageWrapper> {ImageWrapper {ShareRef(*_value)}} : std::nullopt;
    }
    KRYS_NODISCARD RefPtr<StyleImage> TryStyleImage() const noexcept
    {
      return _value;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsNone())
      {
        return visitor(css::keywords::None {});
      }
      return visitor(ImageWrapper {ShareRef(*_value)});
    }

    KRYS_NODISCARD bool operator==(const ImageOrNone &other) const noexcept
    {
      return ArePointingToEqualData(_value, other._value);
    }

  private:
    RefPtr<StyleImage> _value {};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ImageOrNone>
  {
    KRYS_NODISCARD ImageOrNone operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ImageOrNone>
  {
    KRYS_NODISCARD bool CanBlend(const ImageOrNone &, const ImageOrNone &) noexcept;
    KRYS_NODISCARD ImageOrNone Blend(const ImageOrNone &, const ImageOrNone &,
                                     const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ImageOrNone)
