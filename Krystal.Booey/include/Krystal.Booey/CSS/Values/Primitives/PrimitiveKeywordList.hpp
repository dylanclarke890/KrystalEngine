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

#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <ranges>
#include <tuple>

namespace krys::boo::css
{
  /// @brief Concept for use in generic contexts to filter on Constant keyword CSS types.
  template <typename Keyword>
  concept PrimitiveKeyword = SameType<Keyword, KeywordValueConstant<Keyword::Value>>;

  /// @brief Concept for use in generic contexts to filter on keywords that are valid for the provided
  /// `Keywords` list.
  template <typename Keyword, typename KeywordsList>
  concept ValidKeywordForList = PrimitiveKeyword<Keyword> && KeywordsList::IsValidKeyword(Keyword());

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