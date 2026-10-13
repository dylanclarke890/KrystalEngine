#pragma once

#include "Krystal.Booey/CSS/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesEvaluateCalc.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesSymbolReplacement.hpp"
#include "Krystal.Booey/CSS/Values/ValueId.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/Variant.hpp"
#include <brigand/algorithms/transform.hpp>
#include <brigand/algorithms/wrap.hpp>
#include <brigand/sequences/list.hpp>

namespace krys::boo::css
{
  enum class ColorFunctionSyntax
  {
    Legacy,
    Modern
  };

  enum class ColorFunctionForm
  {
    Relative,
    Absolute
  };

  template <typename... Ts>
  struct ColorComponent
  {
    using ResultTypeList = brigand::list<Ts...>;
    using Result = brigand::wrap<ResultTypeList, VariantOrSingle>;

    /// @brief Symbol used to represent component for relative color form.
    ValueId Symbol;

    /// @brief The range parsed <number> and <percentage> values are clamped to after a multiplier has been
    /// applied.
    double Min = -std::numeric_limits<double>::infinity();
    double Max = std::numeric_limits<double>::infinity();

    /// @brief The value parsed <number> values are multiplied by for normalization to typed-color value.
    double NumberMultiplier = 1.0;

    /// @brief The value parsed <percentage> values are multiplied by for normalization to <number>.
    double PercentMultiplier = 1.0 / 100.0;

    /// @brief Value the corresponding origin color component is multiplied by for use as a symbol for
    /// relative color form.
    double SymbolMultiplier = 1.0;

    /// @brief Component type for normalization. Angles get normalized using modular arithmetic, numbers get
    /// normalized using clamping.
    ColorComponentType Type = ColorComponentType::Number;
  };

  /// @brief e.g. for GetComponent<ColorRGBFunction<SRGB<float>>, 2> ->
  /// ColorComponent<css::Percentage<>, css::Number<>, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetComponent = decay_t<decltype(std::get<Index>(Descriptor::Components))>;

  /// @brief e.g. for GetComponentResult<ColorRGBFunction<SRGB<float>>, 2>
  /// -> Variant<css::Percentage<>, css::Number<>, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetComponentResult = typename GetComponent<Descriptor, Index>::Result;

  /// @brief e.g. for GetComponentResultTypeList<ColorRGBFunction<SRGB<float>>, 2>
  /// -> brigand::list<css::Percentage<>, css::Number<>, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetComponentResultTypeList = typename GetComponent<Descriptor, Index>::ResultTypeList;

  /// @brief e.g. for GetColorType<ColorRGBFunction<SRGB<float>>> -> SRGB<float>
  template <typename Descriptor>
  using GetColorType = typename Descriptor::ColorType;

  /// @brief e.g. for GetColorType<ColorRGBFunction<T>> -> typename SRGB<float>::ComponentType -> float
  template <typename Descriptor>
  using GetColorTypeComponentType = typename GetColorType<Descriptor>::ComponentType;

  // MARK: Style resolved parse type (components are Variants with Style namespaced primitives)

  // e.g. for GetStyleColorParseTypeComponentTypeList<ColorRGBFunction<SRGB<float>>, 2> ->
  // brigand::list<style::Percentage, style::Number, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetStyleColorParseTypeComponentTypeList =
    brigand::transform<typename GetComponent<Descriptor, Index>::ResultTypeList,
                       style::ToStyleMapping<brigand::_1>>;

  /// @brief e.g. for GetStyleColorParseTypeComponentResult<ColorRGBFunction<SRGB<float>>, 2>
  /// -> Variant<style::Percentage, style::Number, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetStyleColorParseTypeComponentResult =
    brigand::wrap<GetStyleColorParseTypeComponentTypeList<Descriptor, Index>, VariantOrSingle>;

  /// @brief e.g. for StyleColorParseType<ColorRGBFunction<T>> ->
  /// std::tuple<Variant<style::Percentage, style::Number, keywords::None>, ...>
  template <typename Descriptor>
  using StyleColorParseType = std::tuple<GetStyleColorParseTypeComponentResult<Descriptor, 0>,
                                         GetStyleColorParseTypeComponentResult<Descriptor, 1>,
                                         GetStyleColorParseTypeComponentResult<Descriptor, 2>,
                                         Maybe<GetStyleColorParseTypeComponentResult<Descriptor, 3>>>;

  template <typename Descriptor>
  KRYS_NODISCARD constexpr bool
    ComponentsContainsUnevaluatedCalc(const StyleColorParseType<Descriptor> &) noexcept
  {
    return false;
  }

  template <typename Descriptor>
  KRYS_NODISCARD constexpr bool
    ComponentsRequireConversionData(const StyleColorParseType<Descriptor> &) noexcept
  {
    return false;
  }

  // MARK: CSS absolute color parse type (components are Variants with CSS namespaced primitive, but no symbol
  // values)

  /// @brief e.g. for GetCSSColorParseTypeWithCalcComponentTypeList<ColorRGBFunction<SRGB<float>>, 2> ->
  /// brigand::list<css::Percentage<>, css::Number<>, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetCSSColorParseTypeWithCalcComponentTypeList =
    typename GetComponent<Descriptor, Index>::ResultTypeList;

  /// @brief e.g. for GetCSSColorParseTypeWithCalcComponentResult<ColorRGBFunction<SRGB<float>>, 2>
  /// -> Variant<css::Percentage<>, css::Number<>, keywords::None>
  template <typename Descriptor, size_t Index>
  using GetCSSColorParseTypeWithCalcComponentResult =
    brigand::wrap<GetCSSColorParseTypeWithCalcComponentTypeList<Descriptor, Index>, VariantOrSingle>;

  /// @brief e.g. for CSSColorParseTypeWithCalc<ColorRGBFunction<T>>
  /// -> std::tuple<Variant<css::Percentage<>, css::Number<>, keywords::None>, ...>
  template <typename Descriptor>
  using CSSColorParseTypeWithCalc =
    std::tuple<GetCSSColorParseTypeWithCalcComponentResult<Descriptor, 0>,
               GetCSSColorParseTypeWithCalcComponentResult<Descriptor, 1>,
               GetCSSColorParseTypeWithCalcComponentResult<Descriptor, 2>,
               Maybe<GetCSSColorParseTypeWithCalcComponentResult<Descriptor, 3>>>;

  template <typename Descriptor>
  KRYS_NODISCARD bool
    ComponentsContainsUnevaluatedCalc(const CSSColorParseTypeWithCalc<Descriptor> &components) noexcept
  {
    return IsUnevaluatedCalc(std::get<0>(components)) || IsUnevaluatedCalc(std::get<1>(components))
           || IsUnevaluatedCalc(std::get<2>(components)) || IsUnevaluatedCalc(std::get<3>(components));
  }

  template <typename Descriptor>
  KRYS_NODISCARD bool
    ComponentsRequireConversionData(const CSSColorParseTypeWithCalc<Descriptor> &components) noexcept
  {
    return RequiresConversionData(std::get<0>(components)) || RequiresConversionData(std::get<1>(components))
           || RequiresConversionData(std::get<2>(components))
           || RequiresConversionData(std::get<3>(components));
  }

  // MARK: CSS relative color parse type (components are Variants with CSS namespaced primitive, includes
  // symbol values)

  /// @brief e.g. for GetCSSColorParseTypeWithCalcAndSymbolsComponentTypeList<ColorRGBFunction<SRGB<float>>,
  /// 2>
  /// -> brigand::list<css::Percentage<>, css::Number<>, keywords::None, css::Symbol>
  template <typename Descriptor, size_t Index>
  using GetCSSColorParseTypeWithCalcAndSymbolsComponentTypeList =
    PlusSymbol<typename GetComponent<Descriptor, Index>::ResultTypeList>;

  // e.g. for GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<ColorRGBFunction<SRGB<float>>, 2> ->
  // Variant<css::Percentage<>, css::Number<>, keywords::None, css::Symbol>
  template <typename Descriptor, size_t Index>
  using GetCSSColorParseTypeWithCalcAndSymbolsComponentResult =
    brigand::wrap<GetCSSColorParseTypeWithCalcAndSymbolsComponentTypeList<Descriptor, Index>, VariantOrSingle>;

  // e.g. for CSSColorParseTypeWithCalcAndSymbols<ColorRGBFunction<T>> ->
  // std::tuple<Variant<css::Percentage<>, css::Number<>, keywords::None, css::Symbol>, ...>
  template <typename Descriptor>
  using CSSColorParseTypeWithCalcAndSymbols =
    std::tuple<GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<Descriptor, 0>,
               GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<Descriptor, 1>,
               GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<Descriptor, 2>,
               Maybe<GetCSSColorParseTypeWithCalcAndSymbolsComponentResult<Descriptor, 3>>>;

  template <typename Descriptor>
  KRYS_NODISCARD bool ComponentsContainsUnevaluatedCalc(
    const CSSColorParseTypeWithCalcAndSymbols<Descriptor> &components) noexcept
  {
    return IsUnevaluatedCalc(std::get<0>(components)) || IsUnevaluatedCalc(std::get<1>(components))
           || IsUnevaluatedCalc(std::get<2>(components)) || IsUnevaluatedCalc(std::get<3>(components));
  }

  template <typename Descriptor>
  KRYS_NODISCARD bool ComponentsRequireConversionData(
    const CSSColorParseTypeWithCalcAndSymbols<Descriptor> &components) noexcept
  {
    return RequiresConversionData(std::get<0>(components)) || RequiresConversionData(std::get<1>(components))
           || RequiresConversionData(std::get<2>(components))
           || RequiresConversionData(std::get<3>(components));
  }

  // MARK: - Shared Component Descriptors

  constexpr auto AlphaComponent = ColorComponent<css::Percentage<>, css::Number<>, keywords::None> {
    .Symbol = ValueId::Alpha, .Min = 0.0, .Max = 1.0};

  constexpr auto AlphaLegacyComponent =
    ColorComponent<css::Percentage<>, css::Number<>> {.Symbol = ValueId::Alpha, .Min = 0.0, .Max = 1.0};

  // MARK: - Color Function Descriptors

  // <modern-rgb-syntax>  =  rgb( [<number> | <percentage> | none]{3} [ / [<alpha-value> | none] ]? )
  // <modern-rgba-syntax> = rgba( [<number> | <percentage> | none]{3} [ / [<alpha-value> | none] ]? )
  struct RGBFunctionModernAbsolute
  {
    using ColorType = SRGBA<float>;
    using Canonical = RGBFunctionModernAbsolute;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = true;
    constexpr static bool AllowConversionTo8BitSRGB = true;
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"rgb";

    using R = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using G = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using B = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components = std::make_tuple(R {.Symbol = ValueId::R,
                                                          .Min = 0.0,
                                                          .Max = 255.0,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       G {.Symbol = ValueId::G,
                                                          .Min = 0.0,
                                                          .Max = 255.0,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       B {.Symbol = ValueId::B,
                                                          .Min = 0.0,
                                                          .Max = 255.0,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       AlphaComponent);
  };

  template <typename T>
  concept RGBFunctionLegacyComponent = SameType<T, css::Percentage<>> || SameType<T, css::Number<>>;

  // <legacy-rgb-syntax>  =  rgb( <percentage>#{3} , <alpha-value>? ) |  rgb( <number>#{3} , <alpha-value>? )
  // <legacy-rgba-syntax> = rgba( <percentage>#{3} , <alpha-value>? ) | rgba( <number>#{3} , <alpha-value>? )
  template <RGBFunctionLegacyComponent Component>
  struct RGBFunctionLegacy
  {
    using ColorType = SRGBA<float>;
    using Canonical = RGBFunctionModernAbsolute;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = true;
    constexpr static bool AllowConversionTo8BitSRGB = true;
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Legacy;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"rgb";

    using R = ColorComponent<Component>;
    using G = ColorComponent<Component>;
    using B = ColorComponent<Component>;

    constexpr static auto Components = std::make_tuple(R {.Symbol = ValueId::R,
                                                          .Min = 0.0,
                                                          .Max = 255.0,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       G {.Symbol = ValueId::G,
                                                          .Min = 0.0,
                                                          .Max = 255.0,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       B {.Symbol = ValueId::B,
                                                          .Min = 0.0,
                                                          .Max = 255.0,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       AlphaLegacyComponent);
  };

  // <modern-rgb-syntax>  =  rgb( [from <color>] [<number> | <percentage> | none]{3}
  //                             [ / [<alpha-value> | none] ]? )
  // <modern-rgba-syntax> = rgba( [from <color>] [<number> | <percentage> | none]{3}
  //                             [ / [<alpha-value> | none] ]? )
  struct RGBFunctionModernRelative
  {
    using ColorType = ExtendedSRGBA<float>;
    using Canonical = RGBFunctionModernRelative;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = true;
    constexpr static bool AllowConversionTo8BitSRGB = true;
    constexpr static ColorFlags FlagsForRelative = ColorFlags::UseColorFunctionSerialisation;
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"rgb";

    using R = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using G = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using B = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components = std::make_tuple(R {.Symbol = ValueId::R,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       G {.Symbol = ValueId::G,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       B {.Symbol = ValueId::B,
                                                          .NumberMultiplier = 1.0 / 255.0,
                                                          .PercentMultiplier = 255.0 / 100.0,
                                                          .SymbolMultiplier = 255.0},
                                                       AlphaComponent);
  };

  // <modern-hsl-syntax>  =  hsl( [from <color>]? [<hue> | none] [<percentage> | <number> | none]{2}
  //                             [ / [<alpha-value> | none] ]? )
  // <modern-hsla-syntax> = hsla( [from <color>]? [<hue> | none] [<percentage> | <number> | none]{2}
  //                             [ / [<alpha-value> | none] ]? )
  struct HSLFunctionModern
  {
    using ColorType = HSLA<float>;
    using Canonical = HSLFunctionModern;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = true;
    constexpr static bool AllowConversionTo8BitSRGB = true;
    constexpr static ColorFlags FlagsForRelative = ColorFlags::UseColorFunctionSerialisation;
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"hsl";

    using H = ColorComponent<css::Angle<>, css::Number<>, keywords::None>;
    using S = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using L = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components =
      std::make_tuple(H {.Symbol = ValueId::H, .Type = ColorComponentType::Angle},
                      S {.Symbol = ValueId::S, .Min = 0.0, .PercentMultiplier = 1.0},
                      L {.Symbol = ValueId::L, .PercentMultiplier = 1.0}, AlphaComponent);
  };

  // <legacy-hsl-syntax>  =  hsl( <hue>, <percentage>, <percentage>, <alpha-value>? )
  // <legacy-hsla-syntax> = hsla( <hue>, <percentage>, <percentage>, <alpha-value>? )
  struct HSLFunctionLegacy
  {
    using ColorType = HSLA<float>;
    using Canonical = HSLFunctionModern;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = true;
    constexpr static bool AllowConversionTo8BitSRGB = true;
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Legacy;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"hsl";

    using H = ColorComponent<css::Angle<>, css::Number<>>;
    using S = ColorComponent<css::Percentage<>>;
    using L = ColorComponent<css::Percentage<>>;

    constexpr static auto Components =
      std::make_tuple(H {.Symbol = ValueId::H, .Type = ColorComponentType::Angle},
                      S {.Symbol = ValueId::S, .Min = 0.0, .PercentMultiplier = 1.0},
                      L {.Symbol = ValueId::L, .PercentMultiplier = 1.0}, AlphaLegacyComponent);
  };

  // hwb() = hwb( [from <color>]? [<hue> | none] [<percentage> | <number> | none]{2} [ / [<alpha-value> |
  // none] ]? )
  struct HWBFunction
  {
    using ColorType = HWBA<float>;
    using Canonical = HWBFunction;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = true;
    constexpr static bool AllowConversionTo8BitSRGB = true;
    constexpr static ColorFlags FlagsForRelative = ColorFlags::UseColorFunctionSerialisation;
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"hwb";

    using H = ColorComponent<css::Angle<>, css::Number<>, keywords::None>;
    using W = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using B = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components =
      std::make_tuple(H {.Symbol = ValueId::H, .Type = ColorComponentType::Angle},
                      W {.Symbol = ValueId::W, .PercentMultiplier = 1.0},
                      B {.Symbol = ValueId::B, .PercentMultiplier = 1.0}, AlphaComponent);
  };

  // lab() = lab( [from <color>]? [<percentage> | <number> | none]{3} [ / [<alpha-value> | none] ]? )
  struct LabFunction
  {
    using ColorType = Lab<float>;
    using Canonical = LabFunction;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = false;
    constexpr static bool AllowConversionTo8BitSRGB = false;
    constexpr static ColorFlags FlagsForRelative = {};
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"lab";

    using L = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using A = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using B = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components =
      std::make_tuple(L {.Symbol = ValueId::L, .Min = 0.0, .Max = 100.0, .PercentMultiplier = 1.0},
                      A {.Symbol = ValueId::A, .PercentMultiplier = 125.0 / 100.0},
                      B {.Symbol = ValueId::B, .PercentMultiplier = 125.0 / 100.0}, AlphaComponent);
  };

  // lch() = lch( [from <color>]? [<percentage> | <number> | none]{2} [<hue> | none] [ / [<alpha-value> |
  // none] ]? )
  struct LCHFunction
  {
    using ColorType = LCHA<float>;
    using Canonical = LCHFunction;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = false;
    constexpr static bool AllowConversionTo8BitSRGB = false;
    constexpr static ColorFlags FlagsForRelative = {};
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"lch";

    using L = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using C = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using H = ColorComponent<css::Angle<>, css::Number<>, keywords::None>;

    constexpr static auto Components =
      std::make_tuple(L {.Symbol = ValueId::L, .Min = 0.0, .Max = 100.0, .PercentMultiplier = 1.0},
                      C {.Symbol = ValueId::C, .Min = 0.0, .PercentMultiplier = 150.0 / 100.0},
                      H {.Symbol = ValueId::H, .Type = ColorComponentType::Angle}, AlphaComponent);
  };

  // oklab() = oklab( [from <color>]? [<percentage> | <number> | none]{3} [ / [<alpha-value> | none] ]? )
  struct OKLabFunction
  {
    using ColorType = OKLab<float>;
    using Canonical = OKLabFunction;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = false;
    constexpr static bool AllowConversionTo8BitSRGB = false;
    constexpr static ColorFlags FlagsForRelative = {};
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"oklab";

    using L = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using A = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using B = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components =
      std::make_tuple(L {.Symbol = ValueId::L, .Min = 0.0, .Max = 1.0},
                      A {.Symbol = ValueId::A, .PercentMultiplier = 0.4 / 100.0},
                      B {.Symbol = ValueId::B, .PercentMultiplier = 0.4 / 100.0}, AlphaComponent);
  };

  // oklch() = oklch( [from <color>]? [<percentage> | <number> | none]{2} [<hue> | none] [ / [<alpha-value> |
  // none] ]? )
  struct OKLCHFunction
  {
    using ColorType = OKLCHA<float>;
    using Canonical = OKLCHFunction;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = false;
    constexpr static bool AllowConversionTo8BitSRGB = false;
    constexpr static ColorFlags FlagsForRelative = {};
    constexpr static ColorFlags FlagsForAbsolute = {};
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = false;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"oklch";

    using L = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using C = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using H = ColorComponent<css::Angle<>, css::Number<>, keywords::None>;

    constexpr static auto Components =
      std::make_tuple(L {.Symbol = ValueId::L, .Min = 0.0, .Max = 1.0},
                      C {.Symbol = ValueId::C, .Min = 0.0, .PercentMultiplier = 0.4 / 100.0},
                      H {.Symbol = ValueId::H, .Type = ColorComponentType::Angle}, AlphaComponent);
  };

  // color() = color( [from <color>]? <rgb-params> [ / [ <alpha-value> | none ] ]? )
  // <rgb-params> = <rgb-color-space> [ <number> | <percentage> | none ]{3}
  // <rgb-color-space> = srgb | srgb-linear | display-p3 | a98-rgb | prophoto-rgb | rec2020
  template <typename T>
  struct ColorRGBFunction
  {
    using ColorType = T;
    using Canonical = ColorRGBFunction<T>;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = false;
    constexpr static bool AllowConversionTo8BitSRGB = false;
    constexpr static ColorFlags FlagsForRelative = ColorFlags::UseColorFunctionSerialisation;
    constexpr static ColorFlags FlagsForAbsolute = ColorFlags::UseColorFunctionSerialisation;
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = true;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"color";

    using R = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using G = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using B = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components = std::make_tuple(R {.Symbol = ValueId::R}, G {.Symbol = ValueId::G},
                                                       B {.Symbol = ValueId::B}, AlphaComponent);
  };

  // color() = color( [from <color>]? <xyz-params> [ / [ <alpha-value> | none ] ]? )
  // <xyz-params> = <xyz-color-space> [ <number> | <percentage> | none ]{3}
  // <xyz-color-space> = xyz | xyz-d50 | xyz-d65
  template <typename T>
  struct ColorXYZFunction
  {
    using ColorType = T;
    using Canonical = ColorXYZFunction<T>;
    constexpr static bool AllowEagerEvaluationOfResolvableCalc = false;
    constexpr static bool AllowConversionTo8BitSRGB = false;
    constexpr static ColorFlags FlagsForRelative = ColorFlags::UseColorFunctionSerialisation;
    constexpr static ColorFlags FlagsForAbsolute = ColorFlags::UseColorFunctionSerialisation;
    constexpr static auto Syntax = ColorFunctionSyntax::Modern;
    constexpr static bool UsesColorFunctionForSerialisation = true;
    constexpr static CSSOMStringView SerialisationFunctionName = u8"color";

    using X = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using Y = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;
    using Z = ColorComponent<css::Percentage<>, css::Number<>, keywords::None>;

    constexpr static auto Components = std::make_tuple(X {.Symbol = ValueId::X}, Y {.Symbol = ValueId::Y},
                                                       Z {.Symbol = ValueId::Z}, AlphaComponent);
  };
}
