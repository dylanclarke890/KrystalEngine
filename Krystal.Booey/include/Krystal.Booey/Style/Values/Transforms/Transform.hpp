#pragma once

#include "Krystal.Booey/Graphics/Transforms/TransformOperations.hpp"
#include "Krystal.Booey/Style/Values/Transforms/TransformList.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo
{

  namespace style
  {
    // <'transform'> = none | <transform-list>
    // https://drafts.csswg.org/css-transforms-1/#propdef-transform
    struct Transform : css::ListOrNone<TransformList>
    {
      friend struct Blending<Transform>;

      using css::ListOrNone<TransformList>::ListOrNone;

      // Convenience constructors for creating a `transform` directly from transform functions.
      Transform(std::initializer_list<TransformFunction>) noexcept;
      Transform(TransformFunction &&) noexcept;

      template <TransformFunctionType operationType>
      KRYS_NODISCARD bool HasTransformOfType() const noexcept;

      void Apply(TransformationMatrix &, const FloatSize &, size_t start = 0) const noexcept;

      // Return true if any of the operation types are 3D operation types (even if the
      // values describe affine transforms)
      KRYS_NODISCARD bool Has3DOperation() const noexcept;
      KRYS_NODISCARD bool IsRepresentableIn2D() const noexcept;
      KRYS_NODISCARD bool AffectedByTransformOrigin() const noexcept;
      KRYS_NODISCARD bool ContainsNonInvertibleMatrix(const LayoutSize &) const noexcept;

      KRYS_NODISCARD TransformFunctionSizeDependencies ComputeSizeDependencies() const noexcept;
    };

    inline Transform::Transform(std::initializer_list<TransformFunction> transformFunctions) noexcept
        : Transform {TransformList {transformFunctions}}
    {
    }

    inline Transform::Transform(TransformFunction &&transformFunction) noexcept
        : Transform {TransformList {krys::move(transformFunction)}}
    {
    }

    template <TransformFunctionType operationType>
    bool Transform::HasTransformOfType() const noexcept
    {
      return _value.template HasTransformOfType<operationType>();
    }

    inline void Transform::Apply(TransformationMatrix &matrix, const FloatSize &size,
                                 size_t start) const noexcept
    {
      _value.Apply(matrix, size, start);
    }

    inline bool Transform::Has3DOperation() const noexcept
    {
      return _value.Has3DOperation();
    }

    inline bool Transform::IsRepresentableIn2D() const noexcept
    {
      return _value.IsRepresentableIn2D();
    }

    inline bool Transform::AffectedByTransformOrigin() const noexcept
    {
      return _value.AffectedByTransformOrigin();
    }

    inline bool Transform::ContainsNonInvertibleMatrix(const LayoutSize &size) const noexcept
    {
      return _value.ContainsNonInvertibleMatrix(size);
    }

    inline TransformFunctionSizeDependencies Transform::ComputeSizeDependencies() const noexcept
    {
      return _value.ComputeSizeDependencies();
    }

    // MARK: - Conversion

    template <>
    struct CSSValueConversion<Transform>
    {
      KRYS_NODISCARD Transform operator()(BuilderState &, const css::Value &) noexcept;
    };
    template <>
    struct CSSValueCreation<Transform>
    {
      KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                                const Transform &) noexcept;
    };

    // MARK: - Blending

    template <>
    struct Blending<Transform>
    {
      KRYS_NODISCARD bool CanBlend(const Transform &, const Transform &, CompositeOperation) noexcept;
      KRYS_NODISCARD constexpr bool RequiresInterpolationForAccumulativeIteration(const Transform &,
                                                                                  const Transform &) noexcept
      {
        return true;
      }
      KRYS_NODISCARD Transform Blend(const Transform &, const Transform &, const BlendingContext &) noexcept;
    };

    // MARK: - Platform

    template <>
    struct ToPlatform<Transform>
    {
      KRYS_NODISCARD TransformOperations operator()(const Transform &, const FloatSize &) noexcept;
    };

  }
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::Transform)
