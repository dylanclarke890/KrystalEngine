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

#include "Krystal.Booey/Style/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/PositionValue.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesEvaluation.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  namespace
  {
    // MARK: Core Keyword Resolution

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Top, const BuilderState &) noexcept
    {
      return 0_css_percentage;
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Top, const BuilderState &state,
                                                     const css::LengthPercentage<> &length) noexcept
    {
      return ConvertToStyle(length, state);
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Right, const BuilderState &) noexcept
    {
      return 100_css_percentage;
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Right, const BuilderState &state,
                                                     const css::LengthPercentage<> &length) noexcept
    {
      return Reflect(ConvertToStyle(length, state));
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Bottom, const BuilderState &) noexcept
    {
      return 100_css_percentage;
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Bottom, const BuilderState &state,
                                                     const css::LengthPercentage<> &length) noexcept
    {
      return Reflect(ConvertToStyle(length, state));
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Left, const BuilderState &) noexcept
    {
      return 0_css_percentage;
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Left, const BuilderState &state,
                                                     const css::LengthPercentage<> &length) noexcept
    {
      return ConvertToStyle(length, state);
    }

    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::Center, const BuilderState &) noexcept
    {
      return 50_css_percentage;
    }

    // MARK: Mapped value resolution

    template <typename... Args>
    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::XStart, const BuilderState &state,
                                                     Args &&...args) noexcept
    {
      return state.Style().GetWritingMode().IsAnyLeftToRight()
               ? ResolveKeyword(css::keywords::Left {}, state, std::forward<Args>(args)...)
               : ResolveKeyword(css::keywords::Right {}, state, std::forward<Args>(args)...);
    }

    template <typename... Args>
    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::XEnd, const BuilderState &state,
                                                     Args &&...args) noexcept
    {
      return state.Style().GetWritingMode().IsAnyLeftToRight()
               ? ResolveKeyword(css::keywords::Right {}, state, std::forward<Args>(args)...)
               : ResolveKeyword(css::keywords::Left {}, state, std::forward<Args>(args)...);
    }

    template <typename... Args>
    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::YStart, const BuilderState &state,
                                                     Args &&...args) noexcept
    {
      return state.Style().GetWritingMode().IsAnyTopToBottom()
               ? ResolveKeyword(css::keywords::Top {}, state, std::forward<Args>(args)...)
               : ResolveKeyword(css::keywords::Bottom {}, state, std::forward<Args>(args)...);
    }

    template <typename... Args>
    KRYS_NODISCARD LengthPercentage<> ResolveKeyword(css::keywords::YEnd, const BuilderState &state,
                                                     Args &&...args) noexcept
    {
      return state.Style().GetWritingMode().IsAnyTopToBottom()
               ? ResolveKeyword(css::keywords::Bottom {}, state, std::forward<Args>(args)...)
               : ResolveKeyword(css::keywords::Top {}, state, std::forward<Args>(args)...);
    }

    // MARK: Horizontal/Vertical

    KRYS_NODISCARD LengthPercentage<> Resolve(const css::TwoComponentPositionHorizontal &value,
                                              const BuilderState &state) noexcept
    {
      return krys::SwitchOn(
        value.Offset, [&](auto keyword) { return ResolveKeyword(keyword, state); },
        [&](const css::LengthPercentage<> &value) { return ConvertToStyle(value, state); });
    }

    KRYS_NODISCARD LengthPercentage<> Resolve(const css::TwoComponentPositionVertical &value,
                                              const BuilderState &state) noexcept
    {
      return krys::SwitchOn(
        value.Offset, [&](auto keyword) { return ResolveKeyword(keyword, state); },
        [&](const css::LengthPercentage<> &value) { return ConvertToStyle(value, state); });
    }

    KRYS_NODISCARD LengthPercentage<> Resolve(const css::ThreeComponentPositionHorizontal &value,
                                              const BuilderState &state) noexcept
    {
      return krys::SwitchOn(value.Offset, [&](auto keyword) { return ResolveKeyword(keyword, state); });
    }

    KRYS_NODISCARD LengthPercentage<> Resolve(const css::ThreeComponentPositionVertical &value,
                                              const BuilderState &state) noexcept
    {
      return krys::SwitchOn(value.Offset, [&](auto keyword) { return ResolveKeyword(keyword, state); });
    }

    KRYS_NODISCARD LengthPercentage<> Resolve(const css::FourComponentPositionHorizontal &value,
                                              const BuilderState &state) noexcept
    {
      return krys::SwitchOn(get<0>(value.Offset), [&](auto keyword)
                            { return ResolveKeyword(keyword, state, get<1>(value.Offset)); });
    }

    KRYS_NODISCARD LengthPercentage<> Resolve(const css::FourComponentPositionVertical &value,
                                              const BuilderState &state) noexcept
    {
      return krys::SwitchOn(get<0>(value.Offset), [&](auto keyword)
                            { return ResolveKeyword(keyword, state, get<1>(value.Offset)); });
    }
  }

  css::TwoComponentPositionHorizontal
    ToCSS<TwoComponentPositionHorizontal>::operator()(const TwoComponentPositionHorizontal &value,
                                                      const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(
      value.Offset, [&](const auto &value)
      { return css::TwoComponentPositionHorizontal {ConvertToCSS(LengthPercentage<> {value}, style)}; });
  }

  TwoComponentPositionHorizontal
    ToStyle<css::TwoComponentPositionHorizontal>::operator()(const css::TwoComponentPositionHorizontal &value,
                                                             const BuilderState &state) noexcept
  {
    return {PositionX {Resolve(value, state)}};
  }

  css::TwoComponentPositionVertical
    ToCSS<TwoComponentPositionVertical>::operator()(const TwoComponentPositionVertical &value,
                                                    const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(
      value.Offset, [&](const auto &value)
      { return css::TwoComponentPositionVertical {ConvertToCSS(LengthPercentage<> {value}, style)}; });
  }

  TwoComponentPositionVertical
    ToStyle<css::TwoComponentPositionVertical>::operator()(const css::TwoComponentPositionVertical &value,
                                                           const BuilderState &state) noexcept
  {
    return {PositionY {Resolve(value, state)}};
  }

  // MARK: <position> conversion

  css::Position ToCSS<Position>::operator()(const Position &value, const RenderStyle &style) noexcept
  {
    return css::TwoComponentPositionHorizontalVertical {
      krys::SwitchOn(
        value.X, [&](const auto &value)
        { return css::TwoComponentPositionHorizontal {ConvertToCSS(LengthPercentage<> {value}, style)}; }),
      krys::SwitchOn(
        value.Y, [&](const auto &value)
        { return css::TwoComponentPositionVertical {ConvertToCSS(LengthPercentage<> {value}, style)}; })};
  }

  Position ToStyle<css::Position>::operator()(const css::Position &position,
                                              const BuilderState &state) noexcept
  {
    return krys::SwitchOn(position.Value,
                          [&](const auto &components)
                          {
                            return Position {
                              PositionX {Resolve(get<0>(components), state)},
                              PositionY {Resolve(get<1>(components), state)},
                            };
                          });
  }

  // MARK: <position-x> conversion

  css::PositionX ToCSS<PositionX>::operator()(const PositionX &value, const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(
      value, [&](const auto &value) -> css::PositionX
      { return css::TwoComponentPositionHorizontal {ConvertToCSS(LengthPercentage<> {value}, style)}; });
  }

  PositionX ToStyle<css::PositionX>::operator()(const css::PositionX &positionX,
                                                const BuilderState &state) noexcept
  {
    return krys::SwitchOn(positionX.Value,
                          [&](const auto &value) { return PositionX {Resolve(value, state)}; });
  }

  // MARK: <position-y> conversion

  css::PositionY ToCSS<PositionY>::operator()(const PositionY &value, const RenderStyle &style) noexcept
  {
    return krys::SwitchOn(
      value, [&](const auto &value) -> css::PositionY
      { return css::TwoComponentPositionVertical {ConvertToCSS(LengthPercentage<> {value}, style)}; });
  }

  PositionY ToStyle<css::PositionY>::operator()(const css::PositionY &positionY,
                                                const BuilderState &state) noexcept
  {
    return krys::SwitchOn(positionY.Value,
                          [&](const auto &value) { return PositionY {Resolve(value, state)}; });
  }

  Position CSSValueConversion<Position>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *positionValue = RequiredDowncast<css::PositionValue>(state, value);
    if (!positionValue)
    {
      return {0_css_px, 0_css_px};
    }
    return ConvertToStyle(positionValue->GetPosition(), state);
  }

  PositionX CSSValueConversion<PositionX>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *positionXValue = RequiredDowncast<css::PositionXValue>(state, value);
    if (!positionXValue)
    {
      return 0_css_px;
    }
    return ConvertToStyle(positionXValue->GetPosition(), state);
  }

  PositionY CSSValueConversion<PositionY>::operator()(BuilderState &state, const css::Value &value) noexcept
  {
    auto *positionYValue = RequiredDowncast<css::PositionYValue>(state, value);
    if (!positionYValue)
    {
      return 0_css_px;
    }
    return ConvertToStyle(positionYValue->GetPosition(), state);
  }

  // MARK: - Evaluation

  FloatPoint Evaluation<Position, FloatPoint>::operator()(const Position &position, FloatSize referenceBox,
                                                          ZoomNeeded token) noexcept
  {
    return {Evaluate<float>(position.X, referenceBox.Width(), token),
            Evaluate<float>(position.Y, referenceBox.Height(), token)};
  }
}
