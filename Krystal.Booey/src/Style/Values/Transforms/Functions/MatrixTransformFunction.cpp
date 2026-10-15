#include "Krystal.Booey/Style/Values/Transforms/Functions/MatrixTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/MatrixTransformOperation.hpp"

#include <algorithm>

namespace krys::boo::style
{
  MatrixTransformFunction::MatrixTransformFunction(Number<> a, Number<> b, Number<> c, Number<> d, Number<> e,
                                                   Number<> f) noexcept
      : TransformFunctionBase(TransformFunctionBase::Type::Matrix), _a(a), _b(b), _c(c), _d(d), _e(e), _f(f)
  {
  }

  MatrixTransformFunction::MatrixTransformFunction(const TransformationMatrix &matrix) noexcept
      : MatrixTransformFunction(matrix.A(), matrix.B(), matrix.C(), matrix.D(), matrix.E(), matrix.F())
  {
  }

  Ref<const MatrixTransformFunction> MatrixTransformFunction::CreateIdentity() noexcept
  {
    return AdoptRef(*new MatrixTransformFunction(1, 0, 0, 1, 0, 0));
  }

  Ref<const MatrixTransformFunction> MatrixTransformFunction::Create(Number<> a, Number<> b, Number<> c,
                                                                     Number<> d, Number<> e,
                                                                     Number<> f) noexcept
  {
    return AdoptRef(*new MatrixTransformFunction(a, b, c, d, e, f));
  }

  Ref<const MatrixTransformFunction>
    MatrixTransformFunction::Create(const TransformationMatrix &matrix) noexcept
  {
    return AdoptRef(*new MatrixTransformFunction(matrix));
  }

  Ref<const TransformFunctionBase> MatrixTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new MatrixTransformFunction(_a, _b, _c, _d, _e, _f));
  }

  Ref<TransformOperation> MatrixTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return MatrixTransformOperation::Create(_a.Value, _b.Value, _c.Value, _d.Value, _e.Value, _f.Value);
  }

  bool MatrixTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherMatrix = Downcast<MatrixTransformFunction>(other);
    return _a == otherMatrix._a && _b == otherMatrix._b && _c == otherMatrix._c && _d == otherMatrix._d
           && _e == otherMatrix._e && _f == otherMatrix._f;
  }

  void MatrixTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    transform.Multiply(Matrix());
  }

  Ref<const TransformFunctionBase> MatrixTransformFunction::Blend(const TransformFunctionBase *from,
                                                                  const BlendingContext &context,
                                                                  bool blendToIdentity) const noexcept
  {
    auto createOperation =
      [](TransformationMatrix &to, TransformationMatrix &from, const BlendingContext &context)
    {
      to.Blend(from, context.Progress, context.CompositeOperation);
      return MatrixTransformFunction::Create(to);
    };

    if (!SharedPrimitiveType(from))
    {
      return ShareRef(*this);
    }

    // convert the TransformFunctions into matrices
    TransformationMatrix toT = Matrix();
    TransformationMatrix fromT;
    if (from)
    {
      fromT = Downcast<MatrixTransformFunction>(*from).Matrix();
    }

    if (blendToIdentity)
    {
      return createOperation(fromT, toT, context);
    }
    return createOperation(toT, fromT, context);
  }

}
