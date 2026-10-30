#pragma once

#include "Krystal.Booey/CSS/Values/FilterEffects/BlurFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/BrightnessFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/ContrastFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/DropShadowFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/FilterReference.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/GrayscaleFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/HueRotateFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/InvertFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/OpacityFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/SaturateFunction.hpp"
#include "Krystal.Booey/CSS/Values/FilterEffects/SepiaFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Types.hpp"

namespace krys::boo::css
{
  // Any <filter-function> or a reference to filter via <url>.
  // https://drafts.fxtf.org/filter-effects/#typedef-filter-function
  using FilterValueKind = Variant<BlurFunction, BrightnessFunction, ContrastFunction, DropShadowFunction,
                                  GrayscaleFunction, HueRotateFunction, InvertFunction, OpacityFunction,
                                  SaturateFunction, SepiaFunction, FilterReference>;

  struct FilterData
  {
    FilterValueKind Value;

    template <typename T>
    requires std::constructible_from<FilterValueKind, T>
    FilterData(T &&value) noexcept : Value(std::forward<T>(value))
    {
    }

    KRYS_FORWARD_VARIANT_FUNCTIONS(FilterData, Value)

    KRYS_NODISCARD bool operator==(const FilterData &) const noexcept = default;
  };

  // <filter-value-list> = [ <filter-function> | <url> ]+
  // https://drafts.fxtf.org/filter-effects/#typedef-filter-value-list
  using FilterValueList = SpaceSeparatedList<FilterData>;

  // <'filter'> = none | <filter-value-list>
  // https://drafts.fxtf.org/filter-effects/#propdef-filter
  struct Filter : ListOrNone<FilterValueList>
  {
    using ListOrNone<FilterValueList>::ListOrNone;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::FilterData)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::css::Filter)
