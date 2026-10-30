#pragma once

#include "Krystal.Booey/Style/Values/Shapes/PathOperationWrappers.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"

namespace krys::boo::style
{
  // <'offset-path'> = none | [ [ <ray()> | <url> | <basic-shape> ] || <coord-box> ]
  // https://drafts.fxtf.org/motion-1/#propdef-offset-path
  struct OffsetPath
  {
    OffsetPath(css::keywords::None) noexcept : _operation {nullptr}
    {
    }
    OffsetPath(RayPath &&ray) noexcept : _operation {krys::move(ray._operation)}
    {
    }
    OffsetPath(const RayPath &ray) noexcept : _operation {ray._operation}
    {
    }
    OffsetPath(ReferencePath &&reference) noexcept : _operation {krys::move(reference._operation)}
    {
    }
    OffsetPath(const ReferencePath &reference) noexcept : _operation {reference._operation}
    {
    }
    OffsetPath(BasicShapePath &&basicShape) noexcept : _operation {krys::move(basicShape._operation)}
    {
    }
    OffsetPath(const BasicShapePath &basicShape) noexcept : _operation {basicShape._operation}
    {
    }
    OffsetPath(BoxPath &&box) noexcept : _operation {krys::move(box._operation)}
    {
    }
    OffsetPath(const BoxPath &box) noexcept : _operation {box._operation}
    {
    }

    explicit OffsetPath(RefPtr<PathOperation> &&operation) noexcept : _operation {krys::move(operation)}
    {
    }
    explicit OffsetPath(const RefPtr<PathOperation> &operation) noexcept : _operation {operation}
    {
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_operation;
    }
    KRYS_NODISCARD bool IsRay() const noexcept
    {
      return _operation && Is<RayPathOperation>(*_operation);
    }
    KRYS_NODISCARD bool IsReference() const noexcept
    {
      return _operation && Is<ReferencePathOperation>(*_operation);
    }
    KRYS_NODISCARD bool IsBasicShape() const noexcept
    {
      return _operation && Is<ShapePathOperation>(*_operation);
    }
    KRYS_NODISCARD bool IsBox() const noexcept
    {
      return _operation && Is<BoxPathOperation>(*_operation);
    }

    KRYS_NODISCARD Maybe<RayPath> TryRay() const noexcept;
    KRYS_NODISCARD Maybe<ReferencePath> TryReference() const noexcept;
    KRYS_NODISCARD Maybe<BasicShapePath> TryBasicShape() const noexcept;
    KRYS_NODISCARD Maybe<BoxPath> TryBox() const noexcept;

    KRYS_NODISCARD bool AffectedByTransformOrigin() const noexcept
    {
      return !IsNone();
    }

    template <typename>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept;
    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...) const noexcept;

    KRYS_NODISCARD bool operator==(const OffsetPath &other) const noexcept
    {
      return ArePointingToEqualData(_operation, other._operation);
    }

  private:
    friend struct Blending<OffsetPath>;
    friend struct ToPlatform<OffsetPath>;
    friend Maybe<boo::Path> TryPath(const OffsetPath &, const TransformOperationData &) noexcept;

    RefPtr<PathOperation> _operation;
  };

  KRYS_NODISCARD inline Maybe<boo::Path> TryPath(const OffsetPath &offsetPath,
                                                 const TransformOperationData &data) noexcept
  {
    const auto &operation = offsetPath._operation;
    if (!operation)
    {
      return {};
    }
    return operation->GetPath(data);
  }

  template <typename T>
  bool OffsetPath::HoldsAlternative() const noexcept
  {
    if constexpr (std::same_as<T, css::keywords::None>)
    {
      return IsNone();
    }
    else if constexpr (std::same_as<T, RayPath>)
    {
      return IsRay();
    }
    else if constexpr (std::same_as<T, ReferencePath>)
    {
      return IsReference();
    }
    else if constexpr (std::same_as<T, BasicShapePath>)
    {
      return IsBasicShape();
    }
    else if constexpr (std::same_as<T, BoxPath>)
    {
      return IsBox();
    }
  }

  template <typename... F>
  decltype(auto) OffsetPath::SwitchOn(F &&...f) const noexcept
  {
    auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

    if (!_operation)
    {
      return visitor(css::keywords::None {});
    }

    switch (_operation->GetType())
    {
      case PathOperation::Type::Reference:
        return visitor(ReferencePath {ShareRef(Downcast<ReferencePathOperation>(*_operation))});
      case PathOperation::Type::Shape:
        return visitor(BasicShapePath {ShareRef(Downcast<ShapePathOperation>(*_operation))});
      case PathOperation::Type::Box:
        return visitor(BoxPath {ShareRef(Downcast<BoxPathOperation>(*_operation))});
      case PathOperation::Type::Ray:
        return visitor(RayPath {ShareRef(Downcast<RayPathOperation>(*_operation))});
    }
    krys_unreachable();
  }

  inline Maybe<RayPath> OffsetPath::TryRay() const noexcept
  {
    if (auto *ray = DynamicDowncast<RayPathOperation>(_operation.get()))
    {
      return RayPath {ShareRef(*ray)};
    }
    return {};
  }

  inline Maybe<ReferencePath> OffsetPath::TryReference() const noexcept
  {
    if (auto *reference = DynamicDowncast<ReferencePathOperation>(_operation.get()))
    {
      return ReferencePath {ShareRef(*reference)};
    }
    return {};
  }

  inline Maybe<BasicShapePath> OffsetPath::TryBasicShape() const noexcept
  {
    if (auto *basicShape = DynamicDowncast<ShapePathOperation>(_operation.get()))
    {
      return BasicShapePath {ShareRef(*basicShape)};
    }
    return {};
  }

  inline Maybe<BoxPath> OffsetPath::TryBox() const noexcept
  {
    if (auto *box = DynamicDowncast<BoxPathOperation>(_operation.get()))
    {
      return BoxPath {ShareRef(*box)};
    }
    return {};
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<OffsetPath>
  {
    KRYS_NODISCARD OffsetPath operator()(BuilderState &, const css::Value &) noexcept;
  };
  template <>
  struct CSSValueCreation<OffsetPath>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const OffsetPath &) noexcept;
  };

  // MARK: - Serialization

  template <>
  struct Serialiser<OffsetPath>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const OffsetPath &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<OffsetPath>
  {
    KRYS_NODISCARD bool CanBlend(const OffsetPath &, const OffsetPath &) noexcept;
    KRYS_NODISCARD OffsetPath Blend(const OffsetPath &, const OffsetPath &, const BlendingContext &) noexcept;
  };

  // MARK: - Platform

  template <>
  struct ToPlatform<OffsetPath>
  {
    KRYS_NODISCARD RefPtr<PathOperation> operator()(const OffsetPath &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::OffsetPath)
