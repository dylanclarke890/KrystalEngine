#pragma once

#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/Style/Values/Types.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"

namespace krys::boo::style
{
  struct WillChange;

  // <animateable-feature>
  struct WillChangeAnimatableFeature
  {
    // FIXME: This should be storing <custom-ident> for non-css::PropertyId values to correctly implement
    // computed value serialization. It should likely be a Variant<css::keywords::ScrollPosition,
    // css::keywords::Contents, css::PropertyIdentifier, css::CustomIdentifier>;

    enum class Feature : uint8
    {
      ScrollPosition,
      Contents,
      Property,
    };

    constexpr static int NumCSSPropertyIdBits = 14;
    static_assert(css::TotalPropertyIdValues <= (1 << NumCSSPropertyIdBits),
                  "css::PropertyId should fit in 14 bits");

  private:
    Feature _feature {Feature::Property};
    uint16 _cssPropertyId : NumCSSPropertyIdBits {static_cast<uint16>(css::PropertyId::Invalid)};

  public:
    WillChangeAnimatableFeature(Feature, css::PropertyId = css::PropertyId::Invalid) noexcept;

    KRYS_NODISCARD Feature GetFeature() const noexcept
    {
      return _feature;
    }

    KRYS_NODISCARD css::PropertyId Property() const noexcept
    {
      return GetFeature() == Feature::Property ? static_cast<css::PropertyId>(_cssPropertyId)
                                               : css::PropertyId::Invalid;
    }

    KRYS_NODISCARD static bool PropertyCreatesStackingContext(css::PropertyId) noexcept;
    KRYS_NODISCARD static bool PropertyTriggersCompositing(css::PropertyId) noexcept;
    KRYS_NODISCARD static bool PropertyTriggersCompositingOnBoxesOnly(css::PropertyId) noexcept;

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      switch (_feature)
      {
        case Feature::ScrollPosition: return visitor(css::keywords::ScrollPosition {});
        case Feature::Contents:       return visitor(css::keywords::Contents {});
        case Feature::Property:       return visitor(css::PropertyIdentifier {Property()});
      }
      krys_unreachable();
    }

    KRYS_NODISCARD bool operator==(const WillChangeAnimatableFeature &) const noexcept = default;
  };

  // <animateable-feature>#
  struct WillChangeAnimatableFeatures
  {
    using Feature = WillChangeAnimatableFeature::Feature;
    using value_type = List<WillChangeAnimatableFeature>::value_type;
    using const_iterator = List<WillChangeAnimatableFeature>::const_iterator;

  private:
    friend struct WillChange;

    class Data : public RefCounted<Data>
    {
    private:
      List<WillChangeAnimatableFeature> _animatableFeatures;
      bool _canCreateStackingContext {false};
      bool _canTriggerCompositing {false};
      bool _canTriggerCompositingOnInline {false};

      Data() noexcept = default;

      Data(Feature feature, css::PropertyId propertyId) noexcept
      {
        AddFeature(feature, propertyId);
      }

    public:
      KRYS_NODISCARD static Ref<Data> Create() noexcept
      {
        return AdoptRef(*new Data);
      }

      KRYS_NODISCARD static Ref<Data> Create(Feature feature, css::PropertyId propertyId) noexcept
      {
        return AdoptRef(*new Data(feature, propertyId));
      }

      KRYS_NODISCARD bool ContainsScrollPosition() const noexcept;
      KRYS_NODISCARD bool ContainsContents() const noexcept;
      KRYS_NODISCARD bool ContainsProperty(css::PropertyId) const noexcept;
      KRYS_NODISCARD bool CreatesContainingBlockForAbsolutelyPositioned(bool isRootElement) const noexcept;
      KRYS_NODISCARD bool CreatesContainingBlockForOutOfFlowPositioned(bool isRootElement) const noexcept;

      KRYS_NODISCARD bool CanCreateStackingContext() const noexcept
      {
        return _canCreateStackingContext;
      }

      KRYS_NODISCARD bool CanBeBackdropRoot() const noexcept;

      KRYS_NODISCARD bool CanTriggerCompositing() const noexcept
      {
        return _canTriggerCompositing;
      }

      KRYS_NODISCARD bool CanTriggerCompositingOnInline() const noexcept
      {
        return _canTriggerCompositingOnInline;
      }

      void AddFeature(Feature, css::PropertyId) noexcept;

      KRYS_NODISCARD const_iterator begin() const noexcept
      {
        return _animatableFeatures.begin();
      }

      KRYS_NODISCARD const_iterator end() const noexcept
      {
        return _animatableFeatures.end();
      }

      KRYS_NODISCARD bool operator==(const Data &) const noexcept;
    };

    Ref<Data> _data;

    WillChangeAnimatableFeatures(Ref<Data> data) noexcept : _data {krys::move(data)}
    {
    }

  public:
    WillChangeAnimatableFeatures() noexcept : _data {Data::Create()}
    {
    }

    WillChangeAnimatableFeatures(Feature feature,
                                 css::PropertyId propertyId = css::PropertyId::Invalid) noexcept
        : _data {Data::Create(feature, propertyId)}
    {
    }

    KRYS_NODISCARD bool ContainsScrollPosition() const noexcept
    {
      return _data->ContainsScrollPosition();
    }

    KRYS_NODISCARD bool ContainsContents() const noexcept
    {
      return _data->ContainsContents();
    }

    KRYS_NODISCARD bool ContainsProperty(css::PropertyId property) const noexcept
    {
      return _data->ContainsProperty(property);
    }

    KRYS_NODISCARD bool CreatesContainingBlockForAbsolutelyPositioned(bool isRootElement) const noexcept
    {
      return _data->CreatesContainingBlockForAbsolutelyPositioned(isRootElement);
    }

    KRYS_NODISCARD bool CreatesContainingBlockForOutOfFlowPositioned(bool isRootElement) const noexcept
    {
      return _data->CreatesContainingBlockForOutOfFlowPositioned(isRootElement);
    }

    KRYS_NODISCARD bool CanCreateStackingContext() const noexcept
    {
      return _data->CanCreateStackingContext();
    }

    KRYS_NODISCARD bool CanBeBackdropRoot() const noexcept
    {
      return _data->CanBeBackdropRoot();
    }

    KRYS_NODISCARD bool CanTriggerCompositing() const noexcept
    {
      return _data->CanTriggerCompositing();
    }

    KRYS_NODISCARD bool CanTriggerCompositingOnInline() const noexcept
    {
      return _data->CanTriggerCompositingOnInline();
    }

    void AddFeature(Feature feature, css::PropertyId property = css::PropertyId::Invalid) noexcept
    {
      _data->AddFeature(feature, property);
    }

    KRYS_NODISCARD const_iterator begin() const noexcept
    {
      return _data->begin();
    }

    KRYS_NODISCARD const_iterator end() const noexcept
    {
      return _data->end();
    }
  };

  // <'will-change'> = auto | <animateable-feature>#
  // https://drafts.csswg.org/css-will-change/#propdef-will-change
  struct WillChange
  {
  private:
    RefPtr<WillChangeAnimatableFeatures::Data> _data;

  public:
    WillChange(css::keywords::Auto) noexcept : _data {nullptr}
    {
    }

    WillChange(WillChangeAnimatableFeatures &&animateableFeatures) noexcept
        : _data {krys::move(animateableFeatures._data)}
    {
    }

    WillChange(WillChangeAnimatableFeature::Feature feature,
               css::PropertyId propertyId = css::PropertyId::Invalid) noexcept
        : _data {WillChangeAnimatableFeatures::Data::Create(feature, propertyId)}
    {
    }

    KRYS_NODISCARD bool IsAuto() const noexcept
    {
      return !_data;
    }

    KRYS_NODISCARD bool IsAnimateableFeatures() const noexcept
    {
      return !!_data;
    }

    template <typename... F>
    decltype(auto) SwitchOn(F &&...f) const noexcept
    {
      auto visitor = krys::CreateVisitor(std::forward<F>(f)...);

      if (IsAuto())
      {
        return visitor(css::keywords::Auto {});
      }
      return visitor(WillChangeAnimatableFeatures {ShareRef(*_data)});
    }

    KRYS_NODISCARD bool ContainsScrollPosition() const noexcept
    {
      return _data && _data->ContainsScrollPosition();
    }

    KRYS_NODISCARD bool ContainsContents() const noexcept
    {
      return _data && _data->ContainsContents();
    }

    KRYS_NODISCARD bool ContainsProperty(css::PropertyId property) const noexcept
    {
      return _data && _data->ContainsProperty(property);
    }

    KRYS_NODISCARD bool CreatesContainingBlockForAbsolutelyPositioned(bool isRootElement) const noexcept
    {
      return _data && _data->CreatesContainingBlockForAbsolutelyPositioned(isRootElement);
    }

    KRYS_NODISCARD bool CreatesContainingBlockForOutOfFlowPositioned(bool isRootElement) const noexcept
    {
      return _data && _data->CreatesContainingBlockForOutOfFlowPositioned(isRootElement);
    }

    KRYS_NODISCARD bool CanCreateStackingContext() const noexcept
    {
      return _data && _data->CanCreateStackingContext();
    }

    KRYS_NODISCARD bool CanBeBackdropRoot() const noexcept
    {
      return _data && _data->CanBeBackdropRoot();
    }

    KRYS_NODISCARD bool CanTriggerCompositing() const noexcept
    {
      return _data && _data->CanTriggerCompositing();
    }

    KRYS_NODISCARD bool CanTriggerCompositingOnInline() const noexcept
    {
      return _data && _data->CanTriggerCompositingOnInline();
    }

    KRYS_NODISCARD bool operator==(const WillChange &other) const noexcept
    {
      return _data.get() == other._data.get() || (_data && other._data && *_data == *other._data);
    }
  };

  // MARK: - Conversion

  template <>
  struct CSSValueConversion<WillChange>
  {
    KRYS_NODISCARD WillChange operator()(BuilderState &, const css::Value &) noexcept;
  };
}

DEFINE_COMMA_SEPARATED_RANGE_LIKE_CONFORMANCE(krys::boo::style::WillChangeAnimatableFeatures)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WillChangeAnimatableFeature)
DEFINE_VARIANT_LIKE_CONFORMANCE(krys::boo::style::WillChange)
