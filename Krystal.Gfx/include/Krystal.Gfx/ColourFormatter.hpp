#pragma once

#include "Krystal.Gfx/Colour.hpp"
#include <format>

namespace std
{
  template <>
  struct formatter<Krys::Gfx::Colourf>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Krys::Gfx::Colourf &colour, FormatContext &ctx)
    {
      return format_to(ctx.out(), "Colour(r: {}, g: {}, b: {}, a: {})", colour.red, colour.green, colour.blue,
                       colour.alpha);
    }
  };

  template <>
  struct formatter<Krys::Gfx::Colourb>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Krys::Gfx::Colourb &colour, FormatContext &ctx)
    {
      return format_to(ctx.out(), "Colour(r: {}, g: {}, b: {}, a: {})", colour.red, colour.green, colour.blue,
                       colour.alpha);
    }
  };

  template <>
  struct formatter<Krys::Gfx::ColourbPremultiplied>
  {
    constexpr auto parse(format_parse_context &ctx)
    {
      return ctx.begin();
    }

    template <typename FormatContext>
    auto format(const Krys::Gfx::ColourbPremultiplied &colour, FormatContext &ctx)
    {
      return format_to(ctx.out(), "Colour(r: {}, g: {}, b: {}, a: {})", colour.red, colour.green, colour.blue,
                       colour.alpha);
    }
  };
}