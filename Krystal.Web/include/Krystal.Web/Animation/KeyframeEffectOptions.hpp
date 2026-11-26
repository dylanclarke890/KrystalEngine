#pragma once

#include "Krystal.Core/Maths/CompositeOperation.hpp"
#include "Krystal.Web/Animation/Timing/EffectTiming.hpp"
#include "Krystal.Web/Graphics/Animation/IterationCompositeOperation.hpp"

namespace krys::web
{
  /// @brief A keyframe effect's timing, composite operations and pseudo-element target.
  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-keyframeeffectoptions
  struct KeyframeEffectOptions : EffectTiming
  {
    IterationCompositeOperation IterationComposite {IterationCompositeOperation::Replace};
    CompositeOperation Composite {CompositeOperation::Replace};
    /// @brief A pseudo-element selector (`::before`); empty for the element itself.
    DOMString PseudoElement;
  };
}
