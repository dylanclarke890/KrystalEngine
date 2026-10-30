#pragma once

#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'scrollbar-gutter'> = auto | stable && both-edges?
  // https://www.w3.org/TR/css-overflow-3/#propdef-scrollbar-gutter
  struct ScrollbarGutter
  {
    constexpr ScrollbarGutter(css::keywords::Auto) noexcept : _type {Type::Auto}
    {
    }

    constexpr ScrollbarGutter(css::keywords::Stable) noexcept : _type {Type::Stable}
    {
    }

    constexpr ScrollbarGutter(css::keywords::Stable, css::keywords::BothEdges) noexcept
        : _type {Type::StableBothEdges}
    {
    }

    KRYS_NODISCARD constexpr bool IsAuto() const noexcept
    {
      return _type == Type::Auto;
    }
    KRYS_NODISCARD constexpr bool IsStable() const noexcept
    {
      return _type == Type::Stable;
    }
    KRYS_NODISCARD constexpr bool IsStableBothEdges() const noexcept
    {
      return _type == Type::StableBothEdges;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case Type::Auto:   return visitor(css::keywords::Auto {});
        case Type::Stable: return visitor(css::keywords::Stable {});
        case Type::StableBothEdges:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Stable {}, css::keywords::BothEdges {}});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD constexpr bool operator==(const ScrollbarGutter &) const noexcept = default;

  private:
    enum Type : uint8
    {
      Auto,
      Stable,
      StableBothEdges
    };
    Type _type;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ScrollbarGutter>
  {
    KRYS_NODISCARD ScrollbarGutter operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ScrollbarGutter)
