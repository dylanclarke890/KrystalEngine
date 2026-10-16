#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/Style/Update.hpp"

namespace krys::boo
{
  RenderTreeUpdater::Parent::Parent(dom::ContainerNode &) noexcept
  {
  }

  RenderTreeUpdater::Parent::Parent(dom::Element &element, const style::ElementUpdate *update) noexcept
      : Element(&element), Update(update)
  {
  }

  RenderTreeUpdater::RenderTreeUpdater(dom::Document &document,
                                       style::PostResolutionCallbackDisabler &) noexcept
      : _document(ShareRef(document))
  {
  }

  RenderTreeUpdater::~RenderTreeUpdater() noexcept = default;

  // FIXME: The document always has a renderer (its render view); elements have one if they keep a renderer
  // style.
  static bool HasRenderer(const dom::Node &node) noexcept
  {
    if (Is<dom::Document>(node))
    {
      return true;
    }
    return node.GetRenderStyle();
  }

  static dom::Element *FindRenderingAncestor(dom::Node &node) noexcept
  {
    for (auto &ancestor : dom::ComposedTreeAncestors(node))
    {
      if (HasRenderer(ancestor))
      {
        return &ancestor;
      }
      if (!ancestor.HasDisplayContents())
      {
        return nullptr;
      }
    }
    return nullptr;
  }

  static dom::ContainerNode *FindRenderingRoot(dom::ContainerNode &node) noexcept
  {
    if (HasRenderer(node))
    {
      return &node;
    }
    return FindRenderingAncestor(node);
  }

  void RenderTreeUpdater::Commit(UniquePtr<style::Update> styleUpdate) noexcept
  {
    krys_debug_assert(_document.get() == &styleUpdate->Document());

    _styleUpdate = krys::move(styleUpdate);

    // FIXME: Rebuild roots (updateRebuildRoots: continuations, anonymous flex items, blocks in inlines), once
    // there's a render tree.
    (void)_styleUpdate->TakeRebuildRoots();

    UpdateRenderViewStyle();

    for (auto &root : _styleUpdate->Roots())
    {
      if (&root->NodeDocument() != _document.get())
      {
        continue;
      }
      auto *renderingRoot = FindRenderingRoot(*root);
      if (!renderingRoot)
      {
        continue;
      }
      UpdateRenderTree(*renderingRoot);
    }

    // FIXME: Quotes and counters (generatedContent().updateRemainingQuotes/updateCounters), once there's
    // generated content.

    _styleUpdate = nullptr;
  }

  void RenderTreeUpdater::UpdateRenderTree(dom::ContainerNode &root) noexcept
  {
    krys_debug_assert(HasRenderer(root));
    krys_debug_assert(_parentStack.empty());

    _parentStack.push_back(Parent(root));

    auto descendants = dom::ComposedTreeDescendants(root);
    auto it = descendants.begin();
    auto end = descendants.end();

    while (it != end)
    {
      PopParentsToDepth(it.Depth());

      auto &node = *it;

      if (Is<dom::Text>(node))
      {
        // FIXME: Text renderers (updateTextRenderer), once there's a render tree.
        it.TraverseNextSkippingChildren();
        continue;
      }

      auto &element = Downcast<dom::Element>(node);

      auto *elementUpdate = _styleUpdate->GetElementUpdate(element);

      // We hop through display: contents elements in findRenderingRoot, so
      // there may be other updates down the tree.
      if (!elementUpdate && !element.HasDisplayContents())
      {
        it.TraverseNextSkippingChildren();
        continue;
      }

      if (elementUpdate)
      {
        UpdateElementRenderer(element, *elementUpdate);
      }

      // FIXME: Skipped content in the top layer, and whether the parent renderer can have children
      // (shouldCreateRenderer), once there's a render tree.
      auto mayHaveRenderedDescendants = HasRenderer(element) || element.HasDisplayContents();

      if (!mayHaveRenderedDescendants)
      {
        it.TraverseNextSkippingChildren();
        continue;
      }

      PushParent(element, elementUpdate);

      it.TraverseNext();
    }

    PopParentsToDepth(0);
  }

  void RenderTreeUpdater::PushParent(dom::Element &element, const style::ElementUpdate *update) noexcept
  {
    _parentStack.push_back(Parent(element, update));

    // FIXME: The ::before pseudo-element (updateBeforeDescendants), once there's generated content.
  }

  void RenderTreeUpdater::PopParent() noexcept
  {
    // FIXME: The ::after and ::backdrop pseudo-elements (updateAfterDescendants), once there's generated
    // content.
    _parentStack.pop_back();
  }

  void RenderTreeUpdater::PopParentsToDepth(uint32 depth) noexcept
  {
    krys_debug_assert(_parentStack.size() >= depth);

    while (_parentStack.size() > depth)
    {
      PopParent();
    }
  }

  void RenderTreeUpdater::UpdateElementRenderer(dom::Element &element,
                                                const style::ElementUpdate &elementUpdate) noexcept
  {
    if (!elementUpdate.Style)
    {
      return;
    }

    auto elementUpdateStyle = RenderStyle::CloneIncludingPseudoElements(*elementUpdate.Style);

    // FIXME: Elements in the top layer whose skipped content changed, once there's a top layer.
    bool shouldTearDownRenderers = elementUpdate.Changes.Contains(style::Change::Renderer)
                                   && (HasRenderer(element) || element.HasDisplayContents());

    if (shouldTearDownRenderers)
    {
      // display:none cancels animations.
      auto teardownType = elementUpdate.Style->Display() == DisplayType::None
                            ? TeardownType::RendererUpdateCancelingAnimations
                            : TeardownType::RendererUpdate;

      TearDownRenderers(element, teardownType);
    }

    bool hasDisplayContents = elementUpdate.Style->Display() == DisplayType::Contents;
    bool hasDisplayNonePreventingRendererCreation =
      elementUpdate.Style->Display() == DisplayType::None && !element.RendererIsNeeded(elementUpdateStyle);
    bool hasDisplayContentsOrNone = hasDisplayContents || hasDisplayNonePreventingRendererCreation;
    if (hasDisplayContentsOrNone)
    {
      element.StoreDisplayContentsOrNoneStyle(CreateUnique<RenderStyle>(krys::move(elementUpdateStyle)));
    }
    else
    {
      element.ClearDisplayContentsOrNoneStyle();
    }

    // FIXME: The last remembered size for contain-intrinsic-size, once there's layout.

    bool shouldCreateNewRenderer = !HasRenderer(element) && !hasDisplayContentsOrNone;
    if (shouldCreateNewRenderer)
    {
      element._rendererStyle = CreateUnique<RenderStyle>(krys::move(elementUpdateStyle));
      return;
    }

    if (!HasRenderer(element))
    {
      return;
    }

    // FIXME: Pseudo-element styles that changed without the element's style changing
    // (pseudoStyleCacheIsInvalid), once there's a render tree.
    if (!elementUpdate.RecompositeLayer && elementUpdate.Changes.IsEmpty())
    {
      return;
    }

    element._rendererStyle = CreateUnique<RenderStyle>(krys::move(elementUpdateStyle));
  }

  void RenderTreeUpdater::UpdateRenderViewStyle() noexcept
  {
    if (_styleUpdate->InitialContainingBlockUpdate())
    {
      _document->_renderViewStyle = RenderStyle::ClonePtr(*_styleUpdate->InitialContainingBlockUpdate());
    }
  }

  void RenderTreeUpdater::TearDownRenderers(dom::Element &root) noexcept
  {
    if (!HasRenderer(root) && !root.HasDisplayContents())
    {
      return;
    }

    TearDownRenderers(root, TeardownType::Full);
    // FIXME: Invalidate the rendering ancestor if it's a rebuild root for its children, once there's a render
    // tree.
  }

  void RenderTreeUpdater::TearDownRenderersForShadowRootInsertion(dom::Element &host) noexcept
  {
    krys_debug_assert(!dom::TreeQueries::GetShadowRoot(host));
    if (!HasRenderer(host) && !host.HasDisplayContents())
    {
      return;
    }
    TearDownRenderers(host, TeardownType::FullAfterSlotOrShadowRootChange);
  }

  void RenderTreeUpdater::TearDownRenderersAfterSlotChange(dom::Element &host) noexcept
  {
    krys_debug_assert(dom::TreeQueries::GetShadowRoot(host));
    if (!HasRenderer(host) && !host.HasDisplayContents())
    {
      return;
    }
    TearDownRenderers(host, TeardownType::FullAfterSlotOrShadowRootChange);
  }

  void RenderTreeUpdater::TearDownRenderers(dom::Element &root, TeardownType teardownType) noexcept
  {
    // FIXME: Animations (cancelling style-originated animations), generated content, repaint and layout
    // invalidation, once there are any.
    auto tearDown = [&](dom::Element &element)
    {
      // Make sure we don't leave any renderers behind in nodes outside the composed tree.
      // See ComposedTreeIterator::ComposedTreeIterator().
      if (Is<html::HTMLSlotElement>(element) || dom::TreeQueries::GetShadowRoot(element))
      {
        TearDownLeftoverChildrenOfComposedTree(element);
      }

      switch (teardownType)
      {
        case TeardownType::FullAfterSlotOrShadowRootChange:
          if (&element != &root)
          {
            element.ClearHoverAndActiveStatusBeforeDetachingRenderer();
          }
          break;
        case TeardownType::Full:                              element.ClearHoverAndActiveStatusBeforeDetachingRenderer(); break;
        case TeardownType::RendererUpdate:
        case TeardownType::RendererUpdateCancelingAnimations: break;
      }

      element._rendererStyle = nullptr;
    };

    tearDown(root);

    for (auto &descendant : dom::ComposedTreeDescendants(root))
    {
      if (auto *element = DynamicDowncast<dom::Element>(descendant))
      {
        tearDown(*element);
      }
    }
  }

  void RenderTreeUpdater::TearDownLeftoverChildrenOfComposedTree(dom::Element &element) noexcept
  {
    for (auto *child = element.FirstChild(); child; child = child->NextSibling())
    {
      if (!HasRenderer(*child))
      {
        continue;
      }
      if (auto *childElement = DynamicDowncast<dom::Element>(*child))
      {
        TearDownRenderers(*childElement, TeardownType::Full);
      }
    }
  }
}
