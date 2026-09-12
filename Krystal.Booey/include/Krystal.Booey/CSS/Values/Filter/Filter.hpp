#pragma once

#include "Krystal.Booey/CSS/Values/CSSValueTypes.hpp"
#include "Krystal.Booey/CSS/Values/Filter/BlurFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/BrightnessFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/ContrastFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/DropShadowFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/FilterReference.hpp"
#include "Krystal.Booey/CSS/Values/Filter/GrayscaleFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/HueRotateFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/InvertFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/OpacityFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/SaturateFunction.hpp"
#include "Krystal.Booey/CSS/Values/Filter/SepiaFunction.hpp"

namespace krys::boo::css
{
  // Any <filter-function> or a reference to filter via <url>.
  // https://drafts.fxtf.org/filter-effects/#typedef-filter-function
  using FilterValueKind = Variant<BlurFunction, BrightnessFunction, ContrastFunction, DropShadowFunction,
                                  GrayscaleFunction, HueRotateFunction, InvertFunction, OpacityFunction,
                                  SaturateFunction, SepiaFunction, FilterReference>;

  struct FilterValue
  {
    FilterValueKind value;

    template <typename T>
    requires std::constructible_from<FilterValueKind, T>
    FilterValue(T &&value) : value(std::forward<T>(value))
    {
    }

    KRYS_FORWARD_VARIANT_FUNCTIONS(FilterValue, value)

    bool operator==(const FilterValue &) const = default;
  };

  // <filter-value-list> = [ <filter-function> | <url> ]+
  // https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list
  using FilterValueList = SpaceSeparatedList<FilterValue>;

  // <'filter'> = none | <filter-value-list>
  // https://drafts.fxtf.org/filter-effects/#propdef-filter
  struct Filter : ListOrNone<FilterValueList>
  {
    using ListOrNone<FilterValueList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::FilterValue)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::Filter)
