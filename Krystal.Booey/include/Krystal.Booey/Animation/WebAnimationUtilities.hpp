#pragma once

#include "Krystal.Booey/Animation/WebAnimationTime.hpp"
#include "Krystal.Booey/Animation/WebAnimationTypes.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyId.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Visitor.hpp"
#include <cmath>

namespace krys::boo::dom
{
  class Document;
}

namespace krys::boo
{
  class AnimationEventBase;
  class RenderStyle;
  class WebAnimation;

  /// @brief A time in milliseconds, as the Web Animations interfaces use, rounded to microseconds.
  /// @see https://drafts.csswg.org/web-animations-1/#precision-of-time-values
  KRYS_NODISCARD inline double SecondsToWebAnimationsAPITime(const Seconds time) noexcept
  {
    // The internal representation of time values is implementation dependent however, it is RECOMMENDED that
    // user agents be able to represent input time values with microsecond precision so that a time value
    // (which nominally represents milliseconds) of 0.001 is distinguishable from 0.0.
    auto roundedTime = std::round(time.count() * 1'000'000) / 1'000;
    if (roundedTime == -0)
    {
      return 0;
    }
    return roundedTime;
  }

  /// @brief Whether @p a sorts before @p b in composite order.
  /// @see https://drafts.csswg.org/web-animations-1/#animation-composite-order
  KRYS_NODISCARD bool CompareAnimationsByCompositeOrder(const WebAnimation &a,
                                                        const WebAnimation &b) noexcept;

  /// @brief Whether @p a is dispatched before @p b.
  /// @see https://drafts.csswg.org/web-animations-1/#update-animations-and-send-events
  KRYS_NODISCARD bool CompareAnimationEventsByCompositeOrder(const AnimationEventBase &a,
                                                             const AnimationEventBase &b) noexcept;

  KRYS_NODISCARD css::CSSOMString PseudoElementIdentifierAsString(
    const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;

  /// @return Whether @p pseudoElement parsed, and the pseudo-element it identifies if it did.
  /// @see https://drafts.csswg.org/web-animations-1/#dom-keyframeeffect-pseudoelement
  KRYS_NODISCARD Pair<bool, Maybe<style::PseudoElementIdentifier>>
    PseudoElementIdentifierFromString(const css::CSSOMString &pseudoElement,
                                      const dom::Document *document) noexcept;

  /// @brief The name of an animatable property: the property's name, or the custom property's name.
  KRYS_NODISCARD inline css::CSSOMStringAtom
    AnimatablePropertyAsString(const AnimatableCSSProperty &property) noexcept
  {
    return krys::SwitchOn(
      property, [](css::PropertyId propertyId) { return css::CSSOMStringAtom {css::ToString(propertyId)}; },
      [](const css::CSSOMStringAtom &customProperty) { return customProperty; });
  }

  KRYS_NODISCARD bool
    AnimatablePropertiesContainTransformRelatedProperty(const AnimatableCSSPropertySet &properties) noexcept;

  /// @brief Whether @p style specifies a transition on the `display` property.
  KRYS_NODISCARD bool StyleHasDisplayTransition(const RenderStyle &style) noexcept;
}
