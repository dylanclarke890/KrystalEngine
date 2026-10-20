#pragma once

#include "Krystal.Booey/Animation/AnimationEventBase.hpp"
#include "Krystal.Booey/CSS/Types/CSSOMString.hpp"
#include "Krystal.Booey/Style/PseudoElementIdentifier.hpp"

namespace krys::boo
{
  /// @brief An event about a CSS animation or a CSS transition, fired at its owning element.
  class StyleOriginatedAnimationEvent : public AnimationEventBase
  {
  private:
    double _elapsedTime;
    css::CSSOMString _pseudoElement;
    Maybe<style::PseudoElementIdentifier> _pseudoElementIdentifier;

  protected:
    StyleOriginatedAnimationEvent(
      dom::DOMStringAtom type, WebAnimation *animation, Maybe<Seconds> scheduledTime, double elapsedTime,
      const Maybe<style::PseudoElementIdentifier> &pseudoElementIdentifier) noexcept;
    StyleOriginatedAnimationEvent(dom::DOMStringAtom type, const dom::EventInit &eventInitDict,
                                  double elapsedTime, const css::CSSOMString &pseudoElement) noexcept;

  public:
    ~StyleOriginatedAnimationEvent() noexcept override;

    /// @brief In seconds.
    KRYS_NODISCARD double ElapsedTime() const noexcept
    {
      return _elapsedTime;
    }

    KRYS_NODISCARD const css::CSSOMString &PseudoElement() noexcept;

    KRYS_NODISCARD const Maybe<style::PseudoElementIdentifier> &PseudoElementIdentifier() const noexcept
    {
      return _pseudoElementIdentifier;
    }
  };
}
