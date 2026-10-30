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
