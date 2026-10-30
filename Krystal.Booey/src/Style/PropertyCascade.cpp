#include "Krystal.Booey/Style/PropertyCascade.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/Style/PropertyAllowlist.hpp"
#include <algorithm>

namespace krys::boo::style
{
  PropertyCascade::PropertyCascade(const MatchResult &matchResult, IncludedProperties &&includedProperties,
                                   const HashSet<AnimatableCSSProperty> *animatedProperties,
                                   const css::StyleProperties *positionTryFallbackProperties) noexcept
      : _matchResult(matchResult), _includedProperties(krys::move(includedProperties)),
        _maximumOrigin(positionTryFallbackProperties ? Origin::PositionFallback : Origin::Author)
  {
    krys_debug_assert(!_includedProperties.IsEmpty());

    if (animatedProperties)
    {
      _animationLayer.emplace(*animatedProperties);
    }

    if (positionTryFallbackProperties)
    {
      _positionTryFallbackProperties = MatchedProperties {ShareRef(*positionTryFallbackProperties)};
    }

    BuildCascade();
  }

  PropertyCascade::PropertyCascade(
    const PropertyCascade &parent, Origin maximumOrigin, Maybe<ScopeOrdinal> rollbackScope,
    Maybe<style::CascadeLayerPriority> maximumCascadeLayerPriorityForRollback) noexcept
      : _matchResult(parent._matchResult),
        // Include all properties to the rollback cascade, lower prority layers may not get included
        // otherwise.
        _includedProperties(NormalProperties()), _maximumOrigin(maximumOrigin), _rollbackScope(rollbackScope),
        _maximumCascadeLayerPriorityForRollback(maximumCascadeLayerPriorityForRollback),
        _animationLayer(parent._animationLayer),
        _positionTryFallbackProperties(parent._positionTryFallbackProperties)
  {
    BuildCascade();
  }

  PropertyCascade::~PropertyCascade() noexcept = default;

  PropertyCascade::AnimationLayer::AnimationLayer(const HashSet<AnimatableCSSProperty> &properties) noexcept
      : Properties(properties)
  {
    HasCustomProperties = std::ranges::any_of(
      properties, [](auto &property) { return std::holds_alternative<css::CSSOMStringAtom>(property); });

    HasFontSize = properties.contains(css::PropertyId::FontSize);
    HasLineHeight = properties.contains(css::PropertyId::LineHeight);
  }

  void PropertyCascade::BuildCascade() noexcept
  {
    Array<bool, 3> originsWithImportant = {};

    for (auto origin : {Origin::UserAgent, Origin::User, Origin::Author})
    {
      if (origin > _maximumOrigin)
      {
        break;
      }
      bool hasImportant = AddNormalMatches(origin);
      if (hasImportant)
      {
        originsWithImportant[ToUnderlying(origin)] = true;
      }
    }

    if (_positionTryFallbackProperties)
    {
      AddPositionTryFallbackProperties();
    }

    for (auto origin : {Origin::Author, Origin::User, Origin::UserAgent})
    {
      if (!originsWithImportant[ToUnderlying(origin)])
      {
        continue;
      }
      AddImportantMatches(origin);
    }

    SortLogicalGroupPropertyIds();
  }

  void PropertyCascade::SetPropertyInternal(Property &property, css::PropertyId id,
                                            const css::Value &cssValue,
                                            const MatchedProperties &matchedProperties,
                                            Origin origin) noexcept
  {
    krys_debug_assert(matchedProperties.LinkMatchType <= css::SelectorChecker::MatchAll);
    property.Id = id;
    property.Origin = origin;
    property.StyleScopeOrdinal = matchedProperties.StyleScopeOrdinal;
    property.CascadeLayerPriority = matchedProperties.CascadeLayerPriority;
    property.FromStyleAttribute = matchedProperties.FromStyleAttribute;

    if (matchedProperties.LinkMatchType == css::SelectorChecker::MatchAll)
    {
      property.Origins[css::SelectorChecker::MatchDefault] = origin;
      property.CssValue[css::SelectorChecker::MatchDefault] = &cssValue;

      property.Origins[css::SelectorChecker::MatchLink] = origin;
      property.CssValue[css::SelectorChecker::MatchLink] = &cssValue;

      property.Origins[css::SelectorChecker::MatchVisited] = origin;
      property.CssValue[css::SelectorChecker::MatchVisited] = &cssValue;
    }
    else
    {
      property.Origins[matchedProperties.LinkMatchType] = origin;
      property.CssValue[matchedProperties.LinkMatchType] = &cssValue;
    }
  }

  void PropertyCascade::Set(css::PropertyId id, const css::Value &cssValue,
                            const MatchedProperties &matchedProperties, Origin origin) noexcept
  {
    krys_debug_assert(!css::Property::IsInLogicalPropertyGroup(id));
    krys_debug_assert(id < css::FirstLogicalGroupProperty);

    krys_debug_assert(ToUnderlying(id) < _propertyIsPresent.size());
    if (id == css::PropertyId::Custom)
    {
      _propertyIsPresent.set(ToUnderlying(id));
      const auto &customValue = Downcast<css::CustomPropertyValue>(cssValue);
      auto [iterator, isNewEntry] = _customProperties.try_emplace(customValue.Name());
      SetPropertyInternal(iterator->second, id, cssValue, matchedProperties, origin);
      return;
    }

    auto &property = _properties[ToUnderlying(id)];
    if (!_propertyIsPresent.test(ToUnderlying(id)))
    {
      _propertyIsPresent.set(ToUnderlying(id));
      property.CssValue = {};
    }
    SetPropertyInternal(property, id, cssValue, matchedProperties, origin);
  }

  void PropertyCascade::SetLogicalGroupProperty(css::PropertyId id, const css::Value &cssValue,
                                                const MatchedProperties &matchedProperties,
                                                Origin origin) noexcept
  {
    krys_debug_assert(id >= css::FirstLogicalGroupProperty);
    krys_debug_assert(id <= css::LastLogicalGroupProperty);

    auto &property = _properties[ToUnderlying(id)];
    if (!HasLogicalGroupProperty(id))
    {
      property.CssValue = {};
      _lowestSeenLogicalGroupProperty = std::min(_lowestSeenLogicalGroupProperty, id);
      _highestSeenLogicalGroupProperty = std::max(_highestSeenLogicalGroupProperty, id);
    }
    SetLogicalGroupPropertyIndex(id, ++_lastIndexForLogicalGroup);
    SetPropertyInternal(property, id, cssValue, matchedProperties, origin);
  }

  bool PropertyCascade::HasProperty(css::PropertyId propertyId, const css::Value &value) const noexcept
  {
    if (propertyId == css::PropertyId::Custom)
    {
      return HasCustomProperty(Downcast<css::CustomPropertyValue>(value).Name());
    }
    return propertyId < css::FirstLogicalGroupProperty ? HasNormalProperty(propertyId)
                                                       : HasLogicalGroupProperty(propertyId);
  }

  bool PropertyCascade::MayOverrideExistingProperty(css::PropertyId propertyId,
                                                    const css::Value &value) const noexcept
  {
    if (propertyId == css::PropertyId::Custom)
    {
      return HasCustomProperty(Downcast<css::CustomPropertyValue>(value).Name());
    }
    if (propertyId < css::FirstLogicalGroupProperty)
    {
      return HasNormalProperty(propertyId);
    }

    // Apply all logical group properties if we have applied any. They may override the ones we already
    // applied.
    // FIXME: This could check if any existing properties are actually in the same group.
    return !!_lastIndexForLogicalGroup;
  }

  const PropertyCascade::Property *
    PropertyCascade::LastPropertyResolvingLogicalPropertyPair(css::PropertyId propertyId,
                                                              WritingMode writingMode) const noexcept
  {
    krys_debug_assert(css::Property::IsInLogicalPropertyGroup(propertyId));

    auto pairId = [&]
    {
      if (css::Property::IsDirectionAwareProperty(propertyId))
      {
        return css::Property::ResolveDirectionAwareProperty(propertyId, writingMode);
      }
      return css::Property::UnresolvePhysicalProperty(propertyId, writingMode);
    }();
    krys_debug_assert(pairId != css::PropertyId::Invalid);

    auto indexForPropertyId = LogicalGroupPropertyIndex(propertyId);
    auto indexForPairId = LogicalGroupPropertyIndex(pairId);
    if (indexForPropertyId > indexForPairId)
    {
      return &LogicalGroupProperty(propertyId);
    }
    if (indexForPropertyId < indexForPairId)
    {
      return &LogicalGroupProperty(pairId);
    }
    krys_debug_assert(!HasLogicalGroupProperty(propertyId));
    krys_debug_assert(!HasLogicalGroupProperty(pairId));
    return nullptr;
  }

  bool PropertyCascade::AddMatch(const MatchedProperties &matchedProperties, Origin origin,
                                 css::IsImportant important) noexcept
  {
    auto includePropertiesForRollback = [&]
    {
      if (_rollbackScope && matchedProperties.StyleScopeOrdinal > *_rollbackScope)
      {
        return true;
      }
      if (origin < _maximumOrigin)
      {
        return true;
      }
      if (matchedProperties.FromStyleAttribute == FromStyleAttribute::Yes)
      {
        return false;
      }
      return matchedProperties.CascadeLayerPriority <= *_maximumCascadeLayerPriorityForRollback;
    };
    if (_maximumCascadeLayerPriorityForRollback && !includePropertiesForRollback())
    {
      return false;
    }

    if (matchedProperties.UsedRuleTypes.Contains(UsedRuleType::StartingStyle)
        && !_includedProperties.Types.Contains(PropertyType::StartingStyle))
    {
      return false;
    }

    auto propertyAllowlist = matchedProperties.AllowlistType;
    bool hasImportantProperties = false;

    for (auto current : *matchedProperties.Properties)
    {
      if (current.IsImportant())
      {
        hasImportantProperties = true;
      }
      if ((!important && current.IsImportant()) || (important && !current.IsImportant()))
      {
        continue;
      }

      auto propertyId = css::CascadeAliasProperty(current.Id());

      auto shouldIncludeProperty = [&]
      {
        if (propertyAllowlist == PropertyAllowlist::Cue && !IsValidCueStyleProperty(propertyId))
        {
          return false;
        }
        if (propertyAllowlist == PropertyAllowlist::CueSelector
            && !IsValidCueSelectorStyleProperty(propertyId))
        {
          return false;
        }
        if (propertyAllowlist == PropertyAllowlist::CueBackground
            && !IsValidCueBackgroundStyleProperty(propertyId))
        {
          return false;
        }
        if (propertyAllowlist == PropertyAllowlist::Marker && !IsValidMarkerStyleProperty(propertyId))
        {
          return false;
        }

        if (_includedProperties.Types.ContainsAll(NormalPropertyTypes()))
        {
          return true;
        }

        if (std::ranges::contains(_includedProperties.Ids, propertyId))
        {
          return true;
        }

        if (matchedProperties.IsCacheable == IsCacheable::Partially
            && _includedProperties.Types.Contains(PropertyType::NonCacheable))
        {
          return true;
        }

        // If we have applied this property for some reason already we must apply anything that overrides it.
        if (MayOverrideExistingProperty(propertyId, *current.Value()))
        {
          return true;
        }

        if (_includedProperties.Types.ContainsAny(
              {PropertyType::AfterAnimation, PropertyType::AfterTransition}))
        {
          if (ShouldApplyAfterAnimation(current))
          {
            _animationLayer->OverriddenProperties.insert(propertyId);
            return true;
          }
          return false;
        }

        bool currentIsInherited = css::Property::IsInheritedProperty(current.Id());
        if (_includedProperties.Types.Contains(PropertyType::Inherited) && currentIsInherited)
        {
          return true;
        }
        if (_includedProperties.Types.Contains(PropertyType::ExplicitlyInherited)
            && css::IsValueId(*current.Value(), css::ValueId::Inherit))
        {
          return true;
        }
        if (_includedProperties.Types.Contains(PropertyType::NonInherited) && !currentIsInherited)
        {
          return true;
        }

        return false;
      }();

      if (!shouldIncludeProperty)
      {
        continue;
      }

      if (propertyId < css::FirstLogicalGroupProperty)
      {
        Set(propertyId, *current.Value(), matchedProperties, origin);
      }
      else
      {
        SetLogicalGroupProperty(propertyId, *current.Value(), matchedProperties, origin);
      }
    }

    return hasImportantProperties;
  }

  bool PropertyCascade::ShouldApplyAfterAnimation(
    const css::StyleProperties::PropertyReference &property) noexcept
  {
    krys_debug_assert(_animationLayer);

    auto id = property.Id();
    auto *customProperty = DynamicDowncast<css::CustomPropertyValue>(*property.Value());

    auto isAnimatedProperty = [&]
    {
      if (customProperty)
      {
        return _animationLayer->Properties.contains(customProperty->Name());
      }
      return _animationLayer->Properties.contains(id);
    }();

    if (isAnimatedProperty)
    {
      // "Important declarations from all origins take precedence over animations."
      // https://drafts.csswg.org/css-cascade-5/#importance
      return _includedProperties.Types.Contains(PropertyType::AfterAnimation) && property.IsImportant();
    }

    // If we are animating custom properties they may affect other properties so we need to re-resolve them.
    if (_animationLayer->HasCustomProperties)
    {
      // We could check if the we are actually animating the referenced variable. Indirect cases would need to
      // be taken into account.
      if (customProperty && customProperty->IsVariableReference())
      {
        return true;
      }
      if (property.Value()->HasVariableReferences())
      {
        return true;
      }
    }

    // Check for 'em' units and similar property dependencies.
    if (_animationLayer->HasFontSize || _animationLayer->HasLineHeight)
    {
      auto dependencies = property.Value()->ComputedStyleDependencies();
      if (_animationLayer->HasFontSize
          && std::ranges::contains(dependencies.Properties, css::PropertyId::FontSize))
      {
        return true;
      }
      if (_animationLayer->HasLineHeight
          && std::ranges::contains(dependencies.Properties, css::PropertyId::LineHeight))
      {
        return true;
      }
    }

    return false;
  }

  void PropertyCascade::AddPositionTryFallbackProperties() noexcept
  {
    krys_debug_assert(_positionTryFallbackProperties);

    // "All of the properties in a @position-try are applied to the box as part of the Position Fallback
    // Origin, a new cascade origin that lies between the Author Origin and the Animation Origin"
    // https://drafts.csswg.org/css-anchor-position-1/#fallback-rule
    if (_maximumOrigin < Origin::PositionFallback)
    {
      return;
    }

    AddMatch(*_positionTryFallbackProperties, Origin::PositionFallback, css::IsImportant(false));
  }

  namespace
  {
    KRYS_NODISCARD const List<MatchedProperties> &
      DeclarationsForOrigin(const MatchResult &matchResult, PropertyCascade::Origin origin) noexcept
    {
      switch (origin)
      {
        case PropertyCascade::Origin::UserAgent:        return matchResult.UserAgentDeclarations;
        case PropertyCascade::Origin::User:             return matchResult.UserDeclarations;
        case PropertyCascade::Origin::Author:           return matchResult.AuthorDeclarations;
        case PropertyCascade::Origin::PositionFallback: break;
      }
      krys_debug_assert_msg(false, "The position fallback origin has no match result declarations.");
      return matchResult.AuthorDeclarations;
    }

    KRYS_NODISCARD bool HasImportantProperties(const css::StyleProperties &properties) noexcept
    {
      for (auto property : properties)
      {
        if (property.IsImportant())
        {
          return true;
        }
      }
      return false;
    }
  }

  bool PropertyCascade::AddNormalMatches(Origin origin) noexcept
  {
    bool hasImportant = false;
    for (auto &matchedDeclarations : DeclarationsForOrigin(_matchResult, origin))
    {
      hasImportant |= AddMatch(matchedDeclarations, origin, css::IsImportant(false));
    }

    return hasImportant;
  }

  void PropertyCascade::AddImportantMatches(Origin origin) noexcept
  {
    struct ImportantMatch
    {
      uint32 Index;
      ScopeOrdinal Ordinal;
      style::CascadeLayerPriority LayerPriority;
      style::FromStyleAttribute FromStyleAttribute;
    };
    List<ImportantMatch> importantMatches;
    bool hasMatchesFromOtherScopesOrLayers = false;

    auto &matchedDeclarations = DeclarationsForOrigin(_matchResult, origin);

    for (uint32 i = 0; i < matchedDeclarations.size(); ++i)
    {
      const MatchedProperties &matchedProperties = matchedDeclarations[i];

      if (!HasImportantProperties(*matchedProperties.Properties))
      {
        continue;
      }

      importantMatches.push_back({i, matchedProperties.StyleScopeOrdinal,
                                  matchedProperties.CascadeLayerPriority,
                                  matchedProperties.FromStyleAttribute});

      if (matchedProperties.StyleScopeOrdinal != ScopeOrdinal::Element
          || matchedProperties.CascadeLayerPriority != RuleSet::CascadeLayerPriorityForUnlayered)
      {
        hasMatchesFromOtherScopesOrLayers = true;
      }
    }

    if (importantMatches.empty())
    {
      return;
    }

    if (hasMatchesFromOtherScopesOrLayers)
    {
      // Match results are sorted in reverse tree context order so this is not needed for normal properties.
      std::ranges::stable_sort(importantMatches,
                               [](auto &a, auto &b)
                               {
                                 // For !important properties a later shadow tree wins.
                                 if (a.Ordinal != b.Ordinal)
                                 {
                                   return a.Ordinal < b.Ordinal;
                                 }
                                 // Lower priority layer wins, except if style attribute is involved.
                                 if (a.FromStyleAttribute != b.FromStyleAttribute)
                                 {
                                   return a.FromStyleAttribute == FromStyleAttribute::No;
                                 }
                                 return a.LayerPriority > b.LayerPriority;
                               });
    }

    for (auto &match : importantMatches)
    {
      AddMatch(matchedDeclarations[match.Index], origin, css::IsImportant(true));
    }
  }

  void PropertyCascade::SortLogicalGroupPropertyIds() noexcept
  {
    uint32 endIndex = 0;
    for (auto id = ToUnderlying(_lowestSeenLogicalGroupProperty);
         id <= ToUnderlying(_highestSeenLogicalGroupProperty); ++id)
    {
      auto propertyId = static_cast<css::PropertyId>(id);
      if (HasLogicalGroupProperty(propertyId))
      {
        _logicalGroupPropertyIds[endIndex++] = propertyId;
      }
    }
    _seenLogicalGroupPropertyCount = endIndex;
    auto logicalGroupPropertyIds = Span<css::PropertyId> {_logicalGroupPropertyIds}.first(endIndex);
    std::ranges::sort(logicalGroupPropertyIds, [&](auto id1, auto id2)
                      { return LogicalGroupPropertyIndex(id1) < LogicalGroupPropertyIndex(id2); });
  }

  HashSet<AnimatableCSSProperty> PropertyCascade::OverriddenAnimatedProperties() const noexcept
  {
    if (_animationLayer)
    {
      return _animationLayer->OverriddenProperties;
    }
    return {};
  }
}
