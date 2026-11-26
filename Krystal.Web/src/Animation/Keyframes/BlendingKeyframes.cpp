#include "Krystal.Web/Animation/Keyframes/BlendingKeyframes.hpp"
#include "Krystal.Web/Animation/CSSAnimation.hpp"
#include "Krystal.Web/Animation/KeyframeEffect.hpp"
#include "Krystal.Web/Css/Properties/MutableStyleProperties.hpp"
#include "Krystal.Web/Css/Properties/StyleProperties.hpp"
#include "Krystal.Web/Css/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Web/Css/Rules/KeyframeRule.hpp"
#include "Krystal.Web/Css/Values/ComputedStyleDependencies.hpp"
#include "Krystal.Web/Css/Values/CustomPropertyValue.hpp"
#include "Krystal.Web/Css/Values/PrimitiveValue.hpp"
#include "Krystal.Web/Dom/Nodes/Element.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Web/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Web/Style/Interpolation/Interpolation.hpp"
#include "Krystal.Web/Style/Resolver.hpp"
#include <algorithm>

namespace krys::web
{
  namespace
  {
    /// @brief An empty keyframe rule at @p key, which the style of an implicit keyframe is resolved from.
    KRYS_NODISCARD Ref<css::KeyframeRule> EmptyKeyframeRule(double key) noexcept
    {
      auto keyframe = css::KeyframeRule::Create(css::MutableStyleProperties::Create());
      keyframe->SetKey({css::ValueId::Invalid, key});
      return keyframe;
    }

    KRYS_NODISCARD const css::KeyframeRule &ZeroPercentKeyframe() noexcept
    {
      static NeverDestroyed<Ref<css::KeyframeRule>> rule {EmptyKeyframeRule(0)};
      return **rule;
    }

    KRYS_NODISCARD const css::KeyframeRule &HundredPercentKeyframe() noexcept
    {
      static NeverDestroyed<Ref<css::KeyframeRule>> rule {EmptyKeyframeRule(1)};
      return **rule;
    }

    /// @brief The properties in @p all that aren't in @p excluded.
    KRYS_NODISCARD AnimatableCSSPropertySet PropertiesNotIn(const AnimatableCSSPropertySet &all,
                                                            const AnimatableCSSPropertySet &excluded) noexcept
    {
      AnimatableCSSPropertySet difference;
      for (const auto &property : all)
      {
        if (!excluded.contains(property))
        {
          difference.insert(property);
        }
      }
      return difference;
    }
  }

  Maybe<CompositeOperation> KeyframeRuleCompositeOperation(const css::KeyframeRule &keyframe) noexcept
  {
    auto value = keyframe.Properties().GetPropertyCSSValue(css::PropertyId::AnimationComposition);
    if (!value || !Is<css::PrimitiveValue>(*value))
    {
      return null;
    }
    switch (value->ValueId())
    {
      case css::ValueId::Add:        return CompositeOperation::Add;
      case css::ValueId::Accumulate: return CompositeOperation::Accumulate;
      case css::ValueId::Replace:    return CompositeOperation::Replace;
      default:                       return null;
    }
  }

#pragma region BlendingKeyframe

  BlendingKeyframe::BlendingKeyframe(KeyframeOffset &&offset, UniquePtr<RenderStyle> &&style) noexcept
      : _specifiedOffset(krys::move(offset)), _style(krys::move(style))
  {
    // A range offset is resolved later, against the timeline.
    if (!UsesRangeOffset())
    {
      _computedOffset = _specifiedOffset.Value;
    }
  }

  BlendingKeyframe::BlendingKeyframe(const BlendingKeyframe &source) noexcept
      : _specifiedOffset(source._specifiedOffset), _computedOffset(source._computedOffset),
        _properties(source._properties), _style(RenderStyle::ClonePtr(*source.Style())),
        _timingFunction(source._timingFunction), _compositeOperation(source._compositeOperation),
        _containsDirectionAwareProperty(source._containsDirectionAwareProperty)
  {
  }

  bool BlendingKeyframe::AnimatesProperty(KeyframeInterpolation::Property property) const noexcept
  {
    return _properties.contains(property);
  }

  void BlendingKeyframe::AddProperty(const AnimatableCSSProperty &property) noexcept
  {
    krys_debug_assert(!std::holds_alternative<css::PropertyId>(property)
                      || std::get<css::PropertyId>(property) != css::PropertyId::Custom);
    _properties.insert(property);
  }

  bool BlendingKeyframe::UsesRangeOffset() const noexcept
  {
    return _specifiedOffset.Name != style::SingleAnimationRangeName::Omitted
           && _specifiedOffset.Name != style::SingleAnimationRangeName::Normal;
  }

#pragma endregion

#pragma region BlendingKeyframes

  BlendingKeyframes::~BlendingKeyframes() noexcept = default;

  uint64 BlendingKeyframes::NextAnonymousIdentifier() noexcept
  {
    static uint64 numericIdentifier = 0;
    return ++numericIdentifier;
  }

  const StringAtom &BlendingKeyframes::KeyframesName() const noexcept
  {
    if (const auto *name = std::get_if<StringAtom>(&_identifier))
    {
      return *name;
    }
    return StringAtom::Null();
  }

  const utf8_string &BlendingKeyframes::AcceleratedAnimationName() const noexcept
  {
    if (_acceleratedAnimationName.empty())
    {
      if (const auto *name = std::get_if<StringAtom>(&_identifier))
      {
        _acceleratedAnimationName = utf8_string(name->View());
      }
      else
      {
        auto numericName = std::format("keyframe-effect-{}", std::get<uint64>(_identifier));
        _acceleratedAnimationName = utf8_string(numericName.begin(), numericName.end());
      }
    }
    return _acceleratedAnimationName;
  }

  void BlendingKeyframes::Insert(BlendingKeyframe &&keyframe) noexcept
  {
    if (!keyframe.UsesRangeOffset() && (keyframe.Offset() < 0 || keyframe.Offset() > 1))
    {
      return;
    }

    AnalyzeKeyframe(keyframe);

    // After the keyframes at the same offset, so they keep their order.
    auto position = std::ranges::find_if(_keyframes, [&](const BlendingKeyframe &existing)
                                         { return existing.Offset() > keyframe.Offset(); });
    auto inserted = _keyframes.insert(position, krys::move(keyframe));
    for (const auto &property : inserted->Properties())
    {
      _properties.insert(property);
    }
  }

  void BlendingKeyframes::CopyKeyframes(const BlendingKeyframes &other) noexcept
  {
    for (const auto &keyframe : other)
    {
      auto copy = keyframe;
      Insert(krys::move(copy));
    }
  }

  void BlendingKeyframes::Clear() noexcept
  {
    // Everything but the name: the analysis goes with the keyframes it came from.
    _keyframes.clear();
    _properties.clear();
    _explicitToProperties.clear();
    _explicitFromProperties.clear();
    _propertiesSetToInherit.clear();
    _propertiesSetToCurrentColor.clear();
    _usesRelativeFontWeight = false;
    _containsCSSVariableReferences = false;
    _usesAnchorFunctions = false;
    _hasWidthDependentTransform = false;
    _hasHeightDependentTransform = false;
    _hasDiscreteTransformInterval = false;
    _hasExplicitlyInheritedKeyframeProperty = false;
    _hasKeyframeNotUsingRangeOffset = false;
  }

  void BlendingKeyframes::UpdatedComputedOffsets(
    KRYS_NOESCAPE const Func<double(const BlendingKeyframe::KeyframeOffset &)> &callback) noexcept
  {
    for (auto &keyframe : _keyframes)
    {
      keyframe.SetComputedOffset(callback(keyframe.SpecifiedOffset()));
    }
    std::ranges::stable_sort(_keyframes, {}, &BlendingKeyframe::Offset);
  }

  bool BlendingKeyframes::ContainsProperty(const AnimatableCSSProperty &property) const noexcept
  {
    return _properties.contains(property);
  }

  bool BlendingKeyframes::ContainsAnimatableCSSProperty() const noexcept
  {
    return std::ranges::any_of(_properties, [](const AnimatableCSSProperty &property)
                               { return style::interpolation::CanInterpolate(property); });
  }

  bool BlendingKeyframes::ContainsDirectionAwareProperty() const noexcept
  {
    return std::ranges::any_of(_keyframes, [](const BlendingKeyframe &keyframe)
                               { return keyframe.ContainsDirectionAwareProperty(); });
  }

#pragma endregion

#pragma region Implicit keyframes

  bool BlendingKeyframes::HasImplicitKeyframes() const noexcept
  {
    return Size() && (_keyframes[0].Offset() || _keyframes[Size() - 1].Offset() != 1);
  }

  bool BlendingKeyframes::HasImplicitKeyframeForProperty(const AnimatableCSSProperty &property) const noexcept
  {
    return HasImplicitKeyframes()
           && (!_explicitFromProperties.contains(property) || !_explicitToProperties.contains(property));
  }

  void BlendingKeyframes::FillImplicitKeyframes(const KeyframeEffect &effect,
                                                const RenderStyle &underlyingStyle) noexcept
  {
    if (IsEmpty())
    {
      return;
    }

    krys_debug_assert(effect.Target());
    auto element = ShareRef(*effect.Target());
    if (!element->IsConnected())
    {
      return;
    }
    auto &styleResolver = element->StyleResolver();

    // Every property a keyframe sets is expected at 0% and at 100%; those missing there are implicit.
    AnimatableCSSPropertySet expectedExplicitProperties;
    for (const auto &keyframe : _keyframes)
    {
      if (keyframe.HasResolvedOffset())
      {
        expectedExplicitProperties.insert(keyframe.Properties().begin(), keyframe.Properties().end());
      }
    }

    // An existing 0% or 100% keyframe takes the implicit values if it's at a plain offset and has no timing
    // function of its own, or (for a CSS animation) the one its keyframes default to.
    auto canTakeImplicitValues = [&](const BlendingKeyframe &keyframe)
    {
      if (keyframe.UsesRangeOffset())
      {
        return false;
      }
      auto *timingFunction = keyframe.GetTimingFunction();
      if (!timingFunction)
      {
        return true;
      }
      if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(effect.Animation()))
      {
        // The default is the animation's keyframes' default timing function if it has one, and `ease`
        // otherwise.
        if (const auto &animationTimingFunction =
              cssAnimation->BackingStyleAnimation().DefaultTimingFunctionForKeyframes())
        {
          return timingFunction == animationTimingFunction->Value.get();
        }
        return timingFunction == &CubicBezierTimingFunction::DefaultTimingFunction();
      }
      return false;
    };

    // Gives @p implicitProperties their underlying values at @p key: in @p existingKeyframe if there's one
    // that can take them, or else in a new keyframe resolved from @p keyframeRule.
    auto addImplicitKeyframe = [&](double key, const AnimatableCSSPropertySet &implicitProperties,
                                   const css::KeyframeRule &keyframeRule, BlendingKeyframe *existingKeyframe)
    {
      if (existingKeyframe)
      {
        krys_debug_assert(existingKeyframe->Style());
        auto keyframeStyle = RenderStyle::ClonePtr(*existingKeyframe->Style());
        for (const auto &property : implicitProperties)
        {
          style::interpolation::Interpolate(property, *keyframeStyle, underlyingStyle, underlyingStyle, 1,
                                            krys::CompositeOperation::Replace, effect);
          existingKeyframe->AddProperty(property);
        }
        existingKeyframe->SetStyle(krys::move(keyframeStyle));
        return;
      }

      BlendingKeyframe blendingKeyframe(key, nullptr);
      blendingKeyframe.SetStyle(
        styleResolver.StyleForKeyframe(*element, underlyingStyle, {nullptr}, keyframeRule, blendingKeyframe));
      for (const auto &property : implicitProperties)
      {
        blendingKeyframe.AddProperty(property);
      }
      // A CSS animation's keyframes replace by default (https://drafts.csswg.org/css-animations-2/#keyframes,
      // step 2).
      if (effect.Animation() && Is<CSSAnimation>(*effect.Animation()))
      {
        blendingKeyframe.SetCompositeOperation(krys::CompositeOperation::Replace);
      }
      Insert(krys::move(blendingKeyframe));
    };

    // The properties the keyframes at @p isAtKey's offsets set, and the first of them that can take the
    // implicit values.
    auto explicitPropertiesAt = [&](auto isAtKey)
    {
      AnimatableCSSPropertySet explicitProperties;
      BlendingKeyframe *implicitKeyframe = nullptr;
      for (auto &keyframe : _keyframes)
      {
        if (!isAtKey(keyframe.Offset()))
        {
          continue;
        }
        explicitProperties.insert(keyframe.Properties().begin(), keyframe.Properties().end());
        if (!implicitKeyframe && canTakeImplicitValues(keyframe))
        {
          implicitKeyframe = &keyframe;
        }
      }
      return Pair<AnimatableCSSPropertySet, BlendingKeyframe *>(krys::move(explicitProperties),
                                                                implicitKeyframe);
    };

    auto [zeroExplicitProperties, zeroKeyframe] =
      explicitPropertiesAt([](double offset) { return offset <= 0; });
    if (auto implicitProperties = PropertiesNotIn(expectedExplicitProperties, zeroExplicitProperties);
        !implicitProperties.empty())
    {
      addImplicitKeyframe(0, implicitProperties, ZeroPercentKeyframe(), zeroKeyframe);
    }

    // After the 0% keyframe's added, as adding it moves the keyframes.
    auto [oneExplicitProperties, oneKeyframe] =
      explicitPropertiesAt([](double offset) { return offset >= 1; });
    if (auto implicitProperties = PropertiesNotIn(expectedExplicitProperties, oneExplicitProperties);
        !implicitProperties.empty())
    {
      addImplicitKeyframe(1, implicitProperties, HundredPercentKeyframe(), oneKeyframe);
    }
  }

#pragma endregion

#pragma region Dependencies

  bool BlendingKeyframes::UsesContainerUnits() const noexcept
  {
    return std::ranges::any_of(_keyframes, [](const BlendingKeyframe &keyframe)
                               { return keyframe.Style()->UsesContainerUnits(); });
  }

  bool BlendingKeyframes::UsesRelativeFontWeight() const noexcept
  {
    return _usesRelativeFontWeight;
  }

  bool BlendingKeyframes::HasCSSVariableReferences() const noexcept
  {
    return _containsCSSVariableReferences;
  }

  bool BlendingKeyframes::HasColorSetToCurrentColor() const noexcept
  {
    return _propertiesSetToCurrentColor.contains(AnimatableCSSProperty {css::PropertyId::Color});
  }

  bool BlendingKeyframes::HasPropertySetToCurrentColor() const noexcept
  {
    return !_propertiesSetToCurrentColor.empty();
  }

  const AnimatableCSSPropertySet &BlendingKeyframes::PropertiesSetToInherit() const noexcept
  {
    return _propertiesSetToInherit;
  }

  void BlendingKeyframes::UpdatePropertiesMetadata(const css::StyleProperties &properties) noexcept
  {
    for (auto propertyReference : properties)
    {
      const auto *cssValue = propertyReference.Value();
      if (!cssValue)
      {
        continue;
      }

      if (!_containsCSSVariableReferences && cssValue->HasVariableReferences())
      {
        _containsCSSVariableReferences = true;
      }

      if (const auto *primitiveValue = DynamicDowncast<css::PrimitiveValue>(*cssValue))
      {
        auto propertyId = propertyReference.Id();
        auto valueId = primitiveValue->ValueId();
        if (valueId == css::ValueId::Inherit)
        {
          _propertiesSetToInherit.insert(propertyId);
        }
        else if (valueId == css::ValueId::Currentcolor)
        {
          _propertiesSetToCurrentColor.insert(propertyId);
        }
        else if (!_usesRelativeFontWeight && propertyId == css::PropertyId::FontWeight
                 && (valueId == css::ValueId::Bolder || valueId == css::ValueId::Lighter))
        {
          _usesRelativeFontWeight = true;
        }

        if ((style::AnchorPositionEvaluator::PropertyAllowsAnchorFunction(propertyId)
             || style::AnchorPositionEvaluator::PropertyAllowsAnchorSizeFunction(propertyId))
            && cssValue->ComputedStyleDependencies().Anchors)
        {
          _usesAnchorFunctions = true;
        }
      }
      else if (const auto *customPropertyValue = DynamicDowncast<css::CustomPropertyValue>(*cssValue))
      {
        if (customPropertyValue->TryGlobalKeyword() == css::GlobalKeyword::Inherit)
        {
          _propertiesSetToInherit.insert(customPropertyValue->Name());
        }
        else if (customPropertyValue->IsCurrentColor())
        {
          _propertiesSetToCurrentColor.insert(customPropertyValue->Name());
        }
      }
    }
  }

  void BlendingKeyframes::AnalyzeKeyframe(const BlendingKeyframe &keyframe) noexcept
  {
    const auto *style = keyframe.Style();
    if (!style)
    {
      return;
    }

    // Any keyframe's dependency makes the animation depend on the size.
    if (!_hasWidthDependentTransform || !_hasHeightDependentTransform)
    {
      if (keyframe.AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Transform}))
      {
        auto [isWidthDependent, isHeightDependent] = style->Transform().ComputeSizeDependencies();
        _hasWidthDependentTransform |= isWidthDependent;
        _hasHeightDependentTransform |= isHeightDependent;
      }
      if (keyframe.AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Translate}))
      {
        auto [isWidthDependent, isHeightDependent] = style->Translate().ComputeSizeDependencies();
        _hasWidthDependentTransform |= isWidthDependent;
        _hasHeightDependentTransform |= isHeightDependent;
      }
    }

    if (!_hasDiscreteTransformInterval
        && keyframe.AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Transform}))
    {
      _hasDiscreteTransformInterval = style->Transform().ContainsNonInvertibleMatrix({});
    }

    if (!_hasExplicitlyInheritedKeyframeProperty)
    {
      _hasExplicitlyInheritedKeyframeProperty = style->HasExplicitlyInheritedProperties();
    }

    const auto &properties = keyframe.Properties();
    if (!keyframe.Offset())
    {
      _explicitFromProperties.insert(properties.begin(), properties.end());
    }
    if (keyframe.Offset() == 1)
    {
      _explicitToProperties.insert(properties.begin(), properties.end());
    }

    if (!_hasKeyframeNotUsingRangeOffset)
    {
      _hasKeyframeNotUsingRangeOffset = !keyframe.UsesRangeOffset();
    }
  }

#pragma endregion
}
