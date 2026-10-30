#include "Krystal.Booey/Graphics/Transforms/IdentityTransformOperation.hpp"

namespace krys::boo
{
  Ref<IdentityTransformOperation> IdentityTransformOperation::Create() noexcept
  {
    return AdoptRef(*new IdentityTransformOperation());
  }

  IdentityTransformOperation::IdentityTransformOperation() noexcept
      : TransformOperation(TransformOperation::Type::Identity)
  {
  }
}
