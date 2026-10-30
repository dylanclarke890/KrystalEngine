#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  // <'offset-rotate'> = [ auto | reverse ] || <angle>
  // https://drafts.fxtf.org/motion/#propdef-offset-rotate
  struct OffsetRotate
  {
    using Angle = style::Angle<css::All, float>;

    constexpr OffsetRotate(css::keywords::Auto keyword) noexcept : _angle {0}, _autoKeyword {keyword}
    {
    }
    constexpr OffsetRotate(Maybe<css::keywords::Auto> autoKeyword, Angle angle) noexcept
        : _angle(angle), _autoKeyword(autoKeyword)
    {
    }

    KRYS_NODISCARD bool HasAuto() const noexcept
    {
      return _autoKeyword.has_value();
    }
    KRYS_NODISCARD Maybe<css::keywords::Auto> AutoKeyword() const noexcept
    {
      return _autoKeyword;
    }

    KRYS_NODISCARD Angle GetAngle() const noexcept
    {
      return _angle;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (_autoKeyword)
      {
        return visitor(css::SpaceSeparatedTuple {*_autoKeyword, _angle});
      }
      return visitor(_angle);
    }

    KRYS_NODISCARD constexpr bool operator==(const OffsetRotate &) const noexcept = default;

  private:
    Angle _angle;
    Maybe<css::keywords::Auto> _autoKeyword;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<OffsetRotate>
  {
    KRYS_NODISCARD OffsetRotate operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<OffsetRotate>
  {
    KRYS_NODISCARD bool CanBlend(const OffsetRotate &, const OffsetRotate &) noexcept;
    KRYS_NODISCARD OffsetRotate Blend(const OffsetRotate &, const OffsetRotate &,
                                      const BlendingContext &) noexcept;
  };

  // MARK: - Evaluation
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::OffsetRotate)
