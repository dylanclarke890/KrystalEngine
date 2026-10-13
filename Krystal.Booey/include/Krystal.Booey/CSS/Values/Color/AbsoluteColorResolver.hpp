#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToTypedColor.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::css
{
  template <typename D>
  struct AbsoluteColorResolver
  {
    using Descriptor = D;

    CSSColorParseTypeWithCalc<Descriptor> Components;
    uint32 NestingLevel;
  };

  template <typename D>
  KRYS_NODISCARD boo::Color Resolve(const AbsoluteColorResolver<D> &absolute,
                                    const ToLengthConversionData &conversionData) noexcept
  {
    // Evaluated any calc values to their corresponding channel value.
    auto components =
      StyleColorParseType<D> {style::ConvertToStyle(std::get<0>(absolute.Components), conversionData),
                              style::ConvertToStyle(std::get<1>(absolute.Components), conversionData),
                              style::ConvertToStyle(std::get<2>(absolute.Components), conversionData),
                              style::ConvertToStyle(std::get<3>(absolute.Components), conversionData)};

    // Normalize values into their numeric form, forming a validated typed color.
    auto typedColor = ConvertToTypedColor<D>(components, 1.0);

    // Convert the validated typed color into a `boo::Color`,
    return ConvertToColor<D, ColorFunctionForm::Absolute>(typedColor, absolute.NestingLevel);
  }

  // This resolve function should only be called if the components have been checked and don't require
  // conversion data to be resolved.
  template <typename D>
  KRYS_NODISCARD boo::Color ResolveNoConversionDataRequired(const AbsoluteColorResolver<D> &absolute) noexcept
  {
    krys_debug_assert(!ComponentsRequireConversionData<D>(absolute.Components));

    // Evaluated any calc values to their corresponding channel value.
    auto components = StyleColorParseType<D> {
      style::ConvertToStyleNoConversionDataRequired(std::get<0>(absolute.Components)),
      style::ConvertToStyleNoConversionDataRequired(std::get<1>(absolute.Components)),
      style::ConvertToStyleNoConversionDataRequired(std::get<2>(absolute.Components)),
      style::ConvertToStyleNoConversionDataRequired(std::get<3>(absolute.Components))};

    // Normalize values into their numeric form, forming a validated typed color.
    auto typedColor = ConvertToTypedColor<D>(components, 1.0);

    // Convert the validated typed color into a `boo::Color`,
    return ConvertToColor<D, ColorFunctionForm::Absolute>(typedColor, absolute.NestingLevel);
  }
}
