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
#include "Krystal.Booey/Rendering/RenderFlexibleBox.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
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

    UpdateRebuildRoots();

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

    GetGeneratedContent().UpdateRemainingQuotes();
    GetGeneratedContent().UpdateCounters();

    _builder->UpdateAfterDescendants(GetRenderView());

    _styleUpdate = nullptr;
  }

  void RenderTreeUpdater::UpdateRebuildRoots() noexcept
  {
    auto findNewRebuildRoot = [&](dom::Element &root) -> dom::Element *
    {
      auto *renderingAncestor = FindRenderingAncestor(root);
      if (!renderingAncestor)
      {
        return nullptr;
      }
      auto *rootRenderer = root.Renderer();
      // FIXME: Renderers inside a continuation, once there are continuations.
      auto isInsideAnonymousFlexItemWithSiblings = [&]
      {
        if (!Is<RenderFlexibleBox>(renderingAncestor->Renderer()))
        {
          return false;
        }
        if (!root.PreviousSibling() || !root.PreviousSibling()->Renderer() || !root.NextSibling()
            || !root.NextSibling()->Renderer())
        {
          return false;
        }
        // Direct children of a flex box are supposed to be individual flex items.
        if (auto *parent = root.PreviousSibling()->Renderer()->Parent(); parent && parent->IsAnonymousBlock())
        {
          return true;
        }
        return false;
      };
      auto isBlockInInline = [&]
      {
        if (!Is<RenderInline>(renderingAncestor->Renderer()))
        {
          return false;
        }
        return rootRenderer && rootRenderer->IsInFlow() && !rootRenderer->IsInline();
      };
      if (isInsideAnonymousFlexItemWithSiblings() || isBlockInInline()
          || RenderTreeBuilder::IsRebuildRootForChildren(*renderingAncestor->Renderer()))
      {
        return renderingAncestor;
      }
      return nullptr;
    };

    auto addForRebuild = [&](dom::Element &element)
    {
      auto *existingUpdate = _styleUpdate->GetElementUpdate(element);
      if (existingUpdate)
      {
        if (existingUpdate->Changes.Contains(style::Change::Renderer))
        {
          return false;
        }
        existingUpdate->Changes.Add(style::Change::Renderer);
        return true;
      }

      auto *existingStyle = element.RenderOrDisplayContentsStyle();
      if (!existingStyle)
      {
        return false;
      }

      auto *parent = dom::ComposedTreeAncestors(element).First();
      _styleUpdate->AddElement(
        element, parent,
        style::ElementUpdate {
          CreateUnique<RenderStyle>(RenderStyle::CloneIncludingPseudoElements(*existingStyle)),
          style::Change::Renderer});
      return true;
    };

    auto addSubtreeForRebuild = [&](dom::Element &root)
    {
      if (!addForRebuild(root))
      {
        return;
      }
      auto descendants = dom::ComposedTreeDescendants(root);
      auto it = descendants.begin();
      auto end = descendants.end();
      while (it != end)
      {
        auto *descendant = DynamicDowncast<dom::Element>(*it);
        if (!descendant)
        {
          it.TraverseNext();
          continue;
        }
        if (!addForRebuild(*descendant))
        {
          it.TraverseNextSkippingChildren();
          continue;
        }
        it.TraverseNext();
      }
    };

    while (true)
    {
      auto rebuildRoots = _styleUpdate->TakeRebuildRoots();
      if (rebuildRoots.isEmpty())
      {
        break;
      }
      for (auto &rebuildRoot : rebuildRoots)
      {
        if (auto *newRebuildRoot = findNewRebuildRoot(*rebuildRoot))
        {
          addSubtreeForRebuild(*newRebuildRoot);
        }
      }
    }
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

      if (auto *text = DynamicDowncast<dom::Text>(node))
      {
        auto *textUpdate = _styleUpdate->GetTextUpdate(*text);
        bool didCreateParent =
          GetParent().Update && GetParent().Update->Changes.Contains(style::Change::Renderer);
        bool mayNeedUpdateWhitespaceOnlyRenderer =
          RenderingParent().DidCreateOrDestroyChildRenderer && text->ContainsOnlyASCIIWhitespace();
        if (didCreateParent || textUpdate || mayNeedUpdateWhitespaceOnlyRenderer)
        {
          UpdateTextRenderer(*text, textUpdate, nullptr);
        }

        StorePreviousRenderer(*text);
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
        StorePreviousRenderer(element);
        it.TraverseNextSkippingChildren();
        continue;
      }

      if (elementUpdate)
      {
        UpdateElementRenderer(element, *elementUpdate);
      }

      StorePreviousRenderer(element);

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

    if (auto *before = element.BeforePseudoElement())
    {
      StorePreviousRenderer(*before);
    }
  }

  void RenderTreeUpdater::UpdateAfterDescendants(dom::Element &element,
                                                 const style::ElementUpdate *update) noexcept
  {
    if (update)
    {
      GetGeneratedContent().UpdateBeforeOrAfterPseudoElement(element, *update, PseudoElementType::After);
    }

    auto *renderer = element.Renderer();
    if (!renderer)
    {
      // FIXME: View transitions, once there are any.
      return;
    }

    // FIXME: The ::backdrop renderer, writing suggestions and view transitions, once there are any.

    _builder->UpdateAfterDescendants(*renderer);

    // FIXME: Custom style resolve callbacks (didAttachRenderers), once there are any.
  }

  void RenderTreeUpdater::UpdateRendererStyle(RenderElement &renderer, RenderStyle &&newStyle,
                                              style::DifferenceResult minimalStyleDifference) noexcept
  {
    auto oldStyle = RenderStyle::Clone(renderer.Style());
    renderer.SetStyle(krys::move(newStyle), minimalStyleDifference);
    _builder->NormalizeTreeAfterStyleChange(renderer, oldStyle);
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

  bool RenderTreeUpdater::TextRendererIsNeeded(const dom::Text &textNode) noexcept
  {
    auto &renderingParent = RenderingParent();
    auto &parentRenderer = renderingParent.Position->Parent();
    if (!parentRenderer.CanHaveChildren())
    {
      return false;
    }
    if (parentRenderer.GetElement() && !parentRenderer.GetElement()->ChildShouldCreateRenderer(textNode))
    {
      return false;
    }
    // FIXME: Editing text always gets a renderer, once there's editing.
    if (!textNode.Length())
    {
      return false;
    }
    if (!textNode.ContainsOnlyASCIIWhitespace())
    {
      return true;
    }
    if (Is<RenderText>(renderingParent.PreviousChildRenderer))
    {
      return true;
    }
    // This text node has nothing but white space. We may still need a renderer in some cases.
    if (parentRenderer.IsRenderTable() || parentRenderer.IsRenderTableRow()
        || parentRenderer.IsRenderTableSection() || parentRenderer.IsRenderTableCol()
        || parentRenderer.IsRenderFrameSet() || parentRenderer.IsRenderGrid()
        || (parentRenderer.IsRenderFlexibleBox() && !parentRenderer.IsRenderButton()))
    {
      return false;
    }
    if (parentRenderer.Style().PreserveNewline()) // pre/pre-wrap/pre-line always make renderers.
    {
      return true;
    }

    auto *previousRenderer = renderingParent.PreviousChildRenderer;
    if (previousRenderer && previousRenderer->IsBR()) // <span><br/> <br/></span>
    {
      return false;
    }

    if (parentRenderer.IsRenderInline())
    {
      // <span><div/> <div/></span>
      if (previousRenderer && !previousRenderer->IsInline() && !previousRenderer->IsOutOfFlowPositioned())
      {
        return false;
      }

      return true;
    }

    auto wouldBeFirstInlineContentInsideBlock = [&]
    {
      if (!parentRenderer.IsRenderBlock())
      {
        return false;
      }

      if (!previousRenderer)
      {
        return !parentRenderer.ChildrenInline();
      }

      if (previousRenderer->Parent() == &parentRenderer)
      {
        return !parentRenderer.ChildrenInline() && !previousRenderer->IsInline();
      }

      if (auto *parent = previousRenderer->Parent(); parent->IsAnonymous())
      {
        return !parent->ChildrenInline() && !previousRenderer->IsInline();
      }

      // Can't tell yet.
      return false;
    };
    if (wouldBeFirstInlineContentInsideBlock())
    {
      return false;
    }

    return renderingParent.HasPrecedingInFlowChild;
  }

  void RenderTreeUpdater::CreateTextRenderer(dom::Text &textNode,
                                             const style::TextUpdate *textUpdate) noexcept
  {
    krys_debug_assert(!textNode.Renderer());

    auto &renderTreePosition = GetRenderTreePosition();
    auto textRenderer = textNode.CreateTextRenderer(renderTreePosition.Parent().Style());

    renderTreePosition.ComputeNextSibling(textNode);

    if (!renderTreePosition.Parent().IsChildAllowed(*textRenderer, renderTreePosition.Parent().Style()))
    {
      return;
    }

    textNode.SetRenderer(textRenderer.get());

    if (textUpdate && textUpdate->InheritedDisplayContentsStyle && *textUpdate->InheritedDisplayContentsStyle)
    {
      // Wrap text renderer into anonymous inline so we can give it a style.
      // This is to support "<div style='display:contents;color:green'>text</div>" type cases
      auto newDisplayContentsAnonymousWrapper =
        boo::CreateRenderer<RenderInline>(RenderObject::Type::Inline, textNode.NodeDocument(),
                                          RenderStyle::Clone(**textUpdate->InheritedDisplayContentsStyle));
      newDisplayContentsAnonymousWrapper->InitializeStyle();
      auto &displayContentsAnonymousWrapper = *newDisplayContentsAnonymousWrapper;
      _builder->Attach(renderTreePosition.Parent(), krys::move(newDisplayContentsAnonymousWrapper),
                       renderTreePosition.NextSibling());

      textRenderer->SetInlineWrapperForDisplayContents(&displayContentsAnonymousWrapper);
      _builder->Attach(displayContentsAnonymousWrapper, krys::move(textRenderer));
      return;
    }

    _builder->Attach(renderTreePosition.Parent(), krys::move(textRenderer), renderTreePosition.NextSibling());

    // FIXME: The text manipulation controller and the accessibility cache, once there are any.
  }

  void RenderTreeUpdater::UpdateTextRenderer(dom::Text &text, const style::TextUpdate *textUpdate,
                                             const dom::ContainerNode *root) noexcept
  {
    auto *existingRenderer = text.Renderer();
    bool needsRenderer = TextRendererIsNeeded(text);

    if (existingRenderer && textUpdate && textUpdate->InheritedDisplayContentsStyle)
    {
      if (existingRenderer->InlineWrapperForDisplayContents() || *textUpdate->InheritedDisplayContentsStyle)
      {
        // FIXME: We could update without teardown.
        TearDownTextRenderer(text, root, *_builder);
        existingRenderer = nullptr;
      }
    }

    if (existingRenderer)
    {
      if (needsRenderer)
      {
        if (textUpdate)
        {
          existingRenderer->SetTextWithOffset(text.Data(), textUpdate->Offset);
        }
        return;
      }
      TearDownTextRenderer(text, root, *_builder);
      RenderingParent().DidCreateOrDestroyChildRenderer = true;
      return;
    }
    if (!needsRenderer)
    {
      return;
    }
    CreateTextRenderer(text, textUpdate);
    RenderingParent().DidCreateOrDestroyChildRenderer = true;
  }

  void RenderTreeUpdater::StorePreviousRenderer(dom::Node &node) noexcept
  {
    auto *renderer = node.Renderer();
    if (!renderer)
    {
      return;
    }
    krys_debug_assert(RenderingParent().PreviousChildRenderer != renderer);
    RenderingParent().PreviousChildRenderer = renderer;
    if (renderer->IsInFlow())
    {
      RenderingParent().HasPrecedingInFlowChild = true;
    }
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

  void RenderTreeUpdater::TearDownRenderer(dom::Text &text) noexcept
  {
    auto *view = text.NodeDocument().GetRenderView();
    if (!view)
    {
      return;
    }

    RenderTreeBuilder builder(*view);
    TearDownTextRenderer(text, nullptr, builder);
    InvalidateRebuildRootIfNeeded(text);
  }

  enum class DidRepaintAndMarkContainingBlock : bool
  {
    No,
    Yes
  };
  static Maybe<DidRepaintAndMarkContainingBlock>
    RepaintAndMarkContainingBlockDirtyBeforeTearDown(const dom::Element &root,
                                                     auto composedTreeDescendants) noexcept
  {
    auto *destroyRootRenderer = root.Renderer();
    if (destroyRootRenderer && destroyRootRenderer->RenderTreeBeingDestroyed())
    {
      return null;
    }

    auto markContainingBlockDirty = [&](RenderElement &renderer)
    {
      auto *container = renderer.Container();
      if (!container)
      {
        krys_debug_assert(false);
        renderer.SetNeedsLayout();
        return;
      }
      if (!renderer.IsOutOfFlowPositioned())
      {
        container->SetChildNeedsLayout();
        container->SetNeedsPreferredWidthsUpdate();
        return;
      }
      container->SetNeedsLayoutForOverflowChange();
    };

    // FIXME: Repaint the destroy root and the ::backdrop renderers, once there's painting.
    if (destroyRootRenderer)
    {
      markContainingBlockDirty(*destroyRootRenderer);
    }

    for (auto it = composedTreeDescendants.begin(), end = composedTreeDescendants.end(); it != end;
         it.TraverseNext())
    {
      auto *renderElement = DynamicDowncast<RenderElement>(it->Renderer());
      if (!renderElement)
      {
        continue;
      }
      // FIXME: Clear the selection if the renderer is at one of its ends, and repaint floats and positioned
      // boxes, once there's selection and painting.
      if (renderElement->IsOutOfFlowPositioned())
      {
        // FIXME: Ideally we would check if containing block is the destory root or a descendent of the
        // destroy root.
        markContainingBlockDirty(*renderElement);
      }
    }
    return destroyRootRenderer ? DidRepaintAndMarkContainingBlock::Yes : DidRepaintAndMarkContainingBlock::No;
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

    auto pop = [&](uint32 depth, NeedsRepaintAndLayout needsRepaintAndLayout)
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

        // FIXME: The ::marker's animations, once there are animations.

        if (auto *renderer = element.Renderer())
        {
          if (needsRepaintAndLayout == NeedsRepaintAndLayout::Yes)
          {
            // FIXME: Repaint the renderer, once there's painting.
            if (auto *parent = renderer->Parent())
            {
              parent->SetChildNeedsLayout();
              parent->SetNeedsPreferredWidthsUpdate();
            }
          }
          // FIXME: The ::backdrop renderer, once there's a top layer.
          builder.DestroyAndCleanUpAnonymousWrappers(*renderer, root.Renderer());
          element.SetRenderer(nullptr);
        }

        // FIXME: Custom style resolve callbacks (didDetachRenderers), once there are any.
      }
    };

    push(root);

    auto descendants = dom::ComposedTreeDescendants(root);
    auto didRepaintRoot = RepaintAndMarkContainingBlockDirtyBeforeTearDown(root, descendants);
    auto needsDescendantRepaintAndLayout =
      !didRepaintRoot || *didRepaintRoot == DidRepaintAndMarkContainingBlock::Yes
        ? NeedsRepaintAndLayout::No
        : NeedsRepaintAndLayout::Yes;
    for (auto it = descendants.begin(), end = descendants.end(); it != end; it.TraverseNext())
    {
      pop(it.Depth(), needsDescendantRepaintAndLayout);

      if (auto *text = DynamicDowncast<dom::Text>(*it))
      {
        TearDownTextRenderer(*text, &root, builder, needsDescendantRepaintAndLayout);
        continue;
      }

      push(Downcast<dom::Element>(*it));
    }

    pop(0, needsDescendantRepaintAndLayout);

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
      if (auto *text = DynamicDowncast<dom::Text>(*child))
      {
        TearDownTextRenderer(*text, &element, builder, NeedsRepaintAndLayout::No);
        continue;
      }
      if (auto *childElement = DynamicDowncast<dom::Element>(*child))
      {
        TearDownRenderers(*childElement, TeardownType::Full, builder);
      }
    }
  }

  void RenderTreeUpdater::TearDownTextRenderer(dom::Text &text, const dom::ContainerNode *root,
                                               RenderTreeBuilder &builder,
                                               NeedsRepaintAndLayout needsRepaintAndLayout) noexcept
  {
    auto *renderer = text.Renderer();
    if (!renderer)
    {
      return;
    }
    if (needsRepaintAndLayout == NeedsRepaintAndLayout::Yes)
    {
      // FIXME: Repaint the renderer, once there's painting.
      if (auto *parent = renderer->Parent())
      {
        parent->SetChildNeedsLayout();
        parent->SetNeedsPreferredWidthsUpdate();
      }
    }
    builder.DestroyAndCleanUpAnonymousWrappers(
      *renderer, root ? DynamicDowncast<RenderElement>(root->Renderer()) : nullptr);
    text.SetRenderer(nullptr);
  }
}
