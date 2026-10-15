#include "Krystal.Booey/Rendering/PathOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/AnimationUtilities.hpp"
#include "Krystal.Booey/Rendering/MotionPath.hpp"
#include "Krystal.Booey/Style/Values/Primitives/LengthWrapperBlending.hpp"
#include "Krystal.Booey/Style/Values/Primitives/NumericTypesBlending.hpp"

namespace krys::boo
{
  // MARK: - ReferencePathOperation

  Ref<ReferencePathOperation> ReferencePathOperation::Create(const style::URL &url,
                                                             const css::CSSOMStringAtom &fragment,
                                                             const svg::SVGElement *element) noexcept
  {
    return AdoptRef(*new ReferencePathOperation(url, fragment, element));
  }

  Ref<ReferencePathOperation> ReferencePathOperation::Create(Maybe<Path> &&path) noexcept
  {
    return AdoptRef(*new ReferencePathOperation(krys::move(path)));
  }

  Ref<PathOperation> ReferencePathOperation::Clone() const noexcept
  {
    auto path = _path;
    return AdoptRef(*new ReferencePathOperation(krys::move(path)));
  }

  bool ReferencePathOperation::Equals(const PathOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    return _url == Downcast<ReferencePathOperation>(other)._url;
  }

  ReferencePathOperation::ReferencePathOperation(const style::URL &url, const css::CSSOMStringAtom &fragment,
                                                 const svg::SVGElement *) noexcept
      : PathOperation(Type::Reference), _url(url), _fragment(fragment)
  {
    // FIXME: Take the path from the element when it's an SVG path or geometry element
    // (`pathFromGraphicsElement`), once SVG geometry is ported.
  }

  ReferencePathOperation::ReferencePathOperation(Maybe<Path> &&path) noexcept
      : PathOperation(Type::Reference), _fragment(StringAtom::Null()), _path(krys::move(path))
  {
  }

  // MARK: - ShapePathOperation

  Ref<ShapePathOperation> ShapePathOperation::Create(style::BasicShape shape,
                                                     CSSBoxType referenceBox) noexcept
  {
    return AdoptRef(*new ShapePathOperation(krys::move(shape), referenceBox));
  }

  Ref<PathOperation> ShapePathOperation::Clone() const noexcept
  {
    auto shape = _shape;
    return AdoptRef(*new ShapePathOperation(krys::move(shape), _referenceBox));
  }

  bool ShapePathOperation::CanBlend(const PathOperation &to) const noexcept
  {
    const auto *toOperation = DynamicDowncast<ShapePathOperation>(to);
    return toOperation && style::CanBlend(_shape, toOperation->_shape);
  }

  RefPtr<PathOperation> ShapePathOperation::Blend(const PathOperation *to,
                                                  const BlendingContext &context) const noexcept
  {
    const auto &toShapePathOperation = Downcast<ShapePathOperation>(*to);
    return ShapePathOperation::Create(style::Blend(_shape, toShapePathOperation._shape, context));
  }

  Maybe<Path> ShapePathOperation::GetPath(const TransformOperationData &data) const noexcept
  {
    return MotionPath::ComputePathForShape(*this, data);
  }

  bool ShapePathOperation::Equals(const PathOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    const auto &shapeOther = Downcast<ShapePathOperation>(other);
    return _shape == shapeOther._shape && _referenceBox == shapeOther._referenceBox;
  }

  // MARK: - BoxPathOperation

  Ref<BoxPathOperation> BoxPathOperation::Create(CSSBoxType referenceBox) noexcept
  {
    return AdoptRef(*new BoxPathOperation(referenceBox));
  }

  Ref<PathOperation> BoxPathOperation::Clone() const noexcept
  {
    return AdoptRef(*new BoxPathOperation(ReferenceBox()));
  }

  Maybe<Path> BoxPathOperation::GetPath(const TransformOperationData &data) const noexcept
  {
    return MotionPath::ComputePathForBox(*this, data);
  }

  // MARK: - RayPathOperation

  Ref<RayPathOperation> RayPathOperation::Create(style::RayFunction &&ray, CSSBoxType referenceBox) noexcept
  {
    return AdoptRef(*new RayPathOperation(krys::move(ray), referenceBox));
  }

  Ref<RayPathOperation> RayPathOperation::Create(const style::RayFunction &ray,
                                                 CSSBoxType referenceBox) noexcept
  {
    return AdoptRef(*new RayPathOperation(ray, referenceBox));
  }

  Ref<PathOperation> RayPathOperation::Clone() const noexcept
  {
    return AdoptRef(*new RayPathOperation(_ray, _referenceBox));
  }

  bool RayPathOperation::CanBlend(const PathOperation &to) const noexcept
  {
    const auto *toRayPathOperation = DynamicDowncast<RayPathOperation>(to);
    return toRayPathOperation && style::CanBlend(_ray, toRayPathOperation->_ray)
           && _referenceBox == toRayPathOperation->ReferenceBox();
  }

  RefPtr<PathOperation> RayPathOperation::Blend(const PathOperation *to,
                                                const BlendingContext &context) const noexcept
  {
    const auto &toRayPathOperation = Downcast<RayPathOperation>(*to);
    return RayPathOperation::Create(style::Blend(_ray, toRayPathOperation._ray, context), _referenceBox);
  }

  Maybe<Path> RayPathOperation::GetPath(const TransformOperationData &data) const noexcept
  {
    return MotionPath::ComputePathForRay(*this, data);
  }

  bool RayPathOperation::Equals(const PathOperation &other) const noexcept
  {
    if (!IsSameType(other))
    {
      return false;
    }
    return _ray == Downcast<RayPathOperation>(other)._ray;
  }
}
