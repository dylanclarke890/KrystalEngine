#pragma once

#include "Krystal.HTML/CSS/Values/Enums/CSSValueId.hpp"
#include "Krystal.Lib/Core/Concepts.hpp"
#include "Krystal.Lib/Core/Visitor.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include <ranges>
#include <tuple>

namespace Krys::HTML
{
  /// @brief Concept for use in generic contexts to filter on Constant keyword CSS types.
  template <typename Keyword>
  concept PrimitiveKeyword = SameType<Keyword, ValueKeywordConstant<Keyword::value>>;

  /// @brief Concept for use in generic contexts to filter on keywords that are valid for the provided
  /// `Keywords` list.
  template <typename Keyword, typename KeywordsList>
  concept ValidKeywordForList = PrimitiveKeyword<Keyword> && KeywordsList::isValidKeyword(Keyword());

  template <PrimitiveKeyword... Ks>
  struct PrimitiveKeywordList
  {
    constexpr static auto Count = sizeof...(Ks);
    constexpr static auto Identifiers = Array {Ks::Value...};
    constexpr static auto Tuple = std::tuple {Ks {}...};

    KRYS_NODISCARD consteval static bool IsValidKeyword(PrimitiveKeyword auto keyword) noexcept
    {
      return std::ranges::find(Identifiers, keyword.Value) != Identifiers.end();
    }

    KRYS_NODISCARD consteval static size_t OffsetForKeyword(PrimitiveKeyword auto keyword) noexcept
    {
      return std::distance(Identifiers.begin(), std::ranges::find(Identifiers, keyword.Value));
    }

    template <typename F>
    KRYS_NODISCARD constexpr static decltype(auto) VisitKeywordAtOffset(size_t offset, F &&f) noexcept
    {
      return VisitTupleElementAtIndex(std::forward<F>(f), offset, Tuple);
    }
  };

  // Specialization for an empty keyword list.
  template <>
  struct PrimitiveKeywordList<>
  {
    constexpr static auto Count = 0uz;

    KRYS_NODISCARD consteval static bool IsValidKeyword(PrimitiveKeyword auto) noexcept
    {
      return false;
    }

    KRYS_NODISCARD consteval static size_t OffsetForKeyword(PrimitiveKeyword auto) noexcept
    {
      return 0uz;
    }
  };
}