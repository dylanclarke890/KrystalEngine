#pragma once

#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/UniquePtr.hpp"
#include "Krystal.Core/Move.hpp"
#include "Krystal.Core/Strings/StringAtom.hpp"
#include "Krystal.Web/Animation/Keyframes/KeyframeInterpolation.hpp"
#include "Krystal.Web/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Web/Style/Values/Animations/SingleAnimationRangeName.hpp"
#include <cmath>
#include <limits>

namespace krys::web
{
  namespace css
  {
    class KeyframeRule;
    class StyleProperties;
  }

  class KeyframeEffect;

  /// @brief The composite operation a `@keyframes` keyframe's `animation-composition` gives, if it has one.
  KRYS_NODISCARD Maybe<CompositeOperation>
    KeyframeRuleCompositeOperation(const css::KeyframeRule &keyframe) noexcept;

  /// @brief A keyframe of an animation: its offset, the computed style it gives the properties it sets, and
  /// the timing function and composite operation it applies to them.
  /// @see https://drafts.csswg.org/web-animations-1/#keyframe
  class BlendingKeyframe final : public KeyframeInterpolation::Keyframe
  {
  public:
    /// @brief The offset as written: a fraction of the animation, or of a named timeline range.
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
    /// @brief The offset in the animation, or NaN until a range offset has been resolved.
    double _computedOffset {std::numeric_limits<double>::quiet_NaN()};
    /// @brief The properties the keyframe sets.
    AnimatableCSSPropertySet _properties;
    UniquePtr<RenderStyle> _style;
    /// @brief The timing function to the next keyframe, or null for the effect's.
    RefPtr<TimingFunction> _timingFunction;
    /// @brief The composite operation, or none for the effect's.
    Maybe<krys::CompositeOperation> _compositeOperation;
    bool _containsDirectionAwareProperty {false};

  public:
    /// @brief A keyframe at @p offset with @p style, which may be set later.
    BlendingKeyframe(KeyframeOffset &&offset, UniquePtr<RenderStyle> &&style) noexcept;

    /// @brief A copy with its own copy of the style.
    BlendingKeyframe(const BlendingKeyframe &source) noexcept;

    BlendingKeyframe(BlendingKeyframe &&) noexcept = default;

    BlendingKeyframe &operator=(BlendingKeyframe &&) noexcept = default;

#pragma region KeyframeInterpolation::Keyframe

    KRYS_NODISCARD double Offset() const noexcept final
    {
      return _computedOffset;
    }

    KRYS_NODISCARD Maybe<krys::CompositeOperation> CompositeOperation() const noexcept final
    {
      return _compositeOperation;
    }

    KRYS_NODISCARD bool AnimatesProperty(KeyframeInterpolation::Property property) const noexcept final;

    KRYS_NODISCARD bool IsBlendingKeyframe() const noexcept final
    {
      return true;
    }

#pragma endregion

    /// @brief Adds @p property to those the keyframe sets. Custom properties are added by name.
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

    /// @brief Whether the offset is into a named timeline range (`cover 20%`), resolved once the
    /// animation's timeline is known.
    KRYS_NODISCARD bool UsesRangeOffset() const noexcept;

    KRYS_NODISCARD bool HasResolvedOffset() const noexcept
    {
      return !std::isnan(_computedOffset);
    }

    KRYS_NODISCARD const RenderStyle *Style() const noexcept
    {
      return _style.get();
    }

    void SetStyle(UniquePtr<RenderStyle> &&style) noexcept
    {
      _style = krys::move(style);
    }

    KRYS_NODISCARD web::TimingFunction *GetTimingFunction() const noexcept
    {
      return _timingFunction.get();
    }

    void SetTimingFunction(const RefPtr<web::TimingFunction> &timingFunction) noexcept
    {
      _timingFunction = timingFunction;
    }

    void SetCompositeOperation(Maybe<krys::CompositeOperation> operation) noexcept
    {
      _compositeOperation = operation;
    }

    /// @brief Whether the keyframe sets a logical property, which resolves differently with the writing
    /// mode or direction.
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

  /// @brief An animation's keyframes, sorted by offset, with what they animate and what they depend on.
  /// @note What they depend on is worked out as keyframes are inserted and their declarations read; it
  /// tells the effect when to recompute the keyframes or lay out first, and whether it can be accelerated.
  class BlendingKeyframes
  {
  private:
    KeyframesIdentifier _identifier;
    /// @brief AcceleratedAnimationName(), made when it's first asked for.
    mutable utf8_string _acceleratedAnimationName;
    /// @brief Sorted by offset, keyframes with the same offset in insertion order.
    List<BlendingKeyframe> _keyframes;
    /// @brief The properties any keyframe sets.
    AnimatableCSSPropertySet _properties;
    /// @brief The properties a 100% keyframe sets.
    AnimatableCSSPropertySet _explicitToProperties;
    /// @brief The properties a 0% keyframe sets.
    AnimatableCSSPropertySet _explicitFromProperties;
    /// @brief The properties a keyframe sets to `inherit`.
    AnimatableCSSPropertySet _propertiesSetToInherit;
    /// @brief The properties a keyframe sets to `currentcolor`.
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
    /// @brief Keyframes with a new number as their identifier.
    BlendingKeyframes() noexcept : _identifier(NextAnonymousIdentifier())
    {
    }

    explicit BlendingKeyframes(KeyframesIdentifier identifier) noexcept : _identifier(krys::move(identifier))
    {
    }

    ~BlendingKeyframes() noexcept;

    BlendingKeyframes(BlendingKeyframes &&) noexcept = default;

    BlendingKeyframes &operator=(BlendingKeyframes &&) noexcept = default;

    KRYS_NODISCARD const KeyframesIdentifier &Identifier() const noexcept
    {
      return _identifier;
    }

    /// @brief The `@keyframes` rule's name, or null if the keyframes don't come from one.
    KRYS_NODISCARD const StringAtom &KeyframesName() const noexcept KRYS_LIFETIME_BOUND;

    /// @brief The name the keyframes' animation has on a graphics layer.
    KRYS_NODISCARD const utf8_string &AcceleratedAnimationName() const noexcept;

#pragma region Keyframes

    /// @brief Inserts @p keyframe after those with the same or a smaller offset, and notes what it depends
    /// on. A keyframe with an offset outside 0 to 1 that isn't a range offset is dropped.
    void Insert(BlendingKeyframe &&keyframe) noexcept;

    /// @brief Inserts a copy of each of @p other's keyframes.
    void CopyKeyframes(const BlendingKeyframes &other) noexcept;

    /// @brief Removes the keyframes and everything noted about them; the identifier stays.
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

    KRYS_NODISCARD auto begin() const noexcept KRYS_LIFETIME_BOUND
    {
      return _keyframes.begin();
    }

    KRYS_NODISCARD auto end() const noexcept KRYS_LIFETIME_BOUND
    {
      return _keyframes.end();
    }

    /// @brief Resolves each keyframe's offset with @p callback, then sorts the keyframes again.
    void UpdatedComputedOffsets(
      KRYS_NOESCAPE const Func<double(const BlendingKeyframe::KeyframeOffset &)> &callback) noexcept;

#pragma endregion

#pragma region Properties

    KRYS_NODISCARD bool ContainsProperty(const AnimatableCSSProperty &property) const noexcept;

    /// @brief The properties any keyframe sets.
    KRYS_NODISCARD const AnimatableCSSPropertySet &Properties() const noexcept
    {
      return _properties;
    }

    /// @brief Whether any of the properties can be interpolated, rather than only changing discretely.
    KRYS_NODISCARD bool ContainsAnimatableCSSProperty() const noexcept;

    /// @brief Whether any keyframe sets a logical property.
    KRYS_NODISCARD bool ContainsDirectionAwareProperty() const noexcept;

#pragma endregion

#pragma region Implicit keyframes

    /// @brief Whether the first keyframe isn't at 0% or the last isn't at 100%, so the underlying value fills
    /// in there.
    KRYS_NODISCARD bool HasImplicitKeyframes() const noexcept;

    /// @brief Whether @p property takes its value at 0% or 100% from the underlying style.
    KRYS_NODISCARD bool HasImplicitKeyframeForProperty(const AnimatableCSSProperty &property) const noexcept;

    /// @brief Adds the 0% and 100% keyframes the properties without one there take from @p underlyingStyle.
    /// @see https://drafts.csswg.org/web-animations-1/#the-effect-value-of-a-keyframe-animation-effect
    void FillImplicitKeyframes(const KeyframeEffect &effect, const RenderStyle &underlyingStyle) noexcept;

#pragma endregion

#pragma region Dependencies

    /// @brief Notes what @p properties, a keyframe's declarations, depend on: `inherit`, `currentcolor`,
    /// relative font weights, custom property references and anchor functions.
    void UpdatePropertiesMetadata(const css::StyleProperties &properties) noexcept;

    /// @brief Whether a keyframe's style uses container query units.
    KRYS_NODISCARD bool UsesContainerUnits() const noexcept;

    /// @brief Whether a keyframe sets `font-weight` to `bolder` or `lighter`.
    KRYS_NODISCARD bool UsesRelativeFontWeight() const noexcept;

    /// @brief Whether a keyframe's declarations reference custom properties.
    KRYS_NODISCARD bool HasCSSVariableReferences() const noexcept;

    /// @brief Whether a keyframe sets `color` to `currentcolor`.
    KRYS_NODISCARD bool HasColorSetToCurrentColor() const noexcept;

    /// @brief Whether a keyframe sets any property to `currentcolor`.
    KRYS_NODISCARD bool HasPropertySetToCurrentColor() const noexcept;

    /// @brief The properties a keyframe sets to `inherit`.
    KRYS_NODISCARD const AnimatableCSSPropertySet &PropertiesSetToInherit() const noexcept;

    /// @brief Whether a keyframe's transform or translate depends on the element's width.
    KRYS_NODISCARD bool HasWidthDependentTransform() const noexcept
    {
      return _hasWidthDependentTransform;
    }

    /// @brief Whether a keyframe's transform or translate depends on the element's height.
    KRYS_NODISCARD bool HasHeightDependentTransform() const noexcept
    {
      return _hasHeightDependentTransform;
    }

    /// @brief Whether a keyframe's transform has a matrix that can't be inverted, which keeps the animation
    /// from being accelerated.
    KRYS_NODISCARD bool HasDiscreteTransformInterval() const noexcept
    {
      return _hasDiscreteTransformInterval;
    }

    /// @brief Whether a keyframe's style has a property set to `inherit`.
    KRYS_NODISCARD bool HasExplicitlyInheritedKeyframeProperty() const noexcept
    {
      return _hasExplicitlyInheritedKeyframeProperty;
    }

    /// @brief Whether a keyframe's declarations use `anchor()` or `anchor-size()`.
    KRYS_NODISCARD bool UsesAnchorFunctions() const noexcept
    {
      return _usesAnchorFunctions;
    }

    /// @brief Whether a keyframe's offset is a plain fraction rather than a timeline range offset.
    KRYS_NODISCARD bool HasKeyframeNotUsingRangeOffset() const noexcept
    {
      return _hasKeyframeNotUsingRangeOffset;
    }

#pragma endregion

  private:
    /// @brief Notes what @p keyframe's style depends on, and which properties it sets at 0% and 100%.
    void AnalyzeKeyframe(const BlendingKeyframe &keyframe) noexcept;

    KRYS_NODISCARD static uint64 NextAnonymousIdentifier() noexcept;
  };
}

KRYS_SPECIALIZE_TYPE_TRAITS_KEYFRAME_INTERPOLATION_KEYFRAME(BlendingKeyframe, IsBlendingKeyframe())
