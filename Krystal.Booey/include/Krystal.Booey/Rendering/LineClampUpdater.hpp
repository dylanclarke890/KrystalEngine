#pragma once

#include "Krystal.Booey/Rendering/RenderLayoutState.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief Sets up `line-clamp` (`max-lines`) for a block container's subtree while it's laid out: a new
  /// clamp, or the one propagated from an ancestor. When it goes out of scope, it restores the propagated
  /// clamp minus the lines the block container used.
  class LineClampUpdater
  {
    KRYS_NON_COPY_MOVABLE(LineClampUpdater);

  private:
    const RenderBlockFlow &_blockContainer;
    Maybe<RenderLayoutState::LineClamp> _previousLineClamp {};
    // Whether the block container establishes an independent formatting context, whose lines aren't counted.
    bool _skipsLineCounting {false};

  public:
    explicit LineClampUpdater(const RenderBlockFlow &blockContainer) noexcept;
    ~LineClampUpdater() noexcept;
  };
}
