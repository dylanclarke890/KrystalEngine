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

#include "Krystal.Booey/Graphics/Geometry/IntPoint.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/Style/StyleImage.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <cursor-image> = [ <url> | <url-set> ] <number>{2}?
  // https://drafts.csswg.org/css-ui-4/#typedef-cursor-cursor-image
  struct CursorImage
  {
    Ref<StyleImage> Image;
    IntPoint HotSpot {-1, -1};

    KRYS_NODISCARD bool operator==(const CursorImage &) const noexcept = default;
  };

  using CursorImageList = css::CommaSeparatedRefCountedFixedVector<CursorImage>;

  // <'cursor'> = <cursor-image>#? <cursor-predefined>
  // https://drafts.csswg.org/css-ui-4/#propdef-cursor
  struct Cursor
  {
    using ImagesType = Maybe<CursorImageList>;

    ImagesType Images {};
    CursorType Predefined {CursorType::Auto};

    Cursor(ImagesType &&images, CursorType predefined) noexcept
        : Images {krys::move(images)}, Predefined {predefined}
    {
    }

    Cursor(const ImagesType &images, CursorType predefined) noexcept
        : Images {images}, Predefined {predefined}
    {
    }

    explicit Cursor(CursorType predefined) noexcept : Predefined {predefined}
    {
    }

    // Special constructor for use constructing initial 'auto' value.
    Cursor(css::keywords::Auto) noexcept : Predefined {CursorType::Auto}
    {
    }

    KRYS_NODISCARD bool operator==(const Cursor &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Cursor &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Images;
    }
    else if constexpr (I == 1)
    {
      return value.Predefined;
    }
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<Cursor>
  {
    KRYS_NODISCARD Cursor operator()(BuilderState &, const css::Value &) noexcept;
  };

  template <>
  struct CSSValueCreation<CursorImage>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const CursorImage &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<CursorImage>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const CursorImage &) noexcept;
  };
}

DEFINE_COMMA_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Cursor, 2)
