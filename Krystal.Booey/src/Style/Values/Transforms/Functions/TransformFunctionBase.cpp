#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  Maybe<TransformFunctionBase::Type> TransformFunctionBase::SharedPrimitiveType(Type other) const noexcept
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
    return std::nullopt;
  }

  Maybe<TransformFunctionBase::Type>
    TransformFunctionBase::SharedPrimitiveType(const TransformFunctionBase *other) const noexcept
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

  TransformOperationType ToPlatform<TransformFunctionType>::operator()(TransformFunctionType type) noexcept
  {
    switch (type)
    {
      case TransformFunctionType::ScaleX:      return TransformOperationType::ScaleX;
      case TransformFunctionType::ScaleY:      return TransformOperationType::ScaleY;
      case TransformFunctionType::Scale:       return TransformOperationType::Scale;
      case TransformFunctionType::TranslateX:  return TransformOperationType::TranslateX;
      case TransformFunctionType::TranslateY:  return TransformOperationType::TranslateY;
      case TransformFunctionType::Translate:   return TransformOperationType::Translate;
      case TransformFunctionType::RotateX:     return TransformOperationType::RotateX;
      case TransformFunctionType::RotateY:     return TransformOperationType::RotateY;
      case TransformFunctionType::Rotate:      return TransformOperationType::Rotate;
      case TransformFunctionType::SkewX:       return TransformOperationType::SkewX;
      case TransformFunctionType::SkewY:       return TransformOperationType::SkewY;
      case TransformFunctionType::Skew:        return TransformOperationType::Skew;
      case TransformFunctionType::Matrix:      return TransformOperationType::Matrix;
      case TransformFunctionType::ScaleZ:      return TransformOperationType::ScaleZ;
      case TransformFunctionType::Scale3D:     return TransformOperationType::Scale3D;
      case TransformFunctionType::TranslateZ:  return TransformOperationType::TranslateZ;
      case TransformFunctionType::Translate3D: return TransformOperationType::Translate3D;
      case TransformFunctionType::RotateZ:     return TransformOperationType::RotateZ;
      case TransformFunctionType::Rotate3D:    return TransformOperationType::Rotate3D;
      case TransformFunctionType::Matrix3D:    return TransformOperationType::Matrix3D;
      case TransformFunctionType::Perspective: return TransformOperationType::Perspective;
    }
    krys_unreachable();
  }
}
