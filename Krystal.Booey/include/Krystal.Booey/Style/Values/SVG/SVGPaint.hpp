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

#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"

namespace krys::boo::style
{
  // <paint> = none | <color> | <url> [none | <color>]? | context-fill | context-stroke
  // NOTE: `context-fill` and `context-stroke` are not implemented.
  // https://svgwg.org/svg2-draft/painting.html#SpecifyingPaint
  struct SVGPaint
  {
    struct UrlNone
    {
      style::Url Url;
      css::keywords::None None;

      KRYS_NODISCARD bool operator==(const UrlNone &) const noexcept = default;
    };

    struct UrlColor
    {
      style::Url Url;
      style::Color Color;

      KRYS_NODISCARD bool operator==(const UrlColor &) const noexcept = default;
    };

    SVGPaint(css::keywords::None) noexcept : _type {Type::None}
    {
    }

    SVGPaint(Color &&value) noexcept : _type {Type::Color}, _color {krys::move(value)}
    {
    }

    SVGPaint(Url &&value) noexcept : _type {Type::Url}, _url {krys::move(value)}
    {
    }

    SVGPaint(UrlNone &&value) noexcept : _type {Type::UrlNone}, _url {krys::move(value.Url)}
    {
    }

    SVGPaint(UrlColor &&value) noexcept
        : _type {Type::UrlColor}, _url {krys::move(value.Url)}, _color {krys::move(value.Color)}
    {
    }

    // Convenience constructors for common named colors supported by Color.
    SVGPaint(css::keywords::Currentcolor keyword) noexcept : SVGPaint {Color {keyword}}
    {
    }

    SVGPaint(css::keywords::Transparent keyword) noexcept : SVGPaint {Color {keyword}}
    {
    }

    SVGPaint(css::keywords::Black keyword) noexcept : SVGPaint {Color {keyword}}
    {
    }

    SVGPaint(css::keywords::White keyword) noexcept : SVGPaint {Color {keyword}}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _type == Type::None;
    }

    KRYS_NODISCARD bool IsColor() const noexcept
    {
      return _type == Type::Color;
    }

    KRYS_NODISCARD bool IsUrl() const noexcept
    {
      return _type == Type::Url;
    }

    KRYS_NODISCARD bool IsUrlNone() const noexcept
    {
      return _type == Type::UrlNone;
    }

    KRYS_NODISCARD bool IsUrlColor() const noexcept
    {
      return _type == Type::UrlColor;
    }

    KRYS_NODISCARD bool HasColor() const noexcept
    {
      return IsColor() || IsUrlColor();
    }

    KRYS_NODISCARD bool HasUrl() const noexcept
    {
      return IsUrl() || IsUrlNone() || IsUrlColor();
    }

    KRYS_NODISCARD Maybe<Color> TryColor() const noexcept
    {
      return IsColor() ? Maybe(_color) : std::nullopt;
    }

    KRYS_NODISCARD Maybe<Url> TryUrl() const noexcept
    {
      return IsUrl() ? Maybe(_url) : std::nullopt;
    }

    KRYS_NODISCARD Maybe<UrlNone> TryUrlNone() const noexcept
    {
      return IsUrlNone() ? Maybe(UrlNone {_url, css::keywords::None {}}) : std::nullopt;
    }

    KRYS_NODISCARD Maybe<UrlColor> TryUrlColor() const noexcept
    {
      return IsUrlColor() ? Maybe(UrlColor {_url, _color}) : std::nullopt;
    }

    KRYS_NODISCARD Maybe<Color> TryAnyColor() const noexcept
    {
      return HasColor() ? Maybe(_color) : std::nullopt;
    }

    KRYS_NODISCARD Maybe<Url> TryAnyUrl() const noexcept
    {
      return HasUrl() ? Maybe(_url) : std::nullopt;
    }

    KRYS_NODISCARD const Color &ColorDisregardingType() const noexcept
    {
      return _color;
    }

    KRYS_NODISCARD const Url &UrlDisregardingType() const noexcept
    {
      return _url;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case Type::None:     return visitor(css::keywords::None {});
        case Type::Color:    return visitor(_color);
        case Type::Url:      return visitor(_url);
        case Type::UrlNone:  return visitor(UrlNone {_url, css::keywords::None {}});
        case Type::UrlColor: return visitor(UrlColor {_url, _color});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const SVGPaint &) const noexcept = default;

    KRYS_NODISCARD bool HasSameType(const SVGPaint &other) const noexcept
    {
      return _type == other._type;
    }

  private:
    enum class Type : uint8
    {
      None,
      Color,
      Url,
      UrlNone,
      UrlColor,
    };

    Type _type;
    Url _url {.Resolved = {}, .Modifiers = {}};
    Color _color {boo::Color {}};
  };

  template <size_t I>
  const auto &get(const SVGPaint::UrlNone &value)
  {
    if constexpr (!I)
    {
      return value.Url;
    }
    else if constexpr (I == 1)
    {
      return value.None;
    }
  }

  template <size_t I>
  const auto &get(const SVGPaint::UrlColor &value)
  {
    if constexpr (!I)
    {
      return value.Url;
    }
    else if constexpr (I == 1)
    {
      return value.Color;
    }
  }

  KRYS_NODISCARD bool ContainsCurrentColor(const style::SVGPaint &) noexcept;

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<SVGPaint>
  {
    KRYS_NODISCARD SVGPaint operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<SVGPaint>
  {
    KRYS_NODISCARD static bool Equals(const SVGPaint &, const SVGPaint &, const RenderStyle &,
                                      const RenderStyle &) noexcept;

    KRYS_NODISCARD static bool CanBlend(const SVGPaint &, const SVGPaint &) noexcept;

    KRYS_NODISCARD static SVGPaint Blend(const SVGPaint &, const SVGPaint &, const RenderStyle &,
                                         const RenderStyle &, const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::SVGPaint::UrlNone, 2)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::SVGPaint::UrlColor, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGPaint)
