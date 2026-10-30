#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // FIXME: Make LengthWrapperBase support additional numeric types in addition to the <length-percentage> one
  // and then replace GridTrackBreadth with that.

  struct GridTrackBreadthLength
      : LengthWrapperBase<LengthPercentage<css::NonNegative>, css::keywords::MinContent,
                          css::keywords::MaxContent, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsMinContent() const noexcept
    {
      return HoldsAlternative<css::keywords::MinContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsMaxContent() const noexcept
    {
      return HoldsAlternative<css::keywords::MaxContent>();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }
  };

  // <track-breadth> = <length-percentage [0,inf]> | <flex [0,inf]> | min-content | max-content | auto
  // https://www.w3.org/TR/css-grid-1/#typedef-track-breadth
  class GridTrackBreadth
  {
  public:
    using Fixed = GridTrackBreadthLength::Fixed;
    using Percentage = GridTrackBreadthLength::Percentage;
    using Calc = GridTrackBreadthLength::Calc;
    using Flex = style::Flex<css::NonNegative>;

    GridTrackBreadth(Fixed fixed) noexcept
        : _length(fixed), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(Percentage percent) noexcept
        : _length(percent), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(GridTrackBreadthLength &&length) noexcept
        : _length(krys::move(length)), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(Flex flex) noexcept : _length(0_css_px), _flex(flex), _type(GridTrackBreadthType::Flex)
    {
    }

    GridTrackBreadth(css::keywords::MinContent keyword) noexcept
        : _length(keyword), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(css::keywords::MaxContent keyword) noexcept
        : _length(keyword), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(css::keywords::Auto keyword) noexcept
        : _length(keyword), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _length(literal), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(css::ValueLiteral<css::PercentageUnit::Percentage> literal) noexcept
        : _length(literal), _flex(0_css_fr), _type(GridTrackBreadthType::Length)
    {
    }

    GridTrackBreadth(css::ValueLiteral<css::FlexUnit::Fr> literal) noexcept
        : _length(0_css_px), _flex(literal), _type(GridTrackBreadthType::Flex)
    {
    }

    KRYS_NODISCARD bool IsLength() const noexcept
    {
      return _type == GridTrackBreadthType::Length;
    }
    KRYS_NODISCARD bool IsFlex() const noexcept
    {
      return _type == GridTrackBreadthType::Flex;
    }

    KRYS_NODISCARD const GridTrackBreadthLength &Length() const noexcept
    {
      krys_debug_assert(IsLength());
      return _length;
    }
    KRYS_NODISCARD Flex GetFlex() const noexcept
    {
      krys_debug_assert(IsFlex());
      return _flex;
    }

    KRYS_NODISCARD bool IsPercentOrCalculated() const noexcept
    {
      return _type == GridTrackBreadthType::Length && _length.IsPercentOrCalculated();
    }
    KRYS_NODISCARD bool IsContentSized() const noexcept
    {
      return _type == GridTrackBreadthType::Length
             && (_length.IsAuto() || _length.IsMinContent() || _length.IsMaxContent());
    }
    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return _type == GridTrackBreadthType::Length && _length.IsAuto();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsFlex())
      {
        return visitor(_flex);
      }
      return krys::SwitchOn(_length, [&](const auto &value) { return visitor(value); });
    }

    KRYS_NODISCARD bool operator==(const GridTrackBreadth &) const noexcept = default;

  private:
    GridTrackBreadthLength _length;
    Flex _flex;

    enum class GridTrackBreadthType : bool
    {
      Length,
      Flex
    };
    GridTrackBreadthType _type;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<GridTrackBreadth>
  {
    KRYS_NODISCARD GridTrackBreadth operator()(BuilderState &, const css::PrimitiveValue &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<GridTrackBreadth>
  {
    KRYS_NODISCARD GridTrackBreadth Blend(const GridTrackBreadth &, const GridTrackBreadth &,
                                          const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridTrackBreadthLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::GridTrackBreadth)
