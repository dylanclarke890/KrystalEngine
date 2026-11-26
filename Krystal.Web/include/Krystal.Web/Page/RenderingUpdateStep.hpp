#pragma once

#include "Krystal.Core/Enum.hpp"

namespace krys::web
{
  /// @brief The steps of the rendering update that something asked for.
  // FIXME: The steps for the features Krystal.Web doesn't have yet: revealing documents, fullscreen, paint and
  // event timing, video frame callbacks, validation messages and the scrolling tree update.
  enum class RenderingUpdateStep : uint8
  {
    FlushAutofocusCandidates,
    Resize,
    Scroll,
    MediaQueryEvaluation,
    Animations,
    AnimationFrameCallbacks,
    CaretAnimation,
    Images,
    ResizeObservations,
    FocusFixup,
    UpdateContentRelevancy,
    IntersectionObservations,
    SnapshottedScrollOffsets,
    CursorUpdate,
    PrepareCanvasesForDisplayOrFlush,
    LayerFlush,
  };

  consteval RenderingUpdateStep MaxEnumValue(RenderingUpdateStep) noexcept
  {
    return RenderingUpdateStep::LayerFlush;
  }
}
