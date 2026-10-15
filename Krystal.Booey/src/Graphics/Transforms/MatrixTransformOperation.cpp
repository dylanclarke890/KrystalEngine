#include "Krystal.Booey/Graphics/Transforms/MatrixTransformOperation.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include <algorithm>

namespace krys::boo
{
  Ref<MatrixTransformOperation> MatrixTransformOperation::Create(const TransformationMatrix &t) noexcept
  {
    return AdoptRef(*new MatrixTransformOperation(t));
  }

  MatrixTransformOperation::MatrixTransformOperation(const TransformationMatrix &t) noexcept
      : TransformOperation(TransformOperation::Type::Matrix), _a(t.A()), _b(t.B()), _c(t.C()), _d(t.D()),
        _e(t.E()), _f(t.F())
  {
  }

  bool MatrixTransformOperation::Equals(const TransformOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const MatrixTransformOperation &m = Downcast<MatrixTransformOperation>(other);
    return _a == m._a && _b == m._b && _c == m._c && _d == m._d && _e == m._e && _f == m._f;
  }

  Ref<TransformOperation> MatrixTransformOperation::Blend(const TransformOperation *from,
                                                          const BlendingContext &context,
                                                          bool blendToIdentity) const noexcept
  {
    auto createOperation =
      [](TransformationMatrix &to, TransformationMatrix &from, const BlendingContext &context)
    {
      to.Blend(from, context.Progress, context.CompositeOperation);
      return MatrixTransformOperation::Create(to);
    };

    if (!SharedPrimitiveType(from))
    {
      return ShareRef(const_cast<MatrixTransformOperation &>(*this));
    }

    // convert the TransformOperations into matrices
    TransformationMatrix fromT;
    TransformationMatrix toT(_a, _b, _c, _d, _e, _f);
    if (from)
    {
      const MatrixTransformOperation &m = Downcast<MatrixTransformOperation>(*from);
      fromT.SetMatrix(m._a, m._b, m._c, m._d, m._e, m._f);
    }

    if (blendToIdentity)
    {
      return createOperation(fromT, toT, context);
    }
    return createOperation(toT, fromT, context);
  }

}
