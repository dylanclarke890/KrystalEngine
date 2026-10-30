#include "Krystal.Booey/Rendering/RenderTableCaption.hpp"
#include "Krystal.Booey/Rendering/RenderTable.hpp"

namespace krys::boo
{
  RenderTableCaption::RenderTableCaption(dom::Element &element, RenderStyle &&style) noexcept
      : RenderBlockFlow(Type::TableCaption, element, krys::move(style))
  {
    krys_debug_assert(IsRenderTableCaption());
  }

  // Do not add any code in below destructor. Add it to willBeDestroyed() instead.
  RenderTableCaption::~RenderTableCaption() noexcept = default;

  void RenderTableCaption::InsertedIntoTree() noexcept
  {
    RenderBlockFlow::InsertedIntoTree();
    Table()->AddCaption(*this);
  }

  void RenderTableCaption::WillBeRemovedFromTree() noexcept
  {
    RenderBlockFlow::WillBeRemovedFromTree();
    Table()->RemoveCaption(*this);
  }

  RenderTable *RenderTableCaption::Table() const noexcept
  {
    return Downcast<RenderTable>(Parent());
  }

  LayoutUnit RenderTableCaption::ContainingBlockLogicalWidthForContent() const noexcept
  {
    if (auto *containingBlock = ContainingBlock())
    {
      return containingBlock->LogicalWidth();
    }
    return {};
  }
}
