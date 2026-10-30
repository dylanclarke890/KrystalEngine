#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Perspective.hpp"

namespace krys::boo::style
{
  // perspective() = perspective( [ <length [0,∞]> | none ] )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-perspective

  class PerspectiveTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const PerspectiveTransformFunction> Create(Perspective) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD Perspective GetPerspective() const noexcept
    {
      return _p;
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _p.IsNone();
    }
    KRYS_NODISCARD bool IsAffectedByTransformOrigin() const noexcept override
    {
      return !IsIdentity();
    }
    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept override
    {
      return false;
    }

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    PerspectiveTransformFunction(Perspective) noexcept;

    KRYS_NODISCARD Maybe<float> UnresolvedFloatValue() const noexcept;

    Perspective _p;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(
  krys::boo::style::PerspectiveTransformFunction,
  krys::boo::style::TransformFunctionBase::Type::Perspective ==)
