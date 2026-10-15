#include "Krystal.Booey/Style/Values/Transforms/Functions/Matrix3DTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/Matrix3DTransformOperation.hpp"

#include <algorithm>

namespace krys::boo::style
{
  Matrix3DTransformFunction::Matrix3DTransformFunction(const TransformationMatrix &matrix) noexcept
      : TransformFunctionBase(TransformFunctionBase::Type::Matrix3D), _matrix(matrix)
  {
  }

  Ref<const Matrix3DTransformFunction>
    Matrix3DTransformFunction::Create(const TransformationMatrix &matrix) noexcept
  {
    return AdoptRef(*new Matrix3DTransformFunction(matrix));
  }

  Ref<const TransformFunctionBase> Matrix3DTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new Matrix3DTransformFunction(_matrix));
  }

  Ref<TransformOperation> Matrix3DTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return Matrix3DTransformOperation::Create(_matrix);
  }

  bool Matrix3DTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherMatrix3D = Downcast<Matrix3DTransformFunction>(other);
    return _matrix == otherMatrix3D._matrix;
  }

  void Matrix3DTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    transform.Multiply(_matrix);
  }

  Ref<const TransformFunctionBase> Matrix3DTransformFunction::Blend(const TransformFunctionBase *from,
                                                                    const BlendingContext &context,
                                                                    bool blendToIdentity) const noexcept
  {
    auto createOperation =
      [](TransformationMatrix &to, TransformationMatrix &from, const BlendingContext &context)
    {
      to.Blend(from, context.Progress, context.CompositeOperation);
      return Matrix3DTransformFunction::Create(to);
    };

    if (!SharedPrimitiveType(from))
    {
      return ShareRef(*this);
    }

    // Convert the TransformFunctions into matrices
    TransformationMatrix toT = Matrix();
    TransformationMatrix fromT;
    if (from)
    {
      fromT = Downcast<Matrix3DTransformFunction>(*from).Matrix();
    }

    if (blendToIdentity)
    {
      return createOperation(fromT, toT, context);
    }
    return createOperation(toT, fromT, context);
  }

}
