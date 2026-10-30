#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class RotateTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<RotateTransformOperation> Create(double angle,
                                                               TransformOperation::Type type) noexcept
    {
      return AdoptRef(*new RotateTransformOperation(0, 0, 1, angle, type));
    }

    KRYS_NODISCARD static Ref<RotateTransformOperation> Create(double, double, double, double,
                                                               TransformOperation::Type) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept final
    {
      return AdoptRef(*new RotateTransformOperation(_x, _y, _z, _angle, GetType()));
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
    KRYS_NODISCARD double GetAngle() const noexcept
    {
      return _angle;
    }

    KRYS_NODISCARD TransformOperation::Type PrimitiveType() const noexcept final
    {
      return GetType() == Type::Rotate ? Type::Rotate : Type::Rotate3D;
    }

    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept final;

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept final;

    void Apply(TransformationMatrix &transform) const noexcept final
    {
      if (GetType() == TransformOperation::Type::Rotate)
      {
        transform.Rotate(_angle);
      }
      else
      {
        transform.Rotate3d(_x, _y, _z, _angle);
      }
    }

    void ApplyUnrounded(TransformationMatrix &transform) const noexcept final
    {
      if (GetType() == TransformOperation::Type::Rotate)
      {
        transform.Rotate(_angle, TransformationMatrix::RotationSnapping::None);
      }
      else
      {
        transform.Rotate3d(_x, _y, _z, _angle, TransformationMatrix::RotationSnapping::None);
      }
    }

  private:
    RotateTransformOperation(double, double, double, double, TransformOperation::Type) noexcept;

    double _x;
    double _y;
    double _z;
    double _angle;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::RotateTransformOperation,
                                                krys::boo::TransformOperation::IsRotateTransformOperationType)
