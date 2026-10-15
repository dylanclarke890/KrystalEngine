#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformationMatrix.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // matrix3d() = matrix3d( <number>#{16} )
  // https://drafts.csswg.org/css-transforms-2/#funcdef-matrix3d

  class Matrix3DTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const Matrix3DTransformFunction> Create(const TransformationMatrix &) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD TransformationMatrix Matrix() const noexcept
    {
      return _matrix;
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _matrix.IsIdentity();
    }
    KRYS_NODISCARD bool IsAffectedByTransformOrigin() const noexcept override
    {
      return !IsIdentity();
    }
    KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept override
    {
      return _matrix.IsAffine();
    }

    KRYS_NODISCARD bool Equals(const TransformFunctionBase &) const noexcept override;

    void Apply(TransformationMatrix &, const FloatSize &borderBoxSize) const noexcept override;

    KRYS_NODISCARD Ref<const TransformFunctionBase>
      Blend(const TransformFunctionBase *from, const BlendingContext &,
            bool blendToIdentity = false) const noexcept override;

  private:
    Matrix3DTransformFunction(const TransformationMatrix &) noexcept;

    TransformationMatrix _matrix;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(krys::boo::style::Matrix3DTransformFunction,
                                                     krys::boo::style::TransformFunctionBase::Type::Matrix3D
                                                       ==)
