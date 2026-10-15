#include "Krystal.Booey/Graphics/Transforms/RotateTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Core/Maths/Base.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<RotateTransformOperation> RotateTransformOperation::Create(double x, double y, double z, double angle,
                                                                 TransformOperation::Type type) noexcept
  {
    return AdoptRef(*new RotateTransformOperation(x, y, z, angle, type));
  }

  RotateTransformOperation::RotateTransformOperation(double x, double y, double z, double angle,
                                                     TransformOperation::Type type) noexcept
      : TransformOperation(type), _x(x), _y(y), _z(z), _angle(angle)
  {
    krys_assert(IsRotateTransformOperationType(type));
  }

  bool RotateTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const RotateTransformOperation &r = Downcast<RotateTransformOperation>(other);
    return _angle == r._angle && _x == r._x && _y == r._y && _z == r._z;
  }

  Ref<TransformOperation> RotateTransformOperation::Blend(const TransformOperation *from,
                                                          const BlendingContext &context,
                                                          bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      if (context.CompositeOperation == CompositeOperation::Accumulate)
      {
        return RotateTransformOperation::Create(_x, _y, _z, _angle, GetType());
      }
      return RotateTransformOperation::Create(_x, _y, _z, _angle - _angle * context.Progress, GetType());
    }
    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(const_cast<RotateTransformOperation &>(*this));
    }

    const RotateTransformOperation *fromOp = Downcast<RotateTransformOperation>(from);
    const RotateTransformOperation *toOp = this;

    // Interpolation of primitives and derived transform functions
    //
    // https://drafts.csswg.org/css-transforms-2/#interpolation-of-transform-functions
    //
    // For interpolations with the primitive rotate3d(), the direction vectors of the transform functions get
    // normalized first. If the normalized vectors are not equal and both rotation angles are non-zero the
    // transform functions get converted into 4x4 matrices first and interpolated as defined in section
    // Interpolation of Matrices afterwards. Otherwise the rotation angle gets interpolated numerically and
    // the rotation vector of the non-zero angle is used or (0, 0, 1) if both angles are zero.

    auto normalizedVector = [](const RotateTransformOperation &op) -> FloatPoint3D
    {
      if (auto length = std::hypot(op._x, op._y, op._z))
      {
        return {static_cast<float>(op._x / length), static_cast<float>(op._y / length),
                static_cast<float>(op._z / length)};
      }
      return {};
    };

    double fromAngle = fromOp ? fromOp->_angle : 0.0;
    double toAngle = toOp->_angle;
    auto fromNormalizedVector = fromOp ? normalizedVector(*fromOp) : FloatPoint3D(0, 0, 1);
    auto toNormalizedVector = normalizedVector(*toOp);
    if (!fromAngle || !toAngle || fromNormalizedVector == toNormalizedVector)
    {
      auto vector = (!fromAngle && toAngle) ? toNormalizedVector : fromNormalizedVector;
      return RotateTransformOperation::Create(vector.X(), vector.Y(), vector.Z(),
                                              boo::Blend(fromAngle, toAngle, context), *outputType);
    }

    // Create the 2 rotation matrices
    TransformationMatrix fromT;
    TransformationMatrix toT;
    fromT.Rotate3d((fromOp ? fromOp->_x : 0), (fromOp ? fromOp->_y : 0), (fromOp ? fromOp->_z : 1),
                   (fromOp ? fromOp->_angle : 0));

    toT.Rotate3d((toOp ? toOp->_x : 0), (toOp ? toOp->_y : 0), (toOp ? toOp->_z : 1),
                 (toOp ? toOp->_angle : 0));

    // Blend them
    toT.Blend(fromT, context.Progress, context.CompositeOperation);

    // Extract the result as a quaternion
    TransformationMatrix::Decomposed4Type decomp;
    if (!toT.Decompose4(decomp))
    {
      const RotateTransformOperation *usedOperation = context.Progress > 0.5 ? this : fromOp;
      return RotateTransformOperation::Create(usedOperation->X(), usedOperation->Y(), usedOperation->Z(),
                                              usedOperation->GetAngle(), TransformOperation::Type::Rotate3D);
    }

    // Convert that to Axis/Angle form
    double x = decomp.Quaternion.X;
    double y = decomp.Quaternion.Y;
    double z = decomp.Quaternion.Z;
    double length = std::hypot(x, y, z);
    double angle = 0;

    if (length > 0.00001)
    {
      x /= length;
      y /= length;
      z /= length;
      angle = rad2deg(std::acos(decomp.Quaternion.W) * 2);
    }
    else
    {
      x = 0;
      y = 0;
      z = 1;
    }
    return RotateTransformOperation::Create(x, y, z, angle, Type::Rotate3D);
  }

}
