#pragma once

#include "Krystal.Lib/Color/ColorTypes.hpp"

namespace Krys
{
  enum class ColorSpace : uint8;

  template <typename Output, typename Input, typename = void>
  struct ColorConversion;

  // All color types, other than XYZA or those inheriting from RGBType, must implement
  // the following conversions to and from their "Reference" color.
  //
  //  template<>
  //  struct ColorConversion<`ColorType`<float>::Reference, `ColorType`<float>>
  //  {
  //    KRYS_NODISCARD static `ColorType`<float>::Reference Convert(const `ColorType`<float>&) noexcept;
  //  };
  //
  //  template<>
  //  struct ColorConversion<`ColorType`<float>, `ColorType`<float>::Reference>
  //  {
  //    KRYS_NODISCARD static `ColorType`<float> Convert(const `ColorType`<float>::Reference&) noexcept;
  //  };

  /// @brief Conversion function for typed colors.
  template <typename Output, typename Input>
  KRYS_NODISCARD Output ConvertColor(const Input &color) noexcept
  {
    return ColorConversion<CanonicalColorType<Output>, CanonicalColorType<Input>>::Convert(color);
  }

  /// @brief Utility to update appropriate component in `output` if an analogous component
  /// in `input` is missing (which is encoded as NaN in color types).
  template <typename Output, typename Input, size_t IndexInInput>
  constexpr void TryToCarryForwardComponentIfMissing(const ColorComponents<float, 4> &input,
                                                     ColorComponents<float, 4> &output) noexcept
  {
    constexpr auto analogousComponentIndexInOutput = AnalogousComponentIndex<Output, Input, IndexInInput>();
    if constexpr (analogousComponentIndexInOutput)
    {
      if (IsNaN(input[IndexInInput]))
      {
        output[*analogousComponentIndexInOutput] = Limits<float>::quiet_NaN();
      }
    }
  }

  /// @brief Conversion function for typed colors that carries forward missing component values
  /// from analogous components in the input type.
  /// Performs color space conversion followed by carrying forward missing components
  /// from `Input` for analogous components in `Output` as described by CSS Color 4
  /// @see § 12. Color Interpolation, https://drafts.csswg.org/css-color-4/#interpolation.
  template <typename Output, typename Input>
  KRYS_NODISCARD Output ConvertColorCarryingForwardMissing(const Input &color) noexcept
  {
    if constexpr (SameType<Input, Output>)
    {
      return ConvertColor<Output>(color);
    }
    else if constexpr (SameType<typename Input::ComponentType, uint8>
                       || SameType<typename Output::ComponentType, uint8>)
    {
      return ConvertColor<Output>(color);
    }
    else
    {
      auto input = AsColorComponents(color.Unresolved());
      auto output = AsColorComponents(ConvertColor<Output>(color).Unresolved());

      TryToCarryForwardComponentIfMissing<Output, Input, 0>(input, output);
      TryToCarryForwardComponentIfMissing<Output, Input, 1>(input, output);
      TryToCarryForwardComponentIfMissing<Output, Input, 2>(input, output);
      TryToCarryForwardComponentIfMissing<Output, Input, 3>(input, output);

      return CreateFromComponents<Output>(output);
    }
  }

  // Conversion functions for raw color components with associated color spaces.
  KRYS_NODISCARD ColorComponents<float, 4>
    ConvertAndResolveColorComponents(ColorSpace inputColorSpace,
                                     ColorComponents<float, 4> inputColorComponents,
                                     ColorSpace outputColorSpace) noexcept;

  /// Looks for a an analogous component in `Output` of `Input[IndexInInput]`.
  /// For example, take the following:
  /// auto result = AnalogousComponentIndex<LCHA<float>, HSLA<float>, 0>;
  ///
  /// This returns "2", because the input component specified (the hue in HSLA)
  /// is analogous to component "2" in the output (the hue in LCHA).
  /// If there is no analogous component, `Null` is returned.
  template <typename Output, typename Input, unsigned IndexInInput>
  KRYS_NODISCARD constexpr Maybe<size_t> AnalogousComponentIndex() noexcept
  {
    if constexpr (IndexInInput == 3)
    {
      return 3; // Special case alpha, it always should carry forward.
    }
    else
    {
      constexpr auto inputCategory = Input::Model::componentInfo[IndexInInput].category;
      if constexpr (!inputCategory)
      {
        return Null;
      }
      else if constexpr (*inputCategory == Output::Model::componentInfo[0].category)
      {
        return 0;
      }
      else if constexpr (*inputCategory == Output::Model::componentInfo[1].category)
      {
        return 1;
      }
      else if constexpr (*inputCategory == Output::Model::componentInfo[2].category)
      {
        return 2;
      }
      else
      {
        return Null;
      }
    }
  }

  // MARK: White Point.

  constexpr Array<float, 3> D50WhitePoint {0.3457 / 0.3585, 1.0, (1.0 - 0.3457 - 0.3585) / 0.3585};
  constexpr Array<float, 3> D65WhitePoint {0.3127 / 0.3290, 1.0, (1.0 - 0.3127 - 0.3290) / 0.3290};

  // MARK: Chromatic Adaptation conversions.

  template <WhitePoint From, WhitePoint To>
  struct ChromaticAdaptation;

  // Chromatic Adaptation allows conversion from one white point to another.
  //
  // The values we use are pre-calculated for the two white points we support, D50
  // and D65 using the Bradford method's chromatic adaptation transform (CAT), but
  // can be extended to any pair of white points.
  //
  // The process to compute new ones is:
  //
  //  1. Choose a CAT and lookup its values (these are not derivable).
  //     We currently use the Bradford CAT
  //
  //     let toCone =
  //         [  0.8951000,  0.2664000, -0.1614000 ],
  //         [ -0.7502000,  1.7135000,  0.0367000 ],
  //         [  0.0389000, -0.0685000,  1.0296000 ]
  //
  //  2. In addition, you will need the inverse
  //
  //     let fromCone = toCone ^ -1
  //
  //  3. Choose source and destination XYZ white points
  //
  //     let whitePoint_src = [ ... , ... , ... ]
  //     let whitePoint_dst = [ ... , ... , ... ]
  //
  //  4. Convert the white points into the cone response domain (denoted ρ, γ, β)
  //
  //     let [ ρ_src, γ_src, β_src ] = toCone * whitePoint_src
  //     let [ ρ_dst, γ_dst, β_dst ] = toCone * whitePoint_dst
  //
  //  5. Compute a scale transform
  //
  //     let scale =
  //         [ ρ_dst / ρ_src, 0,             0             ],
  //         [ 0,             γ_dst / γ_src, 0             ],
  //         [ 0,             0,             β_dst / β_src ]
  //
  //  6. Finally, use the scale to compute the adaptation transform
  //     (what is stored ChromaticAdaptation.matrix) as the concatenation
  //     of the toCone, scale and fromCone transforms.
  //
  //     let adaptation = fromCone * scale * toCone
  //
  // Additional details and more CATs / white point values can be found at:
  // http://www.brucelindbloom.com/index.html?Eqn_ChromAdapt.html

  template <>
  struct ChromaticAdaptation<WhitePoint::D65, WhitePoint::D50>
  {
    constexpr static ColorMatrix<3, 3> matrix {
      1.0479297925449969,    0.022946870601609652, -0.05019226628920524,
      0.02962780877005599,   0.9904344267538799,   -0.017073799063418826,
      -0.009243040646204504, 0.015055191490298152, 0.7518742814281371};
  };

  template <>
  struct ChromaticAdaptation<WhitePoint::D50, WhitePoint::D65>
  {
    constexpr static ColorMatrix<3, 3> matrix {
      0.955473421488075,    -0.02309845494876471,  0.06325924320057072,
      -0.0283697093338637,  1.0099953980813041,    0.021041441191917323,
      0.012314014864481998, -0.020507649298898964, 1.330365926242124};
  };

  // MARK: HSLA
  template <>
  struct ColorConversion<ExtendedSRGBA<float>, HSLA<float>>
  {
    KRYS_NODISCARD static ExtendedSRGBA<float> Convert(const HSLA<float> &) noexcept;
  };

  template <>
  struct ColorConversion<HSLA<float>, ExtendedSRGBA<float>>
  {
    KRYS_NODISCARD static HSLA<float> Convert(const ExtendedSRGBA<float> &) noexcept;
  };

  // MARK: HWBA
  template <>
  struct ColorConversion<ExtendedSRGBA<float>, HWBA<float>>
  {
    KRYS_NODISCARD static ExtendedSRGBA<float> Convert(const HWBA<float> &) noexcept;
  };

  template <>
  struct ColorConversion<HWBA<float>, ExtendedSRGBA<float>>
  {
    KRYS_NODISCARD static HWBA<float> Convert(const ExtendedSRGBA<float> &) noexcept;
  };

  // MARK: LCHA
  template <>
  struct ColorConversion<Lab<float>, LCHA<float>>
  {
    KRYS_NODISCARD static Lab<float> Convert(const LCHA<float> &) noexcept;
  };

  template <>
  struct ColorConversion<LCHA<float>, Lab<float>>
  {
    KRYS_NODISCARD static LCHA<float> Convert(const Lab<float> &) noexcept;
  };

  // MARK: Lab
  template <>
  struct ColorConversion<XYZA<float, WhitePoint::D50>, Lab<float>>
  {
    KRYS_NODISCARD static XYZA<float, WhitePoint::D50> Convert(const Lab<float> &) noexcept;
  };

  template <>
  struct ColorConversion<Lab<float>, XYZA<float, WhitePoint::D50>>
  {
    KRYS_NODISCARD static Lab<float> Convert(const XYZA<float, WhitePoint::D50> &) noexcept;
  };

  // MARK: OKLCHA
  template <>
  struct ColorConversion<OKLab<float>, OKLCHA<float>>
  {
    KRYS_NODISCARD static OKLab<float> Convert(const OKLCHA<float> &) noexcept;
  };

  template <>
  struct ColorConversion<OKLCHA<float>, OKLab<float>>
  {
    KRYS_NODISCARD static OKLCHA<float> Convert(const OKLab<float> &) noexcept;
  };

  // MARK: OKLab
  template <>
  struct ColorConversion<XYZA<float, WhitePoint::D65>, OKLab<float>>
  {
    KRYS_NODISCARD static XYZA<float, WhitePoint::D65> Convert(const OKLab<float> &) noexcept;
  };

  template <>
  struct ColorConversion<OKLab<float>, XYZA<float, WhitePoint::D65>>
  {
    KRYS_NODISCARD static OKLab<float> Convert(const XYZA<float, WhitePoint::D65> &) noexcept;
  };

  // Identity conversion.

  template <typename ColorType>
  struct ColorConversion<ColorType, ColorType>
  {
    KRYS_NODISCARD static ColorType Convert(const ColorType &color) noexcept
    {
      return color;
    }
  };

  // MARK: DeltaE color difference algorithms.

  template <typename ColorType1, typename ColorType2>
  KRYS_NODISCARD constexpr float ComputeDeltaEOK(ColorType1 color1, ColorType2 color2) noexcept
  {
    // https://drafts.csswg.org/css-color/#color-difference-OK

    auto [L1, a1, b1, alpha1] = ConvertColor<OKLab<float>>(color1).Resolved();
    auto [L2, a2, b2, alpha2] = ConvertColor<OKLab<float>>(color2).Resolved();

    auto deltaL = (L1 / 100.0f) - (L2 / 100.0f);
    auto deltaA = a1 - a2;
    auto deltaB = b1 - b2;

    return std::hypot(deltaL, deltaA, deltaB);
  }

  // MARK: Gamut mapping algorithms.

  struct ClipGamutMapping
  {
    template <typename ColorType>
    KRYS_NODISCARD static auto MapToBoundedGamut(const ColorType &color) noexcept ->
      typename ColorType::BoundedCounterpart
    {
      return ClipToGamut<typename ColorType::BoundedCounterpart>(color);
    }
  };

  // Main conversion.

  // ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
  // ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
  //  Matrix Conversions    ┌───────────┐│┌───────────┐
  // │                      │ XYZ (D50) │││ XYZ (D65) │ │
  //                        └─────▲─────┘│└─────▲─────┘
  // │                            │      │      │ │
  // ┌─────────────────────────┬───────────┘      │
  // └───────────┬───────────────────────────────┬───────────────────────────────┬───────────────────────────────┬─────────────────────────┐
  // │        │                │                  │                  │                               │ │ │ │ │
  // │                         │                  │                  │                               │ │ │ │
  // │        │                │                  │                  │                               │ │ │ │ │
  // │          ProPhotoRGB───────────────────┐   │   SRGB──────────────────────────┐
  // DisplayP3─────────────────────┐ A98RGB────────────────────────┐ Rec2020───────────────────────┐ │ │ │ │
  // ┌────────────────┐│   │   │           ┌────────────────┐│ │           ┌────────────────┐│ │
  // ┌────────────────┐│ │           ┌────────────────┐│ │        │ │          │     ┌─────▶︎ LinearExtended ││
  // │   │     ┌─────▶︎ LinearExtended ││ │     ┌─────▶︎ LinearExtended ││ │     ┌─────▶︎ LinearExtended ││ │
  // ┌─────▶︎ LinearExtended ││          │ │        │ │     │     └────────▲───────┘│   │   │     │
  // └────────▲───────┘│ │     │     └────────▲───────┘│ │     │     └────────▲───────┘│ │     │
  // └────────▲───────┘│ │        │ │         ─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─ ─│─ ─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─
  // ─ ─ ─│─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─ │
  // │          │┌────────┐          │        │   │   │┌────────┐          │        │ │┌────────┐          │
  // │ │┌────────┐          │        │ │┌────────┐          │        │          │ │          ││ Linear │ │ │
  // │   ││ Linear │          │        │ ││ Linear │          │        │ ││ Linear │          │        │ ││
  // Linear │          │        │          │ │          │└────▲───┘          │        │   │   │└────▲───┘ │ │
  // │└────▲───┘          │        │ │└────▲───┘          │        │ │└────▲───┘          │        │ │ │ │ │
  // │        │   │   │     │              │        │ │     │              │        │ │     │              │
  // │ │     │              │        │          │
  // ┌───────────┐    │┌────────┐ ┌────────────────┐│   │   │┌────────┐ ┌────────────────┐│ │┌────────┐
  // ┌────────────────┐│ │┌────────┐ ┌────────────────┐│ │┌────────┐ ┌────────────────┐│    ┌───────────┐ │
  // Lab    │    ││ Gamma  │─│ GammaExtended  ││   │   ││ Gamma  │─│ GammaExtended  ││ ││ Gamma  │─│
  // GammaExtended  ││ ││ Gamma  │─│ GammaExtended  ││ ││ Gamma  │─│ GammaExtended  ││    │   OKLab   │
  // └─────▲─────┘    │└────────┘ └────────────────┘│   │   │└────▲───┘ └────────────────┘│ │└────────┘
  // └────────────────┘│ │└────────┘ └────────────────┘│ │└────────┘ └────────────────┘│    └─────▲─────┘
  // │          └─────────────────────────────┘   │   └─────┼───────────────────────┘
  // └─────────────────────────────┘ └─────────────────────────────┘ └─────────────────────────────┘ │ │ │
  // ┌──┴──────────┐ │ │                                            │      │             │ │
  // ┌───────────┐                                      │┌───────────┐ ┌───────────┐ ┌───────────┐ │    LCH │
  // ││    HSL    │ │    HWB    │ │   OKLCH   │ └───────────┘ │└───────────┘ └───────────┘ └───────────┘

  template <typename Output, typename Input, typename>
  struct ColorConversion
  {
  public:
    KRYS_NODISCARD constexpr static Output Convert(const Input &color) noexcept
    {
      // 1. Handle the special case of Input or Output with a uint8 component type.
      if constexpr (SameType<typename Input::ComponentType, uint8>)
      {
        return HandleToFloatConversion(color);
      }
      else if constexpr (SameType<typename Output::ComponentType, uint8>)
      {
        return HandleToByteConversion(color);
      }
      // 2. Handle all color types that are not IsRGBType<T> or IsXYZA<T> for Input and Output. For all
      //    these other color types, we can unconditionally Convert them to their "reference" color, as
      //    either they have already been handled by a ColorConversion specialization or this will
      //    get us closer to the final conversion.
      else if constexpr (!IsRGBType<Input> && !IsXYZA<Input>)
      {
        return ConvertColor<Output>(ConvertColor<typename Input::Reference>(color));
      }
      else if constexpr (!IsRGBType<Output> && !IsXYZA<Output>)
      {
        return ConvertColor<Output>(ConvertColor<typename Output::Reference>(color));
      }
      // 3. Handle conversions within a RGBFamily (e.g. all have the same descriptor).
      else if constexpr (IsSameRGBTypeFamily<Output, Input>)
      {
        return HandleRGBFamilyConversion(color);
      }
      // 4. Handle any gamma conversions for the Input and Output.
      else if constexpr (IsRGBGammaEncodedType<Input>)
      {
        return ConvertColor<Output>(ConvertColor<typename Input::LinearCounterpart>(color));
      }
      else if constexpr (IsRGBGammaEncodedType<Output>)
      {
        return ConvertColor<Output>(ConvertColor<typename Output::LinearCounterpart>(color));
      }
      // 5. Handle any bounds conversions for the Input and Output.
      else if constexpr (IsRGBBoundedType<Input>)
      {
        return ConvertColor<Output>(ConvertColor<typename Input::ExtendedCounterpart>(color));
      }
      else if constexpr (IsRGBBoundedType<Output>)
      {
        return ConvertColor<Output>(ConvertColor<typename Output::ExtendedCounterpart>(color));
      }
      // 6. At this point, Input and Output are each either ExtendedLinear-RGB types (of different families)
      // or XYZA
      //    and therefore all additional conversion can happen via matrix transformation.
      else
      {
        return HandleMatrixConversion(color);
      }
    }

  private:
    KRYS_NODISCARD constexpr static Output HandleToFloatConversion(const Input &color) noexcept
    {
      static_assert(IsRGBBoundedType<Input>,
                    "Only bounded ([0..1]) RGB color types support conversion to/from bytes.");

      using InputWithReplacement = ColorTypeWithReplacementComponent<Input, float>;
      if constexpr (SameType<InputWithReplacement, Output>)
        return CreateFromComponents<InputWithReplacement>(
          AsColorComponents(color.Resolved()).map([](uint8 value) -> float { return value / 255.0f; }));
      else
        return ConvertColor<Output>(ConvertColor<InputWithReplacement>(color));
    }

    KRYS_NODISCARD constexpr static Output HandleToByteConversion(const Input &color) noexcept
    {
      static_assert(IsRGBBoundedType<Output>,
                    "Only bounded ([0..1]) RGB color types support conversion to/from bytes.");

      using OutputWithReplacement = ColorTypeWithReplacementComponent<Output, float>;
      if constexpr (SameType<OutputWithReplacement, Input>)
        return CreateFromComponents<Output>(
          AsColorComponents(color.Resolved())
            .map([](float value) -> uint8 { return std::clamp(std::lround(value * 255.0f), 0l, 255l); }));
      else
        return ConvertColor<Output>(ConvertColor<OutputWithReplacement>(color));
    }

    template <typename ColorType>
    KRYS_NODISCARD constexpr static auto ToLinearEncoded(const ColorType &color) noexcept ->
      typename ColorType::LinearCounterpart
    {
      auto [c1, c2, c3, alpha] = color.Resolved();
      return {ColorType::TransferFunction::toLinear(c1), ColorType::TransferFunction::toLinear(c2),
              ColorType::TransferFunction::toLinear(c3), alpha};
    }

    template <typename ColorType>
    KRYS_NODISCARD constexpr static auto ToGammaEncoded(const ColorType &color) noexcept ->
      typename ColorType::GammaEncodedCounterpart
    {
      auto [c1, c2, c3, alpha] = color.Resolved();
      return {ColorType::TransferFunction::ToGammaEncoded(c1),
              ColorType::TransferFunction::ToGammaEncoded(c2),
              ColorType::TransferFunction::ToGammaEncoded(c3), alpha};
    }

    template <typename ColorType>
    KRYS_NODISCARD constexpr static auto ToExtended(const ColorType &color) noexcept ->
      typename ColorType::ExtendedCounterpart
    {
      return CreateFromComponents<typename ColorType::ExtendedCounterpart>(
        AsColorComponents(color.Resolved()));
    }

    template <typename ColorType>
    KRYS_NODISCARD constexpr static auto ToBounded(const ColorType &color) noexcept ->
      typename ColorType::BoundedCounterpart
    {
      return ClipGamutMapping::MapToBoundedGamut(color);
    }

    KRYS_NODISCARD constexpr static Output HandleRGBFamilyConversion(const Input &color) noexcept
    {
      static_assert(IsSameRGBTypeFamily<Output, Input>);

      // RGB Family────────────────────┐
      // │           ┌────────────────┐│
      // │     ┌─────▶︎ LinearExtended ││
      // │     │     └────────▲───────┘│
      // │     │              │        │
      // │┌────────┐          │        │
      // ││ Linear │          │        │
      // │└────▲───┘          │        │
      // │     │              │        │
      // │┌────────┐ ┌────────────────┐│
      // ││ Gamma  │─│ GammaExtended  ││
      // │└────────┘ └────────────────┘│
      // └─────────────────────────────┘

      // This handles conversions between any two of these within the same family, so SRGBLinear -> SRGB, but
      // not SRGBLinear -> A98RGB.

      auto BoundsConversion = [](auto color)
      {
        if constexpr (IsRGBExtendedType<Output> && IsRGBBoundedType<Input>)
        {
          return ToExtended(color);
        }
        else if constexpr (IsRGBBoundedType<Output> && IsRGBExtendedType<Input>)
        {
          return ToBounded(color);
        }
        else
        {
          return color;
        }
      };

      auto GammaConversion = [](auto color)
      {
        if constexpr (IsRGBGammaEncodedType<Output> && IsRGBLinearEncodedType<Input>)
        {
          return ToGammaEncoded(color);
        }
        else if constexpr (IsRGBLinearEncodedType<Output> && IsRGBGammaEncodedType<Input>)
        {
          return ToLinearEncoded(color);
        }
        else
        {
          return color;
        }
      };

      return BoundsConversion(GammaConversion(color));
    }

    KRYS_NODISCARD constexpr static Output HandleMatrixConversion(const Input &color) noexcept
    {
      static_assert((IsRGBLinearEncodedType<Input> && IsRGBExtendedType<Input>) || IsXYZA<Input>);
      static_assert((IsRGBLinearEncodedType<Output> && IsRGBExtendedType<Output>) || IsXYZA<Output>);

      // ┌ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─
      // ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ─ ┐
      //  Matrix Conversions    ┌───────────┐│┌───────────┐
      // │                      │ XYZ (D50) │││ XYZ (D65) │ │
      //                        └─────▲─────┘│└─────▲─────┘
      // │                            │      │      │ │
      //                  ┌───────────┘      │
      //                  └───────────┬───────────────────────────────┬───────────────────────────────┬───────────────────────────────┐
      // │                │                  │                  │                               │ │ │ │
      //                  │                  │                  │                               │ │ │
      // │                │                  │                  │                               │ │ │ │
      //   ProPhotoRGB───────────────────┐   │   SRGB──────────────────────────┐
      //   DisplayP3─────────────────────┐ A98RGB────────────────────────┐ Rec2020───────────────────────┐
      // │ │           ┌────────────────┐│   │   │           ┌────────────────┐│ │ ┌────────────────┐│ │
      // ┌────────────────┐│ │           ┌────────────────┐│ │
      //   │     ┌─────▶︎ LinearExtended ││   │   │     ┌─────▶︎ LinearExtended ││ │     ┌─────▶︎ LinearExtended
      //   ││ │     ┌─────▶︎ LinearExtended ││ │     ┌─────▶︎ LinearExtended ││
      // │ │     │     └────────▲───────┘│   │   │     │     └────────▲───────┘│ │     │ └────────▲───────┘│ │
      // │     └────────▲───────┘│ │     │     └────────▲───────┘│ │
      //  ─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─ ─│─ ─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─
      //  ─│─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─│─ ─ ─│─ ─ ─ ─ ─ ─ ─ ┼ ─ ─ ─ ─│─

      // This handles conversions between extended linear color types that can be converted using pre-defined
      // 3x3 matrices.

      // FIXME: Pre-compute (using constexpr) the concatenation of the matrices prior to applying them
      // to reduce number of matrix multiplications to a minimum. This will likely give subtly different
      // results (due to floating point effects) so if this optimization is considered we should ensure we
      // have sufficient testing coverage to notice any adverse effects.

      auto ApplyMatrices = [](const Input &color, auto... matrices)
      {
        return CreateFromComponents<Output>(
          ApplyMatricesToColorComponents(AsColorComponents(color.Resolved()), matrices...));
      };

      if constexpr (Input::whitePoint == Output::whitePoint)
      {
        if constexpr (IsXYZA<Input>)
        {
          return ApplyMatrices(color, Output::xyzToLinear);
        }
        else if constexpr (IsXYZA<Output>)
        {
          return ApplyMatrices(color, Input::linearToXYZ);
        }
        else
        {
          return ApplyMatrices(color, Input::linearToXYZ, Output::xyzToLinear);
        }
      }
      else
      {
        if constexpr (IsXYZA<Input> && IsXYZA<Output>)
        {
          return ApplyMatrices(color, ChromaticAdaptation<Input::whitePoint, Output::whitePoint>::matrix);
        }
        else if constexpr (IsXYZA<Input>)
        {
          return ApplyMatrices(color, ChromaticAdaptation<Input::whitePoint, Output::whitePoint>::matrix,
                               Output::xyzToLinear);
        }
        else if constexpr (IsXYZA<Output>)
        {
          return ApplyMatrices(color, Input::linearToXYZ,
                               ChromaticAdaptation<Input::whitePoint, Output::whitePoint>::matrix);
        }
        else
        {
          return ApplyMatrices(color, Input::linearToXYZ,
                               ChromaticAdaptation<Input::whitePoint, Output::whitePoint>::matrix,
                               Output::xyzToLinear);
        }
      }
    }
  };
}