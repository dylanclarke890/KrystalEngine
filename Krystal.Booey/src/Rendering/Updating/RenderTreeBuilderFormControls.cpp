#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilderFormControls.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderButton.hpp"
#include "Krystal.Booey/Rendering/RenderMenuList.hpp"
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

  void RenderTreeBuilder::FormControls::Attach(RenderMenuList &parent, RenderPtr<RenderObject> child,
                                               RenderObject *beforeChild) noexcept
  {
    auto &newChild = *child.get();
    _builder.BlockBuilder().Attach(FindOrCreateParentForChild(parent), krys::move(child), beforeChild);
    parent.DidAttachChild(newChild, beforeChild);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::FormControls::Detach(RenderMenuList &parent, RenderObject &child,
                                                                  WillBeDestroyed willBeDestroyed) noexcept
  {
    auto *innerRenderer = parent.InnerRenderer();
    if (!innerRenderer || &child == innerRenderer)
    {
      return _builder.BlockBuilder().Detach(parent, child, willBeDestroyed, CanCollapseAnonymousBlock::Yes);
    }
    return _builder.Detach(*innerRenderer, child, willBeDestroyed);
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

  RenderBlock &RenderTreeBuilder::FormControls::FindOrCreateParentForChild(RenderMenuList &parent) noexcept
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
