#pragma once

namespace krys::boo::style
{
  // <try-tactic> = flip-block || flip-inline || flip-start || flip-x || flip-y
  // https://drafts.csswg.org/css-anchor-position-1/#typedef-position-try-fallbacks-try-tactic
  enum class PositionTryFallbackTactic : uint8
  {
    FlipBlock,
    FlipInline,
    FlipStart,
    FlipX,
    FlipY,
  };
}
