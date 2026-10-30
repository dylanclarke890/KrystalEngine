#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class TranslateTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<TranslateTransformOperation> Create(float tx, float ty,
                                                                  TransformOperation::Type type) noexcept
    {
      return AdoptRef(*new TranslateTransformOperation(tx, ty, 0, type));
    }

    KRYS_NODISCARD static Ref<TranslateTransformOperation> Create(float, float, float,
                                                                  TransformOperation::Type) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new TranslateTransformOperation(_x, _y, _z, GetType()));
    }

    KRYS_NODISCARD float X() const noexcept
    {
      return _x;
    }
    KRYS_NODISCARD float Y() const noexcept
    {
      return _y;
    }
    KRYS_NODISCARD float Z() const noexcept
    {
      return _z;
    }

    KRYS_NODISCARD TransformOperation::Type PrimitiveType() const noexcept final
    {
      return !_z ? Type::Translate : Type::Translate3D;
    }

    void Apply(TransformationMatrix &transform) const noexcept final
    {
      transform.Translate3d(_x, _y, _z);
    }

    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept final;

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept final;

  private:
    TranslateTransformOperation(float, float, float, TransformOperation::Type) noexcept;

    float _x;
    float _y;
    float _z;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(
  krys::boo::TranslateTransformOperation, krys::boo::TransformOperation::IsTranslateTransformOperationType)
