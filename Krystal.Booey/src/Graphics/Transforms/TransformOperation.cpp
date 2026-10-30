#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"

#include "Krystal.Booey/Graphics/Transforms/IdentityTransformOperation.hpp"

namespace krys::boo
{

  Maybe<TransformOperation::Type> TransformOperation::SharedPrimitiveType(Type other) const noexcept
  {
    // https://drafts.csswg.org/css-transforms-2/#interpolation-of-transform-functions
    // "If both transform functions share a primitive in the two-dimensional space, both transform
    // functions get converted to the two-dimensional primitive. If one or both transform functions
    // are three-dimensional transform functions, the common three-dimensional primitive is used."
    auto type = PrimitiveType();
    if (type == other)
    {
      return type;
    }
    static constexpr std::array sharedPrimitives {std::array {Type::Rotate, Type::Rotate3D},
                                                  std::array {Type::Scale, Type::Scale3D},
                                                  std::array {Type::Translate, Type::Translate3D}};
    for (auto typePair : sharedPrimitives)
    {
      if ((type == typePair[0] || type == typePair[1]) && (other == typePair[0] || other == typePair[1]))
      {
        return typePair[1];
      }
    }
    return null;
  }

  Maybe<TransformOperation::Type>
    TransformOperation::SharedPrimitiveType(const TransformOperation *other) const noexcept
  {
    // Blending with a null operation is always supported via blending with identity.
    if (!other)
    {
      return GetType();
    }

    // In case we have the same type, make sure to preserve it.
    if (other->GetType() == GetType())
    {
      return GetType();
    }

    return SharedPrimitiveType(other->PrimitiveType());
  }
}
