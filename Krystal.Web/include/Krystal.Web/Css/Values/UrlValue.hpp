/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer.
 * 2. Redistributions in binary form must reproduce the above
 *    copyright notice, this list of conditions and the following
 *    disclaimer in the documentation and/or other materials
 *    provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDER “AS IS” AND ANY
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

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Web/Css/Values/Primitives/Url.hpp"
#include "Krystal.Web/Css/Values/Value.hpp"

namespace krys::web::css
{
  /// @brief A `<url>` value, e.g. `url("image.png")`.
  class UrlValue final : public Value
  {
  private:
    css::Url _url;

    explicit UrlValue(css::Url &&url) noexcept : Value(ValueType::Url), _url {krys::move(url)}
    {
    }

  public:
    KRYS_NODISCARD static Ref<UrlValue> Create(css::Url url) noexcept
    {
      return AdoptRef(*new UrlValue(krys::move(url)));
    }

    KRYS_NODISCARD const css::Url &Url() const noexcept
    {
      return _url;
    }

    KRYS_NODISCARD CSSOMString StringValue() const noexcept
    {
      return _url.Specified;
    }

    KRYS_NODISCARD CSSOMString CustomCSSText(const SerialisationContext &context) const noexcept;

    KRYS_NODISCARD bool Equals(const UrlValue &other) const noexcept;

    KRYS_NODISCARD IterationStatus
      CustomVisitChildren(KRYS_NOESCAPE const Func<IterationStatus(Value &)> &func) const noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::web::css::UrlValue)
  KRYS_NODISCARD static bool IsType(const krys::web::css::Value &value) noexcept
  {
    return value.Type() == krys::web::css::ValueType::Url;
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
