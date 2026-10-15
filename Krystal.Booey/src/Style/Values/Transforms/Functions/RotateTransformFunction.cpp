#include "Krystal.Booey/Style/Values/Transforms/Functions/RotateTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/RotateTransformOperation.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>

namespace krys::boo::style
{
  RotateTransformFunction::RotateTransformFunction(Number<> x, Number<> y, Number<> z, Angle<> angle,
                                                   TransformFunctionBase::Type type) noexcept
      : TransformFunctionBase(type), _x(x), _y(y), _z(z), _angle(angle)
  {
    krys_assert(IsRotateTransformFunctionType(type));
  }

  Ref<const RotateTransformFunction>
    RotateTransformFunction::Create(Angle<> angle, TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new RotateTransformFunction(0, 0, 1, angle, type));
  }

  Ref<const RotateTransformFunction>
    RotateTransformFunction::Create(Number<> x, Number<> y, Number<> z, Angle<> angle,
                                    TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new RotateTransformFunction(x, y, z, angle, type));
  }

  Ref<const TransformFunctionBase> RotateTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new RotateTransformFunction(_x, _y, _z, _angle, GetType()));
  }

  Ref<TransformOperation> RotateTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return RotateTransformOperation::Create(_x.Value, _y.Value, _z.Value, _angle.Value,
                                            style::ConvertToPlatform(GetType()));
  }

  bool RotateTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherRotate = Downcast<RotateTransformFunction>(other);
    return _angle == otherRotate._angle && _x == otherRotate._x && _y == otherRotate._y
           && _z == otherRotate._z;
  }

  void RotateTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    if (GetType() == TransformFunctionBase::Type::Rotate)
    {
      transform.Rotate(_angle.Value);
    }
    else
    {
      transform.Rotate3d(_x.Value, _y.Value, _z.Value, _angle.Value);
    }
  }

  Ref<const TransformFunctionBase> RotateTransformFunction::Blend(const TransformFunctionBase *from,
                                                                  const BlendingContext &context,
                                                                  bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      if (context.CompositeOperation == CompositeOperation::Accumulate)
      {
        return RotateTransformFunction::Create(_x, _y, _z, _angle, GetType());
      }
      return RotateTransformFunction::Create(_x, _y, _z, _angle.Value - _angle.Value * context.Progress,
                                             GetType());
    }
    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(*this);
    }

    auto *fromOp = Downcast<RotateTransformFunction>(from);
    auto *toOp = this;

    // Interpolation of primitives and derived transform functions
    //
    // https://drafts.csswg.org/css-transforms-2/#interpolation-of-transform-functions
    //
    // For interpolations with the primitive rotate3d(), the direction vectors of the transform functions get
    // normalized first. If the normalized vectors are not equal and both rotation angles are non-zero the
    // transform functions get converted into 4x4 matrices first and interpolated as defined in section
    // Interpolation of Matrices afterwards. Otherwise the rotation angle gets interpolated numerically and
    // the rotation vector of the non-zero angle is used or (0, 0, 1) if both angles are zero.

    auto normalizedVector = [](const RotateTransformFunction &op) -> FloatPoint3D
    {
      if (auto length = std::hypot(op._x.Value, op._y.Value, op._z.Value))
      {
        return {static_cast<float>(op._x.Value / length), static_cast<float>(op._y.Value / length),
                static_cast<float>(op._z.Value / length)};
      }
      return {};
    };

    auto fromAngle = fromOp ? fromOp->_angle : Angle<> {0};
    auto toAngle = toOp->_angle;
    auto fromNormalizedVector = fromOp ? normalizedVector(*fromOp) : FloatPoint3D(0, 0, 1);
    auto toNormalizedVector = normalizedVector(*toOp);
    if (fromAngle.IsZero() || toAngle.IsZero() || fromNormalizedVector == toNormalizedVector)
    {
      auto vector = (fromAngle.IsZero() && !toAngle.IsZero()) ? toNormalizedVector : fromNormalizedVector;
      return RotateTransformFunction::Create(vector.X(), vector.Y(), vector.Z(),
                                             style::Blend(fromAngle, toAngle, context), *outputType);
    }

    // Create the 2 rotation matrices
    TransformationMatrix fromT;
    TransformationMatrix toT;
    fromT.Rotate3d((fromOp ? fromOp->_x.Value : 0), (fromOp ? fromOp->_y.Value : 0),
                   (fromOp ? fromOp->_z.Value : 1), (fromOp ? fromOp->_angle.Value : 0));

    toT.Rotate3d((toOp ? toOp->_x.Value : 0), (toOp ? toOp->_y.Value : 0), (toOp ? toOp->_z.Value : 1),
                 (toOp ? toOp->_angle.Value : 0));

    // Blend them
    toT.Blend(fromT, context.Progress, context.CompositeOperation);

    // Extract the result as a quaternion
    TransformationMatrix::Decomposed4Type decomposed;
    if (!toT.Decompose4(decomposed))
    {
      const auto *usedOperation = context.Progress > 0.5 ? this : fromOp;
      return RotateTransformFunction::Create(usedOperation->X(), usedOperation->Y(), usedOperation->Z(),
                                             usedOperation->GetAngle(),
                                             TransformFunctionBase::Type::Rotate3D);
    }

    // Convert that to Axis/Angle form
    double x = decomposed.Quaternion.X;
    double y = decomposed.Quaternion.Y;
    double z = decomposed.Quaternion.Z;
    double length = std::hypot(x, y, z);
    double angle = 0;

    if (length > 0.00001)
    {
      x /= length;
      y /= length;
      z /= length;
      angle = rad2deg(std::acos(decomposed.Quaternion.W) * 2);
    }
    else
    {
      x = 0;
      y = 0;
      z = 1;
    }
    return RotateTransformFunction::Create(x, y, z, angle, Type::Rotate3D);
  }

}
