#pragma once

#include "Krystal.Booey/Style/Values/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"

namespace krys::boo::style
{
  enum class ForVisitedLink : bool;

  // <paint> = none | <color> | <url> [none | <color>]? | context-fill | context-stroke
  // NOTE: `context-fill` and `context-stroke` are not implemented.
  // https://svgwg.org/svg2-draft/painting.html#SpecifyingPaint
  struct SVGPaint
  {
    struct URLNone
    {
      URL Url;
      css::keywords::None None;

      KRYS_NODISCARD bool operator==(const URLNone &) const noexcept = default;
    };
    struct URLColor
    {
      URL Url;
      style::Color Color;

      KRYS_NODISCARD bool operator==(const URLColor &) const noexcept = default;
    };

    SVGPaint(css::keywords::None) noexcept : _type {Type::None}
    {
    }
    SVGPaint(Color &&value) noexcept : _type {Type::Color}, _color {krys::move(value)}
    {
    }
    SVGPaint(URL &&value) noexcept : _type {Type::URL}, _url {krys::move(value)}
    {
    }
    SVGPaint(URLNone &&value) noexcept : _type {Type::URLNone}, _url {krys::move(value.Url)}
    {
    }
    SVGPaint(URLColor &&value) noexcept
        : _type {Type::URLColor}, _url {krys::move(value.Url)}, _color {krys::move(value.Color)}
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
    KRYS_NODISCARD bool IsURL() const noexcept
    {
      return _type == Type::URL;
    }
    KRYS_NODISCARD bool IsURLNone() const noexcept
    {
      return _type == Type::URLNone;
    }
    KRYS_NODISCARD bool IsURLColor() const noexcept
    {
      return _type == Type::URLColor;
    }

    KRYS_NODISCARD bool HasColor() const noexcept
    {
      return IsColor() || IsURLColor();
    }
    KRYS_NODISCARD bool HasURL() const noexcept
    {
      return IsURL() || IsURLNone() || IsURLColor();
    }

    KRYS_NODISCARD Maybe<Color> TryColor() const noexcept
    {
      return IsColor() ? Maybe(_color) : std::nullopt;
    }
    KRYS_NODISCARD Maybe<URL> TryURL() const noexcept
    {
      return IsURL() ? Maybe(_url) : std::nullopt;
    }
    KRYS_NODISCARD Maybe<URLNone> TryURLNone() const noexcept
    {
      return IsURLNone() ? Maybe(URLNone {_url, css::keywords::None {}}) : std::nullopt;
    }
    KRYS_NODISCARD Maybe<URLColor> TryURLColor() const noexcept
    {
      return IsURLColor() ? Maybe(URLColor {_url, _color}) : std::nullopt;
    }

    KRYS_NODISCARD Maybe<Color> TryAnyColor() const noexcept
    {
      return HasColor() ? Maybe(_color) : std::nullopt;
    }
    KRYS_NODISCARD Maybe<URL> TryAnyURL() const noexcept
    {
      return HasURL() ? Maybe(_url) : std::nullopt;
    }

    KRYS_NODISCARD const Color &ColorDisregardingType() const noexcept
    {
      return _color;
    }
    KRYS_NODISCARD const URL &UrlDisregardingType() const noexcept
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
        case Type::URL:      return visitor(_url);
        case Type::URLNone:  return visitor(URLNone {_url, css::keywords::None {}});
        case Type::URLColor: return visitor(URLColor {_url, _color});
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
      URL,
      URLNone,
      URLColor
    };

    Type _type;
    URL _url {.Resolved = {}, .Modifiers = {}};
    Color _color {boo::Color {}};
  };

  template <size_t I>
  const auto &get(const SVGPaint::URLNone &value)
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
  const auto &get(const SVGPaint::URLColor &value)
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
    KRYS_NODISCARD SVGPaint operator()(BuilderState &, const css::Value &, ForVisitedLink) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<SVGPaint>
  {
    KRYS_NODISCARD bool Equals(const SVGPaint &, const SVGPaint &, const RenderStyle &,
                               const RenderStyle &) noexcept;
    KRYS_NODISCARD bool CanBlend(const SVGPaint &, const SVGPaint &) noexcept;
    KRYS_NODISCARD SVGPaint Blend(const SVGPaint &, const SVGPaint &, const RenderStyle &,
                                  const RenderStyle &, const BlendingContext &) noexcept;
  };
}

DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::SVGPaint::URLNone, 2)
DEFINE_SPACE_SEPARATED_TUPLE_LIKE_CONFORMANCE(krys::boo::style::SVGPaint::URLColor, 2)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::SVGPaint)
