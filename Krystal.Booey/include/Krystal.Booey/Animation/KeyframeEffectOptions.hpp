#pragma once

#include "Krystal.Booey/Animation/EffectTiming.hpp"
#include "Krystal.Booey/Graphics/Animation/CompositeOperation.hpp"
#include "Krystal.Booey/Graphics/Animation/IterationCompositeOperation.hpp"

namespace krys::boo
{
  /// @brief A keyframe effect's timing, composite operations and pseudo-element target.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-keyframeeffectoptions
  struct KeyframeEffectOptions : EffectTiming
  {
    IterationCompositeOperation IterationComposite {IterationCompositeOperation::Replace};
    CompositeOperation Composite {CompositeOperation::Replace};
    /// @brief A pseudo-element selector (`::before`); empty for the element itself.
    dom::DOMString PseudoElement;
  };
}
