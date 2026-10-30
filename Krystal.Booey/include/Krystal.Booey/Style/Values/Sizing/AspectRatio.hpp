#pragma once

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Ratio.hpp"

namespace krys::boo::style
{
  using namespace css::Literals;

  // <'aspect-ratio'> = auto || <ratio>
  // https://drafts.csswg.org/css-sizing-4/#propdef-aspect-ratio
  struct AspectRatio
  {
    AspectRatio(css::keywords::Auto) noexcept
        : _type {AspectRatioType::Auto}, _ratio {0_css_number, 0_css_number}
    {
    }

    AspectRatio(css::keywords::Auto, style::Ratio ratio) noexcept
        : _type {AspectRatioType::AutoAndRatio}, _ratio {ratio}
    {
    }

    AspectRatio(style::Ratio ratio) noexcept
        : _type {(!ratio.Numerator.Value || !ratio.Denominator.Value) ? AspectRatioType::AutoZero
                                                                      : AspectRatioType::Ratio},
          _ratio {ratio}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return _type == AspectRatioType::Auto;
    }
    KRYS_NODISCARD bool IsAutoAndRatio() const noexcept
    {
      return _type == AspectRatioType::AutoAndRatio;
    }
    KRYS_NODISCARD bool IsAutoZero() const noexcept
    {
      return _type == AspectRatioType::AutoZero;
    }
    KRYS_NODISCARD bool IsRatio() const noexcept
    {
      return _type == AspectRatioType::Ratio;
    }

    KRYS_NODISCARD bool HasRatio() const noexcept
    {
      return IsRatio() || IsAutoAndRatio();
    }

    KRYS_NODISCARD Maybe<style::Ratio> TryRatio() const noexcept
    {
      if (IsAuto())
      {
        return {};
      }
      return _ratio;
    }

    KRYS_NODISCARD Number<css::NonNegative> Width() const noexcept
    {
      return _ratio.Numerator;
    }
    KRYS_NODISCARD Number<css::NonNegative> Height() const noexcept
    {
      return _ratio.Denominator;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_type)
      {
        case AspectRatioType::Auto:     return visitor(css::keywords::Auto {});
        case AspectRatioType::AutoZero:
        case AspectRatioType::Ratio:    return visitor(_ratio);
        case AspectRatioType::AutoAndRatio:
          return visitor(css::SpaceSeparatedTuple {css::keywords::Auto {}, _ratio});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const AspectRatio &) const noexcept = default;

  private:
    friend struct Blending<AspectRatio>;

    AspectRatio(AspectRatioType type, style::Ratio ratio) noexcept : _type {type}, _ratio {ratio}
    {
    }

    AspectRatioType _type;
    style::Ratio _ratio;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<AspectRatio>
  {
    KRYS_NODISCARD AspectRatio operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<AspectRatio>
  {
    KRYS_NODISCARD bool CanBlend(const AspectRatio &, const AspectRatio &) noexcept;
    AspectRatio Blend(const AspectRatio &, const AspectRatio &, const RenderStyle &, const RenderStyle &,
                      const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::AspectRatio);
