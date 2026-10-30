#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'block-ellipse'> = none | auto | <string>
  // https://www.w3.org/TR/css-overflow-4/#propdef-block-ellipsis
  struct BlockEllipsis
  {
    BlockEllipsis(css::keywords::None) noexcept
    {
    }

    BlockEllipsis(css::keywords::Auto) noexcept : _type {Type::Auto}
    {
    }

    BlockEllipsis(css::CSSOMStringAtom &&string) noexcept : _type {Type::String}, _string {krys::move(string)}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _type == Type::None;
    }
    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return _type == Type::Auto;
    }
    KRYS_NODISCARD bool IsString() const noexcept
    {
      return _type == Type::String;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case Type::None:   return visitor(css::keywords::None {});
        case Type::Auto:   return visitor(css::keywords::Auto {});
        case Type::String: return visitor(_string);
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const BlockEllipsis &) const noexcept = default;

  private:
    enum class Type : uint8
    {
      None,
      Auto,
      String
    };

    Type _type {Type::None};
    css::CSSOMStringAtom _string {css::CSSOMStringAtom::Null()};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BlockEllipsis>
  {
    KRYS_NODISCARD BlockEllipsis operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BlockEllipsis)
