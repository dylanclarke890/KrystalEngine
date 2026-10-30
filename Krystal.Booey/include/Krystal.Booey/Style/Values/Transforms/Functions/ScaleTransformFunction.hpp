#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // scale() = scale( [ <number> | <percentage> ]#{1,2} )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-scale
  // scaleX() = scaleX( [ <number> | <percentage> ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-scalex
  // scaleY() = scaleY( [ <number> | <percentage> ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-scaley
  // scale3d() = scale3d( [ <number> | <percentage> ]#{3} )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-scale3d

  class ScaleTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const ScaleTransformFunction> Create(NumberOrPercentageResolvedToNumber<>,
                                                                   NumberOrPercentageResolvedToNumber<>,
                                                                   TransformFunctionBase::Type) noexcept;
    KRYS_NODISCARD static Ref<const ScaleTransformFunction> Create(NumberOrPercentageResolvedToNumber<>,
                                                                   NumberOrPercentageResolvedToNumber<>,
                                                                   NumberOrPercentageResolvedToNumber<>,
                                                                   TransformFunctionBase::Type) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD TransformFunctionBase::Type PrimitiveType() const noexcept override
    {
      return (GetType() == Type::ScaleZ || GetType() == Type::Scale3D) ? Type::Scale3D : Type::Scale;
    }

    KRYS_NODISCARD NumberOrPercentageResolvedToNumber<> X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD NumberOrPercentageResolvedToNumber<> Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD NumberOrPercentageResolvedToNumber<> Z() const noexcept
    {
      return _z;
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _x == 1 && _y == 1 && _z == 1;
    }
    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept override
    {
      return _z == 1;
    }
    KRYS_NODISCARD bool IsAffectedByTransformOrigin() const noexcept override
    {
      return !IsIdentity();
    }

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    ScaleTransformFunction(NumberOrPercentageResolvedToNumber<>, NumberOrPercentageResolvedToNumber<>,
                           NumberOrPercentageResolvedToNumber<>, TransformFunctionBase::Type) noexcept;

    NumberOrPercentageResolvedToNumber<> _x;
    NumberOrPercentageResolvedToNumber<> _y;
    NumberOrPercentageResolvedToNumber<> _z;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(
  krys::boo::style::ScaleTransformFunction,
  krys::boo::style::TransformFunctionBase::IsScaleTransformFunctionType)
