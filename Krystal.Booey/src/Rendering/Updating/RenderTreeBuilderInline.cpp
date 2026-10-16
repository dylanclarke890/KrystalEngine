#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderInline.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"

namespace krys::boo
{
  RenderTreeBuilder::Inline::Inline(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::Inline::Attach(RenderInline &parent, RenderPtr<RenderObject> child,
                                         RenderObject *beforeChild) noexcept
  {
    // FIXME: Children moved into the enclosing fragmented flow, once there are fragmented flows.
    AttachIgnoringContinuation(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::Inline::AttachIgnoringContinuation(RenderInline &parent,
                                                             RenderPtr<RenderObject> child,
                                                             RenderObject *beforeChild) noexcept
  {
    // Make sure we don't append things after :after-generated content if we have it.
    if (!beforeChild && RenderElement::IsAfterContent(parent.LastChild()))
    {
      beforeChild = parent.LastChild();
    }

    // In blocks-in-inline case we allow blocks that may need splitting.
    if (beforeChild && beforeChild->Parent() != &parent)
    {
      beforeChild = _builder.SplitAnonymousBoxesAroundChild(parent, *beforeChild);
    }

    // FIXME: The child's layout bits, once there's layout.
    _builder.AttachToRenderElement(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::Inline::ChildBecameNonInline(RenderInline &, RenderElement &) noexcept
  {
    // Blocks stay in inline boxes: there's nothing to split without continuations.
  }
}
