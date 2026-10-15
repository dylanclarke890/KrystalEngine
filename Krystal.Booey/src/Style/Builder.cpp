#include "Krystal.Booey/Style/Builder.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParser.hpp"
#include "Krystal.Booey/CSS/Properties/RegisteredCustomProperty.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/FunctionValue.hpp"
#include "Krystal.Booey/CSS/Values/GlobalKeyword.hpp"
#include "Krystal.Booey/CSS/Values/PendingSubstitutionValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePool.hpp"
#include "Krystal.Booey/CSS/Values/VariableReferenceValue.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Booey/Style/BuilderGenerated.hpp"
#include "Krystal.Booey/Style/BuilderStateInlines.hpp"
#include "Krystal.Booey/Style/Computed/ComputedStyle+InitialInlines.hpp"
#include "Krystal.Booey/Style/CustomProperty.hpp"
#include "Krystal.Booey/Style/CustomPropertyRegistry.hpp"
#include "Krystal.Booey/Style/MatchResult.hpp"
#include "Krystal.Booey/Style/RuleSet.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/Values/Page/PageSize.hpp"
#include "Krystal.Core/Assert.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo::style
{
  namespace
  {
    KRYS_NODISCARD bool IsValidVisitedLinkProperty(css::PropertyId id) noexcept
    {
      switch (id)
      {
        case css::PropertyId::BackgroundColor:
        case css::PropertyId::BorderLeftColor:
        case css::PropertyId::BorderRightColor:
        case css::PropertyId::BorderTopColor:
        case css::PropertyId::BorderBottomColor:
        case css::PropertyId::CaretColor:
        case css::PropertyId::Color:
        case css::PropertyId::OutlineColor:
        case css::PropertyId::ColumnRuleColor:
        case css::PropertyId::TextDecorationColor:
        case css::PropertyId::TextEmphasisColor:
        case css::PropertyId::Fill:
        case css::PropertyId::Stroke:
        case css::PropertyId::StrokeColor:         return true;
        default:                                   break;
      }

      return false;
    }

    KRYS_NODISCARD const css::StyleProperties *
      PositionTryFallbackProperties(const BuilderContext &context) noexcept
    {
      return context.PositionTryFallback ? context.PositionTryFallback->Properties.get() : nullptr;
    }

    KRYS_NODISCARD const CustomPropertyRegistry &Registry(const BuilderState &state) noexcept
    {
      return state.GetDocument().StyleScope().GetCustomPropertyRegistry();
    }
  }

  size_t Builder::RollbackCascadeKeyHash::operator()(const RollbackCascadeKey &key) const noexcept
  {
    return std::hash<uint64> {}((static_cast<uint64>(key.Origin) << 62)
                                ^ (static_cast<uint64>(key.ScopeOrdinal) << 32) ^ key.CascadeLayerPriority);
  }

  Builder::Builder(RenderStyle &style, BuilderContext &&context, const MatchResult &matchResult,
                   PropertyCascade::IncludedProperties &&includedProperties,
                   const HashSet<AnimatableCSSProperty> *animatedProperties) noexcept
      : _cascade(matchResult, krys::move(includedProperties), animatedProperties,
                 PositionTryFallbackProperties(context)),
        _state(CreateUnique<BuilderState>(style, krys::move(context)))
  {
  }

  Builder::~Builder() noexcept = default;

  void Builder::ApplyAllProperties() noexcept
  {
    if (_cascade.IsEmpty())
    {
      return;
    }

    ApplyTopPriorityProperties();
    ApplyHighPriorityProperties();
    ApplyNonHighPriorityProperties();

    AdjustAfterApplying();
  }

  // Top priority properties affect resolution of high priority properties.
  void Builder::ApplyTopPriorityProperties() noexcept
  {
    if (_cascade.ApplyLowPriorityOnly())
    {
      return;
    }

    ApplyProperties(css::FirstTopPriorityProperty, css::LastTopPriorityProperty);
    _state->AdjustStyleForInterCharacterRuby();
  }

  // High priority properties may affect resolution of other properties (they are mostly font related).
  void Builder::ApplyHighPriorityProperties() noexcept
  {
    if (_cascade.ApplyLowPriorityOnly())
    {
      return;
    }

    ApplyProperties(css::FirstHighPriorityProperty, css::LastHighPriorityProperty);
    _state->UpdateFont();
    // This needs to apply before other properties for the `lh` unit, but after updating the font.
    ApplyProperties(css::PropertyId::LineHeight, css::PropertyId::LineHeight);
  }

  void Builder::ApplyNonHighPriorityProperties() noexcept
  {
    krys_debug_assert(!_state->FontDirty());

    ApplyProperties(css::FirstLowPriorityProperty, css::LastLowPriorityProperty);
    ApplyLogicalGroupProperties();
    // Any referenced custom properties are already resolved. This will resolve the remaining ones.
    ApplyCustomProperties();

    krys_debug_assert(!_state->FontDirty());
  }

  void Builder::AdjustAfterApplying() noexcept
  {
    // FIXME: `Adjuster::adjustFromBuilder()` comes with the style adjuster.
  }

  void Builder::ApplyLogicalGroupProperties() noexcept
  {
    // Properties in a logical property group are applied in author specified order which is maintained
    // separately for them.
    for (auto id : _cascade.LogicalGroupPropertyIds())
    {
      ApplyCascadeProperty(_cascade.LogicalGroupProperty(id));
    }
  }

  void Builder::ApplyProperties(css::PropertyId firstProperty, css::PropertyId lastProperty) noexcept
  {
    if (_cascade.CustomProperties().empty()) KRYS_LIKELY
    {
      return ApplyPropertiesImpl<CustomPropertyCycleTracking::Disabled>(firstProperty, lastProperty);
    }

    return ApplyPropertiesImpl<CustomPropertyCycleTracking::Enabled>(firstProperty, lastProperty);
  }

  template <Builder::CustomPropertyCycleTracking trackCycles>
  void Builder::ApplyPropertiesImpl(css::PropertyId firstProperty, css::PropertyId lastProperty) noexcept
  {
    for (auto id = ToUnderlying(firstProperty); id <= ToUnderlying(lastProperty); ++id)
    {
      auto propertyId = static_cast<css::PropertyId>(id);
      krys_debug_assert(propertyId != css::PropertyId::Custom);
      if (!_cascade.HasNormalProperty(propertyId))
      {
        continue;
      }

      const auto &property = _cascade.NormalProperty(propertyId);

      if constexpr (trackCycles == CustomPropertyCycleTracking::Enabled)
      {
        _state->_inProgressProperties.set(id);
        ApplyCascadeProperty(property);
        _state->_inProgressProperties.reset(id);
      }
      else
      {
        // If we don't have any custom properties, then there can't be any cycles.
        ApplyCascadeProperty(property);
      }
    }
  }

  void Builder::ApplyCustomProperties() noexcept
  {
    for (const auto &[name, value] : _cascade.CustomProperties())
    {
      if (_state->_appliedCustomProperties.contains(name))
      {
        continue;
      }
      ApplyCustomPropertyImpl(name, value);
    }
  }

  void Builder::ApplyCustomProperty(const css::CSSOMStringAtom &name) noexcept
  {
    if (_state->_appliedCustomProperties.contains(name))
    {
      return;
    }

    auto iterator = _cascade.CustomProperties().find(name);
    if (iterator == _cascade.CustomProperties().end())
    {
      return;
    }

    ApplyCustomPropertyImpl(name, iterator->second);
  }

  void Builder::ApplyCustomPropertyImpl(const css::CSSOMStringAtom &name,
                                        const PropertyCascade::Property &property) noexcept
  {
    if (!property.CssValue[css::SelectorChecker::MatchDefault])
    {
      return;
    }

    Ref customPropertyValue = ShareRef(const_cast<css::CustomPropertyValue &>(
      Downcast<css::CustomPropertyValue>(*property.CssValue[css::SelectorChecker::MatchDefault])));

    bool inCycle = !_state->_inProgressCustomProperties.insert(name).second;
    if (inCycle)
    {
      auto isNewCycle = _state->_inCycleCustomProperties.insert(name).second;
      if (isNewCycle)
      {
        // Continue resolving dependencies so we detect cycles for them as well.
        std::ignore = ResolveCustomPropertyValue(*customPropertyValue);
      }
      return;
    }

    // There may be multiple cycles through the same property. Avoid interference from any previously detected
    // cycles.
    auto savedInCycleProperties = std::exchange(_state->_inCycleCustomProperties, {});

    auto createInvalidOrUnset = [&]() -> ResolvedCustomProperty
    {
      // https://drafts.csswg.org/css-variables-2/#invalid-variables
      const auto *registered = Registry(*_state).Get(name);
      // The property is a non-registered custom property:
      // The property is a registered custom property with universal syntax:
      // The computed value is the guaranteed-invalid value.
      if (!registered || registered->Syntax.IsUniversal())
      {
        return CustomProperty::CreateForGuaranteedInvalid(name);
      }
      // Otherwise:
      // ...as if the property’s value had been specified as the unset keyword.
      return css::GlobalKeyword::Unset;
    };

    auto resolvedValue = ResolveCustomPropertyValue(*customPropertyValue);

    if (!resolvedValue || _state->_inCycleCustomProperties.contains(name))
    {
      resolvedValue = createInvalidOrUnset();
    }

    {
      SetForScope levelScope(_state->_currentProperty, &property);
      SetForScope scopedLinkMatchMutation(_state->_linkMatch, css::SelectorChecker::MatchDefault);
      ApplyCustomProperty(name, krys::move(*resolvedValue));
    }

    _state->_inProgressCustomProperties.erase(name);
    _state->_appliedCustomProperties.insert(name);
    _state->_inCycleCustomProperties.insert(savedInCycleProperties.begin(), savedInCycleProperties.end());
  }

  void Builder::ApplyCascadeProperty(const PropertyCascade::Property &property) noexcept
  {
    SetForScope levelScope(_state->_currentProperty, &property);

    auto applyWithLinkMatch = [&](css::SelectorChecker::LinkMatchMask linkMatch)
    {
      if (property.CssValue[linkMatch])
      {
        SetForScope scopedLinkMatchMutation(_state->_linkMatch, linkMatch);
        ApplyProperty(property.Id, const_cast<css::Value &>(*property.CssValue[linkMatch]), linkMatch,
                      property.Origins[linkMatch]);
      }
    };

    applyWithLinkMatch(css::SelectorChecker::MatchDefault);

    if (_state->Style().GetInsideLink() == InsideLink::NotInside)
    {
      return;
    }

    applyWithLinkMatch(css::SelectorChecker::MatchLink);
    applyWithLinkMatch(css::SelectorChecker::MatchVisited);

    _state->_linkMatch = css::SelectorChecker::MatchDefault;
  }

  bool Builder::ApplyRollbackCascadeProperty(const PropertyCascade &rollbackCascade,
                                             css::PropertyId propertyId,
                                             css::SelectorChecker::LinkMatchMask linkMatchMask) noexcept
  {
    krys_debug_assert(propertyId != css::PropertyId::Custom);

    const auto *rollbackProperty = [&]() -> const PropertyCascade::Property *
    {
      if (propertyId < css::FirstLogicalGroupProperty)
      {
        if (rollbackCascade.HasNormalProperty(propertyId))
        {
          return &rollbackCascade.NormalProperty(propertyId);
        }
        return nullptr;
      }
      return rollbackCascade.LastPropertyResolvingLogicalPropertyPair(propertyId,
                                                                      _state->Style().GetWritingMode());
    }();

    if (!rollbackProperty)
    {
      return false;
    }

    if (const auto *value = rollbackProperty->CssValue[linkMatchMask])
    {
      SetForScope levelScope(_state->_currentProperty, rollbackProperty);
      ApplyProperty(propertyId, const_cast<css::Value &>(*value), linkMatchMask, rollbackProperty->Origin);
    }
    return true;
  }

  bool Builder::ApplyRollbackCascadeCustomProperty(const PropertyCascade &rollbackCascade,
                                                   const css::CSSOMStringAtom &name) noexcept
  {
    auto iterator = rollbackCascade.CustomProperties().find(name);
    if (iterator == rollbackCascade.CustomProperties().end())
    {
      return false;
    }

    const auto &rollbackProperty = iterator->second;
    if (const auto *value = rollbackProperty.CssValue[css::SelectorChecker::MatchDefault])
    {
      Ref customPropertyValue =
        ShareRef(const_cast<css::CustomPropertyValue &>(Downcast<css::CustomPropertyValue>(*value)));

      SetForScope levelScope(_state->_currentProperty, &rollbackProperty);
      auto resolvedValue = ResolveCustomPropertyValue(*customPropertyValue);
      if (!resolvedValue)
      {
        resolvedValue = CustomProperty::CreateForGuaranteedInvalid(name);
      }

      ApplyCustomProperty(name, krys::move(*resolvedValue));
    }
    return true;
  }

  void Builder::ApplyProperty(css::PropertyId id, css::Value &value,
                              css::SelectorChecker::LinkMatchMask linkMatchMask,
                              PropertyCascade::Origin cascadeOrigin) noexcept
  {
    krys_debug_assert_msg(!css::IsShorthand(id), "A shorthand property wasn't expanded at parsing time");
    krys_debug_assert_msg(id != css::PropertyId::Custom,
                          "Custom properties are applied by ApplyCustomProperty");

    auto valueToApply = ResolveInternalAutoBaseFunction(value);
    valueToApply = ResolveVariableReferences(id, *valueToApply);
    auto &style = _state->Style();

    if (css::Property::IsDirectionAwareProperty(id))
    {
      auto newId = css::Property::ResolveDirectionAwareProperty(id, style.GetWritingMode());
      krys_debug_assert(newId != id);
      return ApplyProperty(newId, *valueToApply, linkMatchMask, cascadeOrigin);
    }

    // FIXME: Resolve the property for the position-try fallback's tactics
    // (`AnchorPositionEvaluator::resolvePositionTryFallbackProperty`) once anchor positioning is ported.

    auto valueId = valueToApply->ValueId();
    auto valueType = [&]
    {
      if (valueId == css::ValueId::Inherit)
      {
        return ApplyValueType::Inherit;
      }
      if (valueId == css::ValueId::Initial)
      {
        return ApplyValueType::Initial;
      }
      return ApplyValueType::Value;
    }();

    bool isUnset = valueId == css::ValueId::Unset;
    bool isRevert = valueId == css::ValueId::Revert;
    bool isRevertLayer = valueId == css::ValueId::RevertLayer;
    bool isRevertOrRevertLayer = isRevert || isRevertLayer;

    if (isRevertOrRevertLayer)
    {
      // In @keyframes, 'revert-layer' rolls back the cascaded value to the author level.
      // We can just not apply the property in order to keep the value from the base style.
      if (isRevertLayer && _state->_isBuildingKeyframeStyle)
      {
        return;
      }

      const auto *rollbackCascade =
        isRevert ? EnsureRollbackCascadeForRevert() : EnsureRollbackCascadeForRevertLayer();

      if (rollbackCascade)
      {
        // With the rollback cascade built, we need to obtain the property and apply it. If the property is
        // not present, then we behave like "unset." Otherwise we apply the property instead of our own.
        if (ApplyRollbackCascadeProperty(*rollbackCascade, id, linkMatchMask))
        {
          return;
        }
      }
    }

    auto isInheritedProperty = [&]
    {
      return css::Property::IsInheritedProperty(id);
    };

    auto unsetValueType = [&]
    {
      // https://drafts.csswg.org/css-cascade-4/#inherit-initial
      // The unset CSS-wide keyword acts as either inherit or initial, depending on whether the property is
      // inherited or not.
      return isInheritedProperty() ? ApplyValueType::Inherit : ApplyValueType::Initial;
    };

    if (isUnset || isRevertOrRevertLayer)
    {
      valueType = unsetValueType();
    }

    if (!_state->ApplyPropertyToRegularStyle() && !IsValidVisitedLinkProperty(id))
    {
      // Limit the properties that can be applied to only the ones honored by :visited.
      return;
    }

    if (valueType == ApplyValueType::Inherit && !isInheritedProperty())
    {
      style.SetHasExplicitlyInheritedProperties();
    }

    // FIXME: Watch the input properties of a `paint()` image's paint worklet (`addCustomPaintWatchProperty`)
    // once paint worklets are ported.

    if (id == css::PropertyId::Size && valueType == ApplyValueType::Value) KRYS_UNLIKELY
    {
      ApplyPageSizeDescriptor(*valueToApply);
      return;
    }

    BuilderGenerated::ApplyProperty(id, *_state, *valueToApply, valueType);

    if (!isRevertOrRevertLayer)
    {
      _state->DisableNativeAppearanceIfNeeded(id, cascadeOrigin);
    }

    if (!isUnset && !isRevertOrRevertLayer && _state->IsCurrentPropertyInvalidAtComputedValueTime())
    {
      // https://drafts.csswg.org/css-variables-2/#invalid-variables
      // A declaration can be invalid at computed-value time if...
      // When this happens, the computed value is one of the following...
      // Otherwise: Either the property’s inherited value or its initial value depending on whether the
      // property is inherited or not, respectively, as if the property’s value had been specified as the
      // unset keyword
      BuilderGenerated::ApplyProperty(id, *_state, *valueToApply, unsetValueType());
    }
  }

  void Builder::ApplyCustomProperty(const css::CSSOMStringAtom &name,
                                    ResolvedCustomProperty &&parsedCustomProperty) noexcept
  {
    auto &style = _state->Style();

    const auto *registeredCustomProperty = Registry(*_state).Get(name);

    auto applyValue = [&](Ref<const CustomProperty> &&valueToApply)
    {
      bool isInherited = !registeredCustomProperty || registeredCustomProperty->Inherits;
      _state->Style().SetCustomPropertyValue(krys::move(valueToApply), isInherited);
    };

    auto applyInitial = [&]
    {
      if (registeredCustomProperty && registeredCustomProperty->InitialValue)
      {
        applyValue(Ref<const CustomProperty> {registeredCustomProperty->InitialValue});
        return;
      }
      applyValue(CustomProperty::CreateForGuaranteedInvalid(name));
    };

    auto applyInherit = [&]
    {
      const auto *parentValue = _state->ParentStyle().InheritedCustomProperties().Get(name);
      if (parentValue && !(registeredCustomProperty && !registeredCustomProperty->Inherits))
      {
        applyValue(ShareRef(*parentValue));
        return;
      }
      if (const auto *nonInheritedParentValue =
            _state->ParentStyle().NonInheritedCustomProperties().Get(name))
      {
        applyValue(ShareRef(*nonInheritedParentValue));
        return;
      }
      applyInitial();
    };

    krys::SwitchOn(
      krys::move(parsedCustomProperty),
      [&](css::GlobalKeyword keyword)
      {
        ApplyValueType valueType = ApplyValueType::Value;
        bool isRevert = false;
        bool isRevertLayer = false;

        auto isInheritedProperty = [&]
        {
          return registeredCustomProperty ? registeredCustomProperty->Inherits : true;
        };

        auto unsetValueType = [&]
        {
          // https://drafts.csswg.org/css-cascade-4/#inherit-initial
          // The unset CSS-wide keyword acts as either inherit or initial, depending on whether the property
          // is inherited or not.
          return isInheritedProperty() ? ApplyValueType::Inherit : ApplyValueType::Initial;
        };

        switch (keyword)
        {
          case css::GlobalKeyword::Initial: valueType = ApplyValueType::Initial; break;
          case css::GlobalKeyword::Inherit: valueType = ApplyValueType::Inherit; break;
          case css::GlobalKeyword::Unset:   valueType = unsetValueType(); break;
          case css::GlobalKeyword::Revert:
            isRevert = true;
            valueType = unsetValueType();
            break;
          case css::GlobalKeyword::RevertLayer:
            isRevertLayer = true;
            valueType = unsetValueType();
            break;
        }

        if (isRevert || isRevertLayer)
        {
          // In @keyframes, 'revert-layer' rolls back the cascaded value to the author level.
          // We can just not apply the property in order to keep the value from the base style.
          if (isRevertLayer && _state->_isBuildingKeyframeStyle)
          {
            return;
          }

          const auto *rollbackCascade =
            isRevert ? EnsureRollbackCascadeForRevert() : EnsureRollbackCascadeForRevertLayer();

          if (rollbackCascade)
          {
            // With the rollback cascade built, we need to obtain the property and apply it. If the property
            // is not present, then we behave like "unset." Otherwise we apply the property instead of our
            // own.
            if (ApplyRollbackCascadeCustomProperty(*rollbackCascade, name))
            {
              return;
            }
          }
        }

        if (!_state->ApplyPropertyToRegularStyle())
        {
          // Limit the properties that can be applied to only the ones honored by :visited.
          return;
        }

        if (valueType == ApplyValueType::Inherit && !isInheritedProperty())
        {
          style.SetHasExplicitlyInheritedProperties();
        }

        switch (valueType)
        {
          case ApplyValueType::Initial: applyInitial(); break;
          case ApplyValueType::Inherit: applyInherit(); break;
          case ApplyValueType::Value:   krys_debug_assert(false); break;
        }
      },
      [&](Ref<const CustomProperty> &&resolved)
      {
        if (!_state->ApplyPropertyToRegularStyle())
        {
          // Limit the properties that can be applied to only the ones honored by :visited.
          return;
        }
        applyValue(krys::move(resolved));
      });
  }

  Ref<css::Value> Builder::ResolveInternalAutoBaseFunction(css::Value &value) noexcept
  {
    const auto *functionValue = DynamicDowncast<css::FunctionValue>(value);
    if (!functionValue)
    {
      return ShareRef(value);
    }

    if (functionValue->Name() != css::ValueId::InternalAutoBase)
    {
      return ShareRef(value);
    }

    const auto *result =
      _state->Style().Appearance() == StyleAppearance::Base ? functionValue->Item(1) : functionValue->Item(0);

    if (!result)
    {
      return ShareRef(value);
    }

    return ShareRef(const_cast<css::Value &>(*result));
  }

  Ref<css::Value> Builder::ResolveVariableReferences(css::PropertyId propertyId, css::Value &value) noexcept
  {
    if (!value.HasVariableReferences())
    {
      return ShareRef(value);
    }

    auto variableValue = [&]() -> RefPtr<css::Value>
    {
      if (const auto *substitution = DynamicDowncast<css::PendingSubstitutionValue>(value))
      {
        return substitution->ResolveValue(*this, propertyId);
      }

      const auto &variableReferenceValue = Downcast<css::VariableReferenceValue>(value);
      return variableReferenceValue.ResolveSingleValue(*this, propertyId);
    }();

    // https://drafts.csswg.org/css-variables-2/#invalid-variables
    // ...as if the property’s value had been specified as the unset keyword.
    if (!variableValue || _state->_invalidAtComputedValueTimeProperties.test(ToUnderlying(propertyId)))
    {
      return css::PrimitiveValue::Create(css::ValueId::Unset);
    }

    return krys::move(variableValue);
  }

  RefPtr<const CustomProperty>
    Builder::ResolveCustomPropertyForContainerQueries(const css::CustomPropertyValue &value) noexcept
  {
    auto resolvedValue = ResolveCustomPropertyValue(const_cast<css::CustomPropertyValue &>(value));
    if (!resolvedValue)
    {
      return CustomProperty::CreateForGuaranteedInvalid(value.Name());
    }

    return krys::SwitchOn(
      *resolvedValue,
      [&](css::GlobalKeyword keyword) -> RefPtr<const CustomProperty>
      {
        const auto &name = value.Name();
        const auto *registered = Registry(*_state).Get(name);
        bool isInherited = !registered || registered->Inherits;

        auto initial = [&]() -> RefPtr<const CustomProperty>
        {
          if (registered)
          {
            return registered->InitialValue;
          }
          return CustomProperty::CreateForGuaranteedInvalid(name);
        };

        auto inherit = [&]() -> RefPtr<const CustomProperty>
        {
          const auto *parentValue = isInherited
                                      ? _state->ParentStyle().InheritedCustomProperties().Get(name)
                                      : _state->ParentStyle().NonInheritedCustomProperties().Get(name);
          if (parentValue)
          {
            return ShareRefPtr(parentValue);
          }

          return initial();
        };

        switch (keyword)
        {
          case css::GlobalKeyword::Initial: return initial();
          case css::GlobalKeyword::Inherit: return inherit();
          case css::GlobalKeyword::Unset:   return isInherited ? inherit() : initial();
          case css::GlobalKeyword::Revert:
          case css::GlobalKeyword::RevertLayer:
            // https://drafts.csswg.org/css-contain-3/#style-container
            // "Cascade-dependent keywords, such as revert and revert-layer, are invalid as values in a style
            // feature, and cause the container style query to be false."
            return nullptr;
        }
        krys_unreachable();
      },
      [](const Ref<const CustomProperty> &resolvedValue) -> RefPtr<const CustomProperty>
      { return resolvedValue; });
  }

  auto Builder::ResolveCustomPropertyValue(css::CustomPropertyValue &value) noexcept
    -> Maybe<ResolvedCustomProperty>
  {
    const auto &name = value.Name();

    if (auto keyword = value.TryGlobalKeyword())
    {
      return ResolvedCustomProperty {*keyword};
    }

    const auto *registered = Registry(*_state).Get(name);

    auto preResolved = krys::SwitchOn(
      value.Value(),
      [&](const Ref<css::VariableReferenceValue> &) -> Maybe<ResolvedCustomProperty> { return std::nullopt; },
      [&](const Ref<css::VariableData> &data) -> Maybe<ResolvedCustomProperty>
      {
        if (!registered)
        {
          return ResolvedCustomProperty {CustomProperty::CreateForVariableData(name, Ref {data})};
        }
        return std::nullopt;
      },
      [&](css::GlobalKeyword keyword) -> Maybe<ResolvedCustomProperty>
      {
        if (!registered)
        {
          return ResolvedCustomProperty {keyword};
        }
        return std::nullopt;
      });
    if (preResolved)
    {
      return preResolved;
    }

    auto resolvedData = krys::SwitchOn(
      value.Value(),
      [&](const Ref<css::VariableReferenceValue> &variableReferenceValue) -> RefPtr<css::VariableData>
      { return variableReferenceValue->ResolveVariableReferences(*this); },
      [&](const Ref<css::VariableData> &data) -> RefPtr<css::VariableData> { return data; },
      [&](css::GlobalKeyword) -> RefPtr<css::VariableData> { return nullptr; });

    if (!resolvedData)
    {
      return std::nullopt;
    }

    if (!registered)
    {
      // CSS-wide keywords are allowed in var() fallbacks of unregistered properties.
      if (auto keyword = css::PropertyParser::ParseGlobalKeyword(resolvedData->TokenRange()))
      {
        return ResolvedCustomProperty {*keyword};
      }

      Ref<css::VariableData> data = krys::move(resolvedData);
      return ResolvedCustomProperty {CustomProperty::CreateForVariableData(name, krys::move(data))};
    }

    auto dependencies = css::PropertyParser::CollectParsedCustomPropertyValueDependencies(
      registered->Syntax, resolvedData->TokenRange(), resolvedData->Context());

    // https://drafts.css-houdini.org/css-properties-values-api/#dependency-cycles
    bool hasCycles = false;
    bool isFontDependent = false;

    auto checkDependencies = [&](const auto &propertyDependencies)
    {
      for (auto property : propertyDependencies)
      {
        if (_state->_inProgressProperties.test(ToUnderlying(property)))
        {
          _state->_invalidAtComputedValueTimeProperties.set(ToUnderlying(property));
          hasCycles = true;
        }
        if (property == css::PropertyId::FontSize)
        {
          isFontDependent = true;
        }
      }
    };

    checkDependencies(dependencies.Properties);

    if (_state->GetElement() && _state->GetElement() == _state->GetDocument().DocumentElement().get())
    {
      checkDependencies(dependencies.RootProperties);
    }

    if (hasCycles)
    {
      return std::nullopt;
    }

    if (isFontDependent)
    {
      _state->UpdateFont();
    }

    return css::PropertyParser::ParseTypedCustomPropertyValue(
      name, registered->Syntax, resolvedData->TokenRange(), *_state, resolvedData->Context());
  }

  void Builder::ApplyPageSizeDescriptor(css::Value &value) noexcept
  {
    _state->Style().SetPageSize(ComputedStyle::InitialPageSize());

    auto convertedPageSize = ConvertCSSValueToStyle<PageSize>(*_state, value);

    if (_state->IsCurrentPropertyInvalidAtComputedValueTime())
    {
      return;
    }

    _state->Style().SetPageSize(krys::move(convertedPageSize));
  }

  const PropertyCascade *Builder::EnsureRollbackCascadeForRevert() noexcept
  {
    auto rollbackOrigin = _state->_currentProperty->Origin;

    switch (rollbackOrigin)
    {
      case PropertyCascade::Origin::PositionFallback:
        // "Similar to the Animation Origin, use of the revert value acts as if the property was part of the
        // Author Origin." https://drafts.csswg.org/css-anchor-position-1/#fallback-rule
      case PropertyCascade::Origin::Author:           rollbackOrigin = PropertyCascade::Origin::User; break;
      case PropertyCascade::Origin::User:             rollbackOrigin = PropertyCascade::Origin::UserAgent; break;
      case PropertyCascade::Origin::UserAgent:        return nullptr;
    }

    auto key = MakeRollbackCascadeKey(rollbackOrigin);
    auto [iterator, isNewEntry] = _rollbackCascades.try_emplace(key);
    if (isNewEntry)
    {
      iterator->second = CreateUnique<const PropertyCascade>(_cascade, rollbackOrigin);
    }
    return iterator->second.get();
  }

  const PropertyCascade *Builder::EnsureRollbackCascadeForRevertLayer() noexcept
  {
    const auto &property = *_state->_currentProperty;
    auto rollbackLayerPriority = property.CascadeLayerPriority;
    if (!rollbackLayerPriority)
    {
      return EnsureRollbackCascadeForRevert();
    }

    krys_debug_assert(property.FromStyleAttribute == FromStyleAttribute::No
                      || property.CascadeLayerPriority == RuleSet::CascadeLayerPriorityForUnlayered);

    // Style attribute reverts to the regular author style.
    if (property.FromStyleAttribute == FromStyleAttribute::No)
    {
      --rollbackLayerPriority;
    }

    auto key = MakeRollbackCascadeKey(property.Origin, property.StyleScopeOrdinal, rollbackLayerPriority);
    auto [iterator, isNewEntry] = _rollbackCascades.try_emplace(key);
    if (isNewEntry)
    {
      iterator->second = CreateUnique<const PropertyCascade>(
        _cascade, property.Origin, property.StyleScopeOrdinal, rollbackLayerPriority);
    }
    return iterator->second.get();
  }

  auto Builder::MakeRollbackCascadeKey(PropertyCascade::Origin cascadeOrigin, ScopeOrdinal scopeOrdinal,
                                       CascadeLayerPriority cascadeLayerPriority) noexcept
    -> RollbackCascadeKey
  {
    return {static_cast<uint32>(cascadeOrigin), static_cast<uint32>(static_cast<int32>(scopeOrdinal)),
            static_cast<uint32>(cascadeLayerPriority)};
  }
}
