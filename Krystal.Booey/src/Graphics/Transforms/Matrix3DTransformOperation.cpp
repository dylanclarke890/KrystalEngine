#include "Krystal.Booey/Graphics/Transforms/Matrix3DTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<Matrix3DTransformOperation>
    Matrix3DTransformOperation::Create(const TransformationMatrix &matrix) noexcept
  {
    return AdoptRef(*new Matrix3DTransformOperation(matrix));
  }

  Matrix3DTransformOperation::Matrix3DTransformOperation(const TransformationMatrix &mat) noexcept
      : TransformOperation(TransformOperation::Type::Matrix3D), _matrix(mat)
  {
  }

  bool Matrix3DTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    return IsSameType(other) && _matrix == Downcast<Matrix3DTransformOperation>(other)._matrix;
  }

  KRYS_NODISCARD static Ref<TransformOperation> CreateOperation(TransformationMatrix &to,
                                                                TransformationMatrix &from,
                                                                const BlendingContext &context) noexcept
  {
    to.Blend(from, context.Progress, context.CompositeOperation);
    return Matrix3DTransformOperation::Create(to);
  }

  Ref<TransformOperation> Matrix3DTransformOperation::Blend(const TransformOperation *from,
                                                            const BlendingContext &context,
                                                            bool blendToIdentity) const noexcept
  {
    if (!SharedPrimitiveType(from))
    {
      return ShareRef(const_cast<Matrix3DTransformOperation &>(*this));
    }

    // Convert the TransformOperations into matrices
    TransformationMatrix fromT;
    TransformationMatrix toT;
    if (from)
    {
      from->Apply(fromT);
    }

    Apply(toT);

    if (blendToIdentity)
    {
      return CreateOperation(fromT, toT, context);
    }
    return CreateOperation(toT, fromT, context);
  }

}
