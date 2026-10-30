#pragma once

#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Path.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Rendering/TransformOperationData.hpp"
#include "Krystal.Booey/Style/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/Style/Values/Primitives/Url.hpp"
#include "Krystal.Booey/Style/Values/Shapes/BasicShape.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/TypeCast.hpp"

namespace krys::boo
{
  namespace svg
  {
    class SVGElement;
  }

  struct BlendingContext;

  /// @brief A path given by `clip-path` or `offset-path`: a reference, a basic shape, a box or a ray.
  class PathOperation : public RefCounted<PathOperation>
  {
  public:
    enum class Type : uint8
    {
      Reference,
      Shape,
      Box,
      Ray
    };

  protected:
    const Type _type;
    CSSBoxType _referenceBox;

  public:
    virtual ~PathOperation() noexcept = default;

    KRYS_NODISCARD virtual Ref<PathOperation> Clone() const noexcept = 0;

    KRYS_NODISCARD bool operator==(const PathOperation &other) const noexcept
    {
      return Equals(other);
    }
    KRYS_NODISCARD virtual bool Equals(const PathOperation &) const noexcept = 0;

    KRYS_NODISCARD virtual bool CanBlend(const PathOperation &) const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual RefPtr<PathOperation> Blend(const PathOperation *,
                                                       const BlendingContext &) const noexcept
    {
      return nullptr;
    }
    KRYS_NODISCARD virtual Maybe<Path> GetPath(const TransformOperationData &) const noexcept = 0;

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    void SetReferenceBox(CSSBoxType type) noexcept
    {
      _referenceBox = type;
    }
    KRYS_NODISCARD CSSBoxType ReferenceBox() const noexcept
    {
      return _referenceBox;
    }

    KRYS_NODISCARD bool IsSameType(const PathOperation &o) const noexcept
    {
      return o.GetType() == _type;
    }

  protected:
    explicit PathOperation(Type type, CSSBoxType referenceBox = CSSBoxType::BoxMissing) noexcept
        : _type(type), _referenceBox(referenceBox)
    {
    }
  };

  /// @brief A path taken from an SVG shape element referenced by URL.
  class ReferencePathOperation final : public PathOperation
  {
  private:
    style::URL _url;
    css::CSSOMStringAtom _fragment;
    Maybe<Path> _path;

  public:
    KRYS_NODISCARD static Ref<ReferencePathOperation>
      Create(const style::URL &, const css::CSSOMStringAtom &fragment, const svg::SVGElement *) noexcept;
    KRYS_NODISCARD static Ref<ReferencePathOperation> Create(Maybe<Path> &&) noexcept;

    KRYS_NODISCARD Ref<PathOperation> Clone() const noexcept override;

    KRYS_NODISCARD const style::URL &Url() const noexcept
    {
      return _url;
    }
    KRYS_NODISCARD const css::CSSOMStringAtom &Fragment() const noexcept
    {
      return _fragment;
    }

    KRYS_NODISCARD Maybe<Path> GetPath(const TransformOperationData &) const noexcept override
    {
      return _path;
    }
    KRYS_NODISCARD const Maybe<Path> &GetPath() const noexcept
    {
      return _path;
    }

    KRYS_NODISCARD bool Equals(const PathOperation &other) const noexcept override;

  private:
    ReferencePathOperation(const style::URL &, const css::CSSOMStringAtom &fragment,
                           const svg::SVGElement *) noexcept;
    explicit ReferencePathOperation(Maybe<Path> &&) noexcept;
  };

  /// @brief A path given by a basic shape.
  class ShapePathOperation final : public PathOperation
  {
  private:
    style::BasicShape _shape;

  public:
    KRYS_NODISCARD static Ref<ShapePathOperation> Create(style::BasicShape,
                                                         CSSBoxType = CSSBoxType::BoxMissing) noexcept;

    KRYS_NODISCARD Ref<PathOperation> Clone() const noexcept override;

    KRYS_NODISCARD bool CanBlend(const PathOperation &) const noexcept override;
    KRYS_NODISCARD RefPtr<PathOperation> Blend(const PathOperation *,
                                               const BlendingContext &) const noexcept override;

    KRYS_NODISCARD const style::BasicShape &GetShape() const noexcept
    {
      return _shape;
    }
    KRYS_NODISCARD WindRule GetWindRule() const noexcept
    {
      return style::ComputeWindRule(_shape);
    }
    KRYS_NODISCARD Path PathForReferenceRect(const FloatRect &boundingRect) const noexcept
    {
      return style::ComputePath(_shape, boundingRect);
    }

    KRYS_NODISCARD Maybe<Path> GetPath(const TransformOperationData &) const noexcept override;

    KRYS_NODISCARD bool Equals(const PathOperation &other) const noexcept override;

  private:
    ShapePathOperation(style::BasicShape &&shape, CSSBoxType referenceBox) noexcept
        : PathOperation(Type::Shape, referenceBox), _shape(krys::move(shape))
    {
    }
  };

  /// @brief A path following a reference box.
  class BoxPathOperation final : public PathOperation
  {
  public:
    KRYS_NODISCARD static Ref<BoxPathOperation> Create(CSSBoxType) noexcept;

    KRYS_NODISCARD Ref<PathOperation> Clone() const noexcept override;

    KRYS_NODISCARD Maybe<Path> GetPath(const TransformOperationData &) const noexcept override;

    KRYS_NODISCARD bool Equals(const PathOperation &other) const noexcept override
    {
      return IsSameType(other) && ReferenceBox() == other.ReferenceBox();
    }

  private:
    explicit BoxPathOperation(CSSBoxType referenceBox) noexcept : PathOperation(Type::Box, referenceBox)
    {
    }
  };

  /// @brief A path given by `ray()`.
  class RayPathOperation final : public PathOperation
  {
  private:
    style::RayFunction _ray;

  public:
    KRYS_NODISCARD static Ref<RayPathOperation> Create(style::RayFunction &&,
                                                       CSSBoxType = CSSBoxType::BoxMissing) noexcept;
    KRYS_NODISCARD static Ref<RayPathOperation> Create(const style::RayFunction &,
                                                       CSSBoxType = CSSBoxType::BoxMissing) noexcept;

    KRYS_NODISCARD Ref<PathOperation> Clone() const noexcept override;

    KRYS_NODISCARD const style::RayFunction &Ray() const noexcept
    {
      return _ray;
    }

    KRYS_NODISCARD bool CanBlend(const PathOperation &) const noexcept override;
    KRYS_NODISCARD RefPtr<PathOperation> Blend(const PathOperation *,
                                               const BlendingContext &) const noexcept override;

    KRYS_NODISCARD Maybe<Path> GetPath(const TransformOperationData &) const noexcept override;

    KRYS_NODISCARD bool Equals(const PathOperation &other) const noexcept override;

  private:
    RayPathOperation(style::RayFunction ray, CSSBoxType referenceBox) noexcept
        : PathOperation(Type::Ray, referenceBox), _ray(krys::move(ray))
    {
    }
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_PATH_OPERATION(ToValueTypeName, predicate)                               \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::ToValueTypeName)                                         \
    KRYS_NODISCARD static bool IsType(const krys::boo::PathOperation &operation) noexcept                    \
    {                                                                                                        \
      return operation.GetType() == krys::boo::predicate;                                                    \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_TRAITS_PATH_OPERATION(ReferencePathOperation, PathOperation::Type::Reference)
KRYS_SPECIALIZE_TYPE_TRAITS_PATH_OPERATION(ShapePathOperation, PathOperation::Type::Shape)
KRYS_SPECIALIZE_TYPE_TRAITS_PATH_OPERATION(BoxPathOperation, PathOperation::Type::Box)
KRYS_SPECIALIZE_TYPE_TRAITS_PATH_OPERATION(RayPathOperation, PathOperation::Type::Ray)
