#include "Krystal.Booey/Animation/BlendingKeyframes.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/CSS/ComputedStyleDependencies.hpp"
#include "Krystal.Booey/CSS/Properties/MutableStyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StyleProperties.hpp"
#include "Krystal.Booey/CSS/Properties/StylePropertiesInlines.hpp"
#include "Krystal.Booey/CSS/Rules/KeyframeRule.hpp"
#include "Krystal.Booey/CSS/Values/CustomPropertyValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleProperties+GettersInlines.hpp"
#include "Krystal.Booey/Style/AnchorPositionEvaluator.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Seam.hpp"
#include <algorithm>

namespace krys::boo
{
  BlendingKeyframes::~BlendingKeyframes() noexcept = default;

  void BlendingKeyframes::Clear() noexcept
  {
    _keyframes.clear();
    _properties.clear();
    _propertiesSetToInherit.clear();
    _propertiesSetToCurrentColor.clear();
    _usesRelativeFontWeight = false;
    _containsCSSVariableReferences = false;
    _usesAnchorFunctions = false;
  }

  bool BlendingKeyframes::operator==(const BlendingKeyframes &o) const noexcept
  {
    if (_keyframes.size() != o._keyframes.size())
    {
      return false;
    }

    for (size_t i = 0; i < _keyframes.size(); ++i)
    {
      if (_keyframes[i].Offset() != o._keyframes[i].Offset())
      {
        return false;
      }
      if (_keyframes[i].Style() != o._keyframes[i].Style())
      {
        return false;
      }
    }

    return true;
  }

  const StringAtom &BlendingKeyframes::KeyframesName() const noexcept
  {
    if (const auto *name = std::get_if<StringAtom>(&_identifier))
    {
      return *name;
    }
    return StringAtom::Null();
  }

  void BlendingKeyframes::Insert(BlendingKeyframe &&keyframe) noexcept
  {
    if (!keyframe.UsesRangeOffset() && (keyframe.Offset() < 0 || keyframe.Offset() > 1))
    {
      return;
    }

    AnalyzeKeyframe(keyframe);

    size_t i = 0;
    for (; i < _keyframes.size(); ++i)
    {
      if (_keyframes[i].Offset() > keyframe.Offset())
      {
        break;
      }
    }

    // Insert before the first keyframe with a greater offset.
    auto inserted = _keyframes.insert(_keyframes.begin() + static_cast<ptrdiff_t>(i), krys::move(keyframe));
    for (const auto &property : inserted->Properties())
    {
      _properties.insert(property);
    }
  }

  bool BlendingKeyframes::HasImplicitKeyframes() const noexcept
  {
    return Size() && (_keyframes[0].Offset() || _keyframes[Size() - 1].Offset() != 1);
  }

  bool BlendingKeyframes::HasImplicitKeyframeForProperty(const AnimatableCSSProperty &property) const noexcept
  {
    return HasImplicitKeyframes()
           && (!_explicitFromProperties.contains(property) || !_explicitToProperties.contains(property));
  }

  void BlendingKeyframes::CopyKeyframes(const BlendingKeyframes &other) noexcept
  {
    for (const auto &keyframe : other)
    {
      auto copy = keyframe;
      Insert(krys::move(copy));
    }
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

  bool BlendingKeyframes::UsesContainerUnits() const noexcept
  {
    return std::ranges::any_of(_keyframes, [](const BlendingKeyframe &keyframe)
                               { return keyframe.Style()->UsesContainerUnits(); });
  }

  void BlendingKeyframes::AddProperty(const AnimatableCSSProperty &property) noexcept
  {
    krys_debug_assert(!std::holds_alternative<css::PropertyId>(property)
                      || std::get<css::PropertyId>(property) != css::PropertyId::Custom);
    _properties.insert(property);
  }

  bool BlendingKeyframes::ContainsProperty(const AnimatableCSSProperty &property) const noexcept
  {
    return _properties.contains(property);
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
        auto propertyID = propertyReference.Id();
        auto valueId = primitiveValue->ValueId();

        if (valueId == css::ValueId::Inherit)
        {
          _propertiesSetToInherit.insert(propertyID);
        }
        else if (valueId == css::ValueId::Currentcolor)
        {
          _propertiesSetToCurrentColor.insert(propertyID);
        }
        else if (!_usesRelativeFontWeight && propertyID == css::PropertyId::FontWeight
                 && (valueId == css::ValueId::Bolder || valueId == css::ValueId::Lighter))
        {
          _usesRelativeFontWeight = true;
        }

        if (style::AnchorPositionEvaluator::PropertyAllowsAnchorFunction(propertyID)
            || style::AnchorPositionEvaluator::PropertyAllowsAnchorSizeFunction(propertyID))
        {
          auto dependencies = cssValue->ComputedStyleDependencies();
          if (dependencies.Anchors)
          {
            _usesAnchorFunctions = true;
          }
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

    auto analyzeSizeDependentTransform = [&]
    {
      if (_hasWidthDependentTransform && _hasHeightDependentTransform)
      {
        return;
      }

      if (keyframe.AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Transform}))
      {
        auto [isWidthDependent, isHeightDependent] = style->GetTransform().ComputeSizeDependencies();
        _hasWidthDependentTransform = isWidthDependent;
        _hasHeightDependentTransform = isHeightDependent;
      }

      if (keyframe.AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Translate}))
      {
        auto [isWidthDependent, isHeightDependent] = style->GetTranslate().ComputeSizeDependencies();
        _hasWidthDependentTransform = isWidthDependent;
        _hasHeightDependentTransform = isHeightDependent;
      }
    };

    auto analyzeDiscreteTransformInterval = [&]
    {
      if (!_hasDiscreteTransformInterval
          && keyframe.AnimatesProperty(AnimatableCSSProperty {css::PropertyId::Transform}))
      {
        _hasDiscreteTransformInterval = style->GetTransform().ContainsNonInvertibleMatrix({});
      }
    };

    auto analyzeExplicitlyInheritedKeyframeProperty = [&]
    {
      if (!_hasExplicitlyInheritedKeyframeProperty)
      {
        _hasExplicitlyInheritedKeyframeProperty = style->HasExplicitlyInheritedProperties();
      }
    };

    auto analyzeKeyframeForExplicitProperties = [&]
    {
      const auto &properties = keyframe.Properties();
      if (!keyframe.Offset())
      {
        _explicitFromProperties.insert(properties.begin(), properties.end());
      }
      if (keyframe.Offset() == 1)
      {
        _explicitToProperties.insert(properties.begin(), properties.end());
      }
    };

    auto analyzeKeyframeRangeOffset = [&]
    {
      if (!_hasKeyframeNotUsingRangeOffset)
      {
        _hasKeyframeNotUsingRangeOffset = !keyframe.UsesRangeOffset();
      }
    };

    analyzeSizeDependentTransform();
    analyzeDiscreteTransformInterval();
    analyzeExplicitlyInheritedKeyframeProperty();
    analyzeKeyframeForExplicitProperties();
    analyzeKeyframeRangeOffset();
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

  namespace
  {
    KRYS_NODISCARD const css::KeyframeRule &ZeroPercentKeyframe() noexcept
    {
      static NeverDestroyed<Ref<css::KeyframeRule>> rule {
        []
        {
          auto keyframe = css::KeyframeRule::Create(css::MutableStyleProperties::Create());
          keyframe->SetKey({css::ValueId::Invalid, 0});
          return keyframe;
        }()};
      return *rule.get().get();
    }

    KRYS_NODISCARD const css::KeyframeRule &HundredPercentKeyframe() noexcept
    {
      static NeverDestroyed<Ref<css::KeyframeRule>> rule {
        []
        {
          auto keyframe = css::KeyframeRule::Create(css::MutableStyleProperties::Create());
          keyframe->SetKey({css::ValueId::Invalid, 1});
          return keyframe;
        }()};
      return *rule.get().get();
    }
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

    // We need to establish which properties are implicit for 0% and 100%.
    // We start each list off with the full list of properties, and see if
    // any 0% and 100% keyframes specify them.
    AnimatableCSSPropertySet expectedExplicitProperties;
    AnimatableCSSPropertySet zeroKeyframeExplicitProperties;

    BlendingKeyframe *implicitZeroKeyframe = nullptr;

    auto isSuitableKeyframeForImplicitValues = [&](const BlendingKeyframe &keyframe)
    {
      if (keyframe.UsesRangeOffset())
      {
        return false;
      }

      auto *timingFunction = keyframe.GetTimingFunction();

      // If there is no timing function set on the keyframe, then it uses the element's
      // timing function, which makes this keyframe suitable.
      if (!timingFunction)
      {
        return true;
      }

      if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(effect.Animation()))
      {
        const auto &animationWideTimingFunction =
          cssAnimation->BackingStyleAnimation().DefaultTimingFunctionForKeyframes();
        // If we're dealing with a CSS Animation and if that CSS Animation's backing animation
        // has a default timing function set, then if that keyframe's timing function matches,
        // that keyframe is suitable.
        if (animationWideTimingFunction)
        {
          return timingFunction == animationWideTimingFunction->Value.get();
        }
        // Otherwise, the keyframe will be suitable if its timing function matches the default.
        return timingFunction == &CubicBezierTimingFunction::DefaultTimingFunction();
      }

      return false;
    };

    for (auto &keyframe : _keyframes)
    {
      if (keyframe.Offset() <= 0)
      {
        for (const auto &property : keyframe.Properties())
        {
          zeroKeyframeExplicitProperties.insert(property);
        }
        if (!implicitZeroKeyframe && isSuitableKeyframeForImplicitValues(keyframe))
        {
          implicitZeroKeyframe = &keyframe;
        }
      }
      if (keyframe.HasResolvedOffset())
      {
        expectedExplicitProperties.insert(keyframe.Properties().begin(), keyframe.Properties().end());
      }
    }

    auto addImplicitKeyframe = [&](double key, const AnimatableCSSPropertySet &implicitProperties,
                                   const css::KeyframeRule &keyframeRule,
                                   BlendingKeyframe *existingImplicitBlendingKeyframe)
    {
      // If we're provided an existing implicit keyframe, we need to add all the styles for the implicit
      // properties.
      if (existingImplicitBlendingKeyframe)
      {
        krys_debug_assert(existingImplicitBlendingKeyframe->Style());
        auto keyframeStyle = RenderStyle::ClonePtr(*existingImplicitBlendingKeyframe->Style());
        for (const auto &property : implicitProperties)
        {
          style::interpolation::Interpolate(property, *keyframeStyle, underlyingStyle, underlyingStyle, 1,
                                            boo::CompositeOperation::Replace, effect);
          existingImplicitBlendingKeyframe->AddProperty(property);
        }
        existingImplicitBlendingKeyframe->SetStyle(krys::move(keyframeStyle));
        return;
      }

      // Otherwise we create a new keyframe.
      BlendingKeyframe blendingKeyframe(key, nullptr);
      blendingKeyframe.SetStyle(
        styleResolver.StyleForKeyframe(*element, underlyingStyle, {nullptr}, keyframeRule, blendingKeyframe));
      for (const auto &property : implicitProperties)
      {
        blendingKeyframe.AddProperty(property);
      }
      // Step 2 of https://drafts.csswg.org/css-animations-2/#keyframes defines the
      // default composite property as "replace" for CSS Animations.
      if (effect.Animation() && Is<CSSAnimation>(*effect.Animation()))
      {
        blendingKeyframe.SetCompositeOperation(boo::CompositeOperation::Replace);
      }
      Insert(krys::move(blendingKeyframe));
    };

    auto differenceWith = [](const AnimatableCSSPropertySet &all, const AnimatableCSSPropertySet &excluded)
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
    };

    auto zeroKeyframeImplicitProperties =
      differenceWith(expectedExplicitProperties, zeroKeyframeExplicitProperties);
    if (!zeroKeyframeImplicitProperties.empty())
    {
      addImplicitKeyframe(0, zeroKeyframeImplicitProperties, ZeroPercentKeyframe(), implicitZeroKeyframe);
    }

    AnimatableCSSPropertySet oneKeyframeExplicitProperties;
    BlendingKeyframe *implicitOneKeyframe = nullptr;

    for (auto &keyframe : _keyframes)
    {
      if (keyframe.Offset() >= 1)
      {
        for (const auto &property : keyframe.Properties())
        {
          oneKeyframeExplicitProperties.insert(property);
        }
        if (!implicitOneKeyframe && isSuitableKeyframeForImplicitValues(keyframe))
        {
          implicitOneKeyframe = &keyframe;
        }
      }
    }

    auto oneKeyframeImplicitProperties =
      differenceWith(expectedExplicitProperties, oneKeyframeExplicitProperties);
    if (!oneKeyframeImplicitProperties.empty())
    {
      addImplicitKeyframe(1, oneKeyframeImplicitProperties, HundredPercentKeyframe(), implicitOneKeyframe);
    }
  }

  uint64 BlendingKeyframes::NextAnonymousIdentifier() noexcept
  {
    static uint64 numericIdentifier = 0;
    return ++numericIdentifier;
  }

  BlendingKeyframe::BlendingKeyframe(KeyframeOffset &&offset, UniquePtr<RenderStyle> &&style) noexcept
      : _specifiedOffset(krys::move(offset)), _style(krys::move(style))
  {
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

  void BlendingKeyframe::AddProperty(const AnimatableCSSProperty &property) noexcept
  {
    krys_debug_assert(!std::holds_alternative<css::PropertyId>(property)
                      || std::get<css::PropertyId>(property) != css::PropertyId::Custom);
    _properties.insert(property);
  }

  bool BlendingKeyframe::AnimatesProperty(KeyframeInterpolation::Property property) const noexcept
  {
    if (const auto *animatableCSSProperty = std::get_if<AnimatableCSSProperty>(&property))
    {
      return _properties.contains(*animatableCSSProperty);
    }
    krys_debug_assert(false);
    return false;
  }

  bool BlendingKeyframe::UsesRangeOffset() const noexcept
  {
    return _specifiedOffset.Name != style::SingleAnimationRangeName::Omitted
           && _specifiedOffset.Name != style::SingleAnimationRangeName::Normal;
  }
}
