#pragma once

#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/ToTypedColor.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSymbolReplacement.hpp"
#include "Krystal.Booey/CSS/Values/UnitType.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesConversions.hpp"

namespace krys::boo::css
{
  template <typename D>
  struct RelativeColorResolver
  {
    using Descriptor = D;

    boo::Color Origin;
    CSSColorParseTypeWithCalcAndSymbols<Descriptor> Components;
  };

  namespace detail_relative_color
  {
    template <typename Descriptor, typename ColorType>
    KRYS_NODISCARD SymbolTable CreateSymbolTable(const ColorType &originColorAsColorType) noexcept
    {
      auto originComponents = AsColourComponents(originColorAsColorType.Resolved());

      return SymbolTable {{std::get<0>(Descriptor::Components).Symbol, UnitType::Number,
                           originComponents[0] * std::get<0>(Descriptor::Components).SymbolMultiplier},
                          {std::get<1>(Descriptor::Components).Symbol, UnitType::Number,
                           originComponents[1] * std::get<1>(Descriptor::Components).SymbolMultiplier},
                          {std::get<2>(Descriptor::Components).Symbol, UnitType::Number,
                           originComponents[2] * std::get<2>(Descriptor::Components).SymbolMultiplier},
                          {std::get<3>(Descriptor::Components).Symbol, UnitType::Number,
                           originComponents[3] * std::get<3>(Descriptor::Components).SymbolMultiplier}};
    }

    template <typename Descriptor>
    KRYS_NODISCARD CSSColorParseTypeWithCalc<Descriptor>
      ReplaceSymbols(const CSSColorParseTypeWithCalcAndSymbols<Descriptor> &components,
                     const SymbolTable &symbolTable) noexcept
    {
      return CSSColorParseTypeWithCalc<Descriptor> {ReplaceSymbol(std::get<0>(components), symbolTable),
                                                    ReplaceSymbol(std::get<1>(components), symbolTable),
                                                    ReplaceSymbol(std::get<2>(components), symbolTable),
                                                    ReplaceSymbol(std::get<3>(components), symbolTable)};
    }
  }

  template <typename Descriptor>
  KRYS_NODISCARD boo::Color Resolve(const RelativeColorResolver<Descriptor> &relative,
                                    const ToLengthConversionData &conversionData) noexcept
  {
    auto originColor = relative.Origin;
    auto originColorAsColorType = originColor.template ToColorTypeLossy<GetColorType<Descriptor>>();
    const auto symbolTable = detail_relative_color::CreateSymbolTable<Descriptor>(originColorAsColorType);

    // Replace any symbol value (e.g. ValueId::R) to their corresponding channel value.
    auto componentsWithUnevaluatedCalc =
      detail_relative_color::ReplaceSymbols<Descriptor>(relative.Components, symbolTable);

    // Evaluated any calc values to their corresponding channel value.
    auto components = StyleColorParseType<Descriptor> {
      style::ConvertToStyle(std::get<0>(componentsWithUnevaluatedCalc), conversionData, symbolTable),
      style::ConvertToStyle(std::get<1>(componentsWithUnevaluatedCalc), conversionData, symbolTable),
      style::ConvertToStyle(std::get<2>(componentsWithUnevaluatedCalc), conversionData, symbolTable),
      style::ConvertToStyle(std::get<3>(componentsWithUnevaluatedCalc), conversionData, symbolTable)};

    // Normalize values into their numeric form, forming a validated typed color.
    auto typedColor = ConvertToTypedColor<Descriptor>(components, originColorAsColorType.Unresolved().Alpha);

    // Convert the validated typed color into a `Color`,
    return ConvertToColor<Descriptor, ColorFunctionForm::Relative>(typedColor);
  }

  // This resolve function should only be called if the components have been checked and don't require
  // conversion data to be resolved.
  template <typename Descriptor>
  KRYS_NODISCARD boo::Color
    ResolveNoConversionDataRequired(const RelativeColorResolver<Descriptor> &relative) noexcept
  {
    krys_debug_assert(!ComponentsRequireConversionData<Descriptor>(relative.Components));

    auto originColor = relative.Origin;
    auto originColorAsColorType = originColor.template ToColorTypeLossy<GetColorType<Descriptor>>();
    const auto symbolTable = detail_relative_color::CreateSymbolTable<Descriptor>(originColorAsColorType);

    // Replace any symbol value (e.g. ValueId::R) to their corresponding channel value.
    auto componentsWithUnevaluatedCalc =
      detail_relative_color::ReplaceSymbols<Descriptor>(relative.Components, symbolTable);

    // Evaluated any calc values to their corresponding channel value.
    auto components = StyleColorParseType<Descriptor> {
      style::ConvertToStyleNoConversionDataRequired(std::get<0>(componentsWithUnevaluatedCalc), symbolTable),
      style::ConvertToStyleNoConversionDataRequired(std::get<1>(componentsWithUnevaluatedCalc), symbolTable),
      style::ConvertToStyleNoConversionDataRequired(std::get<2>(componentsWithUnevaluatedCalc), symbolTable),
      style::ConvertToStyleNoConversionDataRequired(std::get<3>(componentsWithUnevaluatedCalc), symbolTable)};

    // Normalize values into their numeric form, forming a validated typed color.
    auto typedColor = ConvertToTypedColor<Descriptor>(components, originColorAsColorType.Unresolved().Alpha);

    // Convert the validated typed color into a `Color`,
    return ConvertToColor<Descriptor, ColorFunctionForm::Relative>(typedColor);
  }
}
