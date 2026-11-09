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

#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"

namespace krys::boo::css
{
  bool IsCenterPosition(const Position &position) noexcept
  {
    auto isCenter = [](const auto &component)
    {
      return krys::SwitchOn(
        component.Offset, [](auto) { return false; }, [](keywords::Center) { return true; },
        [](const LengthPercentage<> &value)
        {
          return krys::SwitchOn(
            value, [](const LengthPercentage<>::Raw &raw) { return raw == 50_css_percentage; },
            [](const LengthPercentage<>::Calc &) { return false; });
        });
    };

    return position.SwitchOn([&](const TwoComponentPositionHorizontalVertical &components)
                             { return isCenter(get<0>(components)) && isCenter(get<1>(components)); },
                             [&](const auto &) { return false; });
  }

  namespace
  {
    TwoComponentPositionHorizontal ToTwoComponent(const ThreeComponentPositionHorizontal &component) noexcept
    {
      return krys::SwitchOn(component.Offset,
                            [](auto keyword) -> TwoComponentPositionHorizontal { return {keyword}; });
    }

    TwoComponentPositionVertical ToTwoComponent(const ThreeComponentPositionVertical &component) noexcept
    {
      return krys::SwitchOn(component.Offset,
                            [](auto keyword) -> TwoComponentPositionVertical { return {keyword}; });
    }
  }

  Pair<PositionX, PositionY> Split(Position &&position) noexcept
  {
    // PositionX and PositionY don't utilize the three component variants, so the non-length containing item
    // must be converted to its two component variant.

    return krys::SwitchOn(
      krys::move(position.Value), [&](const auto &components) -> Pair<PositionX, PositionY>
      { return {PositionX(get<0>(components)), PositionY(get<1>(components))}; },
      [&](const ThreeComponentPositionHorizontalVerticalLengthFirst &components) -> Pair<PositionX, PositionY>
      { return {PositionX(get<0>(components)), PositionY(ToTwoComponent(get<1>(components)))}; },
      [&](
        const ThreeComponentPositionHorizontalVerticalLengthSecond &components) -> Pair<PositionX, PositionY>
      { return {PositionX(ToTwoComponent(get<0>(components))), PositionY(get<1>(components))}; });
  }
}
