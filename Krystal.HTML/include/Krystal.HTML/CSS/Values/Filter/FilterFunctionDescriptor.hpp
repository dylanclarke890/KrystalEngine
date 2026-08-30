#pragma once

#include "Krystal.HTML/CSS/Values/Color/CurrentColor.hpp"
#include "Krystal.HTML/CSS/Values/Filter/FilterOperationType.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  template <CSSValueId FilterFunction>
  struct CSSFilterFunctionDescriptor;

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-blur
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Blur>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = false;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = LengthRaw<> {LengthUnit::Px, 0};
    constexpr static auto InitialLengthValueForInterpolation = LengthRaw<> {LengthUnit::Px, 0};

    constexpr static auto OperationType = FilterOperationType::Blur;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Brightness>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = true;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = FilterOperationType::Brightness;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-contrast
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Contrast>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = true;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = FilterOperationType::Contrast;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-drop-shadow
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::DropShadow>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = false;

    constexpr static auto DefaultColorValue = CurrentColor {};
    constexpr static auto DefaultStdDeviationValue = LengthRaw<> {LengthUnit::Px, 0};

    // TODO: expose predefined colors on Color as static members
    // constexpr static auto InitialColorValueForInterpolation = Color::TransparentBlack;
    constexpr static auto InitialLengthValueForInterpolation = LengthRaw<> {LengthUnit::Px, 0};

    constexpr static auto OperationType = FilterOperationType::DropShadowWithStyleColor;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-grayscale
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Grayscale>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {0};

    constexpr static auto OperationType = FilterOperationType::Grayscale;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-hue-rotate
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::HueRotate>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static auto DefaultValue = AngleRaw<> {AngleUnit::Deg, 0};
    constexpr static auto InitialValueForInterpolation = AngleRaw<> {AngleUnit::Deg, 0};

    constexpr static auto OperationType = FilterOperationType::HueRotate;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-invert
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Invert>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {0};

    constexpr static auto OperationType = FilterOperationType::Invert;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-opacity
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Opacity>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = FilterOperationType::Opacity;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-saturate
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Saturate>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = true;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = FilterOperationType::Saturate;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-sepia
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::Sepia>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {0};

    constexpr static auto OperationType = FilterOperationType::Sepia;
  };

  // Non-standard addition.
  template <>
  struct CSSFilterFunctionDescriptor<CSSValueId::AppleInvertLightness>
  {
    constexpr static bool IsPixelFilterFunction = false;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static auto OperationType = FilterOperationType::AppleInvertLightness;
  };

  template <auto FilterFunction>
  KRYS_NODISCARD constexpr bool IsPixelFilterFunction() noexcept
  {
    return CSSFilterFunctionDescriptor<FilterFunction>::IsPixelFilterFunction;
  }

  template <auto FilterFunction>
  KRYS_NODISCARD constexpr bool IsColorFilterFunction() noexcept
  {
    return CSSFilterFunctionDescriptor<FilterFunction>::IsColorFilterFunction;
  }

  template <auto FilterFunction>
  KRYS_NODISCARD constexpr bool FilterFunctionAllowsValuesGreaterThanOne() noexcept
  {
    return CSSFilterFunctionDescriptor<FilterFunction>::AllowsValuesGreaterThanOne;
  }

  template <auto FilterFunction>
  KRYS_NODISCARD constexpr decltype(auto) FilterFunctionDefaultValue() noexcept
  {
    return CSSFilterFunctionDescriptor<FilterFunction>::DefaultValue;
  }

  template <auto FilterFunction>
  KRYS_NODISCARD constexpr decltype(auto) FilterFunctionInitialValueForInterpolation() noexcept
  {
    return CSSFilterFunctionDescriptor<FilterFunction>::InitialValueForInterpolation;
  }

  template <auto FilterFunction>
  KRYS_NODISCARD constexpr decltype(auto) FilterFunctionOperationType() noexcept
  {
    return CSSFilterFunctionDescriptor<FilterFunction>::OperationType;
  }
}