#pragma once

#include "Krystal.HTML/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.HTML/CSS/Values/Filter/AppleInvertLightnessFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/BrightnessFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/ContrastFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/GrayscaleFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/HueRotateFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/InvertFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/OpacityFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/SaturateFunction.hpp"
#include "Krystal.HTML/CSS/Values/Filter/SepiaFunction.hpp"
#include "Krystal.HTML/CSS/Values/Primitives/CSSPrimitiveNumericTypes.hpp"

namespace Krys::HTML
{
  // Non-standard types used for the `-apple-color-filter` property. It is similar to <'filter'>,
  // but does not support `blur()`, `drop-shadow()` and reference filters, but adds support for the
  // non-standard function `-apple-invert-lightness-filter()`.

  // Any <apple-color-filter-function>.
  // (Equivalent of https://drafts.fxtf.org/filter-effects/#typedef-filter-function)
  using AppleColorFilterValueKind =
    Variant<AppleInvertLightnessFunction, BrightnessFunction, ContrastFunction, GrayscaleFunction,
            HueRotateFunction, InvertFunction, OpacityFunction, SaturateFunction, SepiaFunction>;

  struct AppleColorFilterValue
  {
    AppleColorFilterValueKind value;

    template <typename T>
    requires std::constructible_from<AppleColorFilterValueKind, T>
    AppleColorFilterValue(T &&value) : value(std::forward<T>(value))
    {
    }

    KRYS_FORWARD_VARIANT_FUNCTIONS(AppleColorFilterValue, value)

    bool operator==(const AppleColorFilterValue &) const = default;
  };

  // <apple-color-filter-value-list> = [ <apple-color-filter-function> | <url> ]+
  // (Equivalent of https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list)
  using AppleColorFilterValueList = SpaceSeparatedList<AppleColorFilterValue>;

  // <'-apple-color-filter'> = none | <-apple-color-filter-value-list>
  // (Equivalent of https://drafts.fxtf.org/filter-effects/#propdef-filter)
  struct AppleColorFilter : public ListOrNone<AppleColorFilterValueList>
  {
    using ListOrNone<AppleColorFilterValueList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::AppleColorFilterValue)
DEFINE_VARIANT_LIKE_CONFORMANCE(Krys::HTML::AppleColorFilter)
