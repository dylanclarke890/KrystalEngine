#include "Krystal.Booey/Style/Styleable.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/KeyframeEffect.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/CSS/Properties/Property.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyShorthand.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/EventLoop.hpp"
#include "Krystal.Booey/Rendering/RenderListItem.hpp"
#include "Krystal.Booey/Rendering/RenderListMarker.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/StyleCustomPropertyData.hpp"
#include "Krystal.Booey/Style/ChangedAnimatableProperties.hpp"
#include "Krystal.Booey/Style/Interpolation.hpp"
#include "Krystal.Booey/Style/Resolver.hpp"
#include "Krystal.Booey/Style/Scope.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"

namespace krys::boo::style
{
  Maybe<Styleable> Styleable::FromRenderer(const RenderElement &renderer) noexcept
  {
    auto pseudoElementType = renderer.Style().GetPseudoElementType();
    if (!pseudoElementType)
    {
      if (auto *element = renderer.GetElement())
      {
        return FromElement(*element);
      }
      return null;
    }

    switch (*pseudoElementType)
    {
      case PseudoElementType::Marker:
      {
        auto *ancestor = renderer.Parent();
        while (ancestor)
        {
          auto *renderListItem = DynamicDowncast<RenderListItem>(ancestor);
          if (renderListItem && ancestor->GetElement() && renderListItem->MarkerRenderer() == &renderer)
          {
            return Styleable(*ancestor->GetElement(),
                             style::PseudoElementIdentifier {PseudoElementType::Marker});
          }
          ancestor = ancestor->Parent();
        }
        break;
      }
      case PseudoElementType::After:
      case PseudoElementType::Before:
        if (auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(renderer.GetElement()))
        {
          if (auto *host = pseudoElement->HostElement())
          {
            return Styleable(*host, style::PseudoElementIdentifier {*pseudoElementType});
          }
        }
        break;
      default:
        // FIXME: The ::backdrop renderer and the view transition pseudo-elements' renderers, once there are
        // those.
        return null;
    }

    return null;
  }

  bool Styleable::ComputeAnimationExtent(LayoutRect &bounds) const noexcept
  {
    const auto *animations = Animations();
    if (!animations)
    {
      return false;
    }

    RefPtr<KeyframeEffect> matchingEffect;
    for (const auto &animation : *animations)
    {
      if (auto *keyframeEffect = animation->GetKeyframeEffect())
      {
        if (AnimatablePropertiesContainTransformRelatedProperty(
              keyframeEffect->GetBlendingKeyframes().Properties()))
        {
          matchingEffect = ShareRefPtr(keyframeEffect);
        }
      }
    }

    if (matchingEffect)
    {
      return matchingEffect->ComputeExtentOfTransformAnimation(bounds);
    }

    return true;
  }

  bool Styleable::MayHaveNonZeroOpacity() const noexcept
  {
    auto *renderer = Renderer();
    if (!renderer)
    {
      return false;
    }

    if (!renderer->Style().GetOpacity().IsTransparent())
    {
      return true;
    }

    if (renderer->Style().GetWillChange().ContainsProperty(css::PropertyId::Opacity))
    {
      return true;
    }

    auto *effectStack = GetKeyframeEffectStack();
    return effectStack && effectStack->ContainsProperty(css::PropertyId::Opacity);
  }

  bool Styleable::IsRunningAcceleratedAnimationOfProperty(css::PropertyId property) const noexcept
  {
    auto *effectStack = GetKeyframeEffectStack();
    return effectStack
           && effectStack->HasMatchingEffect(
             [property](const KeyframeEffect &effect)
             { return effect.IsCurrentlyAffectingProperty(property, KeyframeEffect::Accelerated::Yes); });
  }

  bool Styleable::IsRunningAcceleratedTransformRelatedAnimation() const noexcept
  {
    auto *effectStack = GetKeyframeEffectStack();
    return effectStack
           && effectStack->HasMatchingEffect(
             [](const KeyframeEffect &effect)
             { return effect.IsRunningAcceleratedTransformRelatedAnimation(); });
  }

  bool Styleable::HasRunningAcceleratedAnimations() const noexcept
  {
    if (auto *effectStack = GetKeyframeEffectStack())
    {
      if (effectStack->HasAcceleratedEffects(Element.NodeDocument().GetSettings()))
      {
        return true;
      }
    }

    if (const auto *animations = Animations())
    {
      for (const auto &animation : *animations)
      {
        if (auto *keyframeEffect = animation->GetKeyframeEffect())
        {
          if (keyframeEffect->IsRunningAccelerated())
          {
            return true;
          }
        }
      }
    }

    return false;
  }

  RenderElement *Styleable::Renderer() const noexcept
  {
    if (!PseudoElementIdentifier)
    {
      return Element.Renderer();
    }

    switch (PseudoElementIdentifier->Type)
    {
      case PseudoElementType::After:
        if (auto *afterPseudoElement = Element.AfterPseudoElement())
        {
          return afterPseudoElement->Renderer();
        }
        break;
      case PseudoElementType::Before:
        if (auto *beforePseudoElement = Element.BeforePseudoElement())
        {
          return beforePseudoElement->Renderer();
        }
        break;
      case PseudoElementType::Marker:
        if (auto *renderListItem = DynamicDowncast<RenderListItem>(Element.Renderer()))
        {
          auto *markerRenderer = renderListItem->MarkerRenderer();
          if (markerRenderer && !markerRenderer->Style().HasUsedContentNone())
          {
            return markerRenderer;
          }
        }
        break;
      default:
        // FIXME: The ::backdrop renderer and the view transition pseudo-elements' renderers, once there are
        // those.
        break;
    }

    return nullptr;
  }

  UniquePtr<RenderStyle> Styleable::ComputeAnimatedStyle() const noexcept
  {
    UniquePtr<RenderStyle> animatedStyle;

    auto *effectStack = GetKeyframeEffectStack();
    if (!effectStack)
    {
      return animatedStyle;
    }

    for (const auto &effect : effectStack->SortedEffects())
    {
      effect->GetAnimatedStyle(animatedStyle);
    }

    return animatedStyle;
  }

  void Styleable::AnimationWasAdded(WebAnimation &animation) const noexcept
  {
    (void)EnsureAnimations().add(ShareRef(animation));
  }

  namespace
  {
    bool RemoveCSSTransitionFromMap(CSSTransition &transition,
                                    AnimatableCSSPropertyToTransitionMap &cssTransitionsByProperty) noexcept
    {
      auto transitionIterator = cssTransitionsByProperty.find(transition.Property());
      if (transitionIterator == cssTransitionsByProperty.end()
          || transitionIterator->second.get() != &transition)
      {
        return false;
      }

      cssTransitionsByProperty.erase(transitionIterator);
      return true;
    }
  }

  void
    Styleable::RemoveStyleOriginatedAnimationFromListsForOwningElement(WebAnimation &animation) const noexcept
  {
    krys_debug_assert(Is<StyleOriginatedAnimation>(animation));

    if (auto *transition = DynamicDowncast<CSSTransition>(&animation))
    {
      if (!RemoveCSSTransitionFromMap(*transition, EnsureRunningTransitionsByProperty()))
      {
        (void)RemoveCSSTransitionFromMap(*transition, EnsureCompletedTransitionsByProperty());
      }
    }
  }

  void Styleable::AnimationWasRemoved(WebAnimation &animation) const noexcept
  {
    (void)EnsureAnimations().remove(ShareRef(animation));

    // Now, if we're dealing with a CSS Transition, we remove it from the running transitions map. We don't
    // need to do this for CSS Animations because their timing can be set via CSS to end, which would cause
    // this function to be called, but they should remain associated with their owning element until this is
    // changed via a call to the JS API or changing the target element's animation-name property.
    if (Is<CSSTransition>(animation))
    {
      RemoveStyleOriginatedAnimationFromListsForOwningElement(animation);
    }
  }

  void Styleable::ElementWasRemoved() const noexcept
  {
    CancelStyleOriginatedAnimations();
    // FIXME: Telling the style-originated timelines controller, once there are scroll and view timelines.
  }

  void Styleable::WillChangeRenderer() const noexcept
  {
    if (auto *animations = Animations())
    {
      for (const auto &animation : *animations)
      {
        animation->WillChangeRenderer();
      }
    }
  }

  EnumSet<AnimationImpact>
    Styleable::ApplyKeyframeEffects(RenderStyle &targetStyle, AnimatableCSSPropertySet &affectedProperties,
                                    const RenderStyle *previousLastStyleChangeEventStyle,
                                    const ResolutionContext &resolutionContext) const noexcept
  {
    return EnsureKeyframeEffectStack().ApplyKeyframeEffects(
      targetStyle, affectedProperties, previousLastStyleChangeEventStyle, resolutionContext);
  }

  void Styleable::CancelStyleOriginatedAnimations() const noexcept
  {
    // FIXME: Not canceling style-originated animations when entering the back/forward cache, once there's
    // navigation.
    CancelStyleOriginatedAnimations({});
    // FIXME: Unregistering the named timelines of the element, once there are scroll and view timelines.
  }

  void Styleable::CancelStyleOriginatedAnimations(
    const WeakStyleOriginatedAnimations &animationsToCancelSilently) const noexcept
  {
    if (auto *animations = Animations())
    {
      // Canceling disassociates animations from the element, which may change the collection.
      List<Ref<WebAnimation>> animationsToCancel(animations->begin(), animations->end());
      for (auto &animation : animationsToCancel)
      {
        if (auto *styleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(animation.get()))
        {
          auto silently = std::ranges::any_of(animationsToCancelSilently, [&](const auto &weakAnimation)
                                              { return weakAnimation.get() == styleOriginatedAnimation; });
          styleOriginatedAnimation->CancelFromStyle(silently ? WebAnimation::Silently::Yes
                                                             : WebAnimation::Silently::No);
          SetLastStyleChangeEventStyle(nullptr);
        }
      }
    }

    if (auto *effectStack = GetKeyframeEffectStack())
    {
      effectStack->SetCSSAnimationList(null);
    }
    SetAnimationsCreatedByMarkup({});
  }

  namespace
  {
    bool KeyframesRuleExistsForAnimation(dom::Element &element, const ScopedName &animationName) noexcept
    {
      return Scope::ResolveTreeScopedReference(
        element, animationName,
        [](const Scope &scope, const css::CSSOMStringAtom &name) -> bool
        {
          if (auto *resolver = scope.ResolverIfExists())
          {
            return resolver->IsAnimationNameValid(css::CSSOMString {name.View()});
          }
          return false;
        });
    }
  }

  bool Styleable::AnimationListContainsNewlyValidAnimation(const style::Animations &animations) const noexcept
  {
    auto &keyframeEffectStack = EnsureKeyframeEffectStack();
    if (!keyframeEffectStack.HasInvalidCSSAnimationNames())
    {
      return false;
    }

    for (const auto &currentAnimation : animations.UsedValues())
    {
      if (auto keyframesName = currentAnimation.GetName().TryKeyframesName();
          keyframesName && !keyframesName->Name.View().empty()
          && keyframeEffectStack.ContainsInvalidCSSAnimationName(
            css::CSSOMString {keyframesName->Name.View()})
          && KeyframesRuleExistsForAnimation(Element, *keyframesName))
      {
        return true;
      }
    }

    return false;
  }

  void Styleable::UpdateCSSAnimations(const RenderStyle *currentStyle, const RenderStyle &newStyle,
                                      const ResolutionContext &resolutionContext,
                                      WeakStyleOriginatedAnimations &newStyleOriginatedAnimations,
                                      IsInDisplayNoneTree isInDisplayNoneTree) const noexcept
  {
    auto &keyframeEffectStack = EnsureKeyframeEffectStack();

    // In case this element is newly getting a "display: none" we need to cancel all of its animations and
    // disregard new ones.
    if ((!currentStyle || currentStyle->Display() != DisplayType::None)
        && newStyle.Display() == DisplayType::None)
    {
      List<Ref<CSSAnimation>> animationsToCancel(AnimationsCreatedByMarkup().begin(),
                                                 AnimationsCreatedByMarkup().end());
      for (auto &cssAnimation : animationsToCancel)
      {
        cssAnimation->CancelFromStyle();
      }
      keyframeEffectStack.SetCSSAnimationList(null);
      SetAnimationsCreatedByMarkup({});
      return;
    }

    const auto &currentAnimationList = newStyle.GetAnimations();
    const auto &previousAnimationList = keyframeEffectStack.CSSAnimationList();
    if (!Element.HasPendingKeyframesUpdate(PseudoElementIdentifier) && previousAnimationList
        && !previousAnimationList->IsInitial() && newStyle.HasAnimations()
        && *previousAnimationList == newStyle.GetAnimations()
        && !AnimationListContainsNewlyValidAnimation(newStyle.GetAnimations()))
    {
      return;
    }

    CSSAnimationCollection newAnimations;
    auto &previousAnimations = AnimationsCreatedByMarkup();

    keyframeEffectStack.ClearInvalidCSSAnimationNames();

    // https://www.w3.org/TR/css-animations-1/#animations
    // The same @keyframes rule name may be repeated within an animation-name. Changes to the animation-name
    // update existing animations by iterating over the new list of animations from last to first, and, for
    // each animation, finding the last matching animation in the list of existing animations. If a match is
    // found, the existing animation is updated using the animation properties corresponding to its position
    // in the new list of animations, whilst maintaining its current playback time as described above. The
    // matching animation is removed from the existing list of animations such that it will not match twice.
    // If a match is not found, a new animation is created. As a result, updating animation-name from ‘a’ to
    // ‘a, a’ will cause the existing animation for ‘a’ to become the second animation in the list and a new
    // animation will be created for the first item in the list.
    if (!currentAnimationList.IsInitial())
    {
      auto usedValues = currentAnimationList.UsedValues();
      for (auto it = usedValues.rbegin(); it != usedValues.rend(); ++it)
      {
        const auto &currentAnimation = *it;
        auto keyframesName = currentAnimation.GetName().TryKeyframesName();
        if (!keyframesName || keyframesName->Name.View().empty())
        {
          continue;
        }

        if (!KeyframesRuleExistsForAnimation(Element, *keyframesName))
        {
          keyframeEffectStack.AddInvalidCSSAnimationName(css::CSSOMString {keyframesName->Name.View()});
          continue;
        }

        const auto &currentAnimationName = keyframesName->Name;
        bool foundMatchingAnimation = false;
        for (const auto &previousAnimation : previousAnimations)
        {
          if (previousAnimation->AnimationName() == currentAnimationName)
          {
            Ref matchingAnimation = previousAnimation;
            // Timing properties or play state may have changed so we need to update the backing animation
            // with the Animation found in the current style.
            matchingAnimation->SetBackingStyleAnimation(currentAnimation);
            // Keyframes may have been cleared if the @keyframes rules was changed since the last style
            // update, so we must ensure keyframes are picked up.
            matchingAnimation->UpdateKeyframesIfNeeded(currentStyle, newStyle, resolutionContext);
            newStyleOriginatedAnimations.push_back(
              CreateWeakPtr<StyleOriginatedAnimation>(matchingAnimation.get()));
            (void)newAnimations.add(matchingAnimation);
            // Remove the matched animation from the list of previous animations so we may not match it again.
            (void)previousAnimations.remove(matchingAnimation);
            foundMatchingAnimation = true;
            break;
          }
        }

        if (!foundMatchingAnimation && isInDisplayNoneTree == IsInDisplayNoneTree::No)
        {
          auto cssAnimation = CSSAnimation::Create(*this, Animation {currentAnimation}, currentStyle,
                                                   newStyle, resolutionContext);
          newStyleOriginatedAnimations.push_back(CreateWeakPtr<StyleOriginatedAnimation>(cssAnimation.get()));
          (void)newAnimations.add(krys::move(cssAnimation));
        }
      }
    }

    // Any animation found in previousAnimations but not found in newAnimations is not longer current and
    // should be canceled.
    List<Ref<CSSAnimation>> remainingAnimations(previousAnimations.begin(), previousAnimations.end());
    for (auto &previousAnimation : remainingAnimations)
    {
      if (!newAnimations.contains(previousAnimation))
      {
        if (previousAnimation->OwningElement())
        {
          previousAnimation->CancelFromStyle();
        }
      }
    }

    SetAnimationsCreatedByMarkup(krys::move(newAnimations));

    keyframeEffectStack.SetCSSAnimationList(style::Animations {currentAnimationList});
    Element.CSSAnimationsDidUpdate(PseudoElementIdentifier);
  }

  void Styleable::QueryContainerDidChange() const noexcept
  {
    auto *animations = Animations();
    if (!animations)
    {
      return;
    }

    List<Ref<WebAnimation>> protectedAnimations(animations->begin(), animations->end());
    for (auto &animation : protectedAnimations)
    {
      RefPtr keyframeEffect = ShareRefPtr(animation->GetKeyframeEffect());
      if (keyframeEffect && keyframeEffect->GetBlendingKeyframes().UsesContainerUnits())
      {
        if (auto *cssAnimation = DynamicDowncast<CSSAnimation>(animation.get()))
        {
          cssAnimation->KeyframesRuleDidChange();
        }
        else
        {
          keyframeEffect->RecomputeKeyframesAtNextOpportunity();
        }
      }
    }
  }

  namespace
  {
    KeyframeEffect *KeyframeEffectForElementAndProperty(const Styleable &styleable,
                                                        const AnimatableCSSProperty &property) noexcept
    {
      if (auto *keyframeEffectStack = styleable.GetKeyframeEffectStack())
      {
        const auto &effects = keyframeEffectStack->SortedEffects();
        for (auto it = effects.rbegin(); it != effects.rend(); ++it)
        {
          if ((*it)->AnimatesProperty(property))
          {
            return it->get();
          }
        }
      }
      return nullptr;
    }

    bool PropertyInStyleMatchesValueForTransitionInMap(const AnimatableCSSProperty &property,
                                                       const RenderStyle &style,
                                                       AnimatableCSSPropertyToTransitionMap &transitions,
                                                       const dom::Document &document) noexcept
    {
      auto it = transitions.find(property);
      if (it != transitions.end())
      {
        if (interpolation::Equals(property, style, it->second->TargetStyle(), document))
        {
          return true;
        }
      }
      return false;
    }

    bool TransitionMatchesProperty(const Transition &transition, const AnimatableCSSProperty &property,
                                   const RenderStyle &style) noexcept
    {
      if (transition.IsPropertyFilled())
      {
        return false;
      }

      return transition.GetProperty().SwitchOn(
        [&](const css::keywords::All &) { return true; }, [&](const css::keywords::None &) { return false; },
        [&](const SingleTransitionProperty::UnknownProperty &) { return false; },
        [&](const SingleTransitionProperty::SingleProperty &singleProperty)
        {
          return krys::SwitchOn(
            singleProperty.Value,
            [&](css::PropertyId propertyId)
            {
              const auto *propertyIdToMatch = std::get_if<css::PropertyId>(&property);
              if (!propertyIdToMatch)
              {
                return false;
              }
              auto resolvedPropertyId =
                css::Property::ResolveDirectionAwareProperty(propertyId, style.GetWritingMode());
              if (resolvedPropertyId == *propertyIdToMatch)
              {
                return true;
              }
              for (auto longhand : css::ShorthandForProperty(resolvedPropertyId))
              {
                auto resolvedLonghand =
                  css::Property::ResolveDirectionAwareProperty(longhand, style.GetWritingMode());
                if (resolvedLonghand == *propertyIdToMatch)
                {
                  return true;
                }
              }
              return false;
            },
            [&](const css::CSSOMStringAtom &customProperty)
            {
              const auto *customPropertyToMatch = std::get_if<css::CSSOMStringAtom>(&property);
              return customPropertyToMatch && *customPropertyToMatch == customProperty;
            });
        });
    }

    void CompileTransitionPropertiesInStyle(const RenderStyle &style,
                                            CSSPropertiesBitSet &transitionProperties,
                                            HashSet<css::CSSOMStringAtom> &transitionCustomProperties,
                                            bool &transitionPropertiesContainAll) noexcept
    {
      const auto &transitions = style.GetTransitions();
      if (transitions.IsInitial())
      {
        // If we don't have any transitions in the map, this means that the initial value "all 0s" was set.
        transitionPropertiesContainAll = true;
        return;
      }

      for (const auto &transition : transitions.UsedValues())
      {
        transition.GetProperty().SwitchOn(
          [&](const css::keywords::All &) { transitionPropertiesContainAll = true; },
          [&](const css::keywords::None &)
          {
            // Do nothing.
          },
          [&](const SingleTransitionProperty::UnknownProperty &)
          {
            // Do nothing.
          },
          [&](const SingleTransitionProperty::SingleProperty &property)
          {
            krys::SwitchOn(
              property.Value,
              [&](css::PropertyId propertyId)
              {
                auto resolvedPropertyId =
                  css::Property::ResolveDirectionAwareProperty(propertyId, style.GetWritingMode());
                if (css::IsShorthand(resolvedPropertyId))
                {
                  for (auto longhand : css::ShorthandForProperty(resolvedPropertyId))
                  {
                    transitionProperties.Set(longhand);
                  }
                }
                else if (resolvedPropertyId != css::PropertyId::Invalid)
                {
                  transitionProperties.Set(resolvedPropertyId);
                }
              },
              [&](const css::CSSOMStringAtom &customProperty)
              { transitionCustomProperties.insert(customProperty); });
          });
      }
    }

    void UpdateCSSTransitionsForStyleableAndProperty(
      const Styleable &styleable, const AnimatableCSSProperty &property, const RenderStyle &currentStyle,
      const RenderStyle &newStyle, const Seconds generationTime,
      WeakStyleOriginatedAnimations &newStyleOriginatedAnimations) noexcept
    {
      auto *keyframeEffect = KeyframeEffectForElementAndProperty(styleable, property);
      auto *animation = keyframeEffect ? keyframeEffect->Animation() : nullptr;

      bool isDeclarative = false;
      if (auto *styleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(animation))
      {
        if (auto owningElement = styleOriginatedAnimation->OwningElement())
        {
          isDeclarative = *owningElement == styleable;
        }
      }

      if (animation && !isDeclarative)
      {
        return;
      }

      auto &document = styleable.Element.NodeDocument();

      auto hasMatchingTransitionProperty = false;
      auto matchingTransitionDuration = 0.0;
      Maybe<Transition> matchingTransition;
      if (const auto &transitions = newStyle.GetTransitions(); !transitions.IsInitial())
      {
        for (const auto &transition : transitions.UsedValues())
        {
          if (TransitionMatchesProperty(transition, property, newStyle))
          {
            hasMatchingTransitionProperty = true;
            matchingTransition = transition;
            matchingTransitionDuration =
              std::max(0.0, static_cast<double>(matchingTransition->GetDuration().Value))
              + matchingTransition->GetDelay().Value;
          }
        }
      }
      else
      {
        // If we don't have any transitions in the map, this means that the initial value "all 0s" was set and
        // thus all properties match.
        // FIXME: The quirk for sites relying on resetting transitions not canceling running ones
        // (needsResettingTransitionCancelsRunningTransitionQuirk), once there are quirks.
        hasMatchingTransitionProperty = true;
      }

      // https://drafts.csswg.org/css-transitions-1/#before-change-style
      // Define the before-change style as the computed values of all properties on the element as of the
      // previous style change event, except with any styles derived from declarative animations such as CSS
      // Transitions, CSS Animations, and SMIL Animations updated to the current time.
      auto beforeChangeStyle = [&]() -> RenderStyle
      {
        if (auto *lastStyleChangeEventStyle = styleable.LastStyleChangeEventStyle())
        {
          auto style = RenderStyle::Clone(*lastStyleChangeEventStyle);
          if (auto *keyframeEffectStack = styleable.GetKeyframeEffectStack())
          {
            for (const auto &effect : keyframeEffectStack->SortedEffects())
            {
              if (effect->AnimatesProperty(property))
              {
                Ref protectedEffect = ShareRef(*effect);
                (void)protectedEffect->Apply(style, {nullptr});
              }
            }
          }
          return style;
        }
        return RenderStyle::Clone(currentStyle);
      }();

      // https://drafts.csswg.org/css-transitions-1/#after-change-style
      // Likewise, define the after-change style as the computed values of all properties on the element based
      // on the information known at the start of that style change event, but using the computed values of
      // the animation-* properties from the before-change style, excluding any styles from CSS Transitions in
      // the computation, and inheriting from the after-change style of the parent. Note that this means the
      // after-change style does not differ from the before-change style due to newly created or canceled CSS
      // Animations.
      auto afterChangeStyle = [&]() -> RenderStyle
      {
        if (animation && Is<CSSAnimation>(*animation) && animation->IsRelevant())
        {
          auto animatedStyle = RenderStyle::Clone(newStyle);
          (void)animation->Resolve(animatedStyle, {nullptr});
          return animatedStyle;
        }
        return RenderStyle::Clone(newStyle);
      }();

      auto allowsDiscreteTransitions =
        matchingTransition && matchingTransition->GetBehavior() == TransitionBehavior::AllowDiscrete;
      auto propertyCanBeInterpolated =
        [&](const AnimatableCSSProperty &property, const RenderStyle &a, const RenderStyle &b)
      {
        return allowsDiscreteTransitions || interpolation::CanInterpolate(property, a, b, document);
      };

      auto createCSSTransition = [&](const RenderStyle &oldStyle, Seconds delay, Seconds duration,
                                     const RenderStyle &reversingAdjustedStartStyle,
                                     double reversingShorteningFactor)
      {
        auto cssTransition = CSSTransition::Create(styleable, property, generationTime, *matchingTransition,
                                                   oldStyle, afterChangeStyle, delay, duration,
                                                   reversingAdjustedStartStyle, reversingShorteningFactor);
        newStyleOriginatedAnimations.push_back(CreateWeakPtr<StyleOriginatedAnimation>(cssTransition.get()));
        styleable.EnsureRunningTransitionsByProperty().insert_or_assign(property, krys::move(cssTransition));
      };

      bool hasRunningTransition = styleable.HasRunningTransitionForProperty(property);
      if (!hasRunningTransition && hasMatchingTransitionProperty && matchingTransitionDuration > 0
          && !interpolation::Equals(property, beforeChangeStyle, afterChangeStyle, document)
          && propertyCanBeInterpolated(property, beforeChangeStyle, afterChangeStyle)
          && !PropertyInStyleMatchesValueForTransitionInMap(
            property, afterChangeStyle, styleable.EnsureCompletedTransitionsByProperty(), document))
      {
        // 1. If all of the following are true:
        //   - the element does not have a running transition for the property,
        //   - the before-change style is different from and can be interpolated with the after-change style
        //   for
        //     that property,
        //   - the element does not have a completed transition for the property or the end value of the
        //   completed
        //     transition is different from the after-change style for the property,
        //   - there is a matching transition-property value, and
        //   - the combined duration is greater than 0s,

        // then implementations must remove the completed transition (if present) from the set of completed
        // transitions
        styleable.EnsureCompletedTransitionsByProperty().erase(property);

        // and start a transition whose:
        //   - start time is the time of the style change event plus the matching transition delay,
        //   - end time is the start time plus the matching transition duration,
        //   - start value is the value of the transitioning property in the before-change style,
        //   - end value is the value of the transitioning property in the after-change style,
        //   - reversing-adjusted start value is the same as the start value, and
        //   - reversing shortening factor is 1.
        krys_debug_assert(matchingTransition);
        auto delay = Seconds {matchingTransition->GetDelay().Value};
        auto duration = Seconds {matchingTransition->GetDuration().Value};
        const auto &reversingAdjustedStartStyle = beforeChangeStyle;
        auto reversingShorteningFactor = 1;
        createCSSTransition(beforeChangeStyle, delay, duration, reversingAdjustedStartStyle,
                            reversingShorteningFactor);
        krys_debug_assert(styleable.HasRunningTransitionForProperty(property));
        hasRunningTransition = true;
      }
      else if (styleable.HasCompletedTransitionForProperty(property)
               && !PropertyInStyleMatchesValueForTransitionInMap(
                 property, afterChangeStyle, styleable.EnsureCompletedTransitionsByProperty(), document))
      {
        // 2. Otherwise, if the element has a completed transition for the property and the end value of the
        //    completed transition is different from the after-change style for the property, then
        //    implementations must remove the completed transition from the set of completed transitions.
        styleable.EnsureCompletedTransitionsByProperty().erase(property);
      }

      if (!hasMatchingTransitionProperty)
      {
        // 3. If the element has a running transition or completed transition for the property, and there is
        // not a
        //    matching transition-property value, then implementations must cancel the running transition or
        //    remove the completed transition from the set of completed transitions.
        if (hasRunningTransition)
        {
          auto &runningTransitions = styleable.EnsureRunningTransitionsByProperty();
          auto it = runningTransitions.find(property);
          Ref runningTransition = krys::move(it->second);
          runningTransitions.erase(it);
          runningTransition->CancelFromStyle();
        }
        else if (styleable.HasCompletedTransitionForProperty(property))
        {
          styleable.EnsureCompletedTransitionsByProperty().erase(property);
        }
        return;
      }

      krys_debug_assert(hasMatchingTransitionProperty);

      if (hasRunningTransition
          && !PropertyInStyleMatchesValueForTransitionInMap(
            property, afterChangeStyle, styleable.EnsureRunningTransitionsByProperty(), document))
      {
        auto &runningTransitions = styleable.EnsureRunningTransitionsByProperty();
        auto it = runningTransitions.find(property);
        Ref previouslyRunningTransition = krys::move(it->second);
        runningTransitions.erase(it);

        auto previouslyRunningTransitionCurrentStyle = [&]
        {
          if (auto *lastStyleChangeEventStyle = styleable.LastStyleChangeEventStyle())
          {
            auto style = RenderStyle::Clone(*lastStyleChangeEventStyle);
            krys_debug_assert(previouslyRunningTransition->GetKeyframeEffect());
            Ref protectedEffect = ShareRef(*previouslyRunningTransition->GetKeyframeEffect());
            (void)protectedEffect->Apply(style, {nullptr});
            return style;
          }
          return RenderStyle::Clone(currentStyle);
        }();

        // 4. If the element has a running transition for the property, there is a matching
        // transition-property
        //    value, and the end value of the running transition is not equal to the value of the property in
        //    the after-change style, then:
        if (interpolation::Equals(property, previouslyRunningTransitionCurrentStyle, afterChangeStyle,
                                  document)
            || !propertyCanBeInterpolated(property, currentStyle, afterChangeStyle))
        {
          // 1. If the current value of the property in the running transition is equal to the value of the
          //    property in the after-change style, or if these two values cannot be interpolated, then
          //    implementations must cancel the running transition.
          previouslyRunningTransition->CancelFromStyle();
        }
        else if (matchingTransitionDuration <= 0.0
                 || !propertyCanBeInterpolated(property, previouslyRunningTransitionCurrentStyle,
                                               afterChangeStyle))
        {
          // 2. Otherwise, if the combined duration is less than or equal to 0s, or if the current value of
          // the
          //    property in the running transition cannot be interpolated with the value of the property in
          //    the after-change style, then implementations must cancel the running transition.
          previouslyRunningTransition->CancelFromStyle();
        }
        else if (interpolation::Equals(property, previouslyRunningTransition->ReversingAdjustedStartStyle(),
                                       afterChangeStyle, document))
        {
          // 3. Otherwise, if the reversing-adjusted start value of the running transition is the same as the
          // value
          //    of the property in the after-change style (see the section on reversing of transitions for why
          //    these case exists), implementations must cancel the running transition and start a new
          //    transition whose:
          //   - reversing-adjusted start value is the end value of the running transition,
          //   - reversing shortening factor is the absolute value, clamped to the range [0, 1], of the sum
          //   of:
          //       1. the output of the timing function of the old transition at the time of the style change
          //          event, times the reversing shortening factor of the old transition
          //       2. 1 minus the reversing shortening factor of the old transition.
          //   - start time is the time of the style change event plus:
          //       1. if the matching transition delay is nonnegative, the matching transition delay, or
          //       2. if the matching transition delay is negative, the product of the new transition’s
          //       reversing
          //          shortening factor and the matching transition delay,
          //   - end time is the start time plus the product of the matching transition duration and the new
          //     transition’s reversing shortening factor,
          //   - start value is the current value of the property in the running transition,
          //   - end value is the value of the property in the after-change style
          krys_debug_assert(matchingTransition);
          auto reversingAdjustedStartStyle = RenderStyle::Clone(previouslyRunningTransition->TargetStyle());
          double transformedProgress = 1;
          if (auto *effect = previouslyRunningTransition->Effect())
          {
            if (auto computedTimingProgress = effect->GetComputedTiming().Progress)
            {
              transformedProgress = *computedTimingProgress;
            }
          }
          auto reversingShorteningFactor = std::max(
            std::min(((transformedProgress * previouslyRunningTransition->ReversingShorteningFactor())
                      + (1 - previouslyRunningTransition->ReversingShorteningFactor())),
                     1.0),
            0.0);
          auto delay = matchingTransition->GetDelay().Value < 0
                         ? Seconds {matchingTransition->GetDelay().Value} * reversingShorteningFactor
                         : Seconds {matchingTransition->GetDelay().Value};
          auto duration = Seconds {matchingTransition->GetDuration().Value} * reversingShorteningFactor;

          previouslyRunningTransition->CancelFromStyle();
          createCSSTransition(beforeChangeStyle, delay, duration, reversingAdjustedStartStyle,
                              reversingShorteningFactor);
        }
        else
        {
          // 4. Otherwise, implementations must cancel the running transition and start a new transition
          // whose:
          //   - start time is the time of the style change event plus the matching transition delay,
          //   - end time is the start time plus the matching transition duration,
          //   - start value is the current value of the property in the running transition,
          //   - end value is the value of the property in the after-change style,
          //   - reversing-adjusted start value is the same as the start value, and
          //   - reversing shortening factor is 1.
          krys_debug_assert(matchingTransition);
          auto delay = Seconds {matchingTransition->GetDelay().Value};
          auto duration = Seconds {matchingTransition->GetDuration().Value};
          const auto &reversingAdjustedStartStyle = currentStyle;
          auto reversingShorteningFactor = 1;
          previouslyRunningTransition->CancelFromStyle();
          createCSSTransition(previouslyRunningTransitionCurrentStyle, delay, duration,
                              reversingAdjustedStartStyle, reversingShorteningFactor);
        }
      }
    }
  }

  void Styleable::UpdateCSSTransitions(
    const RenderStyle &currentStyle, const RenderStyle &newStyle,
    WeakStyleOriginatedAnimations &newStyleOriginatedAnimations) const noexcept
  {
    // In case this element previous had "display: none" we can stop considering transitions altogether.
    if (currentStyle.Display() == DisplayType::None)
    {
      return;
    }

    // In case this element is newly getting a "display: none" we need to cancel all of its transitions and
    // disregard new ones, unless it will transition the "display" property itself.
    if (currentStyle.HasTransitions() && currentStyle.Display() != DisplayType::None
        && newStyle.Display() == DisplayType::None && !StyleHasDisplayTransition(newStyle))
    {
      if (HasRunningTransitions())
      {
        auto runningTransitions = EnsureRunningTransitionsByProperty();
        for (const auto &[property, transition] : runningTransitions)
        {
          transition->CancelFromStyle();
        }
      }
      return;
    }

    // Section 3 "Starting of transitions" from the CSS Transitions Level 1 specification.
    // https://drafts.csswg.org/css-transitions-1/#starting

    auto generationTime = Seconds {html::EventLoop::Current().Now()};

    // First, let's compile the list of all CSS properties found in the current style and the after-change
    // style.
    bool transitionPropertiesContainAll = false;
    CSSPropertiesBitSet transitionProperties;
    HashSet<css::CSSOMStringAtom> transitionCustomProperties;
    CompileTransitionPropertiesInStyle(currentStyle, transitionProperties, transitionCustomProperties,
                                       transitionPropertiesContainAll);
    CompileTransitionPropertiesInStyle(newStyle, transitionProperties, transitionCustomProperties,
                                       transitionPropertiesContainAll);

    if (transitionPropertiesContainAll)
    {
      auto addProperty = [&](const AnimatableCSSProperty &property)
      {
        if (const auto *propertyId = std::get_if<css::PropertyId>(&property))
        {
          if (css::IsShorthand(*propertyId))
          {
            for (auto longhand : css::ShorthandForProperty(*propertyId))
            {
              transitionProperties.Set(longhand);
            }
          }
          else if (*propertyId != css::PropertyId::Invalid)
          {
            transitionProperties.Set(*propertyId);
          }
        }
      };

      const RenderStyle *targetStyle = &currentStyle;
      if (auto *lastStyleChangeEventStyle = LastStyleChangeEventStyle())
      {
        targetStyle = lastStyleChangeEventStyle;
      }

      ConservativelyCollectChangedAnimatableProperties(*targetStyle, newStyle, transitionProperties);

      // When we have keyframeEffectStack, it can affect on properties. So we just add them.
      if (auto *keyframeEffectStack = GetKeyframeEffectStack())
      {
        for (const auto &effect : keyframeEffectStack->SortedEffects())
        {
          for (const auto &property : effect->AnimatedProperties())
          {
            addProperty(property);
          }
          if (auto *transition = DynamicDowncast<CSSTransition>(effect->Animation()))
          {
            addProperty(transition->Property());
          }
        }
      }

      if (auto *properties = Element.RunningTransitionsByProperty(PseudoElementIdentifier))
      {
        for (const auto &[property, transition] : *properties)
        {
          addProperty(property);
        }
      }

      if (auto *properties = Element.CompletedTransitionsByProperty(PseudoElementIdentifier))
      {
        for (const auto &[property, transition] : *properties)
        {
          addProperty(property);
        }
      }

      auto gatherAnimatableCustomProperties = [&](const CustomPropertyData &customPropertyData)
      {
        if (!customPropertyData.MayHaveAnimatableProperties())
        {
          return;
        }

        customPropertyData.ForEach(
          [&](const auto &customPropertyAndValuePair)
          {
            const auto &[customProperty, customPropertyValue] = customPropertyAndValuePair;
            if (customPropertyValue->IsAnimatable())
            {
              transitionCustomProperties.insert(customProperty);
            }
            return IterationStatus::Continue;
          });
      };

      gatherAnimatableCustomProperties(currentStyle.InheritedCustomProperties());
      gatherAnimatableCustomProperties(currentStyle.NonInheritedCustomProperties());
      gatherAnimatableCustomProperties(newStyle.InheritedCustomProperties());
      gatherAnimatableCustomProperties(newStyle.NonInheritedCustomProperties());
    }

    for (size_t index = 0; index < transitionProperties.Properties.size(); ++index)
    {
      if (!transitionProperties.Properties.test(index))
      {
        continue;
      }

      auto propertyId = static_cast<css::PropertyId>(index);
      if (css::IsShorthand(propertyId))
      {
        continue;
      }

      UpdateCSSTransitionsForStyleableAndProperty(*this, propertyId, currentStyle, newStyle, generationTime,
                                                  newStyleOriginatedAnimations);
    }

    for (const auto &customProperty : transitionCustomProperties)
    {
      UpdateCSSTransitionsForStyleableAndProperty(*this, customProperty, currentStyle, newStyle,
                                                  generationTime, newStyleOriginatedAnimations);
    }
  }
}
