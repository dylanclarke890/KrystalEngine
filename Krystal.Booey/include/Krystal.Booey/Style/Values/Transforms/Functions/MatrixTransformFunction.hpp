#pragma once

#include "Krystal.Booey/Style/Values/Primitives/NumericTypes.hpp"
#include "Krystal.Booey/Style/Values/Transforms/Functions/TransformFunctionBase.hpp"

namespace krys::boo::style
{
  // matrix() = matrix( <number>#{6} )
  // https://drafts.csswg.org/css-transforms/#funcdef-transform-matrix

  class MatrixTransformFunction final : public TransformFunctionBase
  {
  public:
    KRYS_NODISCARD static Ref<const MatrixTransformFunction> CreateIdentity() noexcept;
    KRYS_NODISCARD static Ref<const MatrixTransformFunction> Create(Number<>, Number<>, Number<>, Number<>,
                                                                    Number<>, Number<>) noexcept;
    KRYS_NODISCARD static Ref<const MatrixTransformFunction> Create(const TransformationMatrix &) noexcept;

    KRYS_NODISCARD Ref<const TransformFunctionBase> Clone() const noexcept override;
    KRYS_NODISCARD Ref<TransformOperation> ToPlatform(const FloatSize &) const noexcept override;

    KRYS_NODISCARD TransformationMatrix Matrix() const noexcept
    {
      return TransformationMatrix(_a.Value, _b.Value, _c.Value, _d.Value, _e.Value, _f.Value);
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override
    {
      return _a == 1 && _b == 0 && _c == 0 && _d == 1 && _e == 0 && _f == 0;
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
    MatrixTransformFunction(Number<>, Number<>, Number<>, Number<>, Number<>, Number<>) noexcept;
    MatrixTransformFunction(const TransformationMatrix &) noexcept;

    Number<> _a;
    Number<> _b;
    Number<> _c;
    Number<> _d;
    Number<> _e;
    Number<> _f;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_STYLE_TRANSFORM_FUNCTION(krys::boo::style::MatrixTransformFunction,
                                                     krys::boo::style::TransformFunctionBase::Type::Matrix ==)
