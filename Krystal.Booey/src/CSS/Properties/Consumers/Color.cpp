#include "Krystal.Booey/CSS/Properties/Consumers/Color.hpp"
#include "Krystal.Booey/CSS/Calc/AllowedSymbols.hpp"
#include "Krystal.Booey/CSS/Calc/SymbolTable.hpp"
#include "Krystal.Booey/CSS/Parser/InputStream.hpp"
#include "Krystal.Booey/CSS/Parser/ParserContext.hpp"
#include "Krystal.Booey/CSS/Parser/ParserFastPaths.hpp"
#include "Krystal.Booey/CSS/Parser/Tokenizer.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/KeywordDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/SymbolDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/Color/AbsoluteColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/AbsoluteColorResolver.hpp"
#include "Krystal.Booey/CSS/Values/Color/Color.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorDescriptors.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorLayers.hpp"
#include "Krystal.Booey/CSS/Values/Color/ColorMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/ContrastColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/Convert/Normalise.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimit.hpp"
#include "Krystal.Booey/CSS/Values/Color/DynamicRangeLimitMix.hpp"
#include "Krystal.Booey/CSS/Values/Color/HexColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/KeywordColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/LightDarkColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/PlatformColorResolutionState.hpp"
#include "Krystal.Booey/CSS/Values/Color/RelativeColor.hpp"
#include "Krystal.Booey/CSS/Values/Color/ResolvedColor.hpp"
#include "Krystal.Booey/CSS/Values/ColorValue.hpp"
#include "Krystal.Booey/CSS/Values/DynamicRangeLimitValue.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/NumericTypesEvaluateCalc.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/Color/ColorInterpolationMethod.hpp"
#include "Krystal.Booey/Graphics/GraphicsTypes.hpp"
#include <brigand/algorithms/wrap.hpp>
#include <string>

namespace krys::boo::css::PropertyParserHelpers
{
  namespace
  {
    // State passed to internal color consumer functions. Used to pass information
    // down the stack and levels of color parsing nesting.
    struct ColorParserState
    {
      css::PropertyParserState &PropertyParserState;
      ColorType AllowedColorTypes;
      bool AcceptsQuirkyColor {false};
      uint32 NestingLevel {0};

      ColorParserState(css::PropertyParserState &propertyParserState,
                       const ColorParsingOptions &options) noexcept
          : PropertyParserState {propertyParserState}, AllowedColorTypes {options.AllowedColorTypes},
            AcceptsQuirkyColor {propertyParserState.Context.Mode == ParserMode::HTMLQuirks
                                && Property::AcceptsQuirkyColor(propertyParserState.CurrentProperty)}
      {
      }
    };

    // RAII helper to increment/decrement nesting level.
    struct ColorParserStateNester
    {
      KRYS_NON_COPY_MOVABLE(ColorParserStateNester);

      ColorParserState &State;

      explicit ColorParserStateNester(ColorParserState &state) noexcept : State {state}
      {
        State.NestingLevel++;
      }

      ~ColorParserStateNester() noexcept
      {
        State.NestingLevel--;
      }
    };

    // Overload of the exposed root functions that take a `ColorParserState&`. Used to implement nesting level
    // tracking.
    KRYS_NODISCARD Maybe<Color> ConsumeColor(TokenRange &tokens, ColorParserState &state) noexcept;

    // MARK: - Generic component consumption

    // Conveniences that invokes a Consumer operator for the component at `Index`.

    template <typename... Ts>
    using MetaConsumerWrapper = MetaConsumer<Ts...>;

    template <typename Descriptor, size_t Index>
    KRYS_NODISCARD Maybe<AbsoluteColorComponent<Descriptor, Index>>
      ConsumeAbsoluteComponent(TokenRange &tokens, ColorParserState &state) noexcept
    {
      using TypeList = GetCSSColorParseTypeWithCalcComponentTypeList<Descriptor, Index>;
      using Consumer = brigand::wrap<TypeList, MetaConsumerWrapper>;

      return Consumer::Consume(tokens, state.PropertyParserState);
    }

    template <typename Descriptor, size_t Index>
    KRYS_NODISCARD Maybe<RelativeColorComponent<Descriptor, Index>>
      ConsumeRelativeComponent(TokenRange &tokens, ColorParserState &state,
                               AllowedSymbols symbolsAllowed) noexcept
    {
      // Append `Symbol` to the TypeList to allow unadorned symbols from the symbol
      // table to be consumed.
      using TypeList = GetCSSColorParseTypeWithCalcAndSymbolsComponentTypeList<Descriptor, Index>;
      using Consumer = brigand::wrap<TypeList, MetaConsumerWrapper>;

      return Consumer::Consume(tokens, state.PropertyParserState, krys::move(symbolsAllowed));
    }

    template <typename Descriptor>
    KRYS_NODISCARD bool ConsumeAlphaDelimiter(TokenRange &args) noexcept
    {
      if constexpr (Descriptor::Syntax == ColorFunctionSyntax::Legacy)
      {
        return ConsumeComma(args);
      }
      else
      {
        return ConsumeSlash(args);
      }
    }

    template <typename Descriptor>
    KRYS_NODISCARD AbsoluteColor<typename Descriptor::Canonical>
      NormaliseNonCalcComponents(const AbsoluteColor<Descriptor> &unresolved,
                                 ColorParserState &state) noexcept
    {
      krys_debug_assert(ComponentsContainsUnevaluatedCalc<Descriptor>(unresolved.Components));

      // The canonical descriptor is normally the descriptor itself, except for legacy rgb and hsl, which use
      // the modern counterparts.
      using CanonicalDescriptor = typename Descriptor::Canonical;

      if (state.NestingLevel > 1)
      {
        // If this is a nested color, we want to only normalize the numeric color channel components, leaving
        // them unclamped. The alpha channel can be both normalized and clamped.
        //
        // This behavior is described in section on the processing of relative colors:
        // https://drafts.csswg.org/css-color-5/#rcs-intro.
        return AbsoluteColor<CanonicalDescriptor> {CSSColorParseTypeWithCalc<CanonicalDescriptor> {
          NormaliseNumericComponentsIntoCanonicalRepresentation<Descriptor, 0>(
            std::get<0>(unresolved.Components)),
          NormaliseNumericComponentsIntoCanonicalRepresentation<Descriptor, 1>(
            std::get<1>(unresolved.Components)),
          NormaliseNumericComponentsIntoCanonicalRepresentation<Descriptor, 2>(
            std::get<2>(unresolved.Components)),
          NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, 3>(
            std::get<3>(unresolved.Components))}};
      }

      // For non-nested colors, we want to normalize and clamp the numeric color and alpha channel components.
      return AbsoluteColor<CanonicalDescriptor> {CSSColorParseTypeWithCalc<CanonicalDescriptor> {
        NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, 0>(
          std::get<0>(unresolved.Components)),
        NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, 1>(
          std::get<1>(unresolved.Components)),
        NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, 2>(
          std::get<2>(unresolved.Components)),
        NormaliseAndClampNumericComponentsIntoCanonicalRepresentation<Descriptor, 3>(
          std::get<3>(unresolved.Components))}};
    }

    template <typename Descriptor>
    KRYS_NODISCARD AbsoluteColor<typename Descriptor::Canonical>
      NormaliseNonCalcRequiringConversionDataComponents(const AbsoluteColor<Descriptor> &unresolved,
                                                        ColorParserState &state) noexcept
    {
      krys_debug_assert(ComponentsContainsUnevaluatedCalc<Descriptor>(unresolved.Components));

      // Evaluated any calc values that don't require conversion data.
      auto partiallyResolved = AbsoluteColor<Descriptor> {CSSColorParseTypeWithCalc<Descriptor> {
        EvaluateCalcIfNoConversionDataRequired(std::get<0>(unresolved.Components), SymbolTable {}),
        EvaluateCalcIfNoConversionDataRequired(std::get<1>(unresolved.Components), SymbolTable {}),
        EvaluateCalcIfNoConversionDataRequired(std::get<2>(unresolved.Components), SymbolTable {}),
        EvaluateCalcIfNoConversionDataRequired(std::get<3>(unresolved.Components), SymbolTable {})}};

      return NormaliseNonCalcComponents(partiallyResolved, state);
    }

    // Overload of `ConsumeAbsoluteFunctionParameters` for callers that already have the initial component
    // consumed.
    template <typename Descriptor>
    KRYS_NODISCARD Maybe<Color>
      ConsumeAbsoluteFunctionParameters(TokenRange &args, ColorParserState &state,
                                        AbsoluteColorComponent<Descriptor, 0> c1) noexcept
    {
      auto c2 = ConsumeAbsoluteComponent<Descriptor, 1>(args, state);
      if (!c2)
      {
        return null;
      }

      if constexpr (Descriptor::Syntax == ColorFunctionSyntax::Legacy)
      {
        if (!ConsumeComma(args))
        {
          return null;
        }
      }

      auto c3 = ConsumeAbsoluteComponent<Descriptor, 2>(args, state);
      if (!c3)
      {
        return null;
      }

      Maybe<AbsoluteColorComponent<Descriptor, 3>> alpha;
      if (ConsumeAlphaDelimiter<Descriptor>(args))
      {
        alpha = ConsumeAbsoluteComponent<Descriptor, 3>(args, state);
        if (!alpha)
        {
          return null;
        }
      }

      if (!args.IsAtEnd())
      {
        return null;
      }

      auto unresolved = AbsoluteColor<Descriptor> {
        .Components = {krys::move(c1), krys::move(*c2), krys::move(*c3), krys::move(alpha)},
      };

      if constexpr (Descriptor::AllowEagerEvaluationOfResolvableCalc)
      {
        // From CSS Color 4:
        //    "For historical reasons, when calc() in sRGB colors resolves to a single
        //     value, the declared value serializes without the calc() wrapper."
        //       - https://drafts.csswg.org/css-color-4/#resolving-sRGB-values
        //
        // calc() will "resolve to a single value" when no conversion data is required.

        // For this legacy / eager evaluating case, we want to preserve any calc() components that require
        // conversion data.
        if (ComponentsRequireConversionData<Descriptor>(unresolved.Components))
        {
          return Color {NormaliseNonCalcRequiringConversionDataComponents(unresolved, state)};
        }
      }
      else
      {
        // For the non-legacy / non-eager evaluating cases, we want preserve any calc(), not just calc()
        // requiring conversion data, so the check is a bit more permissive.
        if (ComponentsContainsUnevaluatedCalc<Descriptor>(unresolved.Components))
        {
          return Color {NormaliseNonCalcComponents(unresolved, state)};
        }
      }

      krys_debug_assert(!ComponentsRequireConversionData<Descriptor>(unresolved.Components));

      // In all other cases, we can fully resolve the color all the way to an absolute Color value.

      auto resolver = AbsoluteColorResolver<Descriptor> {.Components = unresolved.Components,
                                                         .NestingLevel = state.NestingLevel};

      auto resolved = ResolvedColor {ResolveNoConversionDataRequired(krys::move(resolver))};

      return Color {krys::move(resolved)};
    }

    template <typename Descriptor>
    KRYS_NODISCARD Maybe<Color> ConsumeAbsoluteFunctionParameters(TokenRange &args,
                                                                  ColorParserState &state) noexcept
    {
      auto c1 = ConsumeAbsoluteComponent<Descriptor, 0>(args, state);
      if (!c1)
      {
        return null;
      }

      if constexpr (Descriptor::Syntax == ColorFunctionSyntax::Legacy)
      {
        if (!ConsumeComma(args))
        {
          return null;
        }
      }

      return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state, krys::move(*c1));
    }

    template <typename Descriptor>
    KRYS_NODISCARD Maybe<Color> ConsumeRelativeFunctionParameters(TokenRange &args, ColorParserState &state,
                                                                  Color &&originColor) noexcept
    {
      const AllowedSymbols symbolsAllowed {{
        {std::get<0>(Descriptor::Components).Symbol, UnitType::Number},
        {std::get<1>(Descriptor::Components).Symbol, UnitType::Number},
        {std::get<2>(Descriptor::Components).Symbol, UnitType::Number},
        {std::get<3>(Descriptor::Components).Symbol, UnitType::Number},
      }};

      auto c1 = ConsumeRelativeComponent<Descriptor, 0>(args, state, symbolsAllowed);
      if (!c1)
      {
        return null;
      }
      auto c2 = ConsumeRelativeComponent<Descriptor, 1>(args, state, symbolsAllowed);
      if (!c2)
      {
        return null;
      }
      auto c3 = ConsumeRelativeComponent<Descriptor, 2>(args, state, symbolsAllowed);
      if (!c3)
      {
        return null;
      }

      Maybe<RelativeColorComponent<Descriptor, 3>> alpha;
      if (ConsumeSlash(args))
      {
        alpha = ConsumeRelativeComponent<Descriptor, 3>(args, state, symbolsAllowed);
        if (!alpha)
        {
          return null;
        }
      }

      if (!args.IsAtEnd())
      {
        return null;
      }

      auto unresolved = RelativeColor<Descriptor> {
        .Origin = krys::move(originColor),
        .Components = {krys::move(*c1), krys::move(*c2), krys::move(*c3), krys::move(alpha)},
      };

      return Color {krys::move(unresolved)};
    }

    template <typename Descriptor>
    KRYS_NODISCARD Maybe<Color> ConsumeRelativeFunctionParameters(TokenRange &args,
                                                                  ColorParserState &state) noexcept
    {
      krys_debug_assert(args.Peek().ValueId() == ValueId::From);
      DiscardIdent(args);

      auto originColor = ConsumeColor(args, state);
      if (!originColor)
      {
        return null;
      }

      return ConsumeRelativeFunctionParameters<Descriptor>(args, state, krys::move(*originColor));
    }

    // MARK: - Generic parameter parsing

    // MARK: - lch() / lab() / oklch() / oklab() / hwb()

    template <typename Descriptor>
    KRYS_NODISCARD Maybe<Color> ConsumeGenericFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      krys_debug_assert(
        tokens.Peek().FunctionId() == ValueId::Lch || tokens.Peek().FunctionId() == ValueId::Oklch
        || tokens.Peek().FunctionId() == ValueId::Lab || tokens.Peek().FunctionId() == ValueId::Oklab
        || tokens.Peek().FunctionId() == ValueId::Hwb);

      auto args = ConsumeFunction(tokens);

      if (args.Peek().ValueId() == ValueId::From)
      {
        return ConsumeRelativeFunctionParameters<Descriptor>(args, state);
      }
      return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state);
    }

    // MARK: - rgb() / rgba()

    KRYS_NODISCARD Maybe<Color> ConsumeRGBFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::Rgb
                        || tokens.Peek().FunctionId() == ValueId::Rgba);
      auto args = ConsumeFunction(tokens);

      if (args.Peek().ValueId() == ValueId::From)
      {
        using Descriptor = RGBFunctionModernRelative;

        return ConsumeRelativeFunctionParameters<Descriptor>(args, state);
      }

      // To determine whether this is going to use the modern or legacy syntax, we need to consume
      // the first component and the separated after it. If the separator is a `comma`, its using
      // the legacy syntax, if the separator is a space, it is using the modern syntax.
      //
      // We consume using the more accepting syntax, the modern one, and if it turns out that we
      // are actually parsing a legacy syntax function (by virtue of a `comma`), we explicitly
      // check the parsed parameter to see if it was the unsupported type, `none`, and reject
      // the whole function.

      using Descriptor = RGBFunctionModernAbsolute;

      auto red = ConsumeAbsoluteComponent<Descriptor, 0>(args, state);
      if (!red)
      {
        return null;
      }

      if (ConsumeComma(args))
      {
        // A `comma` getting successfully consumed means this is using the legacy syntax.

        return krys::SwitchOn(
          krys::move(*red),
          [&]<typename T>(T red) -> Maybe<Color>
          {
            using Descriptor = RGBFunctionLegacy<T>;

            return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state, krys::move(red));
          },
          [](keywords::None) -> Maybe<Color>
          {
            // `none` is invalid for the legacy syntax, but the initial parameter consumer didn't
            // know we were using the legacy syntax yet, so we need to check for it now.
            return null;
          });
      }
      else
      {
        // A `comma` NOT getting successfully consumed means this is using the modern syntax.
        return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state, krys::move(*red));
      }
    }

    // MARK: - hsl() / hsla()

    KRYS_NODISCARD Maybe<Color> ConsumeHSLFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::Hsl
                        || tokens.Peek().FunctionId() == ValueId::Hsla);
      auto args = ConsumeFunction(tokens);

      if (args.Peek().ValueId() == ValueId::From)
      {
        using Descriptor = HSLFunctionModern;

        return ConsumeRelativeFunctionParameters<Descriptor>(args, state);
      }

      // To determine whether this is going to use the modern or legacy syntax, we need to consume
      // the first component and the separated after it. If the separator is a `comma`, its using
      // the legacy syntax, if the separator is a space, it is using the modern syntax.
      //
      // We consume using the more accepting syntax, the modern one, and if it turns out that we
      // are actually parsing a legacy syntax function (by virtue of a `comma`), we explicitly
      // check the parsed parameter to see if it was the unsupported type, `none`, and reject
      // the whole function.

      using Descriptor = HSLFunctionModern;

      auto hue = ConsumeAbsoluteComponent<Descriptor, 0>(args, state);
      if (!hue)
      {
        return null;
      }

      if (ConsumeComma(args))
      {
        // A `comma` getting successfully consumed means this is using the legacy syntax.

        return krys::SwitchOn(
          krys::move(*hue),
          [&](auto hue) -> Maybe<Color>
          {
            using Descriptor = HSLFunctionLegacy;

            return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state, krys::move(hue));
          },
          [](keywords::None) -> Maybe<Color>
          {
            // `none` is invalid for the legacy syntax, but the initial parameter consumer didn't
            // know we were using the legacy syntax yet, so we need to check for it now.
            return null;
          });
      }
      else
      {
        // A `comma` NOT getting successfully consumed means this is using the modern syntax.
        return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state, krys::move(*hue));
      }
    }

    // MARK: - color()

    template <typename Functor>
    KRYS_NODISCARD auto CallWithColorFunction(ValueId id, Functor &&functor) noexcept
      -> decltype(functor.template operator()<ColorRGBFunction<ExtendedSRGBA<float>>>())
    {
      switch (id)
      {
        case ValueId::A98Rgb:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedA98RGB<float>>>();
        }
        case ValueId::DisplayP3:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedDisplayP3<float>>>();
        }
        case ValueId::DisplayP3Linear:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedLinearDisplayP3<float>>>();
        }
        case ValueId::ProphotoRgb:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedProPhotoRGB<float>>>();
        }
        case ValueId::Rec2020:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedRec2020<float>>>();
        }
        case ValueId::SRGB:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedSRGBA<float>>>();
        }
        case ValueId::SrgbLinear:
        {
          return functor.template operator()<ColorRGBFunction<ExtendedLinearSRGBA<float>>>();
        }
        case ValueId::XyzD50:
        {
          return functor.template operator()<ColorXYZFunction<XYZA<float, WhitePoint::D50>>>();
        }
        case ValueId::Xyz:
        case ValueId::XyzD65:
        {
          return functor.template operator()<ColorXYZFunction<XYZA<float, WhitePoint::D65>>>();
        }
        default:
        {
          return {};
        }
      }
    }

    template <typename Functor>
    KRYS_NODISCARD auto ConsumeColorSpace(TokenRange &args, Functor &&functor) noexcept
      -> decltype(functor.template operator()<ColorRGBFunction<ExtendedSRGBA<float>>>())
    {
      return CallWithColorFunction(args.Peek().ValueId(),
                                   [&]<typename Descriptor>()
                                   {
                                     DiscardIdent(args);

                                     return functor.template operator()<Descriptor>();
                                   });
    }

    KRYS_NODISCARD Maybe<Color> ConsumeColorFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::Color);
      auto args = ConsumeFunction(tokens);

      if (args.Peek().ValueId() == ValueId::From)
      {
        DiscardIdent(args);

        auto originColor = ConsumeColor(args, state);
        if (!originColor)
        {
          return null;
        }

        return ConsumeColorSpace(
          args, [&]<typename Descriptor>()
          { return ConsumeRelativeFunctionParameters<Descriptor>(args, state, krys::move(*originColor)); });
      }

      return ConsumeColorSpace(args, [&]<typename Descriptor>()
                               { return ConsumeAbsoluteFunctionParameters<Descriptor>(args, state); });
    }

    // MARK: - color-layers()

    KRYS_NODISCARD Maybe<Color> ConsumeColorLayersFunction(TokenRange &tokens,
                                                           ColorParserState &state) noexcept
    {
      // color-layers() = color-layers([ <blend-mode>, ]? <color># )

      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::ColorLayers);

      if (!state.PropertyParserState.Context.colorLayersEnabled)
      {
        return null;
      }

      auto args = ConsumeFunction(tokens);

      // FIXME: Parse blend mode.

      CommaSeparatedList<Color>::Container colors;
      do
      {
        auto color = ConsumeColor(args, state);
        if (!color)
        {
          return null;
        }

        colors.push_back(krys::move(*color));
      } while (ConsumeComma(args));

      if (!args.IsAtEnd())
      {
        return null;
      }

      return Color {ColorLayers {.BlendMode = BlendMode::Normal, .Colors = {krys::move(colors)}}};
    }

    // MARK: - color-mix()

    KRYS_NODISCARD Maybe<ColorMix::Component> ConsumeColorMixComponent(TokenRange &args,
                                                                       ColorParserState &state) noexcept
    {
      // [ <color> && <percentage [0,100]>? ]

      auto percentage =
        MetaConsumer<ColorMix::Component::PercentageType>::Consume(args, state.PropertyParserState);

      auto originColor = ConsumeColor(args, state);
      if (!originColor)
      {
        return null;
      }

      if (!percentage)
      {
        if (auto percent =
              MetaConsumer<ColorMix::Component::PercentageType>::Consume(args, state.PropertyParserState))
        {
          percentage = percent;
        }
      }

      return ColorMix::Component {.Color = krys::move(*originColor), .Percentage = krys::move(percentage)};
    }

    KRYS_NODISCARD bool HasNonCalculatedZeroPercentage(const ColorMix::Component &mixComponent) noexcept
    {
      if (auto percentage = mixComponent.Percentage)
      {
        return percentage->IsKnownZero();
      }
      return false;
    }

    KRYS_NODISCARD Maybe<Color> ConsumeColorMixFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      // color-mix() = color-mix( <color-interpolation-method> , [ <color> && <percentage [0,100]>? ]#{2})
      // https://drafts.csswg.org/css-color-5/#color-mix

      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::ColorMix);

      auto args = ConsumeFunction(tokens);

      Maybe<ColorInterpolationMethod> colorInterpolationMethod = DefaultInterpolationMethodForColorMix;
      if (args.Peek().ValueId() == ValueId::In)
      {
        colorInterpolationMethod = ConsumeColorInterpolationMethod(args, state.PropertyParserState);
        if (!colorInterpolationMethod)
        {
          return null;
        }

        if (!ConsumeComma(args))
        {
          return null;
        }
      }

      auto mixComponent1 = ConsumeColorMixComponent(args, state);
      if (!mixComponent1)
      {
        return null;
      }

      if (!ConsumeComma(args))
      {
        return null;
      }

      auto mixComponent2 = ConsumeColorMixComponent(args, state);
      if (!mixComponent2)
      {
        return null;
      }

      if (!args.IsAtEnd())
      {
        return null;
      }

      if (HasNonCalculatedZeroPercentage(*mixComponent1) && HasNonCalculatedZeroPercentage(*mixComponent2))
      {
        // This eagerly marks the parse as invalid if both percentage components are non-calc
        // and equal to 0. This satisfies step 5 of section 2.1. Percentage Normalization in
        // https://w3c.github.io/csswg-drafts/css-color-5/#color-mix which states:
        //    "If the percentages sum to zero, the function is invalid."
        //
        // The only way the percentages can sum to zero is both are zero, since we reject any
        // percentages less than zero, and missing percentages are treated as "100% - p(other)".
        //
        // FIXME: Should we also do this for calculated values? Or should we let the parse be
        // valid and fail to produce a valid color at style building time.
        return null;
      }

      return Color {ColorMix {
        .ColorInterpolationMethod = krys::move(*colorInterpolationMethod),
        .MixComponents1 = krys::move(*mixComponent1),
        .MixComponents2 = krys::move(*mixComponent2),
      }};
    }

    // MARK: - contrast-color()

    KRYS_NODISCARD Maybe<Color> ConsumeContrastColorFunction(TokenRange &tokens,
                                                             ColorParserState &state) noexcept
    {
      // contrast-color() = contrast-color( <color> )
      // https://drafts.csswg.org/css-color-5/#funcdef-contrast-color

      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::ContrastColor);

      if (!state.PropertyParserState.Context.contrastColorEnabled)
      {
        return null;
      }

      auto args = ConsumeFunction(tokens);

      auto color = ConsumeColor(args, state);
      if (!color)
      {
        return null;
      }

      if (!args.IsAtEnd())
      {
        return null;
      }

      return Color {ContrastColor {.Color = krys::move(*color)}};
    }

    // MARK: - light-dark()

    KRYS_NODISCARD Maybe<Color> ConsumeLightDarkFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      // light-dark() = light-dark( <color>, <color> )
      // https://drafts.csswg.org/css-color-5/#light-dark

      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::LightDark);

      auto args = ConsumeFunction(tokens);

      auto lightColor = ConsumeColor(args, state);
      if (!lightColor)
      {
        return null;
      }

      if (!ConsumeComma(args))
      {
        return null;
      }

      auto darkColor = ConsumeColor(args, state);
      if (!darkColor)
      {
        return null;
      }

      if (!args.IsAtEnd())
      {
        return null;
      }

      return Color {
        LightDarkColor {.LightColor = krys::move(*lightColor), .DarkColor = krys::move(*darkColor)}};
    }

    // MARK: - Color function dispatch

    // NOTE: This is named "Consume*A*ColorFunction" to differentiate if from the
    // the function that consumes a `color()` explicitly.
    KRYS_NODISCARD Maybe<Color> ConsumeAColorFunction(TokenRange &tokens, ColorParserState &state) noexcept
    {
      auto colorRange = tokens;
      auto functionId = tokens.Peek().FunctionId();
      Maybe<Color> color;
      switch (functionId)
      {
        case ValueId::Rgb:
        case ValueId::Rgba:
        {
          color = ConsumeRGBFunction(colorRange, state);
          break;
        }
        case ValueId::Hsl:
        case ValueId::Hsla:
        {
          color = ConsumeHSLFunction(colorRange, state);
          break;
        }
        case ValueId::Hwb:
        {
          color = ConsumeGenericFunction<HWBFunction>(colorRange, state);
          break;
        }
        case ValueId::Lab:
        {
          color = ConsumeGenericFunction<LabFunction>(colorRange, state);
          break;
        }
        case ValueId::Lch:
        {
          color = ConsumeGenericFunction<LCHFunction>(colorRange, state);
          break;
        }
        case ValueId::Oklab:
        {
          color = ConsumeGenericFunction<OKLabFunction>(colorRange, state);
          break;
        }
        case ValueId::Oklch:
        {
          color = ConsumeGenericFunction<OKLCHFunction>(colorRange, state);
          break;
        }
        case ValueId::Color:
        {
          color = ConsumeColorFunction(colorRange, state);
          break;
        }
        case ValueId::ColorLayers:
        {
          color = ConsumeColorLayersFunction(colorRange, state);
          break;
        }
        case ValueId::ColorMix:
        {
          color = ConsumeColorMixFunction(colorRange, state);
          break;
        }
        case ValueId::ContrastColor:
        {
          color = ConsumeContrastColorFunction(colorRange, state);
          break;
        }
        case ValueId::LightDark:
        {
          color = ConsumeLightDarkFunction(colorRange, state);
          break;
        }
        default:
        {
          return null;
        }
      }

      if (color)
      {
        tokens = colorRange;
      }
      return color;
    }

    // MARK: - Hex

    KRYS_NODISCARD Maybe<SRGBA<uint8>> ConsumeHexColor(TokenRange &tokens, ColorParserState &state) noexcept
    {
      CSSOMString string;
      CSSOMStringView view;
      auto &token = tokens.Peek();
      if (token.Type() == TokenType::Hash)
      {
        view = token.IdentCodePoints();
      }
      else
      {
        // <quirky-color> should only match for properties that explicitly opt-in and when not nested.
        // https://drafts.csswg.org/css-color-4/#quirky-color
        if (!state.AcceptsQuirkyColor || state.NestingLevel > 1)
        {
          return null;
        }

        if (token.Type() == TokenType::Ident)
        {
          view = token.IdentCodePoints(); // e.g. FF0000
          if (view.size() != 3 && view.size() != 6)
          {
            return null;
          }
        }
        else if (token.Type() == TokenType::Number || token.Type() == TokenType::Dimension)
        {
          if (token.NumericValueType() != NumericValueType::Integer)
          {
            return null;
          }

          auto numericValue = token.NumericValue();
          if (!(numericValue >= 0 && numericValue < 1'000'000))
          {
            return null;
          }

          auto integerValue = std::to_string(static_cast<int>(numericValue));
          string.assign(integerValue.begin(), integerValue.end()); // e.g. 112233
          if (token.Type() == TokenType::Dimension)
          {
            string.append(token.Unit()); // e.g. 0001FF
          }

          if (string.size() < 6)
          {
            string.insert(0, 6 - string.size(), u8'0');
          }

          if (string.size() != 3 && string.size() != 6)
          {
            return null;
          }

          view = string;
        }
        else
        {
          return null;
        }
      }

      auto result = ParserFastPaths::ParseHexColor(view);
      if (!result)
      {
        return null;
      }

      tokens.Discard();
      tokens.DiscardWhitespace();
      return *result;
    }

    // MARK: Color consumer bottleneck

    Maybe<Color> ConsumeColor(TokenRange &tokens, ColorParserState &state) noexcept
    {
      ColorParserStateNester nester {state};

      auto keyword = tokens.Peek().ValueId();
      if (IsColorKeyword(keyword, state.AllowedColorTypes))
      {
        if (!IsColorKeywordAllowed(keyword, state.PropertyParserState.Context))
        {
          return null;
        }

        DiscardIdent(tokens);
        return Color {KeywordColor {keyword}};
      }

      if (auto hexColor = ConsumeHexColor(tokens, state))
      {
        return Color {HexColor {*hexColor}};
      }

      return ConsumeAColorFunction(tokens, state);
    }

    // MARK: - <dynamic-range-limit-mix()> (unresolved)

    KRYS_NODISCARD Maybe<DynamicRangeLimitMixComponent>
      ConsumeUnresolvedDynamicRangeLimitMixComponent(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      // <dynamic-range-limit-mix-component> = <'dynamic-range-limit'> && <percentage [0,100]>

      auto rangeCopy = tokens;

      auto percentage = MetaConsumer<DynamicRangeLimitMixPercentage>::Consume(rangeCopy, state);
      auto limit = ConsumeUnresolvedDynamicRangeLimit(rangeCopy, state);
      if (!limit)
      {
        return null;
      }

      if (!percentage)
      {
        percentage = MetaConsumer<DynamicRangeLimitMixPercentage>::Consume(rangeCopy, state);
        if (!percentage)
        {
          return null;
        }
      }

      tokens = rangeCopy;

      return DynamicRangeLimitMixComponent {krys::move(*limit), krys::move(*percentage)};
    }

    KRYS_NODISCARD Maybe<DynamicRangeLimit>
      ConsumeUnresolvedDynamicRangeLimitMix(TokenRange &tokens, PropertyParserState &state) noexcept
    {
      // dynamic-range-limit-mix() = dynamic-range-limit-mix( [ <'dynamic-range-limit'> && <percentage
      // [0,100]>
      // ]#{2,} )

      krys_debug_assert(tokens.Peek().FunctionId() == ValueId::DynamicRangeLimitMix);

      auto rangeCopy = tokens;
      auto args = ConsumeFunction(rangeCopy);

      DynamicRangeLimitMixParameters::Container resultBuilder;

      do
      {
        auto component = ConsumeUnresolvedDynamicRangeLimitMixComponent(args, state);
        if (!component)
        {
          return null;
        }
        resultBuilder.push_back(krys::move(*component));
      } while (ConsumeComma(args));

      if (!args.IsAtEnd())
      {
        return null;
      }

      if (resultBuilder.size() < 2)
      {
        return null;
      }

      tokens = rangeCopy;
      return DynamicRangeLimit {DynamicRangeLimitMixFunction {
        DynamicRangeLimitMixFunctionValue {DynamicRangeLimitMixParameters {krys::move(resultBuilder)}}}};
    }
  }

  bool IsColorKeywordAllowed(ValueId id, const ParserContext &context) noexcept
  {
    switch (id)
    {
      case ValueId::WebkitFocusRingColor:
      {
        switch (context.Mode)
        {
          case ParserMode::UASheet:
          case ParserMode::HTMLQuirks:
          {
            return true;
          }
          default:
          {
            return false;
          }
        }
      }
      case ValueId::InternalDocumentTextColor:
      {
        switch (context.Mode)
        {
          case ParserMode::UASheet:
          {
            return true;
          }
          default:
          {
            return false;
          }
        }
      }
      default:
      {
        return true;
      }
    }
  }

  // MARK: - Color consuming entry points

  Maybe<Color> ConsumeUnresolvedColor(TokenRange &tokens, PropertyParserState &propertyParserState,
                                      const ColorParsingOptions &options) noexcept
  {
    ColorParserState state {propertyParserState, options};

    return ConsumeColor(tokens, state);
  }

  // MARK: - Value consuming entry points

  RefPtr<Value> ConsumeColor(TokenRange &tokens, PropertyParserState &state,
                             const ColorParsingOptions &options) noexcept
  {
    if (auto color = ConsumeUnresolvedColor(tokens, state, options))
    {
      // Special case top level color keywords, hex (SRGBA<uint8>) and absolute color values to maintain
      // backwards compatibility and existing optimizations.
      if (auto keyword = color->Keyword())
      {
        return PrimitiveValue::Create(keyword->Value);
      }
      if (auto hex = color->Hex())
      {
        return state.Pool.CreateColorValue(boo::Color {hex->Value});
      }
      if (auto resolved = color->Resolved())
      {
        return state.Pool.CreateColorValue(krys::move(resolved->Value));
      }

      return ColorValue::Create(krys::move(*color));
    }

    return nullptr;
  }

  // MARK: - Raw consuming entry points

  boo::Color ConsumeColorRaw(TokenRange &tokens, PropertyParserState &propertyParserState,
                             const ColorParsingOptions &options,
                             PlatformColorResolutionState &eagerResolutionState) noexcept
  {
    ColorParserState state {propertyParserState, options};

    if (auto color = ConsumeColor(tokens, state))
    {
      return CreateColor(*color, eagerResolutionState);
    }
    return {};
  }

  // MARK: - Raw parsing entry points

  boo::Color ParseColorRawGeneral(CSSOMStringView string, const ParserContext &context,
                                  const ColorParsingOptions &options,
                                  PlatformColorResolutionState &eagerResolutionState) noexcept
  {
    InputStream input {CSSOMString(string)};
    Tokenizer tokenizer {input};
    tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = PropertyParserState {.Context = context};
    auto result = ConsumeUnresolvedColor(tokens, state, options);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd() || !result)
    {
      return {};
    }

    return CreateColor(*result, eagerResolutionState);
  }

  boo::Color DeprecatedParseColorRawWithoutContext(CSSOMStringView string,
                                                   const ColorParsingOptions &options) noexcept
  {
    const auto &context = StrictParserContext();
    if (auto color = ParserFastPaths::ParseSimpleColor(string, context))
    {
      return *color;
    }

    InputStream input {CSSOMString(string)};
    Tokenizer tokenizer {input};
    tokenizer.PumpTokenizer();
    auto tokens = tokenizer.Tokens();

    // Handle leading whitespace.
    tokens.DiscardWhitespace();

    auto state = PropertyParserState {.Context = context};
    auto result = ConsumeUnresolvedColor(tokens, state, options);

    // Handle trailing whitespace.
    tokens.DiscardWhitespace();

    if (!tokens.IsAtEnd() || !result)
    {
      return {};
    }

    return result->GetAbsoluteColor();
  }

  // MARK: - <'dynamic-range-limit'> (unresolved)

  Maybe<DynamicRangeLimit> ConsumeUnresolvedDynamicRangeLimit(TokenRange &tokens,
                                                              PropertyParserState &state) noexcept
  {
    // <'dynamic-range-limit'> = standard | high | constrained | <dynamic-range-limit-mix()>
    // https://drafts.csswg.org/css-color-hdr/#propdef-dynamic-range-limit

    switch (tokens.Peek().ValueId())
    {
      case ValueId::Standard:
      {
        DiscardIdent(tokens);
        return DynamicRangeLimit {keywords::Standard {}};
      }
      case ValueId::Constrained:
      {
        if (!state.Context.cssConstrainedDynamicRangeLimitEnabled)
        {
          return null;
        }
        DiscardIdent(tokens);
        return DynamicRangeLimit {keywords::Constrained {}};
      }
      case ValueId::NoLimit:
      {
        DiscardIdent(tokens);
        return DynamicRangeLimit {keywords::NoLimit {}};
      }
      default:
      {
        break;
      }
    }

    if (!state.Context.cssDynamicRangeLimitMixEnabled)
    {
      return null;
    }

    if (tokens.Peek().FunctionId() == ValueId::DynamicRangeLimitMix)
    {
      if (auto mix = ConsumeUnresolvedDynamicRangeLimitMix(tokens, state))
      {
        return mix;
      }
    }

    return null;
  }

  // MARK: - <'dynamic-range-limit'> (Value)

  RefPtr<Value> ConsumeDynamicRangeLimit(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    auto dynamicRangeLimit = ConsumeUnresolvedDynamicRangeLimit(tokens, state);
    if (!dynamicRangeLimit)
    {
      return {};
    }
    return DynamicRangeLimitValue::Create(krys::move(*dynamicRangeLimit));
  }
}
