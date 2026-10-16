#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/DOM/Node.hpp"
#include "Krystal.Booey/Rendering/RenderDescendantIterator.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Core/Utils/SetForScope.hpp"

namespace krys::boo
{
  RenderTreeBuilder *RenderTreeBuilder::s_current = nullptr;

  RenderTreeBuilder::RenderTreeBuilder(RenderView &view) noexcept : _view(view)
  {
    krys_assert(!s_current || &_view != &s_current->_view);
    _previous = s_current;
    s_current = this;
  }

  RenderTreeBuilder::~RenderTreeBuilder() noexcept
  {
    s_current = _previous;
  }

  bool RenderTreeBuilder::IsRebuildRootForChildren(const RenderElement &renderer) noexcept
  {
    // If a (non-anonymous) child is added or removed to a rebuild root then we'll rebuild the full
    // subtree instead of trying to maintain the correct anonymous box structure on per-child basis.
    // This can greatly simplify the code needed to maintain the correct structure.

    auto display = renderer.Style().Display();
    if (display == DisplayType::Ruby || display == DisplayType::RubyBlock)
    {
      return true;
    }

    return false;
  }

  void RenderTreeBuilder::Destroy(RenderObject &renderer,
                                  CanCollapseAnonymousBlock canCollapseAnonymousBlock) noexcept
  {
    krys_debug_assert(renderer.Parent());
    krys_debug_assert(!renderer.BeingDestroyed());

    auto notifyDescendantRenderersBeforeSubtreeTearDownIfApplicable = [&]
    {
      if (renderer.RenderTreeBeingDestroyed())
      {
        return;
      }
      auto *rendererToDelete = DynamicDowncast<RenderElement>(renderer);
      if (!rendererToDelete || !rendererToDelete->FirstChild())
      {
        return;
      }

      for (auto &descendant : DescendantsOfType<RenderObject>(*rendererToDelete))
      {
        descendant.WillBeRemovedFromTree();
      }
    };
    notifyDescendantRenderersBeforeSubtreeTearDownIfApplicable();

    auto toDestroy = Detach(*renderer.Parent(), renderer, WillBeDestroyed::Yes, canCollapseAnonymousBlock);

    // FIXME: The first letter and continuation clean-up, once there are text fragments and continuations.

    auto tearDownSubTreeIfApplicable = [&]
    {
      auto *rendererToDelete = DynamicDowncast<RenderElement>(toDestroy.get());
      if (!rendererToDelete)
      {
        return;
      }

      SetForScope subtreeTearDownType(_tearDownType, TearDownType::SubtreeWithRootAlreadyDetached);
      while (rendererToDelete->FirstChild())
      {
        auto &firstChild = *rendererToDelete->FirstChild();
        if (auto *node = firstChild.GetNode())
        {
          node->SetRenderer(nullptr);
        }
        Destroy(firstChild);
      }
    };
    // FIXME: webkit.org/b/182909.
    tearDownSubTreeIfApplicable();

    // FIXME: Renderers detached during layout wait in the layout context's detached renderer list, once
    // there's layout. Until then `toDestroy` is destroyed here.
  }

  void RenderTreeBuilder::Attach(RenderElement &parent, RenderPtr<RenderObject> child,
                                 RenderObject *beforeChild) noexcept
  {
    // FIXME: Report visually non-empty content to the view, once there's painting.
    krys_debug_assert(!parent.BeingDestroyed());
    krys_debug_assert(child);
    AttachInternal(parent, krys::move(child), beforeChild);
  }

  void RenderTreeBuilder::AttachInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                                         RenderObject *beforeChild) noexcept
  {
    krys_debug_assert(&parent.View() == &_view);

    // FIXME: The display: contents inline wrapper of a text beforeChild, column spanners, multi-column flows,
    // and the per-type builders (SVG, table, ruby, list items, form controls, blocks, inlines), once there
    // are any.

    AttachToRenderElement(parent, krys::move(child), beforeChild);
  }

  RenderPtr<RenderObject> RenderTreeBuilder::Detach(RenderElement &parent, RenderObject &child,
                                                    WillBeDestroyed willBeDestroyed,
                                                    CanCollapseAnonymousBlock) noexcept
  {
    krys_debug_assert(!parent.BeingDestroyed());
    krys_debug_assert(!child.BeingDestroyed());

    // FIXME: The per-type builders (SVG, blocks, form controls, grids), once there are any.

    return DetachFromRenderElement(parent, child, willBeDestroyed);
  }

  void RenderTreeBuilder::AttachToRenderElement(RenderElement &parent, RenderPtr<RenderObject> child,
                                                RenderObject *beforeChild) noexcept
  {
    // FIXME: Anonymous table wrappers for table parts outside a table, once there's a table builder.
    auto &newChild = *child.get();
    AttachToRenderElementInternal(parent, krys::move(child), beforeChild);
    parent.DidAttachChild(newChild, beforeChild);
  }

  void RenderTreeBuilder::AttachToRenderElementInternal(RenderElement &parent, RenderPtr<RenderObject> child,
                                                        RenderObject *beforeChild) noexcept
  {
    // FIXME: Assert layout isn't mutating the render tree, once there's layout.
    krys_debug_assert(parent.CanHaveChildren() || parent.CanHaveGeneratedChildren());
    krys_debug_assert(!child->Parent());

    while (beforeChild && beforeChild->Parent() && beforeChild->Parent() != &parent)
    {
      beforeChild = beforeChild->Parent();
    }

    if (beforeChild && !beforeChild->Parent())
    {
      // Should never let the RenderView be beforeChild (as who is the parent then)
      krys_debug_assert(false);
      beforeChild = nullptr;
    }

    krys_debug_assert(!beforeChild || beforeChild->Parent() == &parent);

    // Take the ownership.
    auto *newChild = parent.AttachRendererInternal(krys::move(child), beforeChild);
    if (parent.RenderTreeBeingDestroyed())
    {
      krys_debug_assert(false);
      return;
    }

    newChild->InsertedIntoTree();

    // FIXME: Line layout invalidation, fragmented flow state, multi-column descendants, list marker numbers,
    // the layout bits of the child and its parent, the accessibility cache and outline-auto ancestors, once
    // there are any.
  }

  RenderPtr<RenderObject> RenderTreeBuilder::DetachFromRenderElement(RenderElement &parent,
                                                                     RenderObject &child,
                                                                     WillBeDestroyed) noexcept
  {
    // FIXME: Assert layout isn't mutating the render tree, once there's layout.
    krys_debug_assert(parent.CanHaveChildren() || parent.CanHaveGeneratedChildren());
    krys_debug_assert(child.Parent() == &parent);

    if (parent.RenderTreeBeingDestroyed() || _tearDownType == TearDownType::SubtreeWithRootAlreadyDetached)
    {
      return parent.DetachRendererInternal(child);
    }

    // FIXME: Resetting the state of renderers that had layout, line layout invalidation and the fragmented
    // flow state, once there's layout.

    child.WillBeRemovedFromTree();
    // WARNING: There should be no code running between willBeRemovedFromTree() and the actual removal below.
    // This is needed to avoid race conditions where willBeRemovedFromTree() would dirty the tree's structure
    // and the code running here would force an untimely rebuilding, leaving |child| dangling.
    auto childToTake = parent.DetachRendererInternal(child);

    // FIXME: The accessibility cache, once there is one.

    return childToTake;
  }

  void RenderTreeBuilder::DestroyAndCleanUpAnonymousWrappers(RenderObject &rendererToDestroy,
                                                             const RenderElement *subtreeDestroyRoot) noexcept
  {
    SetForScope tearDownType(_tearDownType, !subtreeDestroyRoot || &rendererToDestroy == subtreeDestroyRoot
                                              ? TearDownType::Root
                                              : TearDownType::SubtreeWithRootStillAttached);

    // If the tree is destroyed, there is no need for a clean-up phase.
    if (rendererToDestroy.RenderTreeBeingDestroyed())
    {
      Destroy(rendererToDestroy);
      return;
    }

    // FIXME: Destroying the anonymous wrappers left empty, removing floats and out-of-flow boxes from their
    // containing blocks, collapsing anonymous table siblings and removing the anonymous wrappers of inline
    // children, once there are anonymous boxes.
    Destroy(rendererToDestroy);
  }
}
