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

#include "Krystal.Booey/CSS/Values/Primitives/URL.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Types/Markable.hpp"

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo::style
{
  /// @brief A computed `<url>`: the resolved URL and its modifiers.
  struct URL
  {
    url::URL Resolved;
    css::URLModifiers Modifiers;

    KRYS_NODISCARD bool operator==(const URL &) const noexcept = default;
  };

  template <size_t I>
  KRYS_NODISCARD const auto &get(const URL &value) noexcept
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
  KRYS_NODISCARD URL ToStyleWithScriptExecutionContext(const css::CSSURL &, const dom::Document &) noexcept;

  template <>
  struct ToCSS<URL>
  {
    KRYS_NODISCARD css::CSSURL operator()(const URL &, const RenderStyle &) noexcept;
  };

  template <>
  struct ToStyle<css::CSSURL>
  {
    KRYS_NODISCARD URL operator()(const css::CSSURL &, const BuilderState &) noexcept;
  };

  // `URL` is special-cased to return a `css::UrlValue`.
  template <>
  struct CSSValueCreation<URL>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const URL &) noexcept;
  };

  template <>
  struct CSSValueConversion<URL>
  {
    KRYS_NODISCARD URL operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: Serialisation

  template <>
  struct Serialiser<URL>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const URL &) noexcept;
  };
}

namespace krys
{
  template <>
  struct MarkableTraits<boo::style::URL>
  {
    KRYS_NODISCARD static bool IsEmptyValue(const boo::style::URL &value) noexcept
    {
      return value.Resolved.IsNull();
    }

    KRYS_NODISCARD static boo::style::URL EmptyValue() noexcept
    {
      return {.Resolved = {}, .Modifiers = {}};
    }
  };
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::style::URL, 2)
