#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdater.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeAncestorIterator.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeBuilder.hpp"
#include "Krystal.Booey/Rendering/Updating/RenderTreeUpdaterGeneratedContent.hpp"
#include "Krystal.Booey/Style/TreeResolver.hpp"
#include "Krystal.Booey/Style/Update.hpp"

namespace krys::boo
{
  RenderTreeUpdater::Parent::Parent(dom::ContainerNode &root) noexcept
      : Element(DynamicDowncast<dom::Element>(root)),
        Position(RenderTreePosition(Downcast<RenderElement>(*root.Renderer())))
  {
  }

  RenderTreeUpdater::Parent::Parent(dom::Element &element, const style::ElementUpdate *update) noexcept
      : Element(&element), Update(update)
  {
    if (auto *renderer = element.Renderer())
    {
      Position = RenderTreePosition(*renderer);
    }
  }

  RenderTreeUpdater::RenderTreeUpdater(dom::Document &document,
                                       style::PostResolutionCallbackDisabler &) noexcept
      : _document(ShareRef(document)), _generatedContent(CreateUnique<GeneratedContent>(*this))
  {
    if (auto *renderView = document.GetRenderView())
    {
      _builder = CreateUnique<RenderTreeBuilder>(*renderView);
    }
  }

  RenderTreeUpdater::~RenderTreeUpdater() noexcept = default;

  static dom::Element *FindRenderingAncestor(dom::Node &node) noexcept
  {
    for (auto &ancestor : dom::ComposedTreeAncestors(node))
    {
      if (ancestor.Renderer())
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
    if (node.Renderer())
    {
      return &node;
    }
    return FindRenderingAncestor(node);
  }

  void RenderTreeUpdater::Commit(UniquePtr<style::Update> styleUpdate) noexcept
  {
    krys_debug_assert(_document.get() == &styleUpdate->Document());

    if (!_document->GetRenderView())
    {
      // Documents without a render view are still styled; they keep the style the render view would have.
      if (styleUpdate->InitialContainingBlockUpdate())
      {
        _document->_renderViewStyle = RenderStyle::ClonePtr(*styleUpdate->InitialContainingBlockUpdate());
      }
      return;
    }

    _styleUpdate = krys::move(styleUpdate);

    // FIXME: Rebuild roots (updateRebuildRoots: continuations, anonymous flex items, blocks in inlines), once
    // there are anonymous boxes.
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

    // FIXME: Quotes and counters (generatedContent().updateRemainingQuotes/updateCounters) and
    // RenderTreeBuilder's updateAfterDescendants of the render view, once there's generated content.

    _styleUpdate = nullptr;
  }

  static bool ShouldCreateRenderer(const dom::Element &element, const RenderElement &parentRenderer) noexcept
  {
    if (!parentRenderer.CanHaveChildren()
        && !(Is<dom::PseudoElement>(element) && parentRenderer.CanHaveGeneratedChildren()))
    {
      return false;
    }
    if (parentRenderer.GetElement() && !parentRenderer.GetElement()->ChildShouldCreateRenderer(element))
    {
      return false;
    }
    return true;
  }

  void RenderTreeUpdater::UpdateRenderTree(dom::ContainerNode &root) noexcept
  {
    krys_debug_assert(root.Renderer());
    krys_debug_assert(_parentStack.empty());

    _parentStack.push_back(Parent(root));

    auto descendants = dom::ComposedTreeDescendants(root);
    auto it = descendants.begin();
    auto end = descendants.end();

    while (it != end)
    {
      PopParentsToDepth(it.Depth());

      auto &node = *it;

      if (auto *renderer = node.Renderer())
      {
        GetRenderTreePosition().InvalidateNextSibling(*renderer);
      }
      else if (auto *element = DynamicDowncast<dom::Element>(node); element && element->HasDisplayContents())
      {
        GetRenderTreePosition().InvalidateNextSibling();
      }

      if (Is<dom::Text>(node))
      {
        // FIXME: Text renderers (updateTextRenderer) and the previous renderer (storePreviousRenderer), once
        // there are text renderers.
        it.TraverseNextSkippingChildren();
        continue;
      }

      auto &element = Downcast<dom::Element>(node);

      // FIXME: SVG renderer updates, once there are SVG renderers.

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

      // FIXME: Skipped content in the top layer, once there's a top layer.
      auto mayHaveRenderedDescendants = [&]
      {
        if (element.Renderer())
        {
          return true;
        }
        return element.HasDisplayContents()
               && ShouldCreateRenderer(element, GetRenderTreePosition().Parent());
      }();

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

  auto RenderTreeUpdater::RenderingParent() noexcept -> Parent &
  {
    for (auto i = _parentStack.size(); i--;)
    {
      if (_parentStack[i].Position)
      {
        return _parentStack[i];
      }
    }
    krys_debug_assert(false);
    return _parentStack.back();
  }

  RenderTreePosition &RenderTreeUpdater::GetRenderTreePosition() noexcept
  {
    return *RenderingParent().Position;
  }

  void RenderTreeUpdater::PushParent(dom::Element &element, const style::ElementUpdate *update) noexcept
  {
    _parentStack.push_back(Parent(element, update));

    UpdateBeforeDescendants(element, update);
  }

  void RenderTreeUpdater::PopParent() noexcept
  {
    auto &parent = _parentStack.back();
    if (parent.Element)
    {
      UpdateAfterDescendants(*parent.Element, parent.Update);
    }

    if (&parent != &RenderingParent())
    {
      GetRenderTreePosition().InvalidateNextSibling();
    }

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

  void RenderTreeUpdater::UpdateBeforeDescendants(dom::Element &element,
                                                  const style::ElementUpdate *update) noexcept
  {
    if (update)
    {
      GetGeneratedContent().UpdateBeforeOrAfterPseudoElement(element, *update, PseudoElementType::Before);
    }

    // FIXME: The previous renderer (storePreviousRenderer), once there are text renderers.
  }

  void RenderTreeUpdater::UpdateAfterDescendants(dom::Element &element,
                                                 const style::ElementUpdate *update) noexcept
  {
    if (update)
    {
      GetGeneratedContent().UpdateBeforeOrAfterPseudoElement(element, *update, PseudoElementType::After);
    }

    // FIXME: The ::backdrop renderer, writing suggestions, view transitions, RenderTreeBuilder's
    // updateAfterDescendants and didAttachRenderers, once there are any.
  }

  void RenderTreeUpdater::UpdateRendererStyle(RenderElement &renderer, RenderStyle &&newStyle,
                                              style::DifferenceResult minimalStyleDifference) noexcept
  {
    // FIXME: Normalize the tree after the style change (RenderTreeBuilder::normalizeTreeAfterStyleChange),
    // once there are anonymous boxes.
    renderer.SetStyle(krys::move(newStyle), minimalStyleDifference);
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
                                   && (element.Renderer() || element.HasDisplayContents());

    if (shouldTearDownRenderers)
    {
      if (!element.Renderer())
      {
        // We may be tearing down a descendant renderer cached in renderTreePosition.
        GetRenderTreePosition().InvalidateNextSibling();
      }

      // display:none cancels animations.
      // FIXME: Unless display is affected by animations, once there are animations.
      auto teardownType = elementUpdate.Style->Display() == DisplayType::None
                            ? TeardownType::RendererUpdateCancelingAnimations
                            : TeardownType::RendererUpdate;

      TearDownRenderers(element, teardownType, *_builder);

      RenderingParent().DidCreateOrDestroyChildRenderer = true;
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

    // FIXME: Elements in the top layer under skipped content, once there's a top layer.
    bool shouldCreateNewRenderer = !element.Renderer() && !hasDisplayContentsOrNone;
    if (shouldCreateNewRenderer)
    {
      // FIXME: Custom style resolve callbacks (willAttachRenderers), once there are any.
      CreateRenderer(element, krys::move(elementUpdateStyle));

      RenderingParent().DidCreateOrDestroyChildRenderer = true;
      return;
    }

    auto *renderer = element.Renderer();
    if (!renderer)
    {
      return;
    }

    if (elementUpdate.RecompositeLayer)
    {
      UpdateRendererStyle(*renderer, krys::move(elementUpdateStyle),
                          style::DifferenceResult::RecompositeLayer);
      return;
    }

    // FIXME: Pseudo-element styles that changed without the element's style changing
    // (pseudoStyleCacheIsInvalid), once there are generated content renderers.
    if (elementUpdate.Changes.IsEmpty())
    {
      return;
    }

    UpdateRendererStyle(*renderer, krys::move(elementUpdateStyle), style::DifferenceResult::Equal);
  }

  void RenderTreeUpdater::CreateRenderer(dom::Element &element, RenderStyle &&style) noexcept
  {
    auto computeInsertionPosition = [this, &element]() -> RenderTreePosition &
    {
      GetRenderTreePosition().ComputeNextSibling(element);
      return GetRenderTreePosition();
    };

    if (!ShouldCreateRenderer(element, GetRenderTreePosition().Parent()))
    {
      return;
    }

    if (!element.RendererIsNeeded(style))
    {
      return;
    }

    auto &insertionPosition = computeInsertionPosition();
    // FIXME: Pass the insertion position, once an override uses it.
    auto newRenderer = element.CreateElementRenderer(krys::move(style));
    if (!newRenderer)
    {
      return;
    }

    if (!insertionPosition.Parent().IsChildAllowed(*newRenderer, newRenderer->Style()))
    {
      return;
    }

    element.SetRenderer(newRenderer.get());

    newRenderer->InitializeStyle();

    _builder->Attach(insertionPosition.Parent(), krys::move(newRenderer), insertionPosition.NextSibling());

    // FIXME: The text manipulation controller and the accessibility cache, once there are any.
  }

  void RenderTreeUpdater::UpdateRenderViewStyle() noexcept
  {
    if (_styleUpdate->InitialContainingBlockUpdate())
    {
      GetRenderView().SetStyle(RenderStyle::Clone(*_styleUpdate->InitialContainingBlockUpdate()));
    }
  }

  RenderView &RenderTreeUpdater::GetRenderView() noexcept
  {
    return *_document->GetRenderView();
  }

  static void InvalidateRebuildRootIfNeeded(dom::Node &node) noexcept
  {
    auto *ancestor = FindRenderingAncestor(node);
    if (!ancestor)
    {
      return;
    }
    if (!RenderTreeBuilder::IsRebuildRootForChildren(*ancestor->Renderer()))
    {
      return;
    }
    ancestor->InvalidateRenderer();
  }

  void RenderTreeUpdater::TearDownRenderers(dom::Element &root, TeardownType teardownType) noexcept
  {
    if (!root.Renderer() && !root.HasDisplayContents())
    {
      return;
    }
    auto *view = root.NodeDocument().GetRenderView();
    if (!view)
    {
      return;
    }

    RenderTreeBuilder builder(*view);
    TearDownRenderers(root, teardownType, builder);
    InvalidateRebuildRootIfNeeded(root);
  }

  void RenderTreeUpdater::TearDownRenderers(dom::Element &root) noexcept
  {
    TearDownRenderers(root, TeardownType::Full);
  }

  void RenderTreeUpdater::TearDownRenderersForShadowRootInsertion(dom::Element &host) noexcept
  {
    krys_debug_assert(!dom::TreeQueries::GetShadowRoot(host));
    TearDownRenderers(host, TeardownType::FullAfterSlotOrShadowRootChange);
  }

  void RenderTreeUpdater::TearDownRenderersAfterSlotChange(dom::Element &host) noexcept
  {
    krys_debug_assert(dom::TreeQueries::GetShadowRoot(host));
    TearDownRenderers(host, TeardownType::FullAfterSlotOrShadowRootChange);
  }

  void RenderTreeUpdater::TearDownRenderers(dom::Element &root, TeardownType teardownType,
                                            RenderTreeBuilder &builder) noexcept
  {
    List<dom::Element *> teardownStack;

    auto push = [&](dom::Element &element)
    {
      // FIXME: Custom style resolve callbacks (willDetachRenderers), once there are any.
      teardownStack.push_back(&element);
    };

    auto pop = [&](uint32 depth)
    {
      while (teardownStack.size() > depth)
      {
        auto &element = *teardownStack.back();
        teardownStack.pop_back();

        // Make sure we don't leave any renderers behind in nodes outside the composed tree.
        // See ComposedTreeIterator::ComposedTreeIterator().
        if (Is<html::HTMLSlotElement>(element) || dom::TreeQueries::GetShadowRoot(element))
        {
          TearDownLeftoverChildrenOfComposedTree(element, builder);
        }

        // FIXME: Animations (willChangeRenderer, cancelStyleOriginatedAnimations), once there are animations.
        switch (teardownType)
        {
          case TeardownType::FullAfterSlotOrShadowRootChange:
            if (&element != &root)
            {
              element.ClearHoverAndActiveStatusBeforeDetachingRenderer();
            }
            break;
          case TeardownType::Full:                              element.ClearHoverAndActiveStatusBeforeDetachingRenderer(); break;
          case TeardownType::RendererUpdateCancelingAnimations:
          case TeardownType::RendererUpdate:                    break;
        }

        GeneratedContent::RemoveBeforePseudoElement(element, builder);
        GeneratedContent::RemoveAfterPseudoElement(element, builder);

        // FIXME: The ::marker's animations, once there are list markers.

        if (auto *renderer = element.Renderer())
        {
          // FIXME: Repaint and layout invalidation, and the ::backdrop renderer, once there are any.
          builder.DestroyAndCleanUpAnonymousWrappers(*renderer, root.Renderer());
          element.SetRenderer(nullptr);
        }

        // FIXME: Custom style resolve callbacks (didDetachRenderers), once there are any.
      }
    };

    push(root);

    // FIXME: Repaint and mark containing blocks dirty before the teardown, once there's layout.
    auto descendants = dom::ComposedTreeDescendants(root);
    for (auto it = descendants.begin(), end = descendants.end(); it != end; it.TraverseNext())
    {
      pop(it.Depth());

      if (Is<dom::Text>(*it))
      {
        // FIXME: Text renderers (tearDownTextRenderer), once there are any.
        continue;
      }

      push(Downcast<dom::Element>(*it));
    }

    pop(0);

    // FIXME: Leftover pagination renderers, once there's multi-column layout.
  }

  void RenderTreeUpdater::TearDownLeftoverChildrenOfComposedTree(dom::Element &element,
                                                                 RenderTreeBuilder &builder) noexcept
  {
    for (auto *child = element.FirstChild(); child; child = child->NextSibling())
    {
      if (!child->Renderer())
      {
        continue;
      }
      // FIXME: Text renderers (tearDownTextRenderer), once there are any.
      if (auto *childElement = DynamicDowncast<dom::Element>(*child))
      {
        TearDownRenderers(*childElement, TeardownType::Full, builder);
      }
    }
  }
}
