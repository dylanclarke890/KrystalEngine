#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFormControls.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderButton.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderBlock.hpp"

namespace krys::boo
{
  RenderTreeBuilder::FormControls::FormControls(RenderTreeBuilder &builder) noexcept : _builder(builder)
  {
  }

  void RenderTreeBuilder::FormControls::Attach(RenderButton &parent, RenderPtr<RenderObject> child,
                                               RenderObject *beforeChild) noexcept
  {
    _builder.BlockBuilder().Attach(FindOrCreateParentForChild(parent), krys::move(child), beforeChild);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::FormControls::Detach(RenderButton &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (!innerRenderer || &child == innerRenderer || child.Parent() == &parent)
    {
      krys_debug_assert(&child == innerRenderer || !innerRenderer);
      return _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    }
    return _builder.Detach(*innerRenderer, child, willBeDestroyed);
  }

  RenderBlock &RenderTreeBuilder::FormControls::FindOrCreateParentForChild(RenderButton &parent) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (innerRenderer)
    {
      return *innerRenderer;
    }

    auto wrapper = Block::CreateAnonymousBlockWithStyle(parent.GetDocument(), parent.Style());
    innerRenderer = wrapper.get();
    _builder.BlockBuilder().Attach(parent, krys::move(wrapper), nullptr);
    parent.SetInnerRenderer(*innerRenderer);
    return *innerRenderer;
  }
}
