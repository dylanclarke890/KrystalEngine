#pragma once

#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/Core/WritingMode.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/Types/IsImportant.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Types/Array.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"
#include "Krystal.Core/Types/Span.hpp"
#include <bitset>

namespace krys::boo::css
{
  class Value;
}

namespace krys::boo::style
{
  /// @brief The declarations that win the cascade for each property of an element, from its match result.
  class PropertyCascade
  {
    KRYS_NON_COPY_MOVABLE(PropertyCascade);

  public:
    using PropertyBitSet = std::bitset<ToUnderlying(css::LastLowPriorityProperty) + 1>;

    enum class PropertyType : uint8
    {
      NonInherited,
      Inherited,
      ExplicitlyInherited,
      AfterAnimation,
      AfterTransition,
      StartingStyle,
      NonCacheable,
    };

    enum class Origin : uint8
    {
      UserAgent,
      User,
      Author,
      PositionFallback
    };

    KRYS_NODISCARD constexpr static EnumSet<PropertyType> NormalPropertyTypes() noexcept
    {
      return {PropertyType::NonInherited, PropertyType::Inherited};
    }

    KRYS_NODISCARD constexpr static EnumSet<PropertyType> StartingStylePropertyTypes() noexcept
    {
      return NormalPropertyTypes() | PropertyType::StartingStyle;
    }

    struct IncludedProperties
    {
      EnumSet<PropertyType> Types;
      // Ids are mutually exclusive with types. They are low-priority only.
      SmallList<css::PropertyId, 4> Ids {};

      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return !Types && Ids.empty();
      }
    };

    KRYS_NODISCARD static IncludedProperties NormalProperties() noexcept
    {
      return {NormalPropertyTypes()};
    }

    struct Property
    {
      css::PropertyId Id {css::PropertyId::Invalid};
      PropertyCascade::Origin Origin {PropertyCascade::Origin::UserAgent};
      ScopeOrdinal StyleScopeOrdinal {ScopeOrdinal::Element};
      style::CascadeLayerPriority CascadeLayerPriority {0};
      style::FromStyleAttribute FromStyleAttribute {style::FromStyleAttribute::No};
      // Values for link match states MatchDefault, MatchLink and MatchVisited
      Array<const css::Value *, 3> CssValue {};
      Array<PropertyCascade::Origin, 3> Origins {};
    };

  private:
    struct AnimationLayer
    {
      const HashSet<AnimatableCSSProperty> &Properties;
      HashSet<AnimatableCSSProperty> OverriddenProperties;
      bool HasCustomProperties {false};
      bool HasFontSize {false};
      bool HasLineHeight {false};

      explicit AnimationLayer(const HashSet<AnimatableCSSProperty> &properties) noexcept;
    };

    constexpr static size_t LogicalGroupPropertyCount =
      ToUnderlying(css::LastLogicalGroupProperty) - ToUnderlying(css::FirstLogicalGroupProperty) + 1;

    const MatchResult &_matchResult;
    const IncludedProperties _includedProperties;
    const Origin _maximumOrigin;
    const Maybe<ScopeOrdinal> _rollbackScope;
    const Maybe<style::CascadeLayerPriority> _maximumCascadeLayerPriorityForRollback;

    Maybe<AnimationLayer> _animationLayer;
    Maybe<MatchedProperties> _positionTryFallbackProperties;

    // The PropertyId enum is sorted like this:
    // 1. PropertyId::Invalid and PropertyId::Custom.
    // 2. Normal longhand properties (high priority ones followed by low priority ones).
    // 3. Longhand properties in a logical property group.
    // 4. Shorthand properties.
    //
    // '_properties' is used for both normal and logical longhands, so it has size 'LastLogicalGroupProperty +
    // 1'. It could actually be 2 units smaller, but then we would have to subtract 'FirstProperty', which may
    // not be worth it. '_propertyIsPresent' is not used for logical group properties, so we only need to
    // cover up to the last low priority one.
    Array<Property, ToUnderlying(css::LastLogicalGroupProperty) + 1> _properties {};
    PropertyBitSet _propertyIsPresent;

    Array<uint32, LogicalGroupPropertyCount> _logicalGroupPropertyIndices {};
    uint32 _lastIndexForLogicalGroup {0};
    Array<css::PropertyId, LogicalGroupPropertyCount> _logicalGroupPropertyIds {};
    uint32 _seenLogicalGroupPropertyCount {0};
    css::PropertyId _lowestSeenLogicalGroupProperty {css::LastLogicalGroupProperty};
    css::PropertyId _highestSeenLogicalGroupProperty {css::FirstLogicalGroupProperty};

    HashMap<css::CSSOMStringAtom, Property> _customProperties;

  public:
    PropertyCascade(const MatchResult &matchResult, IncludedProperties &&includedProperties,
                    const HashSet<AnimatableCSSProperty> *animatedProperties = nullptr,
                    const css::StyleProperties *positionTryFallbackProperties = nullptr) noexcept;
    PropertyCascade(const PropertyCascade &parent, Origin maximumOrigin,
                    Maybe<ScopeOrdinal> rollbackScope = {},
                    Maybe<style::CascadeLayerPriority> maximumCascadeLayerPriorityForRollback = {}) noexcept;

    ~PropertyCascade() noexcept;

    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _propertyIsPresent.none() && !_seenLogicalGroupPropertyCount;
    }

    KRYS_NODISCARD bool HasNormalProperty(css::PropertyId id) const noexcept;
    KRYS_NODISCARD const Property &NormalProperty(css::PropertyId id) const noexcept;

    KRYS_NODISCARD bool HasLogicalGroupProperty(css::PropertyId id) const noexcept;
    KRYS_NODISCARD const Property &LogicalGroupProperty(css::PropertyId id) const noexcept;
    KRYS_NODISCARD const Property *
      LastPropertyResolvingLogicalPropertyPair(css::PropertyId id, WritingMode writingMode) const noexcept;

    KRYS_NODISCARD bool HasCustomProperty(const css::CSSOMStringAtom &name) const noexcept;
    KRYS_NODISCARD const Property &GetCustomProperty(const css::CSSOMStringAtom &name) const noexcept;

    KRYS_NODISCARD Span<const css::PropertyId> LogicalGroupPropertyIds() const noexcept;
    KRYS_NODISCARD const HashMap<css::CSSOMStringAtom, Property> &CustomProperties() const noexcept
    {
      return _customProperties;
    }

    KRYS_NODISCARD HashSet<AnimatableCSSProperty> OverriddenAnimatedProperties() const noexcept;

    KRYS_NODISCARD PropertyBitSet &PropertyIsPresent() noexcept
    {
      return _propertyIsPresent;
    }

    KRYS_NODISCARD const PropertyBitSet &PropertyIsPresent() const noexcept
    {
      return _propertyIsPresent;
    }

    KRYS_NODISCARD bool ApplyLowPriorityOnly() const noexcept
    {
      return !_includedProperties.Ids.empty();
    }

  private:
    void BuildCascade() noexcept;
    KRYS_NODISCARD bool AddNormalMatches(Origin origin) noexcept;
    void AddImportantMatches(Origin origin) noexcept;
    bool AddMatch(const MatchedProperties &matchedProperties, Origin origin,
                  css::IsImportant important) noexcept;
    KRYS_NODISCARD bool
      ShouldApplyAfterAnimation(const css::StyleProperties::PropertyReference &property) noexcept;
    void AddPositionTryFallbackProperties() noexcept;

    void Set(css::PropertyId id, const css::Value &cssValue, const MatchedProperties &matchedProperties,
             Origin origin) noexcept;
    void SetLogicalGroupProperty(css::PropertyId id, const css::Value &cssValue,
                                 const MatchedProperties &matchedProperties, Origin origin) noexcept;
    static void SetPropertyInternal(Property &property, css::PropertyId id, const css::Value &cssValue,
                                    const MatchedProperties &matchedProperties, Origin origin) noexcept;

    KRYS_NODISCARD bool HasProperty(css::PropertyId propertyId, const css::Value &value) const noexcept;
    KRYS_NODISCARD bool MayOverrideExistingProperty(css::PropertyId propertyId,
                                                    const css::Value &value) const noexcept;

    KRYS_NODISCARD uint32 LogicalGroupPropertyIndex(css::PropertyId id) const noexcept;
    void SetLogicalGroupPropertyIndex(css::PropertyId id, uint32 index) noexcept;
    void SortLogicalGroupPropertyIds() noexcept;
  };

  inline bool PropertyCascade::HasNormalProperty(css::PropertyId id) const noexcept
  {
    krys_debug_assert(id < css::FirstLogicalGroupProperty);
    return _propertyIsPresent.test(ToUnderlying(id));
  }

  inline const PropertyCascade::Property &PropertyCascade::NormalProperty(css::PropertyId id) const noexcept
  {
    krys_debug_assert(HasNormalProperty(id));
    return _properties[ToUnderlying(id)];
  }

  inline uint32 PropertyCascade::LogicalGroupPropertyIndex(css::PropertyId id) const noexcept
  {
    krys_debug_assert(id >= css::FirstLogicalGroupProperty);
    krys_debug_assert(id <= css::LastLogicalGroupProperty);
    return _logicalGroupPropertyIndices[ToUnderlying(id) - ToUnderlying(css::FirstLogicalGroupProperty)];
  }

  inline void PropertyCascade::SetLogicalGroupPropertyIndex(css::PropertyId id, uint32 index) noexcept
  {
    krys_debug_assert(id >= css::FirstLogicalGroupProperty);
    krys_debug_assert(id <= css::LastLogicalGroupProperty);
    _logicalGroupPropertyIndices[ToUnderlying(id) - ToUnderlying(css::FirstLogicalGroupProperty)] = index;
  }

  inline bool PropertyCascade::HasLogicalGroupProperty(css::PropertyId id) const noexcept
  {
    return LogicalGroupPropertyIndex(id);
  }

  inline const PropertyCascade::Property &
    PropertyCascade::LogicalGroupProperty(css::PropertyId id) const noexcept
  {
    krys_debug_assert(HasLogicalGroupProperty(id));
    return _properties[ToUnderlying(id)];
  }

  inline Span<const css::PropertyId> PropertyCascade::LogicalGroupPropertyIds() const noexcept
  {
    return Span<const css::PropertyId> {_logicalGroupPropertyIds}.first(_seenLogicalGroupPropertyCount);
  }

  inline bool PropertyCascade::HasCustomProperty(const css::CSSOMStringAtom &name) const noexcept
  {
    return _customProperties.contains(name);
  }

  inline const PropertyCascade::Property &
    PropertyCascade::GetCustomProperty(const css::CSSOMStringAtom &name) const noexcept
  {
    krys_debug_assert(HasCustomProperty(name));
    return _customProperties.find(name)->second;
  }
}
