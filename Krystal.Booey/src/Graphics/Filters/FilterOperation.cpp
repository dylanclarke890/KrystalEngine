#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/Color/ColorConversion.hpp"
#include "Krystal.Booey/Graphics/Color/ColorMatrix.hpp"
#include "Krystal.Booey/Graphics/Color/ColorTypes.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"

namespace krys::boo
{
  bool DefaultFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }

    return RepresentedType() == Downcast<DefaultFilterOperation>(operation).RepresentedType();
  }

  FilterOperation::Type DefaultFilterOperation::RepresentedType() const noexcept
  {
    return _representedType;
  }

  double FilterOperation::BlendAmounts(double from, double to, const BlendingContext &context) const noexcept
  {
    auto blendedAmount = [&]()
    {
      if (context.CompositeOperation == CompositeOperation::Accumulate)
      {
        // The "initial value for interpolation" is 1 for brightness, contrast, opacity and saturate.
        // Accumulation works differently for such operations per
        // https://drafts.fxtf.org/filter-effects/#accumulation.
        switch (_type)
        {
          case Type::Brightness:
          case Type::Contrast:
          case Type::Opacity:
          case Type::Saturate:   return from + to - 1;
          default:               break;
        }
      }
      return boo::Blend(from, to, context);
    }();

    // Make sure blended values remain within bounds as specified by
    // https://drafts.fxtf.org/filter-effects/#supported-filter-functions
    switch (_type)
    {
      case Type::Grayscale:
      case Type::Invert:
      case Type::Opacity:
      case Type::Sepia:      return std::clamp(blendedAmount, 0.0, 1.0);
      case Type::Brightness:
      case Type::Contrast:
      case Type::Saturate:   return std::max(blendedAmount, 0.0);
      default:               return blendedAmount;
    }
  }

  RefPtr<FilterOperation> BasicColorMatrixFilterOperation::Blend(const FilterOperation *from,
                                                                 const BlendingContext &context,
                                                                 bool blendToPassthrough) noexcept
  {
    if (from && !from->IsSameType(*this))
    {
      return ShareRefPtr(this);
    }

    if (blendToPassthrough)
    {
      return BasicColorMatrixFilterOperation::Create(BlendAmounts(_amount, PassthroughAmount(), context),
                                                     _type);
    }

    const BasicColorMatrixFilterOperation *fromOperation = Downcast<BasicColorMatrixFilterOperation>(from);
    double fromAmount = fromOperation ? fromOperation->Amount() : PassthroughAmount();
    auto blendedAmount = BlendAmounts(fromAmount, _amount, context);
    return BasicColorMatrixFilterOperation::Create(blendedAmount, _type);
  }

  bool BasicColorMatrixFilterOperation::IsIdentity() const noexcept
  {
    return _type == Type::Saturate ? (_amount == 1) : !_amount;
  }

  bool BasicColorMatrixFilterOperation::TransformColor(SRGBA<float> &color) const noexcept
  {
    switch (_type)
    {
      case Type::Grayscale:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          GrayscaleColorMatrix(static_cast<float>(_amount))
            .TransformedColorComponents(AsColorComponents(color.Resolved())));
        return true;
      }
      case Type::Sepia:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          SepiaColorMatrix(static_cast<float>(_amount))
            .TransformedColorComponents(AsColorComponents(color.Resolved())));
        return true;
      }
      case Type::HueRotate:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          HueRotateColorMatrix(static_cast<float>(_amount))
            .TransformedColorComponents(AsColorComponents(color.Resolved())));
        return true;
      }
      case Type::Saturate:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          SaturationColorMatrix(static_cast<float>(_amount))
            .TransformedColorComponents(AsColorComponents(color.Resolved())));
        return true;
      }
      default: krys_debug_assert_msg(false, "Unreachable"); return false;
    }
  }

  inline bool BasicColorMatrixFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }
    const BasicColorMatrixFilterOperation &other = Downcast<BasicColorMatrixFilterOperation>(operation);
    return _amount == other._amount;
  }

  double BasicColorMatrixFilterOperation::PassthroughAmount() const noexcept
  {
    switch (_type)
    {
      case Type::Grayscale:
      case Type::Sepia:
      case Type::HueRotate: return 0;
      case Type::Saturate:  return 1;
      default:              krys_debug_assert_msg(false, "Unreachable"); return 0;
    }
  }

  RefPtr<FilterOperation> BasicComponentTransferFilterOperation::Blend(const FilterOperation *from,
                                                                       const BlendingContext &context,
                                                                       bool blendToPassthrough) noexcept
  {
    if (from && !from->IsSameType(*this))
    {
      return ShareRefPtr(this);
    }

    if (blendToPassthrough)
    {
      return BasicComponentTransferFilterOperation::Create(
        BlendAmounts(_amount, PassthroughAmount(), context), _type);
    }

    const BasicComponentTransferFilterOperation *fromOperation =
      Downcast<BasicComponentTransferFilterOperation>(from);
    double fromAmount = fromOperation ? fromOperation->Amount() : PassthroughAmount();
    auto blendedAmount = BlendAmounts(fromAmount, _amount, context);
    return BasicComponentTransferFilterOperation::Create(blendedAmount, _type);
  }

  bool BasicComponentTransferFilterOperation::IsIdentity() const noexcept
  {
    return _type == Type::Invert ? !_amount : (_amount == 1);
  }

  bool BasicComponentTransferFilterOperation::TransformColor(SRGBA<float> &color) const noexcept
  {
    auto amount = static_cast<float>(_amount);
    switch (_type)
    {
      case Type::Opacity:
        color =
          ColorWithOverriddenAlpha(color, std::clamp<float>(color.Resolved().Alpha * amount, 0.0f, 1.0f));
        return true;
      case Type::Invert:
      {
        float oneMinusAmount = 1.0f - amount;
        color = ColorByModifyingEachNonAlphaComponent(
          color,
          [&](float component) { return 1.0f - (oneMinusAmount + component * (amount - oneMinusAmount)); });
        return true;
      }
      case Type::Contrast:
      {
        float intercept = -(0.5f * amount) + 0.5f;
        color = ColorByModifyingEachNonAlphaComponent(
          color,
          [&](float component) { return std::clamp<float>(intercept + amount * component, 0.0f, 1.0f); });
        return true;
      }
      case Type::Brightness:
        color = ColorByModifyingEachNonAlphaComponent(
          color, [&](float component) { return std::clamp<float>(amount * component, 0.0f, 1.0f); });
        return true;
      default: krys_debug_assert_msg(false, "Unreachable"); return false;
    }
  }

  inline bool BasicComponentTransferFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }
    const BasicComponentTransferFilterOperation &other =
      Downcast<BasicComponentTransferFilterOperation>(operation);
    return _amount == other._amount;
  }

  double BasicComponentTransferFilterOperation::PassthroughAmount() const noexcept
  {
    switch (_type)
    {
      case Type::Opacity:    return 1;
      case Type::Invert:     return 0;
      case Type::Contrast:   return 1;
      case Type::Brightness: return 1;
      default:               krys_debug_assert_msg(false, "Unreachable"); return 0;
    }
  }

  bool BasicComponentTransferFilterOperation::AffectsOpacity() const noexcept
  {
    return _type == Type::Opacity;
  }

  bool InvertLightnessFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }

    return true;
  }

  RefPtr<FilterOperation> InvertLightnessFilterOperation::Blend(const FilterOperation *from,
                                                                const BlendingContext &, bool) noexcept
  {
    if (from && !from->IsSameType(*this))
    {
      return ShareRefPtr(this);
    }

    // This filter is not currently blendable.
    return InvertLightnessFilterOperation::Create();
  }

  // FIXME: This hueRotate code exists to allow InvertLightnessFilterOperation to perform hue rotation
  // on color values outside of the non-extended SRGB value range (0-1) to maintain the behavior of colors
  // prior to clamping being enforced. It should likely just use the existing hueRotateColorMatrix(amount)
  // in ColorMatrix.h
  KRYS_NODISCARD static ColorComponents<float, 4> HueRotate(const ColorComponents<float, 4> &color,
                                                            float amount) noexcept
  {
    auto [r, g, b, alpha] = color;

    auto [min, max] = std::minmax({r, g, b});
    float chroma = max - min;

    float lightness = 0.5f * (max + min);
    float saturation;
    if (!chroma)
    {
      saturation = 0;
    }
    else if (lightness <= 0.5f)
    {
      saturation = (chroma / (max + min));
    }
    else
    {
      saturation = (chroma / (2.0f - (max + min)));
    }

    if (!saturation)
    {
      return {lightness, lightness, lightness, alpha};
    }

    float hue;
    if (!chroma)
    {
      hue = 0;
    }
    else if (max == r)
    {
      hue = (60.0f * ((g - b) / chroma)) + 360.0f;
    }
    else if (max == g)
    {
      hue = (60.0f * ((b - r) / chroma)) + 120.0f;
    }
    else
    {
      hue = (60.0f * ((r - g) / chroma)) + 240.0f;
    }

    if (hue >= 360.0f)
    {
      hue -= 360.0f;
    }

    hue /= 360.0f;

    // Perform rotation.
    hue = std::fmod(hue + amount, 1.0f);

    float temp2 =
      lightness <= 0.5f ? lightness * (1.0f + saturation) : lightness + saturation - lightness * saturation;
    float temp1 = 2.0f * lightness - temp2;

    hue *= 6.0f; // calcHue() wants hue in the 0-6 range.

    // Hue is in the range 0-6, other args in 0-1.
    auto calcHue = [](float temp1, float temp2, float hueVal)
    {
      if (hueVal < 0.0f)
      {
        hueVal += 6.0f;
      }
      else if (hueVal >= 6.0f)
      {
        hueVal -= 6.0f;
      }
      if (hueVal < 1.0f)
      {
        return temp1 + (temp2 - temp1) * hueVal;
      }
      if (hueVal < 3.0f)
      {
        return temp2;
      }
      if (hueVal < 4.0f)
      {
        return temp1 + (temp2 - temp1) * (4.0f - hueVal);
      }
      return temp1;
    };

    return {calcHue(temp1, temp2, hue + 2.0f), calcHue(temp1, temp2, hue), calcHue(temp1, temp2, hue - 2.0f),
            alpha};
  }

  bool InvertLightnessFilterOperation::TransformColor(SRGBA<float> &color) const noexcept
  {
    auto hueRotatedSRGBAComponents = HueRotate(AsColorComponents(color.Resolved()), 0.5f);

    // Apply the matrix. See rdar://problem/41146650 for how this matrix was derived.
    constexpr ColorMatrix<5, 3> toDarkModeMatrix {-0.770f, 0.059f,  -0.089f, 0.0f, 1.0f,
                                                  0.030f,  -0.741f, -0.089f, 0.0f, 1.0f,
                                                  0.030f,  0.059f,  -0.890f, 0.0f, 1.0f};
    color = CreateFromComponentsClamping<SRGBA<float>>(
      toDarkModeMatrix.TransformedColorComponents(hueRotatedSRGBAComponents));
    return true;
  }

  bool InvertLightnessFilterOperation::InverseTransformColor(SRGBA<float> &color) const noexcept
  {
    // Apply the matrix.
    constexpr ColorMatrix<5, 3> toLightModeMatrix {-1.300f, -0.097f, 0.147f,  0.0f, 1.25f,
                                                   -0.049f, -1.347f, 0.146f,  0.0f, 1.25f,
                                                   -0.049f, -0.097f, -1.104f, 0.0f, 1.25f};
    auto convertedToLightModeComponents =
      toLightModeMatrix.TransformedColorComponents(AsColorComponents(color.Resolved()));

    auto hueRotatedSRGBAComponents = HueRotate(convertedToLightModeComponents, 0.5f);

    color = CreateFromComponentsClamping<SRGBA<float>>(hueRotatedSRGBAComponents);
    return true;
  }

  bool BlurFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }

    return _stdDeviation == Downcast<BlurFilterOperation>(operation).StdDeviation();
  }

  RefPtr<FilterOperation> BlurFilterOperation::Blend(const FilterOperation *from,
                                                     const BlendingContext &context,
                                                     bool blendToPassthrough) noexcept
  {
    if (from && !from->IsSameType(*this))
    {
      return ShareRefPtr(this);
    }

    if (blendToPassthrough)
    {
      return BlurFilterOperation::Create(std::max(0.0f, boo::Blend(_stdDeviation, 0.0f, context)));
    }

    const BlurFilterOperation *fromOperation = Downcast<BlurFilterOperation>(from);
    auto fromStdDeviation = fromOperation ? fromOperation->_stdDeviation : 0.0f;
    return BlurFilterOperation::Create(std::max(0.0f, boo::Blend(fromStdDeviation, _stdDeviation, context)));
  }

  bool BlurFilterOperation::IsIdentity() const noexcept
  {
    return _stdDeviation <= 0;
  }

  IntOutsets BlurFilterOperation::Outsets() const noexcept
  {
    return FEGaussianBlur::CalculateOutsets({_stdDeviation, _stdDeviation});
  }

  bool DropShadowFilterOperationBase::NonColorEqual(const DropShadowFilterOperationBase &other) const noexcept
  {
    return _location == other._location && _stdDeviation == other._stdDeviation;
  }

  bool DropShadowFilterOperationBase::IsIdentity() const noexcept
  {
    return _stdDeviation < 0 || (!_stdDeviation && _location.IsZero());
  }

  IntOutsets DropShadowFilterOperationBase::Outsets() const noexcept
  {
    return FEDropShadow::CalculateOutsets(
      FloatSize(static_cast<float>(X()), static_cast<float>(Y())),
      FloatSize(static_cast<float>(_stdDeviation), static_cast<float>(_stdDeviation)));
  }

  bool DropShadowFilterOperation::Equals(const FilterOperation &operation) const noexcept
  {
    if (!IsSameType(operation))
    {
      return false;
    }
    const DropShadowFilterOperation &other = Downcast<DropShadowFilterOperation>(operation);
    return NonColorEqual(other) && _color == other._color;
  }

  RefPtr<FilterOperation> DropShadowFilterOperation::Blend(const FilterOperation *, const BlendingContext &,
                                                           bool) noexcept
  {
    // Only DropShadowFilterOperationWithStyleColor gets blended.
    krys_debug_assert_msg(false, "Unreachable");
    return nullptr;
  }

}
