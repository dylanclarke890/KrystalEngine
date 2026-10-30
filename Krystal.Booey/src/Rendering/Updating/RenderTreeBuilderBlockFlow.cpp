#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"

namespace krys::boo
{
  RenderTreeBuilder::BlockFlow::BlockFlow(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::BlockFlow::Attach(RenderBlockFlow &parent, RenderPtr<RenderObject> child,
                                            RenderObject *beforeChild) noexcept
  {
    // FIXME: Multi-column flows (and fieldset legends, which avoid them), and children moved into the
    // enclosing fragmented flow, once there are any.
    _builder.BlockBuilder().Attach(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::BlockFlow::MoveAllChildrenIncludingFloats(
    RenderBlockFlow &from, RenderBlock &to, NormalizeAfterInsertion normalizeAfterInsertion) noexcept
  {
    _builder.MoveAllChildren(from, to, normalizeAfterInsertion);
    from.AddFloatsToNewParent(Downcast<RenderBlockFlow>(to));
  }
}
