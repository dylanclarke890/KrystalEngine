/*
 * Copyright (C) 2006-2017 Apple Inc. All rights reserved.
 * Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies)
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
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorSerialisation.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasGradient.hpp"
#include "Krystal.Booey/HTML/Canvas/CanvasPattern.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include "Krystal.Core/Visitor.hpp"

namespace krys::boo::html
{
  class CanvasBase;

  /// @brief What a 2D context fills or strokes with: a colour, a gradient or a pattern.
  class CanvasStyle
  {
  private:
    Variant<Color, Ref<CanvasGradient>, Ref<CanvasPattern>> _style;

  public:
    CanvasStyle(Color) noexcept;
    CanvasStyle(Ref<CanvasGradient> &&) noexcept;
    CanvasStyle(Ref<CanvasPattern> &&) noexcept;

    KRYS_NODISCARD Color GetColor() const noexcept;
    KRYS_NODISCARD dom::DOMString ColorString() const noexcept;
    KRYS_NODISCARD RefPtr<CanvasGradient> GetCanvasGradient() const noexcept;
    KRYS_NODISCARD RefPtr<CanvasPattern> GetCanvasPattern() const noexcept;

    /// @brief Calls the visitor with the colour serialised, or with the gradient or pattern.
    template <typename... F>
    KRYS_NODISCARD decltype(auto) Visit(F &&...f) const noexcept
    {
      auto visitor = CreateVisitor(std::forward<F>(f)...);
      return SwitchOn(
        _style, [&](const Color &color) { return visitor(SerialisationForHTML(color)); },
        [&](const Ref<CanvasGradient> &gradient) { return visitor(gradient); },
        [&](const Ref<CanvasPattern> &pattern) { return visitor(pattern); });
    }
  };

  /// @brief Parses @p colorString as a CSS colour for @p canvasBase; `currentcolor` is the canvas's `color`
  /// (from its inline style), or black.
  /// @return The invalid colour if it doesn't parse.
  KRYS_NODISCARD Color ParseColor(dom::DOMStringView colorString, CanvasBase &canvasBase) noexcept;

  /// @brief Parses @p colorString as a CSS colour in standards mode, with `currentcolor` as black.
  /// @return The invalid colour if it doesn't parse.
  KRYS_NODISCARD Color ParseColor(dom::DOMStringView colorString) noexcept;

  inline CanvasStyle::CanvasStyle(Color color) noexcept : _style(krys::move(color))
  {
  }

  inline CanvasStyle::CanvasStyle(Ref<CanvasGradient> &&gradient) noexcept : _style(krys::move(gradient))
  {
  }

  inline CanvasStyle::CanvasStyle(Ref<CanvasPattern> &&pattern) noexcept : _style(krys::move(pattern))
  {
  }

  inline RefPtr<CanvasGradient> CanvasStyle::GetCanvasGradient() const noexcept
  {
    if (!std::holds_alternative<Ref<CanvasGradient>>(_style))
    {
      return nullptr;
    }
    return std::get<Ref<CanvasGradient>>(_style);
  }

  inline RefPtr<CanvasPattern> CanvasStyle::GetCanvasPattern() const noexcept
  {
    if (!std::holds_alternative<Ref<CanvasPattern>>(_style))
    {
      return nullptr;
    }
    return std::get<Ref<CanvasPattern>>(_style);
  }

  inline dom::DOMString CanvasStyle::ColorString() const noexcept
  {
    if (!std::holds_alternative<Color>(_style))
    {
      return {};
    }
    return SerialisationForHTML(std::get<Color>(_style));
  }

  inline Color CanvasStyle::GetColor() const noexcept
  {
    if (const auto *color = std::get_if<Color>(&_style))
    {
      return *color;
    }
    return {};
  }
}
