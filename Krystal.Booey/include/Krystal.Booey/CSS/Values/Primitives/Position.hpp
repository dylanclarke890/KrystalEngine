/*
 * Copyright (C) 2024-2025 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Values/Aggregates.hpp"
#include "Krystal.Booey/CSS/Values/Concepts.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Core/Concepts.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/TypeTraits.hpp"

namespace krys::boo::css
{
  // MARK: Two Component Types

  struct TwoComponentPositionHorizontal
  {
    Variant<keywords::Left, keywords::Right, keywords::Center, keywords::XStart, keywords::XEnd,
            LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionHorizontal, Offset)

  struct TwoComponentPositionVertical
  {
    Variant<keywords::Top, keywords::Bottom, keywords::Center, keywords::YStart, keywords::YEnd,
            LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const TwoComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(TwoComponentPositionVertical, Offset)

  // MARK: Three Component Types

  struct ThreeComponentPositionHorizontal
  {
    Variant<keywords::Left, keywords::Right, keywords::Center, keywords::XStart, keywords::XEnd> Offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionHorizontal, Offset)

  struct ThreeComponentPositionVertical
  {
    Variant<keywords::Top, keywords::Bottom, keywords::Center, keywords::YStart, keywords::YEnd> Offset;

    KRYS_NODISCARD bool operator==(const ThreeComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(ThreeComponentPositionVertical, Offset)

  // MARK: Four Component Types

  struct FourComponentPositionHorizontal
  {
    SpaceSeparatedTuple<Variant<keywords::Left, keywords::Right, keywords::XStart, keywords::XEnd>,
                        LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionHorizontal &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionHorizontal, Offset)

  struct FourComponentPositionVertical
  {
    SpaceSeparatedTuple<Variant<keywords::Top, keywords::Bottom, keywords::YStart, keywords::YEnd>,
                        LengthPercentage<>>
      Offset;

    KRYS_NODISCARD bool operator==(const FourComponentPositionVertical &) const noexcept = default;
  };
  DEFINE_TYPE_WRAPPER_GET(FourComponentPositionVertical, Offset)

  using TwoComponentPositionHorizontalVertical =
    SpaceSeparatedTuple<TwoComponentPositionHorizontal, TwoComponentPositionVertical>;

  using ThreeComponentPositionHorizontalVerticalLengthFirst =
    SpaceSeparatedTuple<FourComponentPositionHorizontal, ThreeComponentPositionVertical>;
  using ThreeComponentPositionHorizontalVerticalLengthSecond =
    SpaceSeparatedTuple<ThreeComponentPositionHorizontal, FourComponentPositionVertical>;

  using FourComponentPositionHorizontalVertical =
    SpaceSeparatedTuple<FourComponentPositionHorizontal, FourComponentPositionVertical>;

  /// @brief A `<position>`, e.g. `left 10px top`, in the two-, three- or four-component form it was written
  /// in.
  struct Position
  {
    using Kind =
      Variant<TwoComponentPositionHorizontalVertical, ThreeComponentPositionHorizontalVerticalLengthFirst,
              ThreeComponentPositionHorizontalVerticalLengthSecond, FourComponentPositionHorizontalVertical>;

    template <typename T>
    requires(!SameType<remove_cvref_t<T>, Position> && Constructible<Kind, T>)
    Position(T &&value) noexcept : Value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const Position &) const noexcept = default;

    Kind Value;
  };
  DEFINE_TYPE_WRAPPER_GET(Position, Value)

  /// @brief The horizontal half of a `<position>` (e.g. `background-position-x`).
  struct PositionX
  {
    using Kind = Variant<TwoComponentPositionHorizontal, FourComponentPositionHorizontal>;

    template <typename T>
    requires(!SameType<remove_cvref_t<T>, PositionX> && Constructible<Kind, T>)
    PositionX(T &&value) noexcept : Value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PositionX &) const noexcept = default;

    Kind Value;
  };
  DEFINE_TYPE_WRAPPER_GET(PositionX, Value)

  /// @brief The vertical half of a `<position>` (e.g. `background-position-y`).
  struct PositionY
  {
    using Kind = Variant<TwoComponentPositionVertical, FourComponentPositionVertical>;

    template <typename T>
    requires(!SameType<remove_cvref_t<T>, PositionY> && Constructible<Kind, T>)
    PositionY(T &&value) noexcept : Value {std::forward<T>(value)}
    {
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(Value, std::forward<F>(f)...);
    }

    KRYS_NODISCARD bool operator==(const PositionY &) const noexcept = default;

    Kind Value;
  };
  DEFINE_TYPE_WRAPPER_GET(PositionY, Value)

  /// @brief Whether `position` is `center center` (or `50% 50%`) in its two-component form.
  KRYS_NODISCARD bool IsCenterPosition(const Position &position) noexcept;

  /// @brief Splits a position into its horizontal and vertical halves.
  KRYS_NODISCARD Pair<PositionX, PositionY> Split(Position &&position) noexcept;
}

DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TwoComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::TwoComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ThreeComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::ThreeComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FourComponentPositionHorizontal, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::FourComponentPositionVertical, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::Position, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PositionX, 1)
DEFINE_TUPLE_LIKE_CONFORMANCE(krys::boo::css::PositionY, 1)
