#pragma once

#include "Krystal.Booey/Core/TextFlags.hpp"
#include "Krystal.Booey/Graphics/FontSelectionAlgorithm.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'font-style'> = normal | italic | oblique <angle [-90deg,90deg]>?
  // https://drafts.csswg.org/css-fonts-4/#propdef-font-style
  struct FontStyle
  {
    using Angle = style::Angle<css::Range {-90, 90}>;

    FontStyle(css::keywords::Normal) noexcept
        : _platformSlope {std::nullopt}, _platformAxis {FontStyleAxis::slnt}
    {
    }
    FontStyle(css::keywords::Italic) noexcept
        : _platformSlope {ItalicValue()}, _platformAxis {FontStyleAxis::ital}
    {
    }
    FontStyle(css::keywords::Oblique) noexcept
        : _platformSlope {ItalicValue()}, _platformAxis {FontStyleAxis::slnt}
    {
    }
    FontStyle(Angle angle) noexcept
        : _platformSlope {FontSelectionValue::ClampFloat(static_cast<float>(angle.Value))},
          _platformAxis {FontStyleAxis::slnt}
    {
    }

    FontStyle(Maybe<FontSelectionValue> slope, FontStyleAxis axis) noexcept
        : _platformSlope {slope}, _platformAxis {axis}
    {
    }

    KRYS_NODISCARD bool IsNormal() const noexcept
    {
      return !_platformSlope && _platformAxis == FontStyleAxis::slnt;
    }
    KRYS_NODISCARD bool IsItalic() const noexcept
    {
      return _platformSlope == ItalicValue() && _platformAxis == FontStyleAxis::ital;
    }
    KRYS_NODISCARD bool IsOblique() const noexcept
    {
      return _platformSlope.has_value() && _platformAxis == FontStyleAxis::slnt;
    }

    KRYS_NODISCARD Maybe<Angle> GetAngle() const noexcept
    {
      return _platformSlope ? Maybe<Angle> {Angle {static_cast<float>(*_platformSlope)}} : std::nullopt;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (!_platformSlope || !*_platformSlope)
      {
        return visitor(css::keywords::Normal {});
      }
      if (*_platformSlope == ItalicValue())
      {
        if (_platformAxis == FontStyleAxis::ital)
        {
          return visitor(css::keywords::Italic {});
        }
        return visitor(css::keywords::Oblique {});
      }
      return visitor(
        css::SpaceSeparatedTuple {css::keywords::Oblique {}, Angle {static_cast<float>(*_platformSlope)}});
    }

    KRYS_NODISCARD Maybe<FontSelectionValue> PlatformSlope() const noexcept
    {
      return _platformSlope;
    }
    KRYS_NODISCARD FontStyleAxis PlatformAxis() const noexcept
    {
      return _platformAxis;
    }

    // NOTE: This is not if the value would compute to the keyword `italic`, but rather a more general whether
    // the slope is large enough to be considered "italic" (see `boo::ItalicThreshold()`).
    KRYS_NODISCARD bool IsConsideredItalic() const noexcept
    {
      return boo::IsItalic(_platformSlope);
    }

    KRYS_NODISCARD bool operator==(const FontStyle &) const noexcept = default;

  private:
    Maybe<FontSelectionValue> _platformSlope;
    FontStyleAxis _platformAxis {FontStyleAxis::slnt};
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<FontStyle>
  {
    KRYS_NODISCARD FontStyle operator()(BuilderState &, const css::Value &) noexcept;
  };
  // `FontStyle` is special-cased to return a `css::FontStyleWithAngleValue`.
  template <>
  struct CSSValueCreation<FontStyle>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const FontStyle &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<FontStyle>
  {
    KRYS_NODISCARD bool CanBlend(const FontStyle &, const FontStyle &) noexcept;
    KRYS_NODISCARD FontStyle Blend(const FontStyle &, const FontStyle &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::FontStyle)
