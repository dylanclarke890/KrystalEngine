#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class ScaleTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<ScaleTransformOperation> Create(double sx, double sy,
                                                              TransformOperation::Type type) noexcept
    {
      return AdoptRef(*new ScaleTransformOperation(sx, sy, 1, type));
    }

    KRYS_NODISCARD static Ref<ScaleTransformOperation> Create(double, double, double,
                                                              TransformOperation::Type) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept final
    {
      return AdoptRef(*new ScaleTransformOperation(_x, _y, _z, GetType()));
    }

    KRYS_NODISCARD double X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD double Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD double Z() const noexcept
    {
      return _z;
    }

    KRYS_NODISCARD TransformOperation::Type PrimitiveType() const noexcept final
    {
      return (GetType() == Type::ScaleZ || GetType() == Type::Scale3D) ? Type::Scale3D : Type::Scale;
    }

    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept final;

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept final;

    void Apply(TransformationMatrix &transform) const noexcept final
    {
      transform.Scale3d(_x, _y, _z);
    }

  private:
    ScaleTransformOperation(double, double, double, TransformOperation::Type) noexcept;

    double _x;
    double _y;
    double _z;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::ScaleTransformOperation,
                                                krys::boo::TransformOperation::IsScaleTransformOperationType)
