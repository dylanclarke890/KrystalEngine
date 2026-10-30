#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class MatrixTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<MatrixTransformOperation> CreateIdentity() noexcept
    {
      return AdoptRef(*new MatrixTransformOperation(1, 0, 0, 1, 0, 0));
    }

    KRYS_NODISCARD static Ref<MatrixTransformOperation> Create(double a, double b, double c, double d,
                                                               double e, double f) noexcept
    {
      return AdoptRef(*new MatrixTransformOperation(a, b, c, d, e, f));
    }

    KRYS_NODISCARD static Ref<MatrixTransformOperation> Create(const TransformationMatrix &) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new MatrixTransformOperation(Matrix()));
    }

    KRYS_NODISCARD TransformationMatrix Matrix() const noexcept
    {
      return TransformationMatrix(_a, _b, _c, _d, _e, _f);
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      TransformationMatrix matrix(_a, _b, _c, _d, _e, _f);
      transform.Multiply(matrix);
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    MatrixTransformOperation(double a, double b, double c, double d, double e, double f) noexcept
        : TransformOperation(TransformOperation::Type::Matrix), _a(a), _b(b), _c(c), _d(d), _e(e), _f(f)
    {
    }

    MatrixTransformOperation(const TransformationMatrix &) noexcept;

    double _a;
    double _b;
    double _c;
    double _d;
    double _e;
    double _f;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::MatrixTransformOperation,
                                                krys::boo::TransformOperation::Type::Matrix ==)
