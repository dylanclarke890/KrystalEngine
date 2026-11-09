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

#include "Krystal.Booey/Graphics/Fonts/FontPalette.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-palette'> = normal | light | dark | <palette-identifier> | <palette-mix()>
  // FIXME: <palette-mix()> is not yet supported.
  // https://drafts.csswg.org/css-fonts/#propdef-font-palette
  struct FontPalette
  {
    FontPalette(css::keywords::Normal) noexcept
        : _platform {boo::FontPalette::Type::Normal, css::CSSOMStringAtom::Null()}
    {
    }

    FontPalette(css::keywords::Light) noexcept
        : _platform {boo::FontPalette::Type::Light, css::CSSOMStringAtom::Null()}
    {
    }

    FontPalette(css::keywords::Dark) noexcept
        : _platform {boo::FontPalette::Type::Dark, css::CSSOMStringAtom::Null()}
    {
    }

    FontPalette(css::CustomIdentifier &&identifier) noexcept
        : _platform {boo::FontPalette::Type::Custom, krys::move(identifier.Value)}
    {
    }

    FontPalette(boo::FontPalette &&platform) noexcept : _platform {krys::move(platform)}
    {
    }

    FontPalette(const boo::FontPalette &platform) noexcept : _platform {platform}
    {
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_platform.PaletteType)
      {
        case boo::FontPalette::Type::Normal: return visitor(css::keywords::Normal {});
        case boo::FontPalette::Type::Light:  return visitor(css::keywords::Light {});
        case boo::FontPalette::Type::Dark:   return visitor(css::keywords::Dark {});
        case boo::FontPalette::Type::Custom: return visitor(css::CustomIdentifier {_platform.Identifier});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD const boo::FontPalette &Platform() const noexcept
    {
      return _platform;
    }

    KRYS_NODISCARD bool operator==(const FontPalette &) const noexcept = default;

  private:
    boo::FontPalette _platform;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontPalette>
  {
    KRYS_NODISCARD FontPalette operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontPalette)
