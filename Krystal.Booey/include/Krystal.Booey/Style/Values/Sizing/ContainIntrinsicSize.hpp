#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // <'contain-intrinsic-*'> = auto? [ none | <length [0,inf]> ]
  // https://drafts.csswg.org/css-sizing-4/#intrinsic-size-override
  struct ContainIntrinsicSize
  {
    using Length = style::Length<css::NonNegative, float>;

    ContainIntrinsicSize(css::keywords::None) noexcept
        : _type {ContainIntrinsicSizeType::None}, _length {0_css_px}
    {
    }

    ContainIntrinsicSize(Length length) noexcept : _type {ContainIntrinsicSizeType::Length}, _length {length}
    {
    }

    ContainIntrinsicSize(css::ValueLiteral<css::LengthUnit::Px> literal) noexcept
        : _type {ContainIntrinsicSizeType::Length}, _length {literal}
    {
    }

    ContainIntrinsicSize(css::keywords::Auto, Length length) noexcept
        : _type {ContainIntrinsicSizeType::AutoAndLength}, _length {length}
    {
    }

    ContainIntrinsicSize(css::keywords::Auto, css::keywords::None) noexcept
        : _type {ContainIntrinsicSizeType::AutoAndNone}, _length {0_css_px}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return _type == ContainIntrinsicSizeType::None;
    }

    KRYS_NODISCARD bool IsAutoAndNone() const noexcept
    {
      return _type == ContainIntrinsicSizeType::AutoAndNone;
    }

    KRYS_NODISCARD bool HasAuto() const noexcept
    {
      return _type == ContainIntrinsicSizeType::AutoAndLength
             || _type == ContainIntrinsicSizeType::AutoAndNone;
    }

    KRYS_NODISCARD bool HasLength() const noexcept
    {
      return _type == ContainIntrinsicSizeType::Length || _type == ContainIntrinsicSizeType::AutoAndLength;
    }

    KRYS_NODISCARD Maybe<Length> TryLength() const noexcept
    {
      switch (_type)
      {
        case ContainIntrinsicSizeType::None:
        case ContainIntrinsicSizeType::AutoAndNone:   return {};
        case ContainIntrinsicSizeType::Length:
        case ContainIntrinsicSizeType::AutoAndLength: return _length;
      }
      krys_unreachable();
    }

    KRYS_NODISCARD ContainIntrinsicSize AddingAuto() const noexcept
    {
      switch (_type)
      {
        case ContainIntrinsicSizeType::None:
          return ContainIntrinsicSize {css::keywords::Auto {}, css::keywords::None {}};
        case ContainIntrinsicSizeType::Length:        return ContainIntrinsicSize {css::keywords::Auto {}, _length};
        case ContainIntrinsicSizeType::AutoAndNone:
        case ContainIntrinsicSizeType::AutoAndLength: return *this;
      }
      krys_unreachable();
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case ContainIntrinsicSizeType::None:   return visitor(css::keywords::None {});
        case ContainIntrinsicSizeType::Length: return visitor(_length);
        case ContainIntrinsicSizeType::AutoAndLength:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Auto {}, _length});
        case ContainIntrinsicSizeType::AutoAndNone:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Auto {}, css::keywords::None {}});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const ContainIntrinsicSize &) const noexcept = default;

  private:
    friend struct Blending<ContainIntrinsicSize>;

    ContainIntrinsicSize(ContainIntrinsicSizeType type, Length length) noexcept
        : _type {type}, _length {length}
    {
    }

    ContainIntrinsicSizeType _type;
    Length _length;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ContainIntrinsicSize>
  {
    KRYS_NODISCARD ContainIntrinsicSize operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ContainIntrinsicSize>
  {
    KRYS_NODISCARD bool CanBlend(const ContainIntrinsicSize &, const ContainIntrinsicSize &) noexcept;
    KRYS_NODISCARD ContainIntrinsicSize Blend(const ContainIntrinsicSize &, const ContainIntrinsicSize &,
                                              const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ContainIntrinsicSize)
