#pragma once

#include "Krystal.Booey/Core/TextSpacing.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'text-spacing-trim'> = space-all | normal | space-first | trim-start | trim-both | trim-all | auto
  // FIXME: Only `space-all`, `trim-all` and `auto` are currently supported.
  // https://drafts.csswg.org/css-text-4/#propdef-text-spacing-trim
  struct TextSpacingTrim
  {
    constexpr TextSpacingTrim(css::keywords::SpaceAll) noexcept
        : Value {boo::TextSpacingTrim::TrimType::SpaceAll}
    {
    }
    constexpr TextSpacingTrim(css::keywords::TrimAll) noexcept
        : Value {boo::TextSpacingTrim::TrimType::TrimAll}
    {
    }
    constexpr TextSpacingTrim(css::keywords::Auto) noexcept : Value {boo::TextSpacingTrim::TrimType::Auto}
    {
    }

    constexpr TextSpacingTrim(boo::TextSpacingTrim platform) noexcept : Value {platform.Type()}
    {
    }
    constexpr TextSpacingTrim(boo::TextSpacingTrim::TrimType platformType) noexcept : Value {platformType}
    {
    }

    KRYS_NODISCARD constexpr boo::TextSpacingTrim::TrimType Platform() const noexcept
    {
      return Value;
    }

    KRYS_NODISCARD constexpr bool operator==(const TextSpacingTrim &) const noexcept = default;

    boo::TextSpacingTrim::TrimType Value;
  };
  DEFINE_TYPE_WRAPPER_GET(TextSpacingTrim, Value);
}

DEFINE_TUPLE_LIKE_CONFORMANCE_FOR_TYPE_WRAPPER(krys::boo::style::TextSpacingTrim);
