#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // rotate() = rotate( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-rotate
  // rotate3d() = rotate3d( <number> , <number> , <number> , [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-rotate3d
  // rotateX() = rotateX( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-rotatex
  // rotateY() = rotateY( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-rotatey
  // rotateZ() = rotateZ( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-rotatez

  class RotateTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const RotateTransformFunction> Create(Angle<>,
                                                                    TransformFunctionBase::Type) noexcept;
    KRYS_NODISCARD static Ref<const RotateTransformFunction> Create(Number<>, Number<>, Number<>, Angle<>,
                                                                    TransformFunctionBase::Type) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD Number<> X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD Number<> Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD Number<> Z() const noexcept
    {
      return _z;
    }
    KRYS_NODISCARD Angle<> GetAngle() const noexcept
    {
      return _angle;
    }

    KRYS_NODISCARD TransformFunctionBase::Type PrimitiveType() const noexcept override
    {
      return GetType() == Type::Rotate ? Type::Rotate : Type::Rotate3D;
    }

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _angle.IsZero();
    }
    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept override
    {
      return (_x.IsZero() && _y.IsZero()) || _angle.IsZero();
    }
    KRYS_NODISCARD bool IsAffectedByTransformOrigin() const noexcept override
    {
      return !IsIdentity();
    }

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    RotateTransformFunction(Number<>, Number<>, Number<>, Angle<>, TransformFunctionBase::Type) noexcept;

    Number<> _x;
    Number<> _y;
    Number<> _z;
    Angle<> _angle;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(
  krys::boo::style::RotateTransformFunction,
  krys::boo::style::TransformFunctionBase::IsRotateTransformFunctionType)
