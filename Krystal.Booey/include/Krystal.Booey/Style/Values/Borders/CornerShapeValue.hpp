#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo
{
  class css::Value;
  class RenderStyle;

  namespace style
  {
    // NOTE: the keyword value "infinity" is represented as the standard double value
    // `std::numeric_limits<double>::infinity()`.
    using SuperellipseFunction = css::FunctionNotation<css::ValueId::Superellipse, Number<css::NonNegative>>;

    // https://drafts.csswg.org/css-borders-4/#typedef-corner-shape-value
    struct CornerShapeValue
    {
      SuperellipseFunction Superellipse;

      constexpr CornerShapeValue(css::keywords::Round) noexcept : Superellipse {2.0}
      {
      }
      constexpr CornerShapeValue(css::keywords::Scoop) noexcept : Superellipse {0.5}
      {
      }
      constexpr CornerShapeValue(css::keywords::Bevel) noexcept : Superellipse {1.0}
      {
      }
      constexpr CornerShapeValue(css::keywords::Notch) noexcept : Superellipse {0.0}
      {
      }
      constexpr CornerShapeValue(css::keywords::Straight) noexcept
          : Superellipse {std::numeric_limits<double>::infinity()}
      {
      }
      constexpr CornerShapeValue(css::keywords::Squircle) noexcept : Superellipse {4.0}
      {
      }
      constexpr CornerShapeValue(SuperellipseFunction value) noexcept : Superellipse {value}
      {
      }

      template <typename F>
      decltype(auto) SwitchOn(F &&functor) const noexcept
      {
        if (*this == CornerShapeValue(css::keywords::Round {}))
        {
          return functor(css::keywords::Round {});
        }
        if (*this == CornerShapeValue(css::keywords::Scoop {}))
        {
          return functor(css::keywords::Scoop {});
        }
        if (*this == CornerShapeValue(css::keywords::Bevel {}))
        {
          return functor(css::keywords::Bevel {});
        }
        if (*this == CornerShapeValue(css::keywords::Notch {}))
        {
          return functor(css::keywords::Notch {});
        }
        if (*this == CornerShapeValue(css::keywords::Straight {}))
        {
          return functor(css::keywords::Straight {});
        }
        if (*this == CornerShapeValue(css::keywords::Squircle {}))
        {
          return functor(css::keywords::Squircle {});
        }
        return functor(Superellipse);
      }

      KRYS_NODISCARD bool operator==(const CornerShapeValue &) const noexcept = default;
    };

    // https://drafts.csswg.org/css-borders-4/#propdef-corner-shape
    using CornerShape = css::MinimallySerialisingSpaceSeparatedRectCorners<CornerShapeValue>;

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<CornerShapeValue>
    {
      KRYS_NODISCARD CornerShapeValue operator()(BuilderState &, const css::Value &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<CornerShapeValue>
    {
      KRYS_NODISCARD CornerShapeValue Blend(const CornerShapeValue &, const CornerShapeValue &,
                                            const BlendingContext &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::CornerShapeValue)
