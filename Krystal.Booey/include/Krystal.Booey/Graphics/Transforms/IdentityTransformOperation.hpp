#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo
{
  struct BlendingContext;

  class IdentityTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<IdentityTransformOperation> Create() noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return Create();
    }

  private:
    KRYS_NODISCARD bool Equals(const TransformOperation &o) const noexcept override
    {
      return IsSameType(o);
    }

    void Apply(TransformationMatrix &) const noexcept override
    {
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *, const BlendingContext &,
                                                 bool = false) const noexcept override
    {
      return ShareRef(const_cast<IdentityTransformOperation &>(*this));
    }

    IdentityTransformOperation() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::IdentityTransformOperation,
                                                krys::boo::TransformOperation::Type::Identity ==)
