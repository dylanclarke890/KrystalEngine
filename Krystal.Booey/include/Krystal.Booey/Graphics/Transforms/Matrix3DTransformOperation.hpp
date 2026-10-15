#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class Matrix3DTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<Matrix3DTransformOperation> Create(const TransformationMatrix &) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new Matrix3DTransformOperation(_matrix));
    }

    KRYS_NODISCARD TransformationMatrix Matrix() const noexcept
    {
      return _matrix;
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      transform.Multiply(_matrix);
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    Matrix3DTransformOperation(const TransformationMatrix &) noexcept;

    TransformationMatrix _matrix;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::Matrix3DTransformOperation,
                                                krys::boo::TransformOperation::Type::Matrix3D ==)
