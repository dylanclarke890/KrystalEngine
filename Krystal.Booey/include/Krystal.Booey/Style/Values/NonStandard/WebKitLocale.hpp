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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'-webkit-locale'> = auto | <string>
  // NOTE: There is no standard associated with this property.
  struct WebkitLocale
  {
    WebkitLocale(css::keywords::Auto) noexcept : _platform {css::CSSOMStringAtom::Null()}
    {
    }
    WebkitLocale(const css::CSSOMStringAtom &value) noexcept : _platform {value}
    {
    }
    WebkitLocale(css::CSSOMStringAtom &&value) noexcept : _platform {krys::move(value)}
    {
    }

    KRYS_NODISCARD const css::CSSOMStringAtom &GetPlatform() const noexcept KRYS_LIFETIME_BOUND
    {
      return _platform;
    }
    KRYS_NODISCARD css::CSSOMStringAtom TakePlatform() noexcept
    {
      return krys::move(_platform);
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return _platform == css::CSSOMStringAtom::Null();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsAuto())
      {
        return visitor(css::keywords::Auto {});
      }

      // FIXME: It seems wrong that we extract/serialize the value as a <custom-ident>, given it is parsed as
      // a <string>, but this maintains existing behavior. See https://bugs.webkit.org/show_bug.cgi?id=302724.
      return visitor(css::CustomIdentifier {_platform});
    }

    KRYS_NODISCARD bool operator==(const WebkitLocale &) const noexcept = default;

  private:
    css::CSSOMStringAtom _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<WebkitLocale>
  {
    KRYS_NODISCARD WebkitLocale operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<WebkitLocale>
  {
    const css::CSSOMStringAtom &operator()(const WebkitLocale &value KRYS_LIFETIME_BOUND) noexcept
    {
      return value.GetPlatform();
    }
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WebkitLocale)
