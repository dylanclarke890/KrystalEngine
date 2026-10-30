#include "Krystal.Booey/Graphics/Transforms/SkewTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo
{
  Ref<SkewTransformOperation> SkewTransformOperation::Create(double angleX, double angleY,
                                                             TransformOperation::Type type) noexcept
  {
    return AdoptRef(*new SkewTransformOperation(angleX, angleY, type));
  }

  SkewTransformOperation::SkewTransformOperation(double angleX, double angleY,
                                                 TransformOperation::Type type) noexcept
      : TransformOperation(type), _angleX(angleX), _angleY(angleY)
  {
    krys_assert(IsSkewTransformOperationType(type));
  }

  bool SkewTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const SkewTransformOperation &s = Downcast<SkewTransformOperation>(other);
    return _angleX == s._angleX && _angleY == s._angleY;
  }

  Ref<TransformOperation> SkewTransformOperation::Blend(const TransformOperation *from,
                                                        const BlendingContext &context,
                                                        bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return SkewTransformOperation::Create(boo::Blend(_angleX, 0.0, context),
                                            boo::Blend(_angleY, 0.0, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(const_cast<SkewTransformOperation &>(*this));
    }

    const SkewTransformOperation *fromOp = Downcast<SkewTransformOperation>(from);
    double fromAngleX = fromOp ? fromOp->_angleX : 0;
    double fromAngleY = fromOp ? fromOp->_angleY : 0;
    return SkewTransformOperation::Create(boo::Blend(fromAngleX, _angleX, context),
                                          boo::Blend(fromAngleY, _angleY, context), *outputType);
  }

}
