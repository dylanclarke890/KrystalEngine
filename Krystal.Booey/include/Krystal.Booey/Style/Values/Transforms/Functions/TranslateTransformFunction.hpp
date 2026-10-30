#pragma once

#include "Krystal.Booey/Style/Values/Primitives/LengthWrapper.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // translate() = translate( <length-percentage> , <length-percentage>? )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-translate
  // translate3d() = translate3d( <length-percentage> , <length-percentage> , <length> )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-translate3d
  // translateX() = translateX( <length-percentage> )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-translatex
  // translateY() = translateY( <length-percentage> )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-translatey
  // translateZ() = translateZ( <length> )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-translatez

  struct TranslateLengthPercentage : LengthWrapperBase<LengthPercentage<>>
  {
    using Base::Base;
  };

  class TranslateTransformFunction final : public TransformFunctionBase
  {
  public:
    using LengthPercentage = style::TranslateLengthPercentage;
    using Length = style::Length<>;

    KRYS_NODISCARD static Ref<const TranslateTransformFunction>
      Create(const LengthPercentage &, const LengthPercentage &, TransformFunctionBase::Type) noexcept;
    KRYS_NODISCARD static Ref<const TranslateTransformFunction> Create(const LengthPercentage &,
                                                                       const LengthPercentage &,
                                                                       const Length &,
                                                                       TransformFunctionBase::Type) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD TransformFunctionBase::Type PrimitiveType() const noexcept override
    {
      return IsRepresentableIn2D() ? Type::Translate : Type::Translate3D;
    }

    const LengthPercentage &X() const
    {
      return _x;
    }
    const LengthPercentage &Y() const
    {
      return _y;
    }
    KRYS_NODISCARD Length Z() const noexcept
    {
      return _z;
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _x.IsKnownZero() && _y.IsKnownZero() && _z.IsZero();
    }
    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept override
    {
      return _z.IsZero();
    }

    KRYS_NODISCARD TransformFunctionSizeDependencies ComputeSizeDependencies() const noexcept override;

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    TranslateTransformFunction(const LengthPercentage &, const LengthPercentage &, const Length &,
                               TransformFunctionBase::Type) noexcept;

    LengthPercentage _x;
    LengthPercentage _y;
    Length _z;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(
  krys::boo::style::TranslateTransformFunction,
  krys::boo::style::TransformFunctionBase::IsTranslateTransformFunctionType)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::TranslateLengthPercentage)
