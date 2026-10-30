#pragma once

#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"

namespace krys::boo
{
  class RenderBlockFlow;

  /// @brief Attaches children to block flows.
  // FIXME: Multi-column flows and fieldset legends in them, once there are multi-column flows.
  class RenderTreeBuilder::BlockFlow
  {
    KRYS_NON_COPY_MOVABLE(BlockFlow);

  private:
    RenderTreeBuilder &_builder;

  public:
    explicit BlockFlow(RenderTreeBuilder &builder) noexcept;

    void Attach(RenderBlockFlow &parent, RenderPtr<RenderObject> child, RenderObject *beforeChild) noexcept;
    void MoveAllChildrenIncludingFloats(RenderBlockFlow &from, RenderBlock &to,
                                        NormalizeAfterInsertion normalizeAfterInsertion) noexcept;
  };
}
