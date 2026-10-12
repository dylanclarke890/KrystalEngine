#pragma once

#include "Krystal.Booey/Graphics/Color/ColorComponents.hpp"
#include "Krystal.Booey/Graphics/Color/ColorMatrix.hpp"
#include "Krystal.Booey/Graphics/Color/ColorModels.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTransferFunctions.hpp"

namespace krys::boo
{
  enum class WhitePoint : uint8
  {
    D50,
    D65
  };

  template <typename, typename>
  struct BoundedGammaEncoded;

  template <typename, typename>
  struct BoundedLinearEncoded;

  template <typename, typename>
  struct ExtendedGammaEncoded;

  template <typename, typename>
  struct ExtendedLinearEncoded;

  template <typename>
  struct HSLA;

  template <typename>
  struct HWBA;

  template <typename>
  struct LCHA;

  template <typename>
  struct Lab;

  template <typename>
  struct OKLCHA;

  template <typename>
  struct OKLab;

  template <typename, WhitePoint>
  struct XYZA;

  // MARK: Make functions.

  template <typename, typename = void>
  constexpr bool HasCanonicalTypeMember = false;

  template <typename T>
  constexpr bool HasCanonicalTypeMember<T, void_t<typename T::CanonicalType>> = true;

  template <typename ColorType, bool hasCononicalType>
  struct CanonicalColorTypeHelper
  {
    using type = ColorType;
  };

  template <typename ColorType>
  struct CanonicalColorTypeHelper<ColorType, true>
  {
    using type = typename ColorType::CanonicalType;
  };

  template <typename ColorType>
  using CanonicalColorType =
    typename CanonicalColorTypeHelper<ColorType, HasCanonicalTypeMember<ColorType>>::type;

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr auto CreateFromComponents(const ColorComponents<T, 4> &c) noexcept
  {
    return CanonicalColorType<ColorType> {c[0], c[1], c[2], c[3]};
  }

  template <typename ColorType, unsigned Index, typename T>
  KRYS_NODISCARD constexpr auto ClampedComponent(T c) noexcept -> typename ColorType::ComponentType
  {
    static_assert(Integral<T>);

    constexpr auto componentInfo = ColorType::Model::ComponentInfo[Index];
    return static_cast<ColorType::ComponentType>(std::clamp<T>(c, componentInfo.Min, componentInfo.Max));
  }

  template <typename ColorType, unsigned Index>
  KRYS_NODISCARD constexpr float ClampedComponent(float c) noexcept
  {
    constexpr auto componentInfo = ColorType::Model::ComponentInfo[Index];

    if constexpr (componentInfo.Type == ColorComponentType::Angle)
    {
      return static_cast<float>(std::fmod(std::fmod(c, 360.0) + 360.0, 360.0));
    }
    else if constexpr (componentInfo.Min == -std::numeric_limits<float>::infinity()
                       && componentInfo.Max == std::numeric_limits<float>::infinity())
    {
      return c;
    }
    else if constexpr (componentInfo.Min == -std::numeric_limits<float>::infinity())
    {
      return std::min(c, componentInfo.Max);
    }
    else if constexpr (componentInfo.Max == std::numeric_limits<float>::infinity())
    {
      return std::max(c, componentInfo.Min);
    }
    else
    {
      return std::clamp(c, componentInfo.Min, componentInfo.Max);
    }
  }

  template <typename ColorType, unsigned Index, typename T>
  KRYS_NODISCARD constexpr T ClampedComponent(const ColorComponents<T, 4> &c) noexcept
  {
    return ClampedComponent<ColorType, Index>(c[Index]);
  }

  template <typename T, typename ComponentType = T>
  KRYS_NODISCARD constexpr ComponentType ClampedAlpha(T alpha) noexcept
  {
    return static_cast<ComponentType>(
      std::clamp<T>(alpha, AlphaTraits<ComponentType>::Transparent, AlphaTraits<ComponentType>::Opaque));
  }

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    ClampedComponents(const ColorComponents<T, 4> &components) noexcept
  {
    return {ClampedComponent<ColorType, 0>(components), ClampedComponent<ColorType, 1>(components),
            ClampedComponent<ColorType, 2>(components), ClampedAlpha(components[3])};
  }

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr ColorComponents<T, 4>
    ClampedComponentsExceptAlpha(const ColorComponents<T, 4> &components) noexcept
  {
    return {ClampedComponent<ColorType, 0>(components), ClampedComponent<ColorType, 1>(components),
            ClampedComponent<ColorType, 2>(components), components[3]};
  }

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr auto CreateFromComponentsClamping(const ColorComponents<T, 4> &components) noexcept
  {
    return CreateFromComponents<ColorType>(ClampedComponents<ColorType>(components));
  }

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr auto CreateFromComponentsClamping(T c1, T c2, T c3) noexcept
  {
    return CreateFromComponents<ColorType>(ColorComponents {
      ClampedComponent<ColorType, 0>(c1), ClampedComponent<ColorType, 1>(c2),
      ClampedComponent<ColorType, 2>(c3), AlphaTraits<typename ColorType::ComponentType>::Opaque});
  }

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr auto CreateFromComponentsClamping(T c1, T c2, T c3, T alpha) noexcept
  {
    return CreateFromComponents<ColorType>(ColorComponents {
      ClampedComponent<ColorType, 0>(c1), ClampedComponent<ColorType, 1>(c2),
      ClampedComponent<ColorType, 2>(c3), ClampedAlpha<T, typename ColorType::ComponentType>(alpha)});
  }

  template <typename ColorType, typename T>
  KRYS_NODISCARD constexpr auto
    CreateFromComponentsClampingExceptAlpha(const ColorComponents<T, 4> &components) noexcept
  {
    return CreateFromComponents<ColorType>(ClampedComponentsExceptAlpha<ColorType>(components));
  }

  template <typename ColorType, typename T, typename Alpha>
  KRYS_NODISCARD constexpr auto CreateFromComponentsClampingExceptAlpha(T c1, T c2, T c3,
                                                                        Alpha alpha) noexcept
  {
    return CreateFromComponents<ColorType>(ColorComponents {ClampedComponent<ColorType, 0>(c1),
                                                            ClampedComponent<ColorType, 1>(c2),
                                                            ClampedComponent<ColorType, 2>(c3), alpha});
  }

  // TODO: enable if asserts are enabled
#if 0

  template <typename ColorType>
  constexpr void AssertInRange(ColorType color) noexcept
  requires SameType<typename ColorType::ComponentType, float>
  {
    auto components = AsColorComponents(color.Unresolved());
    for (size_t i = 0uz; i < 3uz; ++i)
    {
      if (IsNaN(components[i]))
      {
        continue;
      }

      // "Component at index %d is %f and is less than the allowed minimum %f", i, components[i], ColorType::Model::ComponentInfo[i].Min
      krys_assert(components[i] >= ColorType::Model::ComponentInfo[i].Min);
      
      // "Component at index %d is %f and is greater than the allowed maximum %f", i, components[i], ColorType::Model::ComponentInfo[i].Max)
      krys_assert(components[i] <= ColorType::Model::ComponentInfo[i].Max;
    }

    if (!IsNaN(components[3]))
    {
      // "Alpha is %f and is less than the allowed minimum (transparent) %f", components[3], AlphaTraits<typename ColorType::ComponentType>::Transparent
      krys_assert(components[3] >= AlphaTraits<typename ColorType::ComponentType>::Transparent);
      // "Alpha is %f and is greater than the allowed maximum (opaque) %f", components[3], AlphaTraits<typename ColorType::ComponentType>::Opaque
      krys_assert(components[3] <= AlphaTraits<typename ColorType::ComponentType>::Opaque);
    }
  }

  template <typename ColorType>
  constexpr void AssertInRange(ColorType) noexcept
  requires SameType<typename ColorType::ComponentType, uint8>
  {
  }

#else

  template <typename T>
  constexpr void AssertInRange(T) noexcept
  {
  }

#endif

  template <typename T>
  concept IsConvertibleToColorComponents = requires(T t) { AsColorComponents(t.Unresolved()); };

  template <typename, typename = void>
  constexpr bool HasComponentTypeMember = false;

  template <typename T>
  constexpr bool HasComponentTypeMember<T, void_t<typename T::ComponentType>> = true;

  template <typename T, typename U, bool enabled>
  constexpr bool HasComponentTypeValue = false;

  template <typename T, typename U>
  constexpr bool HasComponentTypeValue<T, U, true> = SameType<typename T::ComponentType, U>;

  template <typename T, typename U>
  constexpr bool HasComponentType = HasComponentTypeValue<T, U, HasComponentTypeMember<T>>;

  template <typename T>
  constexpr bool IsColorType = IsConvertibleToColorComponents<T> && HasComponentTypeMember<T>;

  template <typename T, typename U>
  concept IsColorTypeWithComponentType = IsConvertibleToColorComponents<T> && HasComponentType<T, U>;

  template <template <typename> class ColorType, typename Replacement>
  struct ColorTypeReplacingComponentTypeHelper
  {
    using type = ColorType<Replacement>;
  };

  template <template <typename> class ColorType, typename Replacement>
  using ColorTypeReplacingComponentType =
    typename ColorTypeReplacingComponentTypeHelper<ColorType, Replacement>::type;

  template <typename Parent>
  struct ColorWithAlphaHelper
  {
    // Helper to allow convenient syntax for working with color types.
    // e.g. auto yellowWith50PercentAlpha = Color::yellow.ColorWithAlphaByte(128);
    KRYS_NODISCARD constexpr Parent ColorWithAlphaByte(uint8 overrideAlpha) const noexcept
    {
      static_assert(SameType<typename Parent::ComponentType, uint8>,
                    "Only uint8 based color types are supported.");

      auto copy = static_cast<const Parent *>(this)->Unresolved();
      copy.Alpha = overrideAlpha;
      return copy;
    }
  };

  template <IsConvertibleToColorComponents ColorType>
  KRYS_NODISCARD constexpr bool operator==(const ColorType &a, const ColorType &b) noexcept
  {
    return AsColorComponents(a.Unresolved()) == AsColorComponents(b.Unresolved());
  }

  // MARK: - RGB Color Types.

  template <typename T, typename D, typename ColorType, typename M, typename TF>
  struct RGBAType : ColorWithAlphaHelper<ColorType>
  {
    using ComponentType = T;
    using Model = M;
    using TransferFunction = TF;
    using Descriptor = D;

    constexpr static auto WhitePoint = D::WhitePoint;

  protected:
    T Red;
    T Green;
    T Blue;
    T Alpha;

  public:
    constexpr RGBAType(T red, T green, T blue, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Red {red}, Green {green}, Blue {blue}, Alpha {alpha}
    {
      AssertInRange(*static_cast<const ColorType *>(this));
    }

    constexpr RGBAType() noexcept : RGBAType {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*static_cast<const ColorType *>(this));
    }

    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*static_cast<const ColorType *>(this));
    }
  };

  template <typename T, typename D>
  struct BoundedGammaEncoded
      : RGBAType<T, D, BoundedGammaEncoded<T, D>, RGBModel<T, RGBBoundedness::Bounded>,
                 typename D::template TransferFunction<T, TransferFunctionMode::Clamped>>
  {
    using RGBAType<T, D, BoundedGammaEncoded<T, D>, RGBModel<T, RGBBoundedness::Bounded>,
                   typename D::template TransferFunction<T, TransferFunctionMode::Clamped>>::RGBAType;

    using LinearCounterpart = BoundedLinearEncoded<T, D>;
    using ExtendedCounterpart = ExtendedGammaEncoded<T, D>;

    template <typename Replacement>
    using SelfWithReplacementComponent = BoundedGammaEncoded<Replacement, D>;
  };

  template <typename T, typename D>
  struct BoundedLinearEncoded
      : RGBAType<T, D, BoundedLinearEncoded<T, D>, RGBModel<T, RGBBoundedness::Bounded>,
                 typename D::template TransferFunction<T, TransferFunctionMode::Clamped>>
  {
    using RGBAType<T, D, BoundedLinearEncoded<T, D>, RGBModel<T, RGBBoundedness::Bounded>,
                   typename D::template TransferFunction<T, TransferFunctionMode::Clamped>>::RGBAType;

    constexpr static auto LinearToXYZ = D::LinearToXYZ;
    constexpr static auto XyzToLinear = D::XyzToLinear;

    using GammaEncodedCounterpart = BoundedGammaEncoded<T, D>;
    using ExtendedCounterpart = ExtendedLinearEncoded<T, D>;

    template <typename Replacement>
    using SelfWithReplacementComponent = BoundedLinearEncoded<Replacement, D>;
  };

  template <typename T, typename D>
  struct ExtendedGammaEncoded
      : RGBAType<T, D, ExtendedGammaEncoded<T, D>, RGBModel<T, RGBBoundedness::Extended>,
                 typename D::template TransferFunction<T, TransferFunctionMode::Unclamped>>
  {
    using RGBAType<T, D, ExtendedGammaEncoded<T, D>, RGBModel<T, RGBBoundedness::Extended>,
                   typename D::template TransferFunction<T, TransferFunctionMode::Unclamped>>::RGBAType;

    using LinearCounterpart = ExtendedLinearEncoded<T, D>;
    using BoundedCounterpart = BoundedGammaEncoded<T, D>;
    using Reference = LinearCounterpart;
  };

  template <typename T, typename D>
  struct ExtendedLinearEncoded
      : RGBAType<T, D, ExtendedLinearEncoded<T, D>, RGBModel<T, RGBBoundedness::Extended>,
                 typename D::template TransferFunction<T, TransferFunctionMode::Unclamped>>
  {
    using RGBAType<T, D, ExtendedLinearEncoded<T, D>, RGBModel<T, RGBBoundedness::Extended>,
                   typename D::template TransferFunction<T, TransferFunctionMode::Unclamped>>::RGBAType;

    constexpr static auto LinearToXYZ = D::LinearToXYZ;
    constexpr static auto XyzToLinear = D::XyzToLinear;

    using GammaEncodedCounterpart = ExtendedGammaEncoded<T, D>;
    using BoundedCounterpart = BoundedLinearEncoded<T, D>;
    using Reference = XYZA<T, D::WhitePoint>;
  };

  template <typename, typename = void>
  constexpr bool HasDescriptorMember = false;

  template <typename ColorType>
  constexpr bool HasDescriptorMember<ColorType, void_t<typename ColorType::Descriptor>> = true;

  template <typename, typename = void>
  constexpr bool HasExtendedCounterpartMember = false;

  template <typename ColorType>
  constexpr bool HasExtendedCounterpartMember<ColorType, void_t<typename ColorType::ExtendedCounterpart>> =
    true;

  template <typename, typename = void>
  constexpr bool HasBoundedCounterpartMember = false;

  template <typename ColorType>
  constexpr bool HasBoundedCounterpartMember<ColorType, void_t<typename ColorType::BoundedCounterpart>> =
    true;

  template <typename, typename = void>
  constexpr bool HasGammaEncodedCounterpartMember = false;

  template <typename ColorType>
  constexpr bool
    HasGammaEncodedCounterpartMember<ColorType, void_t<typename ColorType::GammaEncodedCounterpart>> = true;

  template <typename, typename = void>
  constexpr bool HasLinearCounterpartMember = false;

  template <typename ColorType>
  constexpr bool HasLinearCounterpartMember<ColorType, void_t<typename ColorType::LinearCounterpart>> = true;

  template <typename, typename = void>
  constexpr bool HasSelfWithReplacementComponentMember = false;

  template <typename ColorType>
  constexpr bool HasSelfWithReplacementComponentMember<
    ColorType, void_t<typename ColorType::SelfWithReplacementComponent>> = true;

  template <typename ColorType, typename Replacement>
  using ColorTypeWithReplacementComponent =
    typename ColorType::template SelfWithReplacementComponent<Replacement>;

  template <typename ColorType>
  constexpr bool IsRGBType = HasDescriptorMember<ColorType>;

  template <typename ColorType>
  constexpr bool IsRGBExtendedType = IsRGBType<ColorType> && HasBoundedCounterpartMember<ColorType>;

  template <typename ColorType>
  constexpr bool IsRGBBoundedType = IsRGBType<ColorType> && HasExtendedCounterpartMember<ColorType>;

  template <typename ColorType>
  constexpr bool IsRGBGammaEncodedType = IsRGBType<ColorType> && HasLinearCounterpartMember<ColorType>;

  template <typename ColorType>
  constexpr bool IsRGBLinearEncodedType = IsRGBType<ColorType> && HasGammaEncodedCounterpartMember<ColorType>;

  template <typename ColorType1, typename ColorType2, bool enabled>
  constexpr bool IsSameRGBTypeFamilyValue = false;

  template <typename ColorType1, typename ColorType2>
  constexpr bool IsSameRGBTypeFamilyValue<ColorType1, ColorType2, true> =
    SameType<typename ColorType1::Descriptor, typename ColorType2::Descriptor>;

  template <typename ColorType1, typename ColorType2>
  constexpr bool IsSameRGBTypeFamily =
    IsSameRGBTypeFamilyValue<ColorType1, ColorType2, IsRGBType<ColorType1> && IsRGBType<ColorType2>>;

  template <typename BoundedColorType>
  KRYS_NODISCARD constexpr bool InGamut(typename BoundedColorType::ComponentType component) noexcept
  {
    static_assert(IsRGBBoundedType<BoundedColorType>);

    return component >= 0.0f && component <= 1.0f;
  }

  template <typename BoundedColorType>
  KRYS_NODISCARD constexpr bool
    InGamut(ColorComponents<typename BoundedColorType::ComponentType, 4> components) noexcept
  {
    static_assert(IsRGBBoundedType<BoundedColorType>);

    return InGamut<BoundedColorType>(components[0]) && InGamut<BoundedColorType>(components[1])
           && InGamut<BoundedColorType>(components[2]);
  }

  template <typename BoundedColorType, typename ColorType>
  KRYS_NODISCARD constexpr bool InGamut(ColorType color) noexcept
  {
    static_assert(IsRGBBoundedType<BoundedColorType>);
    static_assert(SameType<BoundedColorType, typename ColorType::BoundedCounterpart>);

    return InGamut<BoundedColorType>(AsColorComponents(color.Resolved()));
  }

  template <typename BoundedColorType, typename ColorType>
  KRYS_NODISCARD constexpr Maybe<BoundedColorType> ColorIfInGamut(ColorType color) noexcept
  {
    static_assert(IsRGBBoundedType<BoundedColorType>);
    static_assert(SameType<BoundedColorType, typename ColorType::BoundedCounterpart>);

    auto components = AsColorComponents(color.Resolved());
    if (!InGamut<BoundedColorType>(components))
    {
      return null;
    }

    return CreateFromComponents<BoundedColorType>(components);
  }

  template <typename BoundedColorType, typename ColorType>
  KRYS_NODISCARD constexpr BoundedColorType ClipToGamut(ColorType color) noexcept
  {
    static_assert(IsRGBBoundedType<BoundedColorType>);
    static_assert(SameType<BoundedColorType, typename ColorType::BoundedCounterpart>);

    return CreateFromComponentsClampingExceptAlpha<BoundedColorType>(AsColorComponents(color.Resolved()));
  }

  struct SRGBADescriptor
  {
    template <typename T, TransferFunctionMode Mode>
    using TransferFunction = SRGBTransferFunction<T, Mode>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    // https://drafts.csswg.org/css-color/#color-conversion-code
    constexpr static ColorMatrix<3, 3> XyzToLinear {
      12831.0 / 3959.0,   -329.0 / 214.0,  -1974.0 / 3959.0,  -851781.0 / 878810.0, 1648619.0 / 878810.0,
      36519.0 / 878810.0, 705.0 / 12673.0, -2585.0 / 12673.0, 705.0 / 667.0,
    };
    constexpr static ColorMatrix<3, 3> LinearToXYZ {
      506752.0 / 1228815.0, 87881.0 / 245763.0,  12673.0 / 70218.0,
      87098.0 / 409605.0,   175762.0 / 245763.0, 12673.0 / 175545.0,
      7918.0 / 409605.0,    87881.0 / 737289.0,  1001167.0 / 1053270.0,
    };
  };

  template <typename T>
  using SRGBA = BoundedGammaEncoded<T, SRGBADescriptor>;

  template <typename T>
  using LinearSRGBA = BoundedLinearEncoded<T, SRGBADescriptor>;

  template <typename T>
  using ExtendedSRGBA = ExtendedGammaEncoded<T, SRGBADescriptor>;

  template <typename T>
  using ExtendedLinearSRGBA = ExtendedLinearEncoded<T, SRGBADescriptor>;

  struct A98RGBDescriptor
  {
    template <typename T, TransferFunctionMode Mode>
    using TransferFunction = A98RGBTransferFunction<T, Mode>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    // https://drafts.csswg.org/css-color/#color-conversion-code
    constexpr static ColorMatrix<3, 3> XyzToLinear {
      1829569.0 / 896150.0, -506331.0 / 896150.0,  -308931.0 / 896150.0,
      -851781.0 / 878810.0, 1648619.0 / 878810.0,  36519.0 / 878810.0,
      16779.0 / 1248040.0,  -147721.0 / 1248040.0, 1266979.0 / 1248040.0,
    };

    constexpr static ColorMatrix<3, 3> LinearToXYZ {
      573536.0 / 994567.0,  263643.0 / 1420810.0,  187206.0 / 994567.0,
      591459.0 / 1989134.0, 6239551.0 / 9945670.0, 374412.0 / 4972835.0,
      53769.0 / 1989134.0,  351524.0 / 4972835.0,  4929758.0 / 4972835.0,
    };
  };

  template <typename T>
  using A98RGB = BoundedGammaEncoded<T, A98RGBDescriptor>;

  template <typename T>
  using LinearA98RGB = BoundedLinearEncoded<T, A98RGBDescriptor>;

  template <typename T>
  using ExtendedA98RGB = ExtendedGammaEncoded<T, A98RGBDescriptor>;

  template <typename T>
  using ExtendedLinearA98RGB = ExtendedLinearEncoded<T, A98RGBDescriptor>;

  struct DisplayP3Descriptor
  {
    template <typename T, TransferFunctionMode Mode>
    using TransferFunction = SRGBTransferFunction<T, Mode>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    // https://drafts.csswg.org/css-color/#color-conversion-code
    constexpr static ColorMatrix<3, 3> XyzToLinear {
      446124.0 / 178915.0, -333277.0 / 357830.0, -72051.0 / 178915.0, -14852.0 / 17905.0,  63121.0 / 35810.0,
      423.0 / 17905.0,     11844.0 / 330415.0,   -50337.0 / 660830.0, 316169.0 / 330415.0,
    };

    constexpr static ColorMatrix<3, 3> LinearToXYZ {
      608311.0 / 1250200.0,
      189793.0 / 714400.0,
      198249.0 / 1000160.0,
      35783.0 / 156275.0,
      247089.0 / 357200.0,
      198249.0 / 2500400.0,
      0.0 / 1.0,
      32229.0 / 714400.0,
      5220557.0 / 5000800.0,
    };
  };

  template <typename T>
  using DisplayP3 = BoundedGammaEncoded<T, DisplayP3Descriptor>;

  template <typename T>
  using LinearDisplayP3 = BoundedLinearEncoded<T, DisplayP3Descriptor>;

  template <typename T>
  using ExtendedDisplayP3 = ExtendedGammaEncoded<T, DisplayP3Descriptor>;

  template <typename T>
  using ExtendedLinearDisplayP3 = ExtendedLinearEncoded<T, DisplayP3Descriptor>;

  struct ProPhotoRGBDescriptor
  {
    template <typename T, TransferFunctionMode Mode>
    using TransferFunction = ProPhotoRGBTransferFunction<T, Mode>;

    constexpr static auto WhitePoint = boo::WhitePoint::D50;

    // https://drafts.csswg.org/css-color/#color-conversion-code
    constexpr static ColorMatrix<3, 3> XyzToLinear {
      1.34578688164715830,  -0.25557208737979464, -0.05110186497554526,
      -0.54463070512490190, 1.50824774284514680,  0.02052744743642139,
      0.00000000000000000,  0.00000000000000000,  1.21196754563894520};

    constexpr static ColorMatrix<3, 3> LinearToXYZ {
      0.79776664490064230, 0.13518129740053308, 0.03134773412839220, 0.28807482881940130, 0.71183523424187300,
      0.00008993693872564, 0.00000000000000000, 0.00000000000000000, 0.82510460251046020};
  };

  template <typename T>
  using ProPhotoRGB = BoundedGammaEncoded<T, ProPhotoRGBDescriptor>;

  template <typename T>
  using LinearProPhotoRGB = BoundedLinearEncoded<T, ProPhotoRGBDescriptor>;

  template <typename T>
  using ExtendedProPhotoRGB = ExtendedGammaEncoded<T, ProPhotoRGBDescriptor>;

  template <typename T>
  using ExtendedLinearProPhotoRGB = ExtendedLinearEncoded<T, ProPhotoRGBDescriptor>;

  struct Rec2020Descriptor
  {
    template <typename T, TransferFunctionMode Mode>
    using TransferFunction = Rec2020TransferFunction<T, Mode>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    // https://drafts.csswg.org/css-color/#color-conversion-code
    constexpr static ColorMatrix<3, 3> XyzToLinear {
      30757411.0 / 17917100.0,  -6372589.0 / 17917100.0, -4539589.0 / 17917100.0,
      -19765991.0 / 29648200.0, 47925759.0 / 29648200.0, 467509.0 / 29648200.0,
      792561.0 / 44930125.0,    -1921689.0 / 44930125.0, 42328811.0 / 44930125.0,
    };
    constexpr static ColorMatrix<3, 3> LinearToXYZ {
      63426534.0 / 99577255.0,
      20160776.0 / 139408157.0,
      47086771.0 / 278816314.0,
      26158966.0 / 99577255.0,
      472592308.0 / 697040785.0,
      8267143.0 / 139408157.0,
      0.0 / 1.0,
      19567812.0 / 697040785.0,
      295819943.0 / 278816314.0,
    };
  };

  template <typename T>
  using Rec2020 = BoundedGammaEncoded<T, Rec2020Descriptor>;

  template <typename T>
  using LinearRec2020 = BoundedLinearEncoded<T, Rec2020Descriptor>;

  template <typename T>
  using ExtendedRec2020 = ExtendedGammaEncoded<T, Rec2020Descriptor>;

  template <typename T>
  using ExtendedLinearRec2020 = ExtendedLinearEncoded<T, Rec2020Descriptor>;

#pragma region Lab

  template <typename T>
  struct Lab : ColorWithAlphaHelper<Lab<T>>
  {
    using ComponentType = T;
    using Model = LabModel<T>;

    constexpr static auto WhitePoint = boo::WhitePoint::D50;

    using Reference = XYZA<T, WhitePoint>;

  protected:
    T Lightness;
    T A;
    T B;
    T Alpha;

  public:
    constexpr Lab(T lightness, T a, T b, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Lightness {lightness}, A {a}, B {b}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr Lab() noexcept : Lab {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }
    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsLab = SameType<Lab<typename ColorType::ComponentType>, ColorType>;

#pragma endregion

#pragma region LCHA

  template <typename T>
  struct LCHA : ColorWithAlphaHelper<LCHA<T>>
  {
    using ComponentType = T;
    using Model = LCHModel<T>;

    constexpr static auto WhitePoint = boo::WhitePoint::D50;

    using Reference = Lab<T>;

  protected:
    T Lightness;
    T Chroma;
    T Hue;
    T Alpha;

  public:
    constexpr LCHA(T lightness, T chroma, T hue, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Lightness {lightness}, Chroma {chroma}, Hue {hue}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr LCHA() noexcept : LCHA {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }
    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsLCHA = SameType<LCHA<typename ColorType::ComponentType>, ColorType>;

#pragma endregion

#pragma region OKLab

  template <typename T>
  struct OKLab : ColorWithAlphaHelper<OKLab<T>>
  {
    using ComponentType = T;
    using Model = OKLabModel<T>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    using Reference = XYZA<T, WhitePoint>;

  protected:
    T Lightness;
    T A;
    T B;
    T Alpha;

  public:
    constexpr OKLab(T lightness, T a, T b, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Lightness {lightness}, A {a}, B {b}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr OKLab() noexcept : OKLab {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }

    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsOKLab = SameType<OKLab<typename ColorType::ComponentType>, ColorType>;

#pragma endregion

#pragma region OKLCHA

  template <typename T>
  struct OKLCHA : ColorWithAlphaHelper<OKLCHA<T>>
  {
    using ComponentType = T;
    using Model = OKLCHModel<T>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    using Reference = OKLab<T>;

  protected:
    T Lightness;
    T Chroma;
    T Hue;
    T Alpha;

  public:
    constexpr OKLCHA(T lightness, T chroma, T hue, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Lightness {lightness}, Chroma {chroma}, Hue {hue}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr OKLCHA() noexcept : OKLCHA {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }

    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsOKLCHA = SameType<OKLCHA<typename ColorType::ComponentType>, ColorType>;

#pragma endregion

#pragma region HSLA

  template <typename T>
  struct HSLA : ColorWithAlphaHelper<HSLA<T>>
  {
    using ComponentType = T;
    using Model = HSLModel<T>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    using Reference = ExtendedSRGBA<T>;

  protected:
    T Hue;
    T Saturation;
    T Lightness;
    T Alpha;

  public:
    constexpr HSLA(T hue, T saturation, T lightness, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Hue {hue}, Saturation {saturation}, Lightness {lightness}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr HSLA() noexcept : HSLA {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }

    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsHSLA = SameType<HSLA<typename ColorType::ComponentType>, ColorType>;

#pragma endregion

#pragma region HWBA

  template <typename T>
  struct HWBA : ColorWithAlphaHelper<HWBA<T>>
  {
    using ComponentType = T;
    using Model = HWBModel<T>;

    constexpr static auto WhitePoint = boo::WhitePoint::D65;

    using Reference = ExtendedSRGBA<T>;

  protected:
    T Hue;
    T Whiteness;
    T Blackness;
    T Alpha;

  public:
    constexpr HWBA(T hue, T whiteness, T blackness, T alpha = AlphaTraits<T>::Opaque) noexcept
        : Hue {hue}, Whiteness {whiteness}, Blackness {blackness}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr HWBA() noexcept : HWBA {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }

    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsHWBA = SameType<HWBA<typename ColorType::ComponentType>, ColorType>;

#pragma endregion

#pragma region XYZA

  template <typename T, WhitePoint W>
  struct XYZA : ColorWithAlphaHelper<XYZA<T, W>>
  {
    using ComponentType = T;
    using Model = XYZModel<T>;
    using ReferenceXYZ = XYZA<T, W>;

    constexpr static auto WhitePoint = W;

  protected:
    T X;
    T Y;
    T Z;
    T Alpha;

  public:
    constexpr XYZA(T x, T y, T z, T alpha = AlphaTraits<T>::Opaque) noexcept
        : X {x}, Y {y}, Z {z}, Alpha {alpha}
    {
      AssertInRange(*this);
    }

    constexpr XYZA() noexcept : XYZA {0, 0, 0, 0}
    {
    }

    KRYS_NODISCARD constexpr auto Resolved() const noexcept
    {
      return ResolvedColor(*this);
    }

    KRYS_NODISCARD constexpr auto Unresolved() const noexcept
    {
      return UnresolvedColor(*this);
    }
  };

  template <typename ColorType>
  constexpr bool IsXYZA = SameType<XYZA<typename ColorType::ComponentType, ColorType::WhitePoint>, ColorType>;

#pragma endregion

  // Packed Color Formats

  namespace PackedColor
  {
    struct RGBA
    {
      uint32 Value;

      constexpr explicit RGBA(uint32 rgba) noexcept : Value {rgba}
      {
      }

      constexpr explicit RGBA(ResolvedColorType<SRGBA<uint8>> color) noexcept
          : Value {static_cast<uint32>(color.Red << 24 | color.Green << 16 | color.Blue << 8 | color.Alpha)}
      {
      }

      constexpr explicit RGBA(SRGBA<uint8> color) noexcept : RGBA {color.Resolved()}
      {
      }
    };

    struct ARGB
    {
      uint32 Value;

      constexpr explicit ARGB(uint32 argb) noexcept : Value {argb}
      {
      }

      constexpr explicit ARGB(ResolvedColorType<SRGBA<uint8>> color) noexcept
          : Value {static_cast<uint32>(color.Alpha << 24 | color.Red << 16 | color.Green << 8 | color.Blue)}
      {
      }

      constexpr explicit ARGB(SRGBA<uint8> color) noexcept : ARGB {color.Resolved()}
      {
      }
    };

  }

  KRYS_NODISCARD constexpr SRGBA<uint8> AsSRGBA(PackedColor::RGBA color) noexcept
  {
    return {static_cast<uint8>(color.Value >> 24), static_cast<uint8>(color.Value >> 16),
            static_cast<uint8>(color.Value >> 8), static_cast<uint8>(color.Value)};
  }

  KRYS_NODISCARD constexpr SRGBA<uint8> AsSRGBA(PackedColor::ARGB color) noexcept
  {
    return {static_cast<uint8>(color.Value >> 16), static_cast<uint8>(color.Value >> 8),
            static_cast<uint8>(color.Value), static_cast<uint8>(color.Value >> 24)};
  }
}