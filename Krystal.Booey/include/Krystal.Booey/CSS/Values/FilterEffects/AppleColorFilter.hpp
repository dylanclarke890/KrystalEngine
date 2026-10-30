#pragma once

#include "Krystal.Booey/CSS/Values/FilterEffects/AppleInvertLightnessFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/BrightnessFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/ContrastFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/GrayscaleFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/HueRotateFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/InvertFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/OpacityFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/SaturateFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/SepiaFunction.hpp"

namespace krys::boo::css
{
  // Non-standard types used for the `-apple-color-filter` property. It is similar to <'filter'>,
  // but does not support `blur()`, `drop-shadow()` and reference filters, but adds support for the
  // non-standard function `-apple-invert-lightness-filter()`.

  // Any <apple-color-filter-function>.
  // (Equivalent of https://drafts.fxtf.org/filter-effects/#typedef-filter-function)
  using AppleColorFilterValueKind =
    Variant<AppleInvertLightnessFunction, BrightnessFunction, ContrastFunction, GrayscaleFunction,
            HueRotateFunction, InvertFunction, OpacityFunction, SaturateFunction, SepiaFunction>;

  struct AppleColorFilterData
  {
    AppleColorFilterValueKind Value;

    template <typename T>
    requires std::constructible_from<AppleColorFilterValueKind, T>
    AppleColorFilterData(T &&value) noexcept : Value(std::forward<T>(value))
    {
    }

    KRYS_FORWARD_VARIANT_FUNCTIONS(AppleColorFilterData, Value)

    KRYS_NODISCARD bool operator==(const AppleColorFilterData &) const noexcept = default;
  };

  // <apple-color-filter-value-list> = [ <apple-color-filter-function> | <url> ]+
  // (Equivalent of https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list)
  using AppleColorFilterValueList = SpaceSeparatedList<AppleColorFilterData>;

  // <'-apple-color-filter'> = none | <-apple-color-filter-value-list>
  // (Equivalent of https://drafts.fxtf.org/filter-effects/#propdef-filter)
  struct AppleColorFilter : ListOrNone<AppleColorFilterValueList>
  {
    using ListOrNone<AppleColorFilterValueList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::AppleColorFilterData)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::AppleColorFilter)
