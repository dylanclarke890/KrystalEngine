#pragma once

#include "Krystal.Booey/Animation/KeyframeInterpolation.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Values/Animations/SingleAnimationRangeName.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"
#include <limits>

namespace krys::boo
{
  namespace css
  {
    class StyleProperties;
  }

  /// @brief A keyframe of an animation, with the style it gives the properties it sets.
  class BlendingKeyframe final : public KeyframeInterpolation::Keyframe
  {
  public:
    struct KeyframeOffset
    {
      style::SingleAnimationRangeName Name;
      double Value;

      KeyframeOffset(double value) noexcept : Name(style::SingleAnimationRangeName::Omitted), Value(value)
      {
      }

      KeyframeOffset(style::SingleAnimationRangeName name, double value) noexcept : Name(name), Value(value)
      {
      }
    };

  private:
    KeyframeOffset _specifiedOffset;
    double _computedOffset {std::numeric_limits<double>::quiet_NaN()};
    AnimatableCSSPropertySet _properties; // The properties specified in this keyframe.
    UniquePtr<RenderStyle> _style;
    RefPtr<TimingFunction> _timingFunction;
    Maybe<boo::CompositeOperation> _compositeOperation;
    bool _containsDirectionAwareProperty {false};

  public:
    BlendingKeyframe(KeyframeOffset &&offset, UniquePtr<RenderStyle> &&style) noexcept;
    BlendingKeyframe(const BlendingKeyframe &source) noexcept;
    BlendingKeyframe(BlendingKeyframe &&) noexcept = default;
    BlendingKeyframe &operator=(BlendingKeyframe &&) noexcept = default;

    // KeyframeInterpolation::Keyframe
    KRYS_NODISCARD double Offset() const noexcept final
    {
      return _computedOffset;
    }
    KRYS_NODISCARD Maybe<boo::CompositeOperation> CompositeOperation() const noexcept final
    {
      return _compositeOperation;
    }
    KRYS_NODISCARD bool AnimatesProperty(KeyframeInterpolation::Property property) const noexcept final;
    KRYS_NODISCARD bool IsBlendingKeyframe() const noexcept final
    {
      return true;
    }

    void AddProperty(const AnimatableCSSProperty &property) noexcept;
    KRYS_NODISCARD const AnimatableCSSPropertySet &Properties() const noexcept
    {
      return _properties;
    }

    KRYS_NODISCARD const KeyframeOffset &SpecifiedOffset() const noexcept
    {
      return _specifiedOffset;
    }
    void SetComputedOffset(double offset) noexcept
    {
      _computedOffset = offset;
    }
    KRYS_NODISCARD bool UsesRangeOffset() const noexcept;

    KRYS_NODISCARD const RenderStyle *Style() const noexcept
    {
      return _style.get();
    }
    void SetStyle(UniquePtr<RenderStyle> &&style) noexcept
    {
      _style = krys::move(style);
    }

    KRYS_NODISCARD boo::TimingFunction *GetTimingFunction() const noexcept
    {
      return _timingFunction.get();
    }
    void SetTimingFunction(const RefPtr<boo::TimingFunction> &timingFunction) noexcept
    {
      _timingFunction = timingFunction;
    }

    void SetCompositeOperation(Maybe<boo::CompositeOperation> operation) noexcept
    {
      _compositeOperation = operation;
    }

    KRYS_NODISCARD bool ContainsDirectionAwareProperty() const noexcept
    {
      return _containsDirectionAwareProperty;
    }
    void SetContainsDirectionAwareProperty(bool containsDirectionAwareProperty) noexcept
    {
      _containsDirectionAwareProperty = containsDirectionAwareProperty;
    }
  };

  /// @brief A `@keyframes` rule's name, or a number for keyframes that come from elsewhere.
  using KeyframesIdentifier = Variant<StringAtom, uint64>;

  /// @brief An animation's keyframes, kept sorted by offset, with what they animate and depend on.
  // FIXME: `fillImplicitKeyframes` and the accelerated animation name, once there are keyframes from scripts
  // and accelerated animations.
  class BlendingKeyframes
  {
  private:
    KeyframesIdentifier _identifier;
    List<BlendingKeyframe> _keyframes;    // Kept sorted by key.
    AnimatableCSSPropertySet _properties; // The properties being animated.
    AnimatableCSSPropertySet
      _explicitToProperties; // The properties with an explicit value for the 100% keyframe.
    AnimatableCSSPropertySet
      _explicitFromProperties; // The properties with an explicit value for the 0% keyframe.
    AnimatableCSSPropertySet _propertiesSetToInherit;
    AnimatableCSSPropertySet _propertiesSetToCurrentColor;
    bool _usesRelativeFontWeight {false};
    bool _containsCSSVariableReferences {false};
    bool _usesAnchorFunctions {false};
    bool _hasWidthDependentTransform {false};
    bool _hasHeightDependentTransform {false};
    bool _hasDiscreteTransformInterval {false};
    bool _hasExplicitlyInheritedKeyframeProperty {false};
    bool _hasKeyframeNotUsingRangeOffset {false};

  public:
    BlendingKeyframes() noexcept : _identifier(NextAnonymousIdentifier())
    {
    }

    explicit BlendingKeyframes(const KeyframesIdentifier &identifier) noexcept : _identifier(identifier)
    {
    }

    ~BlendingKeyframes() noexcept;

    BlendingKeyframes(BlendingKeyframes &&) noexcept = default;
    BlendingKeyframes &operator=(BlendingKeyframes &&) noexcept = default;

    KRYS_NODISCARD bool operator==(const BlendingKeyframes &other) const noexcept;

    KRYS_NODISCARD const KeyframesIdentifier &Identifier() const noexcept
    {
      return _identifier;
    }
    KRYS_NODISCARD const StringAtom &KeyframesName() const noexcept;

    void Insert(BlendingKeyframe &&keyframe) noexcept;

    void AddProperty(const AnimatableCSSProperty &property) noexcept;
    KRYS_NODISCARD bool ContainsProperty(const AnimatableCSSProperty &property) const noexcept;
    KRYS_NODISCARD const AnimatableCSSPropertySet &Properties() const noexcept
    {
      return _properties;
    }
    KRYS_NODISCARD bool ContainsAnimatableCSSProperty() const noexcept;
    KRYS_NODISCARD bool ContainsDirectionAwareProperty() const noexcept;

    void Clear() noexcept;
    KRYS_NODISCARD bool IsEmpty() const noexcept
    {
      return _keyframes.empty();
    }
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _keyframes.size();
    }
    KRYS_NODISCARD const BlendingKeyframe &operator[](size_t index) const noexcept KRYS_LIFETIME_BOUND
    {
      return _keyframes[index];
    }

    void CopyKeyframes(const BlendingKeyframes &other) noexcept;
    KRYS_NODISCARD bool HasImplicitKeyframes() const noexcept;
    KRYS_NODISCARD bool HasImplicitKeyframeForProperty(const AnimatableCSSProperty &property) const noexcept;

    KRYS_NODISCARD auto begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _keyframes.begin();
    }
    KRYS_NODISCARD auto end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _keyframes.end();
    }

    KRYS_NODISCARD bool UsesContainerUnits() const noexcept;
    KRYS_NODISCARD bool UsesRelativeFontWeight() const noexcept;
    KRYS_NODISCARD bool HasCSSVariableReferences() const noexcept;
    KRYS_NODISCARD bool HasColorSetToCurrentColor() const noexcept;
    KRYS_NODISCARD bool HasPropertySetToCurrentColor() const noexcept;
    KRYS_NODISCARD const AnimatableCSSPropertySet &PropertiesSetToInherit() const noexcept;
    void UpdatePropertiesMetadata(const css::StyleProperties &properties) noexcept;

    KRYS_NODISCARD bool HasWidthDependentTransform() const noexcept
    {
      return _hasWidthDependentTransform;
    }
    KRYS_NODISCARD bool HasHeightDependentTransform() const noexcept
    {
      return _hasHeightDependentTransform;
    }
    KRYS_NODISCARD bool HasDiscreteTransformInterval() const noexcept
    {
      return _hasDiscreteTransformInterval;
    }
    KRYS_NODISCARD bool HasExplicitlyInheritedKeyframeProperty() const noexcept
    {
      return _hasExplicitlyInheritedKeyframeProperty;
    }
    KRYS_NODISCARD bool UsesAnchorFunctions() const noexcept
    {
      return _usesAnchorFunctions;
    }
    KRYS_NODISCARD bool HasKeyframeNotUsingRangeOffset() const noexcept
    {
      return _hasKeyframeNotUsingRangeOffset;
    }

    void UpdatedComputedOffsets(
      KRYS_NOESCAPE const Func<double(const BlendingKeyframe::KeyframeOffset &)> &callback) noexcept;

  private:
    void AnalyzeKeyframe(const BlendingKeyframe &keyframe) noexcept;
    KRYS_NODISCARD static uint64 NextAnonymousIdentifier() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_KEYFRAME_INTERPOLATION_KEYFRAME(BlendingKeyframe, IsBlendingKeyframe())
