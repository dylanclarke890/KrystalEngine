#pragma once

#include "Krystal.Booey/CSS/Values/Color/CurrentColor.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"

namespace krys::boo::css
{
  template <ValueId FilterFunction>
  struct CSSFilterFunctionDescriptor;

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-blur
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Blur>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = false;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = LengthRaw<> {LengthUnit::Px, 0};
    constexpr static auto InitialLengthValueForInterpolation = LengthRaw<> {LengthUnit::Px, 0};

    constexpr static auto OperationType = boo::FilterOperation::Type::Blur;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-brightness
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Brightness>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = true;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = boo::FilterOperation::Type::Brightness;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-contrast
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Contrast>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = true;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = boo::FilterOperation::Type::Contrast;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-drop-shadow
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::DropShadow>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = false;

    constexpr static auto DefaultColorValue = CurrentColor {};
    constexpr static auto DefaultStdDeviationValue = LengthRaw<> {LengthUnit::Px, 0};

    // TODO: expose predefined colors on Color as static members
    // constexpr static auto InitialColorValueForInterpolation = Color::TransparentBlack;
    constexpr static auto InitialLengthValueForInterpolation = LengthRaw<> {LengthUnit::Px, 0};

    constexpr static auto OperationType = boo::FilterOperation::Type::DropShadowWithStyleColor;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-grayscale
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Grayscale>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {0};

    constexpr static auto OperationType = boo::FilterOperation::Type::Grayscale;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-hue-rotate
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::HueRotate>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static auto DefaultValue = AngleRaw<> {AngleUnit::Deg, 0};
    constexpr static auto InitialValueForInterpolation = AngleRaw<> {AngleUnit::Deg, 0};

    constexpr static auto OperationType = boo::FilterOperation::Type::HueRotate;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-invert
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Invert>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {0};

    constexpr static auto OperationType = boo::FilterOperation::Type::Invert;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-opacity
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Opacity>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = boo::FilterOperation::Type::Opacity;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-saturate
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Saturate>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = true;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {1};

    constexpr static auto OperationType = boo::FilterOperation::Type::Saturate;
  };

  // https://drafts.fxtf.org/filter-effects/#funcdef-filter-sepia
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::Sepia>
  {
    constexpr static bool IsPixelFilterFunction = true;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static bool AllowsValuesGreaterThanOne = false;
    constexpr static auto DefaultValue = NumberRaw<> {1};
    constexpr static auto InitialValueForInterpolation = NumberRaw<> {0};

    constexpr static auto OperationType = boo::FilterOperation::Type::Sepia;
  };

  // Non-standard addition.
  template <>
  struct CSSFilterFunctionDescriptor<ValueId::AppleInvertLightness>
  {
    constexpr static bool IsPixelFilterFunction = false;
    constexpr static bool IsColorFilterFunction = true;

    constexpr static auto OperationType = boo::FilterOperation::Type::AppleInvertLightness;
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