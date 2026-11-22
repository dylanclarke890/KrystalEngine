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

#include "Krystal.Booey/CSS/Values/Primitives/Url.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Strings/StringBuilder.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo
{
  class Document;
}

namespace krys::boo::style
{
  /// @brief A computed `<url>`: the resolved URL and its modifiers.
  struct Url
  {
    net::Url Resolved;
    css::UrlModifiers Modifiers;

    KRYS_NODISCARD bool operator==(const Url &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const Url &value) noexcept
  {
    if constexpr (!I)
    {
      return value.Resolved;
    }
    if constexpr (I == 1)
    {
      return value.Modifiers;
    }
  }

  // MARK: Conversion

  /// @brief Special conversion function for use by filters and font-face code.
  KRYS_NODISCARD Url ToStyleWithScriptExecutionContext(const css::CSSUrl &, const Document &) noexcept;

  template <>
  struct ToCSS<Url>
  {
    KRYS_NODISCARD css::CSSUrl operator()(const Url &, const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::CSSUrl>
  {
    KRYS_NODISCARD Url operator()(const css::CSSUrl &, const BuilderState &) noexcept;
  };

  // `Url` is special-cased to return a `css::UrlValue`.
  template <>
  struct CSSValueCreation<Url>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const Url &) noexcept;
  };

  template <>
  struct CSSValueConversion<Url>
  {
    KRYS_NODISCARD Url operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: Serialisation

  template <>
  struct Serialiser<Url>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const Url &) noexcept;
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::style::Url>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::style::Url &value) noexcept
    {
      return value.Resolved.IsNull();
    }

    KRYS_NODISCARD static boo::style::Url EmptyValue() noexcept
    {
      return {.Resolved = {}, .Modifiers = {}};
    }
  };
}

KRYS_DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::Url, 2)
