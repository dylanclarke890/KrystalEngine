#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  struct BackgroundSizeLength : LengthWrapperBase<LengthPercentage<css::NonNegative>, css::keywords::Auto>
  {
    using Base::Base;

    KRYS_ALWAYS_INLINE KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return HoldsAlternative<css::keywords::Auto>();
    }

    KRYS_NODISCARD bool IsKnownZeroOrAuto() const noexcept
    {
      return SwitchOn([](const Base::Fixed &fixed) { return fixed.IsZero(); },
                      [](const Base::Percentage &percentage) { return percentage.IsZero(); },
                      [](const Base::Calc &) { return false; },
                      [](const css::keywords::Auto &) { return true; });
    }
  };

  // <bg-size> = [ <length-percentage [0,∞]> | auto ]{1,2}@(default=auto) | cover | contain
  // https://www.w3.org/TR/css-backgrounds-3/#typedef-bg-size
  struct BackgroundSize
  {
    using LengthSize = css::SpaceSeparatedSize<BackgroundSizeLength>;

    BackgroundSize(css::keywords::Auto keyword) noexcept : _value {LengthSize {keyword, keyword}}
    {
    }

    BackgroundSize(BackgroundSizeLength &&value) noexcept
        : _value {LengthSize {value, css::keywords::Auto {}}}
    {
    }

    BackgroundSize(LengthSize &&value) noexcept : _value {krys::move(value)}
    {
    }

    BackgroundSize(css::keywords::Cover keyword) noexcept : _value {keyword}
    {
    }

    BackgroundSize(css::keywords::Contain contain) noexcept : _value {contain}
    {
    }

    KRYS_NODISCARD bool IsCover() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Cover>(_value);
    }
    KRYS_NODISCARD bool IsContain() const noexcept
    {
      return krys::HoldsAlternative<css::keywords::Contain>(_value);
    }
    KRYS_NODISCARD bool IsLengthSize() const noexcept
    {
      return krys::HoldsAlternative<LengthSize>(_value);
    }

    KRYS_NODISCARD Maybe<LengthSize> TryLengthSize() const noexcept
    {
      if (auto *lengthSize = std::get_if<LengthSize>(&_value))
      {
        return Maybe(*lengthSize);
      }
      return {};
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      return krys::SwitchOn(_value, std::forward<F>(f)...);
    }

    // FIXME: This name is confusing, given it can't really guarantee empty.
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      if (auto *lengthSize = std::get_if<LengthSize>(&_value))
      {
        return lengthSize->Width().IsKnownZeroOrAuto() || lengthSize->Height().IsKnownZeroOrAuto();
      }
      return false;
    }

    KRYS_NODISCARD bool SpecifiedHeight() const noexcept
    {
      if (auto *lengthSize = std::get_if<LengthSize>(&_value))
      {
        return !lengthSize->Height().IsAuto();
      }
      return false;
    }

    KRYS_NODISCARD bool SpecifiedWidth() const noexcept
    {
      if (auto *lengthSize = std::get_if<LengthSize>(&_value))
      {
        return !lengthSize->Width().IsAuto();
      }
      return false;
    }

    KRYS_NODISCARD bool operator==(const BackgroundSize &) const noexcept = default;

    KRYS_NODISCARD bool HasSameType(const BackgroundSize &other) const noexcept
    {
      return _value.index() == other._value.index();
    }

  private:
    Variant<LengthSize, css::keywords::Cover, css::keywords::Contain> _value;
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<BackgroundSize>
  {
    KRYS_NODISCARD BackgroundSize operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<BackgroundSize>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const BackgroundSize &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<BackgroundSize>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const BackgroundSize &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<BackgroundSize>
  {
    KRYS_NODISCARD bool CanBlend(const BackgroundSize &, const BackgroundSize &) noexcept;
    KRYS_NODISCARD BackgroundSize Blend(const BackgroundSize &, const BackgroundSize &,
                                        const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BackgroundSizeLength)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BackgroundSize)
