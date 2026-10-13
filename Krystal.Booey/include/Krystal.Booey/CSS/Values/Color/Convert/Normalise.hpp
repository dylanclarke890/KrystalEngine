#pragma once

#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Graphics/Color/ColorNormalization.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include <limits>

namespace krys::boo::css
{
  // MARK: - NormaliseAndClampNumericComponents

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD css::Number<> NormaliseAndClampNumericComponents(css::NumberRaw<> number) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);

    if constexpr (info.Type == ColorComponentType::Angle)
    {
      return {NormalizeHue(number.Value)};
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity()
                       && info.Max == std::numeric_limits<double>::infinity())
    {
      return {number.Value};
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity())
    {
      return {std::min(number.Value, info.Max)};
    }
    else if constexpr (info.Max == std::numeric_limits<double>::infinity())
    {
      return {std::max(number.Value, info.Min)};
    }
    else
    {
      return {std::clamp(number.Value, info.Min, info.Max)};
    }
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD css::Number<> NormaliseAndClampNumericComponents(css::PercentageRaw<> percent) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);

    if constexpr (info.Min == -std::numeric_limits<double>::infinity()
                  && info.Max == std::numeric_limits<double>::infinity())
    {
      return {percent.Value * info.PercentMultiplier};
    }
    else if constexpr (info.Min == -std::numeric_limits<double>::infinity())
    {
      return {std::min(percent.Value * info.PercentMultiplier, info.Max)};
    }
    else if constexpr (info.Max == std::numeric_limits<double>::infinity())
    {
      return {std::max(percent.Value * info.PercentMultiplier, info.Min)};
    }
    else
    {
      return {std::clamp(percent.Value * info.PercentMultiplier, info.Min, info.Max)};
    }
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD css::Number<> NormaliseAndClampNumericComponents(css::AngleRaw<> angle) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    static_assert(info.Type == ColorComponentType::Angle);

    return {NormalizeHue(css::ConvertToValueInUnitsOf<css::AngleUnit::Deg>(angle))};
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD auto
    NormaliseAndClampNumericComponentsIntoCanonicalRepresentation(const keywords::None &none) noexcept
    -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
  {
    return none;
  }

  template <typename Descriptor, size_t Index, css::Numeric T>
  KRYS_NODISCARD auto NormaliseAndClampNumericComponentsIntoCanonicalRepresentation(const T &value) noexcept
    -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
  {
    return krys::SwitchOn(
      value,
      [](const typename T::Raw &value)
        -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
      { return NormaliseAndClampNumericComponents<Descriptor, Index>(value); },
      [](const typename T::Calc &value)
        -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
      { return T {value}; });
  }

  template <typename Descriptor, size_t Index, typename... Ts>
  KRYS_NODISCARD auto
    NormaliseAndClampNumericComponentsIntoCanonicalRepresentation(const Variant<Ts...> &variant) noexcept
    -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
  {
    return krys::SwitchOn(
      variant, [](auto value)
      { return NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, Index>(value); });
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD auto NormaliseAndClampNumericComponentsIntoCanonicalRepresentation(
    const Maybe<GetCSSColorParseTypeWithCalcComponentResult<Descriptor, Index>> &optional) noexcept
    -> Maybe<GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>>
  {
    return optional
             ? std::make_optional(
                 NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, Index>(*optional))
             : null;
  }

  // MARK: - NormaliseNumericComponents

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD css::Number<> NormaliseNumericComponents(css::NumberRaw<> number) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);

    if constexpr (info.Type == ColorComponentType::Angle)
    {
      return {NormalizeHue(number.Value)};
    }
    else
    {
      return {number.Value};
    }
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD css::Number<> NormaliseNumericComponents(css::PercentageRaw<> percent) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);

    return {percent.Value * info.PercentMultiplier};
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD css::Number<> NormaliseNumericComponents(css::AngleRaw<> angle) noexcept
  {
    constexpr auto info = std::get<Index>(Descriptor::Components);
    static_assert(info.Type == ColorComponentType::Angle);

    return {NormalizeHue(css::ConvertToValueInUnitsOf<css::AngleUnit::Deg>(angle))};
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD auto
    NormaliseNumericComponentsIntoCanonicalRepresentation(const keywords::None &none) noexcept
    -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
  {
    return none;
  }

  template <typename Descriptor, size_t Index, css::Numeric T>
  KRYS_NODISCARD auto NormaliseNumericComponentsIntoCanonicalRepresentation(const T &value) noexcept
    -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
  {
    return krys::SwitchOn(
      value,
      [](const typename T::Raw &value)
        -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
      { return NormaliseNumericComponents<Descriptor, Index>(value); },
      [](const typename T::Calc &value)
        -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
      { return T {value}; });
  }

  template <typename Descriptor, size_t Index, typename... Ts>
  KRYS_NODISCARD auto
    NormaliseNumericComponentsIntoCanonicalRepresentation(const Variant<Ts...> &variant) noexcept
    -> GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>
  {
    return krys::SwitchOn(
      variant, [](auto value)
      { return NormaliseNumericComponentsIntoCanonicalRepresentation<Descriptor, Index>(value); });
  }

  template <typename Descriptor, size_t Index>
  KRYS_NODISCARD auto NormaliseNumericComponentsIntoCanonicalRepresentation(
    const Maybe<GetCSSColorParseTypeWithCalcComponentResult<Descriptor, Index>> &optional) noexcept
    -> Maybe<GetCSSColorParseTypeWithCalcComponentResult<typename Descriptor::Canonical, Index>>
  {
    return optional ? std::make_optional(
                        NormaliseNumericComponentsIntoCanonicalRepresentation<Descriptor, Index>(*optional))
                    : null;
  }
}