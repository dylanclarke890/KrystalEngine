#pragma once

#include "Krystal.Booey/Style/Values/Text/HyphenateLimitEdge.hpp"

namespace krys::boo::style
{
  // <'hyphenate-limit-chars'> = [ auto | <integer [0,∞]> ]{1,3}
  // https://drafts.csswg.org/css-text-4/#propdef-hyphenate-limit-chars
  struct HyphenateLimitChars
  {
    /// @brief The minimum length of a word that can be hyphenated.
    HyphenateLimitEdge Word {css::keywords::Auto {}};
    /// @brief The minimum number of characters before the hyphen.
    HyphenateLimitEdge Before {css::keywords::Auto {}};
    /// @brief The minimum number of characters after the hyphen.
    HyphenateLimitEdge After {css::keywords::Auto {}};

    constexpr HyphenateLimitChars(css::keywords::Auto) noexcept
    {
    }

    constexpr HyphenateLimitChars(HyphenateLimitEdge word, HyphenateLimitEdge before,
                                  HyphenateLimitEdge after) noexcept
        : Word {word}, Before {before}, After {after}
    {
    }

    /// @brief Calls the visitor with the shortest form: a missing third value is the same as the second, and
    /// a missing second value is `auto`.
    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (After == Before)
      {
        if (Before.IsAuto())
        {
          return visitor(Word);
        }
        return visitor(css::SpaceSeparatedTuple {Word, Before});
      }
      return visitor(css::SpaceSeparatedTuple {Word, Before, After});
    }

    KRYS_NODISCARD constexpr bool operator==(const HyphenateLimitChars &) const noexcept = default;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<HyphenateLimitChars>
  {
    KRYS_NODISCARD HyphenateLimitChars operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::HyphenateLimitChars)
