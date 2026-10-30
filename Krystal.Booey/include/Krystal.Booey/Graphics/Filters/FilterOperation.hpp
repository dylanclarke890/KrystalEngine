#pragma once

#include "Krystal.Booey/Core/BoxExtents.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Graphics/Color/Color.hpp"
#include "Krystal.Booey/Graphics/IntPoint.hpp"
#include "Krystal.Core/RefCounted.hpp"

namespace krys::boo
{
  // CSS Filters

  struct BlendingContext;
  using IntOutsets = IntBoxExtent;

  class FilterOperation : public ThreadSafeRefCounted<FilterOperation>
  {
  public:
    enum class Type : uint8
    {
      Reference, // url(#somefilter)
      Grayscale,
      Sepia,
      Saturate,
      HueRotate,
      Invert,
      AppleInvertLightness,
      Opacity,
      Brightness,
      Contrast,
      Blur,
      DropShadow,
      DropShadowWithStyleColor,
      Passthrough,
      Default,
      None
    };

    virtual ~FilterOperation() noexcept = default;

    KRYS_NODISCARD virtual Ref<FilterOperation> Clone() const noexcept = 0;

    KRYS_NODISCARD bool operator==(const FilterOperation &other) const noexcept
    {
      return Equals(other);
    }
    KRYS_NODISCARD virtual bool Equals(const FilterOperation &) const noexcept = 0;

    KRYS_NODISCARD virtual RefPtr<FilterOperation> Blend(const FilterOperation * /*from*/,
                                                         const BlendingContext &,
                                                         bool /*blendToPassthrough*/ = false) noexcept
    {
      return nullptr;
    }

    KRYS_NODISCARD virtual bool TransformColor(SRGBA<float> &) const noexcept
    {
      return false;
    }
    KRYS_NODISCARD virtual bool InverseTransformColor(SRGBA<float> &) const noexcept
    {
      return false;
    }

    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    KRYS_NODISCARD static bool IsBasicColorMatrixFilterOperationType(Type type) noexcept
    {
      return type == Type::Grayscale || type == Type::Sepia || type == Type::Saturate
             || type == Type::HueRotate;
    }

    KRYS_NODISCARD bool IsBasicColorMatrixFilterOperation() const noexcept
    {
      return IsBasicColorMatrixFilterOperationType(_type);
    }

    KRYS_NODISCARD static bool IsBasicComponentTransferFilterOperationType(Type type) noexcept
    {
      return type == Type::Invert || type == Type::Brightness || type == Type::Contrast
             || type == Type::Opacity;
    }

    KRYS_NODISCARD bool IsBasicComponentTransferFilterOperation() const noexcept
    {
      return IsBasicComponentTransferFilterOperationType(_type);
    }

    KRYS_NODISCARD bool IsSameType(const FilterOperation &o) const noexcept
    {
      return o.GetType() == _type;
    }

    KRYS_NODISCARD virtual bool IsIdentity() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD virtual IntOutsets Outsets() const noexcept
    {
      return {};
    }

    // True if the alpha channel of any pixel can change under this operation.
    KRYS_NODISCARD virtual bool AffectsOpacity() const noexcept
    {
      return false;
    }
    // True if the value of one pixel can affect the value of another pixel under this operation, such as
    // blur.
    KRYS_NODISCARD virtual bool MovesPixels() const noexcept
    {
      return false;
    }
    // True if the filter should not be allowed to work on content that is not available from this security
    // origin.
    KRYS_NODISCARD virtual bool ShouldBeRestrictedBySecurityOrigin() const noexcept
    {
      return false;
    }

    KRYS_NODISCARD bool IsDropShadowBase() const noexcept
    {
      return GetType() == boo::FilterOperation::Type::DropShadow
             || GetType() == boo::FilterOperation::Type::DropShadowWithStyleColor;
    }

    KRYS_NODISCARD virtual bool RequiresRepaintForCurrentColorChange() const noexcept
    {
      return false;
    }

  protected:
    FilterOperation(Type type) noexcept : _type(type)
    {
    }

    KRYS_NODISCARD double BlendAmounts(double from, double to, const BlendingContext &) const noexcept;

    Type _type;
  };

  class DefaultFilterOperation : public FilterOperation
  {
  public:
    KRYS_NODISCARD static Ref<DefaultFilterOperation> Create(Type representedType) noexcept
    {
      return AdoptRef(*new DefaultFilterOperation(representedType));
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new DefaultFilterOperation(RepresentedType()));
    }

    KRYS_NODISCARD Type RepresentedType() const noexcept;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;

    DefaultFilterOperation(Type representedType) noexcept
        : FilterOperation(Type::Default), _representedType(representedType)
    {
    }

    Type _representedType;
  };

  class PassthroughFilterOperation : public FilterOperation
  {
  public:
    KRYS_NODISCARD static Ref<PassthroughFilterOperation> Create() noexcept
    {
      return AdoptRef(*new PassthroughFilterOperation());
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new PassthroughFilterOperation());
    }

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &o) const noexcept override
    {
      return IsSameType(o);
    }

    PassthroughFilterOperation() noexcept : FilterOperation(Type::Passthrough)
    {
    }
  };

  // Grayscale, Sepia, Saturate and HueRotate are variations on a basic color matrix effect.
  // For HueRotate, the angle of rotation is stored in _amount.
  class BasicColorMatrixFilterOperation : public FilterOperation
  {
  public:
    KRYS_NODISCARD static Ref<BasicColorMatrixFilterOperation> Create(double amount, Type type) noexcept
    {
      return AdoptRef(*new BasicColorMatrixFilterOperation(amount, type));
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new BasicColorMatrixFilterOperation(Amount(), GetType()));
    }

    KRYS_NODISCARD double Amount() const noexcept
    {
      return _amount;
    }

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;

    KRYS_NODISCARD double PassthroughAmount() const noexcept;

    BasicColorMatrixFilterOperation(double amount, Type type) noexcept
        : FilterOperation(type), _amount(amount)
    {
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override;
    KRYS_NODISCARD bool TransformColor(SRGBA<float> &) const noexcept override;

    double _amount;
  };

  // Invert, Brightness, Contrast and Opacity are variations on a basic component transfer effect.
  class BasicComponentTransferFilterOperation : public FilterOperation
  {
  public:
    KRYS_NODISCARD static Ref<BasicComponentTransferFilterOperation> Create(double amount, Type type) noexcept
    {
      return AdoptRef(*new BasicComponentTransferFilterOperation(amount, type));
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new BasicComponentTransferFilterOperation(Amount(), GetType()));
    }

    KRYS_NODISCARD double Amount() const noexcept
    {
      return _amount;
    }

    KRYS_NODISCARD bool AffectsOpacity() const noexcept override;

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;

    KRYS_NODISCARD double PassthroughAmount() const noexcept;

    BasicComponentTransferFilterOperation(double amount, Type type) noexcept
        : FilterOperation(type), _amount(amount)
    {
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override;
    KRYS_NODISCARD bool TransformColor(SRGBA<float> &) const noexcept override;

    double _amount;
  };

  class InvertLightnessFilterOperation : public FilterOperation
  {
  public:
    KRYS_NODISCARD static Ref<InvertLightnessFilterOperation> Create() noexcept
    {
      return AdoptRef(*new InvertLightnessFilterOperation());
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept final
    {
      return AdoptRef(*new InvertLightnessFilterOperation());
    }

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept final;

    InvertLightnessFilterOperation() noexcept : FilterOperation(Type::AppleInvertLightness)
    {
    }

    KRYS_NODISCARD bool TransformColor(SRGBA<float> &) const noexcept final;
    KRYS_NODISCARD bool InverseTransformColor(SRGBA<float> &) const noexcept final;
  };

  class BlurFilterOperation : public FilterOperation
  {
  public:
    KRYS_NODISCARD static Ref<BlurFilterOperation> Create(float stdDeviation) noexcept
    {
      return AdoptRef(*new BlurFilterOperation(krys::move(stdDeviation)));
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new BlurFilterOperation(StdDeviation()));
    }

    KRYS_NODISCARD float StdDeviation() const noexcept
    {
      return _stdDeviation;
    }

    KRYS_NODISCARD bool AffectsOpacity() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool MovesPixels() const noexcept override
    {
      return true;
    }

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;

    BlurFilterOperation(float stdDeviation) noexcept
        : FilterOperation(Type::Blur), _stdDeviation(stdDeviation)
    {
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override;
    KRYS_NODISCARD IntOutsets Outsets() const noexcept override;

    float _stdDeviation;
  };

  class DropShadowFilterOperationBase : public FilterOperation
  {
  public:
    KRYS_NODISCARD int X() const noexcept
    {
      return _location.X();
    }
    KRYS_NODISCARD int Y() const noexcept
    {
      return _location.Y();
    }
    KRYS_NODISCARD IntPoint Location() const noexcept
    {
      return _location;
    }
    KRYS_NODISCARD int StdDeviation() const noexcept
    {
      return _stdDeviation;
    }

    KRYS_NODISCARD bool AffectsOpacity() const noexcept override
    {
      return true;
    }
    KRYS_NODISCARD bool MovesPixels() const noexcept override
    {
      return true;
    }

  protected:
    KRYS_NODISCARD bool NonColorEqual(const DropShadowFilterOperationBase &) const noexcept;

    DropShadowFilterOperationBase(Type type, const IntPoint &location, int stdDeviation) noexcept
        : FilterOperation(type), _location(location), _stdDeviation(stdDeviation)
    {
    }

    KRYS_NODISCARD bool IsIdentity() const noexcept override;
    KRYS_NODISCARD IntOutsets Outsets() const noexcept override;

    IntPoint _location; // FIXME: Should _location be a FloatPoint?
    int _stdDeviation;  // FIXME: Should _stdDeviation be a float?
  };

  class DropShadowFilterOperation : public DropShadowFilterOperationBase
  {
  public:
    KRYS_NODISCARD static Ref<DropShadowFilterOperation> Create(const IntPoint &location, int stdDeviation,
                                                                const Color &color) noexcept
    {
      return AdoptRef(*new DropShadowFilterOperation(location, stdDeviation, color));
    }

    KRYS_NODISCARD Ref<FilterOperation> Clone() const noexcept override
    {
      return AdoptRef(*new DropShadowFilterOperation(Location(), StdDeviation(), _color));
    }

    const Color &GetColor() const
    {
      return _color;
    }

    KRYS_NODISCARD RefPtr<FilterOperation> Blend(const FilterOperation *from, const BlendingContext &,
                                                 bool blendToPassthrough = false) noexcept override;

  private:
    KRYS_NODISCARD bool Equals(const FilterOperation &) const noexcept override;

    DropShadowFilterOperation(const IntPoint &location, int stdDeviation, const Color &color) noexcept
        : DropShadowFilterOperationBase(Type::DropShadow, location, stdDeviation), _color(color)
    {
    }

    Color _color;
  };
}

#define KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(ToValueTypeName, predicate)                             \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(ToValueTypeName)                                                    \
    KRYS_NODISCARD static bool IsType(const krys::boo::FilterOperation &operation) noexcept                  \
    {                                                                                                        \
      return operation.predicate;                                                                            \
    }                                                                                                        \
  KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()

KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::DefaultFilterOperation,
                                             GetType() == krys::boo::FilterOperation::Type::Default)
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::PassthroughFilterOperation,
                                             GetType() == krys::boo::FilterOperation::Type::Passthrough)
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::BasicColorMatrixFilterOperation,
                                             IsBasicColorMatrixFilterOperation())
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::BasicComponentTransferFilterOperation,
                                             IsBasicComponentTransferFilterOperation())
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::InvertLightnessFilterOperation,
                                             GetType()
                                               == krys::boo::FilterOperation::Type::AppleInvertLightness)
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::BlurFilterOperation,
                                             GetType() == krys::boo::FilterOperation::Type::Blur)
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::DropShadowFilterOperation,
                                             GetType() == krys::boo::FilterOperation::Type::DropShadow)
KRYS_SPECIALIZE_TYPE_TRAITS_FILTER_OPERATION(krys::boo::DropShadowFilterOperationBase, IsDropShadowBase())
