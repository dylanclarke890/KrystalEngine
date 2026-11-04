#include "Krystal.Booey/Graphics/Filters/FilterOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Color/ColorBlending.hpp"
#include "Krystal.Booey/Graphics/Filters/FEDropShadow.hpp"
#include "Krystal.Booey/Graphics/Filters/FEGaussianBlur.hpp"
#include "Krystal.Core/Colour/ColourConversion.hpp"
#include "Krystal.Core/Colour/ColourMatrix.hpp"
#include "Krystal.Core/Colour/ColourTypes.hpp"

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
          GrayscaleColourMatrix(static_cast<float>(_amount))
            .TransformedColourComponents(AsColourComponents(color.Resolved())));
        return true;
      }
      case Type::Sepia:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          SepiaColourMatrix(static_cast<float>(_amount))
            .TransformedColourComponents(AsColourComponents(color.Resolved())));
        return true;
      }
      case Type::HueRotate:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          HueRotateColourMatrix(static_cast<float>(_amount))
            .TransformedColourComponents(AsColourComponents(color.Resolved())));
        return true;
      }
      case Type::Saturate:
      {
        color = CreateFromComponentsClamping<SRGBA<float>>(
          SaturationColourMatrix(static_cast<float>(_amount))
            .TransformedColourComponents(AsColourComponents(color.Resolved())));
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
          ColourWithOverriddenAlpha(color, std::clamp<float>(color.Resolved().Alpha * amount, 0.0f, 1.0f));
        return true;
      case Type::Invert:
      {
        float oneMinusAmount = 1.0f - amount;
        color = ColourByModifyingEachNonAlphaComponent(
          color,
          [&](float component) { return 1.0f - (oneMinusAmount + component * (amount - oneMinusAmount)); });
        return true;
      }
      case Type::Contrast:
      {
        float intercept = -(0.5f * amount) + 0.5f;
        color = ColourByModifyingEachNonAlphaComponent(
          color,
          [&](float component) { return std::clamp<float>(intercept + amount * component, 0.0f, 1.0f); });
        return true;
      }
      case Type::Brightness:
        color = ColourByModifyingEachNonAlphaComponent(
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
