#include "Krystal.Booey/Graphics/Transforms/ScaleTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"

namespace krys::boo
{
  Ref<ScaleTransformOperation> ScaleTransformOperation::Create(double sx, double sy, double sz,
                                                               TransformOperation::Type type) noexcept
  {
    return AdoptRef(*new ScaleTransformOperation(sx, sy, sz, type));
  }

  ScaleTransformOperation::ScaleTransformOperation(double sx, double sy, double sz,
                                                   TransformOperation::Type type) noexcept
      : TransformOperation(type), _x(sx), _y(sy), _z(sz)
  {
    krys_assert(IsScaleTransformOperationType(type));
  }

  bool ScaleTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const ScaleTransformOperation &s = Downcast<ScaleTransformOperation>(other);
    return _x == s._x && _y == s._y && _z == s._z;
  }

  KRYS_NODISCARD static double BlendScaleComponent(double from, double to,
                                                   const BlendingContext &context) noexcept
  {
    switch (context.CompositeOperation)
    {
      case CompositeOperation::Replace:    return boo::Blend(from, to, context);
      case CompositeOperation::Add:        krys_debug_assert(context.Progress == 1.0); return from * to;
      case CompositeOperation::Accumulate: krys_debug_assert(context.Progress == 1.0); return from + to - 1;
    }
    krys_unreachable();
  }

  Ref<TransformOperation> ScaleTransformOperation::Blend(const TransformOperation *from,
                                                         const BlendingContext &context,
                                                         bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return ScaleTransformOperation::Create(BlendScaleComponent(_x, 1.0, context),
                                             BlendScaleComponent(_y, 1.0, context),
                                             BlendScaleComponent(_z, 1.0, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(const_cast<ScaleTransformOperation &>(*this));
    }

    const ScaleTransformOperation *fromOp = Downcast<ScaleTransformOperation>(from);
    double fromX = fromOp ? fromOp->_x : 1.0;
    double fromY = fromOp ? fromOp->_y : 1.0;
    double fromZ = fromOp ? fromOp->_z : 1.0;
    return ScaleTransformOperation::Create(BlendScaleComponent(fromX, _x, context),
                                           BlendScaleComponent(fromY, _y, context),
                                           BlendScaleComponent(fromZ, _z, context), *outputType);
  }

}
