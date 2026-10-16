#pragma once

#include "Krystal.Booey/Style/Computed/ComputedStyle.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  class FloatPoint;
  class FloatPoint3D;
  class FloatRect;
  class RenderStyle;
  class TransformationMatrix;
  struct TransformOperationData;
}

namespace krys::boo::style
{
  enum class TransformResolverOption : uint8
  {
    TransformOrigin = 1 << 0,
    Translate = 1 << 1,
    Rotate = 1 << 2,
    Scale = 1 << 3,
    Offset = 1 << 4
  };

  /// @brief Computes the transformation matrix of a style's transform properties.
  class TransformResolver
  {
  public:
    using Option = TransformResolverOption;

  private:
    TransformationMatrix &_transform;
    const ComputedStyle &_style;

  public:
    static constexpr EnumSet<Option> AllTransformOperations {Option::TransformOrigin, Option::Translate,
                                                             Option::Rotate, Option::Scale, Option::Offset};
    static constexpr EnumSet<Option> IndividualTransformOperations {Option::Translate, Option::Rotate,
                                                                    Option::Scale, Option::Offset};

    explicit TransformResolver(TransformationMatrix &, const RenderStyle &) noexcept;
    explicit TransformResolver(TransformationMatrix &, const ComputedStyle &) noexcept;

    KRYS_NODISCARD static bool AffectedByTransformOrigin(const RenderStyle &) noexcept;
    KRYS_NODISCARD static bool AffectedByTransformOrigin(const ComputedStyle &) noexcept;
    KRYS_NODISCARD bool AffectedByTransformOrigin() const noexcept;

    KRYS_NODISCARD static FloatPoint3D ComputeTransformOrigin(const RenderStyle &,
                                                              const FloatRect &boundingBox) noexcept;
    KRYS_NODISCARD static FloatPoint3D ComputeTransformOrigin(const ComputedStyle &,
                                                              const FloatRect &boundingBox) noexcept;
    KRYS_NODISCARD FloatPoint3D ComputeTransformOrigin(const FloatRect &boundingBox) const noexcept;

    KRYS_NODISCARD static FloatPoint ComputePerspectiveOrigin(const RenderStyle &,
                                                              const FloatRect &boundingBox) noexcept;
    KRYS_NODISCARD static FloatPoint ComputePerspectiveOrigin(const ComputedStyle &,
                                                              const FloatRect &boundingBox) noexcept;
    KRYS_NODISCARD FloatPoint ComputePerspectiveOrigin(const FloatRect &boundingBox) const noexcept;

    void ApplyPerspective(const FloatPoint &originTranslate) noexcept;

    void ApplyTransformOrigin(const FloatPoint3D &originTranslate) noexcept;
    void UnapplyTransformOrigin(const FloatPoint3D &originTranslate) noexcept;

    void ApplyCSSTransform(const TransformOperationData &, EnumSet<Option> = AllTransformOperations) noexcept;

    // `ApplyTransform`/`ComputeTransform` perform the following operations in order:
    //    1. ApplyTransformOrigin()
    //    2. ApplyCSSTransform()
    //    3. UnapplyTransformOrigin()

    void ApplyTransform(const TransformOperationData &, EnumSet<Option> = AllTransformOperations) noexcept;

    static void ApplyTransform(TransformationMatrix &, const ComputedStyle &, const TransformOperationData &,
                               EnumSet<Option> = AllTransformOperations) noexcept;
    static void ApplyTransform(TransformationMatrix &, const RenderStyle &, const TransformOperationData &,
                               EnumSet<Option> = AllTransformOperations) noexcept;

    KRYS_NODISCARD static TransformationMatrix
      ComputeTransform(const ComputedStyle &, const TransformOperationData &,
                       EnumSet<Option> = AllTransformOperations) noexcept;
    KRYS_NODISCARD static TransformationMatrix
      ComputeTransform(const RenderStyle &, const TransformOperationData &,
                       EnumSet<Option> = AllTransformOperations) noexcept;

  private:
    void ApplyMotionPathTransform(const TransformOperationData &) noexcept;
  };
}
