#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class SkewTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<SkewTransformOperation> Create(double, double,
                                                             TransformOperation::Type) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new SkewTransformOperation(_angleX, _angleY, GetType()));
    }

    KRYS_NODISCARD double AngleX() const noexcept
    {
      return _angleX;
    }
    KRYS_NODISCARD double AngleY() const noexcept
    {
      return _angleY;
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      transform.Skew(_angleX, _angleY);
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    SkewTransformOperation(double, double, TransformOperation::Type) noexcept;

    double _angleX;
    double _angleY;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::SkewTransformOperation,
                                                krys::boo::TransformOperation::IsSkewTransformOperationType)
