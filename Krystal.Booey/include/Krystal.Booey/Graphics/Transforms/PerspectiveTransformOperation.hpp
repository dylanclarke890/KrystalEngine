#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperation.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include <optional>

namespace krys::boo
{
  struct BlendingContext;

  class PerspectiveTransformOperation final : public TransformOperation
  {
  public:
    KRYS_NODISCARD static Ref<PerspectiveTransformOperation> Create(const Maybe<float> &) noexcept;

    KRYS_NODISCARD Ref<TransformOperation> Clone() const noexcept override
    {
      return AdoptRef(*new PerspectiveTransformOperation(_p));
    }

    KRYS_NODISCARD Maybe<float> Perspective() const noexcept
    {
      return _p;
    }

  private:
    KRYS_NODISCARD bool IsIdentity() const noexcept
    {
      return !_p;
    }

    KRYS_NODISCARD bool Equals(const TransformOperation &) const noexcept override;

    KRYS_NODISCARD Maybe<float> FloatValue() const noexcept
    {
      if (!_p)
      {
        return {};
      }

      // From https://www.w3.org/TR/css-transforms-2/#perspective-property:
      // "As very small <length> values can produce bizarre rendering results and stress the numerical
      // accuracy of transform calculations, values less than 1px must be treated as 1px for rendering
      // purposes. (This clamping does not affect the underlying value, so perspective: 0; in a stylesheet
      // will still serialize back as 0.)"
      return std::max(1.0f, *_p);
    }

    void Apply(TransformationMatrix &transform) const noexcept override
    {
      if (auto value = FloatValue())
      {
        transform.ApplyPerspective(*value);
      }
    }

    KRYS_NODISCARD Ref<TransformOperation> Blend(const TransformOperation *from, const BlendingContext &,
                                                 bool blendToIdentity = false) const noexcept override;

    PerspectiveTransformOperation(const Maybe<float> &) noexcept;

    Maybe<float> _p;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_TRANSFORM_OPERATION(krys::boo::PerspectiveTransformOperation,
                                                krys::boo::TransformOperation::Type::Perspective ==)
