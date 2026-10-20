#include "Krystal.Booey/Animation/WebAnimationUtilities.hpp"
#include "Krystal.Booey/Animation/AnimationPlaybackEvent.hpp"
#include "Krystal.Booey/Animation/CSSAnimation.hpp"
#include "Krystal.Booey/Animation/CSSAnimationEvent.hpp"
#include "Krystal.Booey/Animation/CSSTransition.hpp"
#include "Krystal.Booey/Animation/CSSTransitionEvent.hpp"
#include "Krystal.Booey/Animation/KeyframeEffectStack.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParser.hpp"
#include "Krystal.Booey/CSS/Selectors/SelectorParserContext.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"
#include "Krystal.Booey/Style/Styleable.hpp"

namespace krys::boo
{
  namespace
  {
    KRYS_NODISCARD bool CompareStyleOriginatedAnimationOwningElementPositionsInDocumentTreeOrder(
      const style::Styleable &a, const style::Styleable &b) noexcept
    {
      // We should not ever be calling this function with two Elements that are the same. If that were the
      // case, then comparing objects of this kind would yield inconsistent results when comparing A == B and
      // B == A. As such, this function should be called with std::stable_sort().
      krys_assert(a != b);

      // With regard to pseudo-elements, the sort order is as follows:
      //
      //     - element
      //     - ::marker
      //     - ::before
      //     - any other pseudo-elements not mentioned specifically in this list, sorted in ascending order by
      //       the Unicode codepoints that make up each selector
      //     - ::after
      //     - element children
      enum SortingIndex : uint8
      {
        NotPseudo,
        Marker,
        Before,
        FirstLetter,
        FirstLine,
        GrammarError,
        Highlight,
        Selection,
        SpellingError,
        TargetText,
        Checkmark,
        After,
        ViewTransition,
        ViewTransitionGroup,
        ViewTransitionImagePair,
        ViewTransitionOld,
        ViewTransitionNew,
        Other
      };

      auto sortingIndex =
        [](const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) -> SortingIndex
      {
        if (!pseudoElementIdentifier)
        {
          return NotPseudo;
        }

        switch (pseudoElementIdentifier->Type)
        {
          case PseudoElementType::Marker:                  return Marker;
          case PseudoElementType::Before:                  return Before;
          case PseudoElementType::FirstLetter:             return FirstLetter;
          case PseudoElementType::FirstLine:               return FirstLine;
          case PseudoElementType::GrammarError:            return GrammarError;
          case PseudoElementType::Highlight:               return Highlight;
          case PseudoElementType::Selection:               return Selection;
          case PseudoElementType::SpellingError:           return SpellingError;
          case PseudoElementType::TargetText:              return TargetText;
          case PseudoElementType::After:                   return After;
          case PseudoElementType::Checkmark:               return Checkmark;
          case PseudoElementType::ViewTransition:          return ViewTransition;
          case PseudoElementType::ViewTransitionGroup:     return ViewTransitionGroup;
          case PseudoElementType::ViewTransitionImagePair: return ViewTransitionImagePair;
          case PseudoElementType::ViewTransitionOld:       return ViewTransitionOld;
          case PseudoElementType::ViewTransitionNew:       return ViewTransitionNew;
          default:                                         krys_debug_assert(false); return Other;
        }
      };

      if (&a.Element == &b.Element)
      {
        // FIXME: Ordering the named view transition pseudo-elements of an element by the active view
        // transition's named elements, once there are view transitions.
        auto aSortingIndex = sortingIndex(a.PseudoElementIdentifier);
        auto bSortingIndex = sortingIndex(b.PseudoElementIdentifier);
        krys_debug_assert(aSortingIndex != bSortingIndex);
        return aSortingIndex < bSortingIndex;
      }

      return dom::TreeQueries::IsPreceding(a.Element, b.Element);
    }

    KRYS_NODISCARD bool CompareCSSTransitions(const CSSTransition &a, const CSSTransition &b) noexcept
    {
      auto aOwningElement = a.OwningElement();
      auto bOwningElement = b.OwningElement();
      krys_debug_assert(aOwningElement);
      krys_debug_assert(bOwningElement);

      // If the owning element of A and B differs, sort A and B by tree order of their corresponding owning
      // elements.
      if (*aOwningElement != *bOwningElement)
      {
        return CompareStyleOriginatedAnimationOwningElementPositionsInDocumentTreeOrder(*aOwningElement,
                                                                                        *bOwningElement);
      }

      // Otherwise, if A and B have different transition generation values, sort by their corresponding
      // transition generation in ascending order.
      if (a.GenerationTime() != b.GenerationTime())
      {
        return a.GenerationTime() < b.GenerationTime();
      }

      // Otherwise, sort A and B in ascending order by the Unicode codepoints that make up the expanded
      // transition property name of each transition (i.e. without attempting case conversion and such that
      // ‘-moz-column-width’ sorts before ‘column-width’).
      return a.TransitionProperty().View() < b.TransitionProperty().View();
    }

    KRYS_NODISCARD bool CompareCSSAnimations(const CSSAnimation &a, const CSSAnimation &b) noexcept
    {
      // https://drafts.csswg.org/css-animations-2/#animation-composite-order
      auto aOwningElement = a.OwningElement();
      auto bOwningElement = b.OwningElement();
      krys_debug_assert(aOwningElement);
      krys_debug_assert(bOwningElement);

      // If the owning element of A and B differs, sort A and B by tree order of their corresponding owning
      // elements.
      if (*aOwningElement != *bOwningElement)
      {
        return CompareStyleOriginatedAnimationOwningElementPositionsInDocumentTreeOrder(*aOwningElement,
                                                                                        *bOwningElement);
      }

      // Sort A and B based on their position in the computed value of the animation-name property of the
      // (common) owning element.
      const auto &cssAnimationList = aOwningElement->EnsureKeyframeEffectStack().CSSAnimationList();
      krys_debug_assert(cssAnimationList);
      krys_debug_assert(!cssAnimationList->IsInitial());

      const auto &aBackingAnimation = a.BackingStyleAnimation();
      const auto &bBackingAnimation = b.BackingStyleAnimation();
      for (const auto &animation : cssAnimationList->UsedValues())
      {
        if (animation.SortingIdentity() == aBackingAnimation.SortingIdentity())
        {
          return true;
        }
        if (animation.SortingIdentity() == bBackingAnimation.SortingIdentity())
        {
          return false;
        }
      }

      // We should have found either of those CSS animations in the CSS animations list.
      krys_unreachable();
    }

    template <typename T>
    KRYS_NODISCARD Maybe<bool> CompareStyleOriginatedAnimationEvents(const AnimationEventBase &a,
                                                                     const AnimationEventBase &b) noexcept
    {
      auto *aAsStyleOriginatedAnimationEvent = DynamicDowncast<T>(&a);
      auto *bAsStyleOriginatedAnimationEvent = DynamicDowncast<T>(&b);
      if (!aAsStyleOriginatedAnimationEvent && !bAsStyleOriginatedAnimationEvent)
      {
        return null;
      }

      if (!!aAsStyleOriginatedAnimationEvent != !!bAsStyleOriginatedAnimationEvent)
      {
        return !bAsStyleOriginatedAnimationEvent;
      }

      auto aScheduledTime = a.ScheduledTime();
      auto bScheduledTime = b.ScheduledTime();
      if (aScheduledTime != bScheduledTime)
      {
        return aScheduledTime < bScheduledTime;
      }

      RefPtr aTarget = a.Target();
      RefPtr bTarget = b.Target();
      if (aTarget == bTarget)
      {
        return false;
      }

      auto aStyleable =
        style::Styleable(*DynamicDowncast<dom::Element>(DynamicDowncast<dom::Node>(aTarget.get())),
                         aAsStyleOriginatedAnimationEvent->PseudoElementIdentifier());
      auto bStyleable =
        style::Styleable(*DynamicDowncast<dom::Element>(DynamicDowncast<dom::Node>(bTarget.get())),
                         bAsStyleOriginatedAnimationEvent->PseudoElementIdentifier());
      return CompareStyleOriginatedAnimationOwningElementPositionsInDocumentTreeOrder(aStyleable, bStyleable);
    }
  }

  bool CompareAnimationsByCompositeOrder(const WebAnimation &a, const WebAnimation &b) noexcept
  {
    // We should not ever be calling this function with two WebAnimation objects that are the same. If that
    // were the case, then comparing objects of this kind would yield inconsistent results when comparing A ==
    // B and B == A. As such, this function should be called with std::stable_sort().
    krys_assert(&a != &b);

    auto *aAsStyleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(&a);
    bool aHasOwningElement = aAsStyleOriginatedAnimation && aAsStyleOriginatedAnimation->OwningElement();
    auto *bAsStyleOriginatedAnimation = DynamicDowncast<StyleOriginatedAnimation>(&b);
    bool bHasOwningElement = bAsStyleOriginatedAnimation && bAsStyleOriginatedAnimation->OwningElement();

    // CSS Transitions sort first.
    auto *aAsCSSTransition = aHasOwningElement ? DynamicDowncast<CSSTransition>(&a) : nullptr;
    auto *bAsCSSTransition = bHasOwningElement ? DynamicDowncast<CSSTransition>(&b) : nullptr;
    if (aAsCSSTransition || bAsCSSTransition)
    {
      if (!!aAsCSSTransition == !!bAsCSSTransition)
      {
        return CompareCSSTransitions(*aAsCSSTransition, *bAsCSSTransition);
      }
      return !bAsCSSTransition;
    }

    // CSS Animations sort next.
    auto *aAsCSSAnimation = aHasOwningElement ? DynamicDowncast<CSSAnimation>(&a) : nullptr;
    auto *bAsCSSAnimation = bHasOwningElement ? DynamicDowncast<CSSAnimation>(&b) : nullptr;
    if (aAsCSSAnimation || bAsCSSAnimation)
    {
      if (!!aAsCSSAnimation == !!bAsCSSAnimation)
      {
        return CompareCSSAnimations(*aAsCSSAnimation, *bAsCSSAnimation);
      }
      return !bAsCSSAnimation;
    }

    // JS-originated animations sort last based on their position in the global animation list.
    // https://drafts.csswg.org/web-animations-1/#animation-composite-order
    krys_assert(a.GlobalPosition() != b.GlobalPosition());
    return a.GlobalPosition() < b.GlobalPosition();
  }

  bool CompareAnimationEventsByCompositeOrder(const AnimationEventBase &a,
                                              const AnimationEventBase &b) noexcept
  {
    // AnimationPlaybackEvent instances sort first.
    bool aIsPlaybackEvent = Is<AnimationPlaybackEvent>(a);
    bool bIsPlaybackEvent = Is<AnimationPlaybackEvent>(b);
    if (aIsPlaybackEvent || bIsPlaybackEvent)
    {
      if (aIsPlaybackEvent != bIsPlaybackEvent)
      {
        return !bIsPlaybackEvent;
      }

      if (a.Animation() == b.Animation())
      {
        return false;
      }

      // https://drafts.csswg.org/web-animations-1/#update-animations-and-send-events
      // 1. Sort the events by their scheduled event time such that events that were scheduled to occur
      // earlier, sort before events scheduled to occur later and events whose scheduled event time is
      // unresolved sort before events with a resolved scheduled event time.
      auto aScheduledTime = a.ScheduledTime();
      auto bScheduledTime = b.ScheduledTime();
      if (aScheduledTime != bScheduledTime)
      {
        if (aScheduledTime && bScheduledTime)
        {
          return *aScheduledTime < *bScheduledTime;
        }
        return !bScheduledTime;
      }

      // 2. Within events with equal scheduled event times, sort by their composite order.

      // CSS Transitions sort first.
      bool aIsCSSTransition = a.Animation() && Is<CSSTransition>(*a.Animation());
      bool bIsCSSTransition = b.Animation() && Is<CSSTransition>(*b.Animation());
      if (aIsCSSTransition || bIsCSSTransition)
      {
        if (aIsCSSTransition == bIsCSSTransition)
        {
          return false;
        }
        return !bIsCSSTransition;
      }

      // CSS Animations sort next.
      bool aIsCSSAnimation = a.Animation() && a.Animation()->IsCSSAnimation();
      bool bIsCSSAnimation = b.Animation() && b.Animation()->IsCSSAnimation();
      if (aIsCSSAnimation || bIsCSSAnimation)
      {
        if (aIsCSSAnimation == bIsCSSAnimation)
        {
          return false;
        }
        return !bIsCSSAnimation;
      }

      // JS-originated animations sort last based on their position in the global animation list.
      RefPtr aAnimation = ShareRefPtr(a.Animation());
      RefPtr bAnimation = ShareRefPtr(b.Animation());
      if (aAnimation == bAnimation)
      {
        return false;
      }

      krys_assert(aAnimation);
      krys_assert(bAnimation);
      krys_assert(aAnimation->GlobalPosition() != bAnimation->GlobalPosition());
      return aAnimation->GlobalPosition() < bAnimation->GlobalPosition();
    }

    // CSSTransitionEvent instances sort next.
    if (auto sorted = CompareStyleOriginatedAnimationEvents<CSSTransitionEvent>(a, b))
    {
      return *sorted;
    }

    // CSSAnimationEvent instances sort last.
    if (auto sorted = CompareStyleOriginatedAnimationEvents<CSSAnimationEvent>(a, b))
    {
      return *sorted;
    }

    return false;
  }

  css::CSSOMString PseudoElementIdentifierAsString(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept
  {
    if (!pseudoElementIdentifier)
    {
      return {};
    }

    auto withName = [&](utf8_stringview prefix)
    {
      css::CSSOMString result {prefix};
      result += u8'(';
      result += pseudoElementIdentifier->NameArgument.View();
      result += u8')';
      return result;
    };

    switch (pseudoElementIdentifier->Type)
    {
      case PseudoElementType::After:                   return u8"::after";
      case PseudoElementType::Before:                  return u8"::before";
      case PseudoElementType::FirstLetter:             return u8"::first-letter";
      case PseudoElementType::FirstLine:               return u8"::first-line";
      case PseudoElementType::GrammarError:            return u8"::grammar-error";
      case PseudoElementType::Highlight:               return withName(u8"::highlight");
      case PseudoElementType::Marker:                  return u8"::marker";
      case PseudoElementType::Selection:               return u8"::selection";
      case PseudoElementType::SpellingError:           return u8"::spelling-error";
      case PseudoElementType::TargetText:              return u8"::target-text";
      case PseudoElementType::Checkmark:               return u8"::checkmark";
      case PseudoElementType::ViewTransition:          return u8"::view-transition";
      case PseudoElementType::ViewTransitionGroup:     return withName(u8"::view-transition-group");
      case PseudoElementType::ViewTransitionImagePair: return withName(u8"::view-transition-image-pair");
      case PseudoElementType::ViewTransitionOld:       return withName(u8"::view-transition-old");
      case PseudoElementType::ViewTransitionNew:       return withName(u8"::view-transition-new");
      default:                                         return {};
    }
  }

  Pair<bool, Maybe<style::PseudoElementIdentifier>>
    PseudoElementIdentifierFromString(const css::CSSOMString &pseudoElement,
                                      const dom::Document *document) noexcept
  {
    // https://drafts.csswg.org/web-animations-1/#dom-keyframeeffect-pseudoelement
    if (pseudoElement.empty())
    {
      return {true, null};
    }

    // FIXME: We should always have a document for accurate settings.
    auto parserContext = document ? css::SelectorParserContext {*document} : css::SelectorParserContext {};
    return css::SelectorParser::ParsePseudoElement(pseudoElement, parserContext);
  }

  bool StyleHasDisplayTransition(const RenderStyle &style) noexcept
  {
    if (!style.HasTransitions())
    {
      return false;
    }

    for (const auto &transition : style.GetTransitions().UsedValues())
    {
      auto result = transition.GetProperty().SwitchOn(
        [&](const css::keywords::All &)
        { return transition.GetBehavior() == TransitionBehavior::AllowDiscrete; },
        [&](const css::keywords::None &) { return false; },
        [&](const style::SingleTransitionProperty::UnknownProperty &) { return false; },
        [&](const style::SingleTransitionProperty::SingleProperty &property)
        {
          if (const auto *propertyId = std::get_if<css::PropertyId>(&property.Value);
              propertyId && *propertyId == css::PropertyId::Display)
          {
            return transition.GetBehavior() == TransitionBehavior::AllowDiscrete;
          }
          return false;
        });

      if (result)
      {
        return true;
      }
    }

    return false;
  }

  bool
    AnimatablePropertiesContainTransformRelatedProperty(const AnimatableCSSPropertySet &properties) noexcept
  {
    return properties.contains(css::PropertyId::Translate) || properties.contains(css::PropertyId::Scale)
           || properties.contains(css::PropertyId::Rotate) || properties.contains(css::PropertyId::Transform);
  }
}
