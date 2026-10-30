#pragma once

#include "Krystal.Booey/Rendering/PathOperation.hpp"
#include "Krystal.Core/Text/StringBuilder.hpp"
#include "Krystal.Core/Utils/PointerComparison.hpp"

namespace krys::boo::style
{
  struct OffsetPath;
  struct ClipPath;

  enum class SupportRayPathOperation : bool
  {
    No,
    Yes
  };

  // <ray-path> = <ray()> || <coord-box>
  struct RayPath
  {
    explicit RayPath(Ref<RayPathOperation> &&operation) noexcept : _operation {krys::move(operation)}
    {
    }
    explicit RayPath(const Ref<RayPathOperation> &operation) noexcept : _operation {operation}
    {
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD const RayFunction &Ray() const noexcept
    {
      return _operation->Ray();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD CSSBoxType ReferenceBox() const noexcept
    {
      return _operation->ReferenceBox();
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (ReferenceBox() == CSSBoxType::BoxMissing)
      {
        return visitor(Ray());
      }
      return visitor(css::SpaceSeparatedTuple {Ray(), ReferenceBox()});
    }

    KRYS_NODISCARD bool operator==(const RayPath &other) const noexcept
    {
      return ArePointingToEqualData(_operation, other._operation);
    }

  private:
    friend ClipPath;
    friend OffsetPath;
    friend Maybe<boo::Path> TryPath(const RayPath &, const TransformOperationData &) noexcept;

    Ref<RayPathOperation> _operation;
  };

  // <reference-path> = <url> || <coord-box>
  struct ReferencePath
  {
    explicit ReferencePath(Ref<ReferencePathOperation> &&operation) noexcept
        : _operation {krys::move(operation)}
    {
    }
    explicit ReferencePath(const Ref<ReferencePathOperation> &operation) noexcept : _operation {operation}
    {
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD const URL &Url() const noexcept
    {
      return _operation->Url();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD const css::CSSOMStringAtom &Fragment() const noexcept
    {
      return _operation->Fragment();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD CSSBoxType ReferenceBox() const noexcept
    {
      return _operation->ReferenceBox();
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (ReferenceBox() == CSSBoxType::BoxMissing)
      {
        return visitor(Url());
      }
      return visitor(css::SpaceSeparatedTuple {Url(), ReferenceBox()});
    }

    KRYS_NODISCARD bool operator==(const ReferencePath &other) const noexcept
    {
      return ArePointingToEqualData(_operation, other._operation);
    }

  private:
    friend ClipPath;
    friend OffsetPath;
    friend Maybe<boo::Path> TryPath(const ReferencePath &, const TransformOperationData &) noexcept;

    Ref<ReferencePathOperation> _operation;
  };

  // <basic-shape-path> = <basic-shape> || <coord-box>
  struct BasicShapePath
  {
    explicit BasicShapePath(Ref<ShapePathOperation> &&operation) noexcept : _operation {krys::move(operation)}
    {
    }
    explicit BasicShapePath(const Ref<ShapePathOperation> &operation) noexcept : _operation {operation}
    {
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD const BasicShape &GetShape() const noexcept
    {
      return _operation->GetShape();
    }
    KRYS_ALWAYS_INLINE KRYS_NODISCARD CSSBoxType ReferenceBox() const noexcept
    {
      return _operation->ReferenceBox();
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (ReferenceBox() == CSSBoxType::BoxMissing)
      {
        return visitor(GetShape());
      }
      return visitor(css::SpaceSeparatedTuple {GetShape(), ReferenceBox()});
    }

    KRYS_NODISCARD bool operator==(const BasicShapePath &other) const noexcept
    {
      return ArePointingToEqualData(_operation, other._operation);
    }

  private:
    friend ClipPath;
    friend OffsetPath;
    friend Maybe<boo::Path> TryPath(const BasicShapePath &, const TransformOperationData &) noexcept;

    Ref<ShapePathOperation> _operation;
  };

  // <box-path> = <coord-box>
  struct BoxPath
  {
    explicit BoxPath(Ref<BoxPathOperation> &&operation) noexcept : _operation {krys::move(operation)}
    {
    }
    explicit BoxPath(const Ref<BoxPathOperation> &operation) noexcept : _operation {operation}
    {
    }

    KRYS_ALWAYS_INLINE KRYS_NODISCARD CSSBoxType ReferenceBox() const noexcept
    {
      return _operation->ReferenceBox();
    }

    template <typename... F>
    KRYS_NODISCARD decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);
      return visitor(ReferenceBox());
    }

    KRYS_NODISCARD bool operator==(const BoxPath &other) const noexcept
    {
      return _operation.get() == other._operation.get();
    }

  private:
    friend ClipPath;
    friend OffsetPath;
    friend Maybe<boo::Path> TryPath(const BoxPath &, const TransformOperationData &) noexcept;

    Ref<BoxPathOperation> _operation;
  };

  KRYS_NODISCARD inline Maybe<boo::Path> TryPath(const RayPath &rayPath,
                                                 const TransformOperationData &data) noexcept
  {
    return rayPath._operation->GetPath(data);
  }

  KRYS_NODISCARD inline Maybe<boo::Path> TryPath(const ReferencePath &referencePath,
                                                 const TransformOperationData &data) noexcept
  {
    return referencePath._operation->GetPath(data);
  }

  KRYS_NODISCARD inline Maybe<boo::Path> TryPath(const BasicShapePath &basicShapePath,
                                                 const TransformOperationData &data) noexcept
  {
    return basicShapePath._operation->GetPath(data);
  }

  KRYS_NODISCARD inline Maybe<boo::Path> TryPath(const BoxPath &boxPath,
                                                 const TransformOperationData &data) noexcept
  {
    return boxPath._operation->GetPath(data);
  }

  // MARK: - Conversion

  // Shared Converter for both ClipPath and OffsetPath
  template <>
  struct CSSValueConversion<RefPtr<PathOperation>>
  {
    KRYS_NODISCARD RefPtr<PathOperation> operator()(BuilderState &, const css::Value &,
                                                    SupportRayPathOperation) noexcept;
  };

  // `RayPath` is special-cased to return a `css::RayValue`.
  template <>
  struct CSSValueCreation<RayPath>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &,
                                              const RayPath &) noexcept;
  };

  // `BasicShapePath` is special-cased to handle non-standard `PathConversion` argument.
  template <>
  struct CSSValueCreation<BasicShapePath>
  {
    KRYS_NODISCARD Ref<css::Value> operator()(css::ValuePool &, const RenderStyle &, const BasicShapePath &,
                                              PathConversion = PathConversion::None) noexcept;
  };

  // MARK: - Serialization

  // `BasicShapePath` is special-cased to handle non-standard `PathConversion` argument.
  template <>
  struct Serialiser<BasicShapePath>
  {
    void operator()(StringBuilder &, const css::SerialisationContext &, const RenderStyle &,
                    const BasicShapePath &, PathConversion = PathConversion::None) noexcept;
  };
}

DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::RayPath)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::ReferencePath)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BasicShapePath)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::BoxPath)
