#pragma once

#include "Krystal.Booey/Style/Values/Shapes/PathOperationWrappers.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"

namespace krys::boo::style
{
  struct ClipPath;
  struct OffsetPath;

  // <'clip-path'> = none | <url> | [ <basic-shape> || <geometry-box> ]
  struct ClipPath
  {
    ClipPath(css::keywords::None) noexcept : _operation {nullptr}
    {
    }
    ClipPath(ReferencePath &&reference) noexcept : _operation {krys::move(reference._operation)}
    {
    }
    ClipPath(const ReferencePath &reference) noexcept : _operation {reference._operation}
    {
    }
    ClipPath(BasicShapePath &&basicShape) noexcept : _operation {krys::move(basicShape._operation)}
    {
    }
    ClipPath(const BasicShapePath &basicShape) noexcept : _operation {basicShape._operation}
    {
    }
    ClipPath(BoxPath &&box) noexcept : _operation {krys::move(box._operation)}
    {
    }
    ClipPath(const BoxPath &box) noexcept : _operation {box._operation}
    {
    }

    explicit ClipPath(RefPtr<PathOperation> &&operation) noexcept : _operation {krys::move(operation)}
    {
      krys_assert(IsValid(_operation));
    }
    explicit ClipPath(const RefPtr<PathOperation> &operation) noexcept : _operation {operation}
    {
      krys_assert(IsValid(_operation));
    }

    KRYS_NODISCARD bool IsNone() const noexcept
    {
      return !_operation;
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

    KRYS_NODISCARD Maybe<ReferencePath> TryReference() const noexcept;
    KRYS_NODISCARD Maybe<BasicShapePath> TryBasicShape() const noexcept;
    KRYS_NODISCARD Maybe<BoxPath> TryBox() const noexcept;

    template <typename>
    KRYS_NODISCARD bool HoldsAlternative() const noexcept;
    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...) const noexcept;

    KRYS_NODISCARD bool operator==(const ClipPath &other) const noexcept
    {
      return ArePointingToEqualData(_operation, other._operation);
    }

  private:
    friend struct Blending<ClipPath>;
    friend CSSBoxType ReferenceBox(const ClipPath &) noexcept;
    friend Maybe<boo::Path> TryPath(const ClipPath &, const TransformOperationData &) noexcept;

    KRYS_NODISCARD static bool IsValid(const RefPtr<PathOperation> &operation) noexcept
    {
      return !operation || !Is<RayPathOperation>(*operation);
    }

    RefPtr<PathOperation> _operation;
  };

  KRYS_NODISCARD inline Maybe<boo::Path> TryPath(const ClipPath &clipPath,
                                                 const TransformOperationData &data) noexcept
  {
    const auto &operation = clipPath._operation;
    if (!operation)
    {
      return {};
    }
    return operation->GetPath(data);
  }

  template <typename T>
  bool ClipPath::HoldsAlternative() const noexcept
  {
    if constexpr (std::same_as<T, css::keywords::None>)
    {
      return IsNone();
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
  decltype(auto) ClipPath::SwitchOn(F &&...f) const noexcept
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
      case PathOperation::Type::Ray: break;
    }
    krys_unreachable();
  }

  inline Maybe<ReferencePath> ClipPath::TryReference() const noexcept
  {
    if (auto *reference = DynamicDowncast<ReferencePathOperation>(_operation.get()))
    {
      return ReferencePath {ShareRef(*reference)};
    }
    return {};
  }

  inline Maybe<BasicShapePath> ClipPath::TryBasicShape() const noexcept
  {
    if (auto *basicShape = DynamicDowncast<ShapePathOperation>(_operation.get()))
    {
      return BasicShapePath {ShareRef(*basicShape)};
    }
    return {};
  }

  inline Maybe<BoxPath> ClipPath::TryBox() const noexcept
  {
    if (auto *box = DynamicDowncast<BoxPathOperation>(_operation.get()))
    {
      return BoxPath {ShareRef(*box)};
    }
    return {};
  }

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<ClipPath>
  {
    KRYS_NODISCARD ClipPath operator()(BuilderState &, const css::Value &) noexcept;
  };

  // MARK: - Blending

  template <>
  struct Blending<ClipPath>
  {
    KRYS_NODISCARD bool CanBlend(const ClipPath &, const ClipPath &) noexcept;
    KRYS_NODISCARD ClipPath Blend(const ClipPath &, const ClipPath &, const BlendingContext &) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ClipPath)
