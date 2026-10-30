#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // skew() = skew( [ <angle> | <zero> ] , [ <angle> | <zero> ]? )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-skew
  // skewX() = skewX( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-skew
  // skewY() = skewY( [ <angle> | <zero> ] )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-skewy

  class SkewTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const SkewTransformFunction> Create(Angle<>, Angle<>,
                                                                  TransformFunctionBase::Type) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD Angle<> AngleX() const noexcept
    {
      return _angleX;
    }
    KRYS_NODISCARD Angle<> AngleY() const noexcept
    {
      return _angleY;
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _angleX.IsZero() && _angleY.IsZero();
    }
    KRYS_NODISCARD bool IsAffectedByTransformOrigin() const noexcept override
    {
      return !IsIdentity();
    }

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    SkewTransformFunction(Angle<>, Angle<>, TransformFunctionBase::Type) noexcept;

    Angle<> _angleX;
    Angle<> _angleY;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(
  krys::boo::style::SkewTransformFunction,
  krys::boo::style::TransformFunctionBase::IsSkewTransformFunctionType)
