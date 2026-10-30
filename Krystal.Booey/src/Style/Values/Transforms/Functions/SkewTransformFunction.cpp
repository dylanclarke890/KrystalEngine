#include "Krystal.Booey/Style/Values/Transforms/Functions/SkewTransformFunction.hpp"

#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Graphics/Transforms/SkewTransformOperation.hpp"

#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo::style
{
  SkewTransformFunction::SkewTransformFunction(Angle<> angleX, Angle<> angleY,
                                               TransformFunctionBase::Type type) noexcept
      : TransformFunctionBase(type), _angleX(angleX), _angleY(angleY)
  {
    krys_assert(IsSkewTransformFunctionType(type));
  }

  Ref<const SkewTransformFunction> SkewTransformFunction::Create(Angle<> angleX, Angle<> angleY,
                                                                 TransformFunctionBase::Type type) noexcept
  {
    return AdoptRef(*new SkewTransformFunction(angleX, angleY, type));
  }

  Ref<const TransformFunctionBase> SkewTransformFunction::Clone() const noexcept
  {
    return AdoptRef(*new SkewTransformFunction(_angleX, _angleY, GetType()));
  }

  Ref<TransformOperation> SkewTransformFunction::ToPlatform(const FloatSize &) const noexcept
  {
    return SkewTransformOperation::Create(_angleX.Value, _angleY.Value, style::ConvertToPlatform(GetType()));
  }

  bool SkewTransformFunction::Equals(const TransformFunctionBase &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }

    auto &otherSkew = Downcast<SkewTransformFunction>(other);
    return _angleX == otherSkew._angleX && _angleY == otherSkew._angleY;
  }

  void SkewTransformFunction::Apply(TransformationMatrix &transform, const FloatSize &) const noexcept
  {
    transform.Skew(_angleX.Value, _angleY.Value);
  }

  Ref<const TransformFunctionBase> SkewTransformFunction::Blend(const TransformFunctionBase *from,
                                                                const BlendingContext &context,
                                                                bool blendToIdentity) const noexcept
  {
    if (blendToIdentity)
    {
      return SkewTransformFunction::Create(style::Blend(_angleX, Angle<> {0.0}, context),
                                           style::Blend(_angleY, Angle<> {0.0}, context), GetType());
    }

    auto outputType = SharedPrimitiveType(from);
    if (!outputType)
    {
      return ShareRef(*this);
    }

    auto *fromOp = Downcast<SkewTransformFunction>(from);
    auto fromAngleX = fromOp ? fromOp->_angleX : Angle<> {0};
    auto fromAngleY = fromOp ? fromOp->_angleY : Angle<> {0};
    return SkewTransformFunction::Create(style::Blend(fromAngleX, _angleX, context),
                                         style::Blend(fromAngleY, _angleY, context), *outputType);
  }

}
