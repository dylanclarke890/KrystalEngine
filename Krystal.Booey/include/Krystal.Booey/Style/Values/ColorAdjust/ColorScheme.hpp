/*
 * Copyright (C) 2022 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/ColorAdjust/ColorScheme.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo::style
{
  /// @brief A computed `color-scheme`.
  struct ColorScheme
  {
    css::SpaceSeparatedList<css::CustomIdentifier> Schemes;
    Maybe<css::keywords::Only> Only;

    ColorScheme(css::keywords::Normal) noexcept : Schemes {}, Only {null}
    {
    }

    ColorScheme(css::SpaceSeparatedList<css::CustomIdentifier> &&schemes,
                Maybe<css::keywords::Only> only) noexcept
        : Schemes {krys::move(schemes)}, Only {only}
    {
    }

    // As an optimization, if `schemes` is empty, that indicates the
    // entire value should be considered `normal`.
    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return Schemes.empty();
    }

    KRYS_NODISCARD EnumSet<boo::ColorScheme> GetColorScheme() const noexcept;

    KRYS_NODISCARD bool operator==(const ColorScheme &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const ColorScheme &colorScheme) noexcept
  {
    if constexpr (!I)
    {
      return colorScheme.Schemes;
    }
    else if constexpr (I == 1)
    {
      return colorScheme.Only;
    }
  }

  DEFINE_TYPE_MAPPING(css::ColorScheme, ColorScheme)

  // MARK: - Conversion

  // `ColorScheme` is special-cased to return a `css::ColorSchemeValue`.
  template <>
  struct CSSValueCreation<ColorScheme>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const ColorScheme &) noexcept;
  };

  template <>
  struct CSSValueConversion<ColorScheme>
  {
    KRYS_NODISCARD ColorScheme operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Serialisation

  template <>
  struct Serialiser<ColorScheme>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const ColorScheme &) noexcept;
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::ColorScheme, 2)
