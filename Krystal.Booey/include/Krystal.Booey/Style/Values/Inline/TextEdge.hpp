#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <text-edge> = [ text | cap | ex | ideographic | ideographic-ink ]
  //               [ text | alphabetic | ideographic | ideographic-ink ]?
  // https://drafts.csswg.org/css-inline-3/#typedef-text-edge

  struct TextEdgePair
  {
    TextEdgeOver Over;
    TextEdgeUnder Under;

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (Over)
      {
        case TextEdgeOver::Text:
        case TextEdgeOver::Cap:
        case TextEdgeOver::Ex:
          if (Under == TextEdgeUnder::Text)
          {
            return visitor(Over);
          }
          break;
        case TextEdgeOver::Ideographic:
          if (Under == TextEdgeUnder::Ideographic)
          {
            return visitor(Over);
          }
          break;
        case TextEdgeOver::IdeographicInk:
          if (Under == TextEdgeUnder::IdeographicInk)
          {
            return visitor(Over);
          }
          break;
      }

      return visitor(css::SpaceSeparatedTuple {Over, Under});
    }

    KRYS_NODISCARD constexpr bool operator==(const TextEdgePair &) const noexcept = default;
  };

  template <typename K>
  struct TextEdge
  {
    using Keyword = K;
    using Base = TextEdge<Keyword>;

    constexpr TextEdge(Keyword keyword) noexcept : _value {keyword}
    {
    }
    constexpr TextEdge(TextEdgeOver over, TextEdgeUnder under) noexcept : _value {TextEdgePair {over, under}}
    {
    }

    KRYS_NODISCARD constexpr bool IsKeyword() const noexcept
    {
      return HoldsAlternative<Keyword>();
    }
    KRYS_NODISCARD constexpr bool IsTextEdgePair() const noexcept
    {
      return HoldsAlternative<TextEdgePair>();
    }
    KRYS_NODISCARD constexpr Maybe<TextEdgePair> TryTextEdgePair() const noexcept
    {
      return IsTextEdgePair() ? Maybe(std::get<TextEdgePair>(_value)) : null;
    }

    template <typename U>
    KRYS_NODISCARD constexpr bool HoldsAlternative() const noexcept
    {
      return krys::HoldsAlternative<U>(_value);
    }

    template <typename... F>
    constexpr decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD constexpr bool operator==(const TextEdge<Keyword> &) const noexcept = default;

  protected:
    Variant<Keyword, TextEdgePair> _value;
  };

  // MARK: - Concepts

  template <typename T>
  concept TextEdgeDerived = krys::IsBaseOfTemplate<TextEdge, T> && css::VariantLike<T>;
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TextEdgePair)
