#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/Position.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/SimpleRange.hpp"
#include "Krystal.Booey/DOM/Text.hpp"
#include "Krystal.Booey/Editing/Editing.hpp"
#include "Krystal.Booey/Editing/VisiblePosition.hpp"
#include "Krystal.Booey/Graphics/Controls/ControlPart.hpp"
#include "Krystal.Booey/Graphics/FloatQuad.hpp"
#include "Krystal.Booey/Graphics/IntRect.hpp"
#include "Krystal.Booey/Graphics/Transforms/AffineTransform.hpp"
#include "Krystal.Booey/Graphics/Transforms/TransformState.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/HTML/HTMLHRElement.hpp"
#include "Krystal.Booey/HTML/HTMLLegendElement.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutBox.hpp"
#include "Krystal.Booey/Page/LocalFrameView.hpp"
#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Booey/Rendering/HitTestResult.hpp"
#include "Krystal.Booey/Rendering/HitTestSource.hpp"
#include "Krystal.Booey/Rendering/ReferencedSVGResources.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderBox.hpp"
#include "Krystal.Booey/Rendering/RenderChildIterator.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderGrid.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderLineBreak.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/RenderTextControl.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+ContainmentInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Style/Seam.hpp"

namespace krys::boo
{
  RenderObject::RenderObject(Type type, dom::Node &node, EnumSet<TypeFlag> typeFlags,
                             TypeSpecificFlags typeSpecificFlags) noexcept
      : _node(node), _typeFlags(Is<dom::Document>(node) ? (typeFlags | TypeFlag::IsAnonymous) : typeFlags),
        _type(type), _typeSpecificFlags(typeSpecificFlags)
  {
    krys_debug_assert(!typeFlags.Contains(TypeFlag::IsAnonymous));
    if (auto *renderView = node.NodeDocument().GetRenderView())
    {
      renderView->DidCreateRenderer();
    }
  }

  RenderObject::~RenderObject() noexcept
  {
    ClearLayoutBox();
  }

  layout::Box *RenderObject::LayoutBox() noexcept
  {
    return _layoutBox.get();
  }

  const layout::Box *RenderObject::LayoutBox() const noexcept
  {
    return _layoutBox.get();
  }

  void RenderObject::SetLayoutBox(layout::Box &box) noexcept
  {
    _layoutBox = ShareCheckedPtr(&box);
    _layoutBox->SetRendererForIntegration(this);
  }

  void RenderObject::ClearLayoutBox() noexcept
  {
    if (!_layoutBox)
    {
      return;
    }

    krys_debug_assert(_layoutBox->RendererForIntegration() == this);

    _layoutBox->SetRendererForIntegration(nullptr);
    _layoutBox = nullptr;
  }

  void RenderObjectDeleter::operator()(RenderObject *renderer) const noexcept
  {
    renderer->Destroy();
  }

  bool RenderObject::IsDescendantOf(const RenderObject *ancestor) const noexcept
  {
    for (auto *renderer = this; renderer; renderer = renderer->_parent)
    {
      if (renderer == ancestor)
      {
        return true;
      }
    }
    return false;
  }

  RenderObject *RenderObject::PreviousInFlowSibling() const noexcept
  {
    auto *previousSibling = PreviousSibling();
    while (previousSibling && !previousSibling->IsInFlow())
    {
      previousSibling = previousSibling->PreviousSibling();
    }
    return previousSibling;
  }

  RenderObject *RenderObject::NextInFlowSibling() const noexcept
  {
    auto *nextSibling = NextSibling();
    while (nextSibling && !nextSibling->IsInFlow())
    {
      nextSibling = nextSibling->NextSibling();
    }
    return nextSibling;
  }

  bool RenderObject::UseDarkAppearance() const noexcept
  {
    return DocumentUseDarkAppearance(GetDocument(), &Style());
  }

  style::ColorOptions RenderObject::StyleColorOptions() const noexcept
  {
    return DocumentStyleColorOptions(GetDocument(), &Style());
  }

  bool RenderObject::IsAtomicInlineLevelBox() const noexcept
  {
    return Style().IsDisplayInlineType()
           && !(Style().Display() == DisplayType::Inline && !IsBlockLevelReplacedOrAtomicInline());
  }

  bool RenderObject::IsNonReplacedAtomicInlineLevelBox() const noexcept
  {
    // FIXME: Check if iframe should really behave like non-replaced here.
    // FIXME: Inline iframes, once there are iframe renderers.
    return !IsRenderReplaced() && IsAtomicInlineLevelBox();
  }

  void RenderObject::SetParent(RenderElement *parent) noexcept
  {
    _parent = parent;
  }

  RenderObject *RenderObject::NextInPreOrder() const noexcept
  {
    if (auto *o = FirstChildSlow())
    {
      return o;
    }

    return NextInPreOrderAfterChildren();
  }

  RenderObject *RenderObject::NextInPreOrderAfterChildren() const noexcept
  {
    RenderObject *o = NextSibling();
    if (!o)
    {
      o = Parent();
      while (o && !o->NextSibling())
      {
        o = o->Parent();
      }
      if (o)
      {
        o = o->NextSibling();
      }
    }

    return o;
  }

  RenderObject *RenderObject::NextInPreOrder(const RenderObject *stayWithin) const noexcept
  {
    if (auto *o = FirstChildSlow())
    {
      return o;
    }

    return NextInPreOrderAfterChildren(stayWithin);
  }

  RenderObject *RenderObject::NextInPreOrderAfterChildren(const RenderObject *stayWithin) const noexcept
  {
    if (this == stayWithin)
    {
      return nullptr;
    }

    const RenderObject *current = this;
    RenderObject *next = nullptr;
    while (!(next = current->NextSibling()))
    {
      current = current->Parent();
      if (!current || current == stayWithin)
      {
        return nullptr;
      }
    }
    return next;
  }

  RenderObject *RenderObject::PreviousInPreOrder() const noexcept
  {
    if (auto *o = PreviousSibling())
    {
      while (auto *last = o->LastChildSlow())
      {
        o = last;
      }
      return o;
    }

    return Parent();
  }

  RenderObject *RenderObject::PreviousInPreOrder(const RenderObject *stayWithin) const noexcept
  {
    if (this == stayWithin)
    {
      return nullptr;
    }

    return PreviousInPreOrder();
  }

  RenderObject *RenderObject::ChildAt(uint32 index) const noexcept
  {
    auto *child = FirstChildSlow();
    for (uint32 i = 0; child && i < index; i++)
    {
      child = child->NextSibling();
    }
    return child;
  }

  RenderObject *RenderObject::FirstLeafChild() const noexcept
  {
    auto *r = FirstChildSlow();
    while (r)
    {
      auto *n = r->FirstChildSlow();
      if (!n)
      {
        break;
      }
      r = n;
    }
    return r;
  }

  RenderObject *RenderObject::LastLeafChild() const noexcept
  {
    auto *r = LastChildSlow();
    while (r)
    {
      auto *n = r->LastChildSlow();
      if (!n)
      {
        break;
      }
      r = n;
    }
    return r;
  }

  bool RenderObject::CanHaveGeneratedChildren() const noexcept
  {
    return CanHaveChildren();
  }

  void RenderObject::SetNeedsBoundariesUpdate() noexcept
  {
  }

  void RenderObject::InvalidateCachedBoundaries() noexcept
  {
    for (auto *renderer = this; renderer && renderer->IsSVGRenderer(); renderer = renderer->Parent())
    {
      if (renderer->UsesBoundaryCaching())
      {
        renderer->SetNeedsBoundariesUpdate();
        break;
      }
    }
  }

  bool RenderObject::UsesBoundaryCaching() const noexcept
  {
    return (_typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::Replaced
            && _typeSpecificFlags.ReplacedFlags().Contains(ReplacedFlag::UsesBoundaryCaching))
           || (_typeSpecificFlags.GetKind() == TypeSpecificFlags::Kind::SVGModelObject
               && _typeSpecificFlags.SVGFlags().Contains(SVGModelObjectFlag::UsesBoundaryCaching));
  }

  FloatRect RenderObject::ObjectBoundingBox() const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  FloatRect RenderObject::StrokeBoundingBox() const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  // Returns the smallest rectangle enclosing all of the painted content
  // respecting clipping, masking, filters, opacity, stroke-width and markers
  FloatRect RenderObject::RepaintRectInLocalCoordinates(RepaintRectCalculation) const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  FloatRect RenderObject::DecoratedBoundingBox() const noexcept
  {
    krys_debug_assert(false);
    return {};
  }

  AffineTransform RenderObject::LocalTransform() const noexcept
  {
    return {};
  }

  const AffineTransform &RenderObject::LocalToParentTransform() const noexcept
  {
    return AffineTransform::Identity;
  }

  bool RenderObject::NodeAtFloatPoint(const HitTestRequest &, HitTestResult &, const FloatPoint &,
                                      HitTestAction) noexcept
  {
    krys_debug_assert(false);
    return false;
  }

  bool RenderObject::IsDocumentElementRenderer() const noexcept
  {
    return GetDocument().DocumentElement().get() == &_node;
  }

  bool RenderObject::IsFieldset() const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(GetNode());
    return element && Is<html::HTMLFieldSetElement>(*element);
  }

  RenderBox *RenderObject::EnclosingScrollableContainer() const noexcept
  {
    // Walk up the container chain to find the scrollable container that contains
    // this RenderObject. The important thing here is that `container()` respects
    // the containing block chain for positioned elements. This is important because
    // scrollable overflow does not establish a new containing block for children.
    for (auto *candidate = Container(); candidate; candidate = candidate->Container())
    {
      // Currently the RenderView can look like it has scrollable overflow, but we never
      // want to return this as our container. Instead we should use the root element.
      if (candidate->IsRenderView())
      {
        break;
      }
      if (candidate->HasPotentiallyScrollableOverflow())
      {
        return Downcast<RenderBox>(candidate);
      }
    }

    // If we reach the root, then the root element is the scrolling container.
    return GetDocument().DocumentElement() ? GetDocument().DocumentElement()->GetRenderBox() : nullptr;
  }

  RenderBox &RenderObject::EnclosingBox() const noexcept
  {
    for (const RenderObject *renderer = this; renderer; renderer = renderer->Parent())
    {
      if (auto *box = DynamicDowncast<RenderBox>(*renderer))
      {
        return const_cast<RenderBox &>(*box);
      }
    }
    // The render view is a box, so every renderer in the tree has one.
    krys_assert(false);
    std::unreachable();
  }

  RenderLayer *RenderObject::EnclosingLayer() const noexcept
  {
    for (const RenderObject *renderer = this; renderer; renderer = renderer->Parent())
    {
      if (auto *layerModelObject = DynamicDowncast<RenderLayerModelObject>(*renderer);
          layerModelObject && layerModelObject->HasLayer())
      {
        return layerModelObject->Layer();
      }
    }
    return nullptr;
  }

  bool RenderObject::IsComposited() const noexcept
  {
    return HasLayer() && Downcast<RenderLayerModelObject>(*this).Layer()->IsComposited();
  }

  bool RenderObject::IsSkippedContent() const noexcept
  {
    if (Is<RenderText>(*this))
    {
      return Style().IsSkippedRootOrSkippedContent();
    }

    if (auto *renderBox = DynamicDowncast<RenderBox>(*this); renderBox && renderBox->IsColumnSpanner())
    {
      // Checking if parent is root or part of a skipped tree does not work in cases when the renderer is
      // moved out of its original position (e.g. column spanners).
      auto *element = renderBox->GetElement();
      return renderBox->Style().IsSkippedRootOrSkippedContent()
             && !(element && boo::IsSkippedContentRoot(renderBox->Style(), *element));
    }
    return Parent() && Parent()->Style().IsSkippedRootOrSkippedContent();
  }

  bool RenderObject::IsTransformed() const noexcept
  {
    // FIXME: SVG transforms (hasSVGTransform) come with the layer-based SVG engine.
    return HasTransformRelatedProperty() && Style().AffectsTransform();
  }

  bool RenderObject::HasTransformOrPerspective() const noexcept
  {
    return HasTransformRelatedProperty() && (IsTransformed() || Style().HasPerspective());
  }

  bool RenderObject::IsLegend() const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(GetNode());
    return element && Is<html::HTMLLegendElement>(*element);
  }

  const RenderStyle &RenderObject::FirstLineStyle() const noexcept
  {
    if (IsRenderText())
    {
      return Parent()->FirstLineStyle();
    }
    return Downcast<RenderElement>(*this).FirstLineStyle();
  }

  const RenderStyle &RenderObject::OutlineStyleForRepaint() const noexcept
  {
    return Style();
  }

  bool RenderObject::IsBody() const noexcept
  {
    auto *node = GetNode();
    if (!node)
    {
      return false;
    }
    auto *element = DynamicDowncast<html::HTMLElement>(node);
    return element && Is<html::HTMLBodyElement>(*element);
  }

  bool RenderObject::IsPseudoElement() const noexcept
  {
    return GetNode() && Is<dom::PseudoElement>(*GetNode());
  }

  dom::Node *RenderObject::NonPseudoNode() const noexcept
  {
    return IsPseudoElement() ? nullptr : GetNode();
  }

  dom::Element *RenderObject::NonPseudoElement() const noexcept
  {
    return DynamicDowncast<dom::Element>(NonPseudoNode());
  }

  bool RenderObject::IsSelectionBorder() const noexcept
  {
    HighlightState st = SelectionState();
    return st == HighlightState::Start || st == HighlightState::End || st == HighlightState::Both
           || View().Selection().Start() == this || View().Selection().End() == this;
  }

  void RenderObject::SetSelectionState(HighlightState state) noexcept
  {
    _selectionState = state;
  }

  bool RenderObject::CanUpdateSelectionOnRootLineBoxes() const noexcept
  {
    if (NeedsLayout())
    {
      return false;
    }

    auto *containingBlock = ContainingBlock();
    return containingBlock ? !containingBlock->NeedsLayout() : true;
  }

  bool RenderObject::IsHR() const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(GetNode());
    return element && Is<html::HTMLHRElement>(*element);
  }

#pragma region Editing

  dom::PositionWithAffinity RenderObject::PositionForPoint(const LayoutPoint &, HitTestSource) noexcept
  {
    return CreatePositionWithAffinity(CaretMinOffset(), Affinity::Downstream);
  }

  VisiblePosition RenderObject::VisiblePositionForPoint(const LayoutPoint &point,
                                                        HitTestSource source) noexcept
  {
    auto positionWithAffinity = PositionForPoint(point, source);
    return VisiblePosition(positionWithAffinity.GetPosition(), positionWithAffinity.GetAffinity());
  }

  int32 RenderObject::CaretMinOffset() const noexcept
  {
    return 0;
  }

  int32 RenderObject::CaretMaxOffset() const noexcept
  {
    if (IsBlockLevelReplacedOrAtomicInline())
    {
      return GetNode() ? static_cast<int32>(std::max(1u, GetNode()->CountChildNodes())) : 1;
    }
    if (IsHR())
    {
      return 1;
    }
    return 0;
  }

  int32 RenderObject::PreviousOffset(int32 current) const noexcept
  {
    return current - 1;
  }

  int32 RenderObject::PreviousOffsetForBackwardDeletion(int32 current) const noexcept
  {
    return current - 1;
  }

  int32 RenderObject::NextOffset(int32 current) const noexcept
  {
    return current + 1;
  }

  dom::PositionWithAffinity RenderObject::CreatePositionWithAffinity(int32 offset,
                                                                     Affinity affinity) const noexcept
  {
    // If this is a non-anonymous renderer in an editable area, then it's simple.
    if (auto *node = NonPseudoNode())
    {
      if (!node->HasEditableStyle())
      {
        // If it can be found, we prefer a visually equivalent position that is editable.
        auto position = dom::MakeDeprecatedLegacyPosition(ShareRefPtr(node), static_cast<uint32>(offset));
        auto candidate = position.Downstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
        if (candidate.DeprecatedNode()->HasEditableStyle())
        {
          return dom::PositionWithAffinity(candidate, affinity);
        }
        candidate = position.Upstream(EditingBoundaryCrossingRule::CanCrossEditingBoundary);
        if (candidate.DeprecatedNode()->HasEditableStyle())
        {
          return dom::PositionWithAffinity(candidate, affinity);
        }
      }
      // FIXME: Eliminate legacy editing positions
      return dom::PositionWithAffinity(
        dom::MakeDeprecatedLegacyPosition(ShareRefPtr(node), static_cast<uint32>(offset)), affinity);
    }

    // We don't want to cross the boundary between editable and non-editable
    // regions of the document, but that is either impossible or at least
    // extremely unlikely in any normal case because we stop as soon as we
    // find a single non-anonymous renderer.

    // Find a nearby non-anonymous renderer.
    const RenderObject *child = this;
    while (const RenderObject *parent = child->Parent())
    {
      // Find non-anonymous content after.
      const RenderObject *renderer = child;
      while ((renderer = renderer->NextInPreOrder(parent)))
      {
        if (auto *node = renderer->NonPseudoNode())
        {
          return dom::PositionWithAffinity(FirstPositionInOrBeforeNode(node));
        }
      }

      // Find non-anonymous content before.
      renderer = child;
      while ((renderer = renderer->PreviousInPreOrder()))
      {
        if (renderer == parent)
        {
          break;
        }
        if (auto *node = renderer->NonPseudoNode())
        {
          return dom::PositionWithAffinity(LastPositionInOrAfterNode(node));
        }
      }

      // Use the parent itself unless it too is anonymous.
      if (auto *element = parent->NonPseudoElement())
      {
        return dom::PositionWithAffinity(FirstPositionInOrBeforeNode(element));
      }

      // Repeat at the next level up.
      child = parent;
    }

    // Everything was anonymous. Give up.
    return {};
  }

  dom::PositionWithAffinity
    RenderObject::CreatePositionWithAffinity(const dom::Position &position) const noexcept
  {
    if (position.IsNotNull())
    {
      return dom::PositionWithAffinity(position);
    }

    krys_debug_assert(!GetNode());
    return CreatePositionWithAffinity(0, Affinity::Downstream);
  }

#pragma endregion

  void RenderObject::SetPositionState(PositionType position) noexcept
  {
    krys_debug_assert((position != PositionType::Absolute && position != PositionType::Fixed)
                      || IsRenderBox());
    // This mask maps PositionType::Fixed and PositionType::Absolute to IsOutOfFlowPositioned, saving one bit.
    _positionedState = static_cast<PositionedState>(static_cast<uint8>(position) & 0x3);
  }

  void RenderObject::SetHasVisibleBoxDecorations(bool b) noexcept
  {
    if (!b)
    {
      _boxDecorationState = BoxDecorationState::None;
      return;
    }
    if (HasVisibleBoxDecorations())
    {
      return;
    }
    _boxDecorationState = BoxDecorationState::InvalidObscurationStatus;
  }

  KRYS_NODISCARD static RenderBlock *
    NearestNonAnonymousContainingBlockIncludingSelf(RenderElement *renderer) noexcept
  {
    while (renderer && (!Is<RenderBlock>(*renderer) || renderer->IsAnonymousBlock()))
    {
      renderer = renderer->ContainingBlock();
    }
    return static_cast<RenderBlock *>(renderer);
  }

  RenderBlock *RenderObject::ContainingBlockForPositionType(PositionType positionType,
                                                            const RenderObject &renderer) noexcept
  {
    if (positionType == PositionType::Static || positionType == PositionType::Relative
        || positionType == PositionType::Sticky)
    {
      auto containingBlockForObjectInFlow = [&]
      {
        auto *ancestor = renderer.Parent();
        while (ancestor
               && ((ancestor->IsInline() && !ancestor->IsBlockLevelReplacedOrAtomicInline())
                   || !ancestor->IsRenderBlock()))
        {
          ancestor = ancestor->Parent();
        }
        return static_cast<RenderBlock *>(ancestor);
      };
      return containingBlockForObjectInFlow();
    }

    if (positionType == PositionType::Absolute)
    {
      auto containingBlockForAbsolutePosition = [&]
      {
        if (auto *renderInline = DynamicDowncast<RenderInline>(renderer);
            renderInline && renderInline->Style().GetPosition() == PositionType::Relative)
        {
          // A relatively positioned RenderInline forwards its absolute positioned descendants to
          // its nearest non-anonymous containing block (to avoid having positioned objects list in
          // RenderInlines).
          return NearestNonAnonymousContainingBlockIncludingSelf(renderer.Parent());
        }
        auto *ancestor = renderer.Parent();
        while (ancestor && !ancestor->CanContainAbsolutelyPositionedObjects())
        {
          ancestor = ancestor->Parent();
        }
        // Make sure we only return non-anonymous RenderBlock as containing block.
        return NearestNonAnonymousContainingBlockIncludingSelf(ancestor);
      };
      return containingBlockForAbsolutePosition();
    }

    if (positionType == PositionType::Fixed)
    {
      auto containingBlockForFixedPosition = [&]() -> RenderBlock *
      {
        auto *ancestor = renderer.Parent();
        while (ancestor && !ancestor->CanContainFixedPositionObjects())
        {
          if (dom::IsInTopLayerOrBackdrop(ancestor->Style(), ancestor->GetElement()))
          {
            return &renderer.View();
          }
          ancestor = ancestor->Parent();
        }
        return NearestNonAnonymousContainingBlockIncludingSelf(ancestor);
      };
      return containingBlockForFixedPosition();
    }

    krys_debug_assert(false);
    return nullptr;
  }

  RenderBlock *RenderObject::ContainingBlock() const noexcept
  {
    // FIXME: See https://bugs.webkit.org/show_bug.cgi?id=270977 for RenderLineBreak special treatment.
    if (IsRenderText() || IsRenderLineBreak())
    {
      return ContainingBlockForPositionType(PositionType::Static, *this);
    }

    auto containingBlockForRenderer = [](const RenderElement &renderer) -> RenderBlock *
    {
      if (dom::IsInTopLayerOrBackdrop(renderer.Style(), renderer.GetElement()))
      {
        return &renderer.View();
      }
      return ContainingBlockForPositionType(renderer.Style().GetPosition(), renderer);
    };

    // FIXME: Scrollbar parts, which have no parent, once there are any.
    return containingBlockForRenderer(static_cast<const RenderElement &>(*this));
  }

  // This method is extremely similar to containingBlock(), but with a few notable
  // exceptions.
  // (1) For normal flow elements, it just returns the parent.
  // (2) For absolute positioned elements, it will return a relative positioned inline, while
  // containingBlock() skips to the non-anonymous containing block.
  // This does mean that computeOutOfFlowPositionedLogicalWidth and computeOutOfFlowPositionedLogicalHeight
  // have to use container().
  // FIXME: The repaint container variant (container(repaintContainer, repaintContainerSkipped)), once there's
  // painting.
  namespace
  {
    RenderElement *ContainerForElement(const RenderObject &renderer,
                                       const RenderLayerModelObject *repaintContainer,
                                       bool *repaintContainerSkipped) noexcept
    {
      // This method is extremely similar to ContainingBlock(), but with a few notable
      // exceptions.
      // (1) For normal flow elements, it just returns the parent.
      // (2) For absolute positioned elements, it will return a relative positioned inline, while
      // ContainingBlock() skips to the non-anonymous containing block.
      // This does mean that ComputeOutOfFlowPositionedLogicalWidth and
      // ComputeOutOfFlowPositionedLogicalHeight have to use Container().
      // FIXME: See https://bugs.webkit.org/show_bug.cgi?id=270977 for RenderLineBreak special treatment.
      if (!renderer.IsRenderElement() || renderer.IsRenderText() || renderer.IsRenderLineBreak())
      {
        return renderer.Parent();
      }

      auto &renderElement = static_cast<const RenderElement &>(renderer);
      auto updateRepaintContainerSkippedFlagIfApplicable = [&]
      {
        if (!repaintContainerSkipped)
        {
          return;
        }
        *repaintContainerSkipped = false;
        if (repaintContainer == &renderElement.View())
        {
          return;
        }
        for (auto *ancestor = renderElement.Parent(); ancestor; ancestor = ancestor->Parent())
        {
          if (repaintContainer == ancestor)
          {
            *repaintContainerSkipped = true;
            break;
          }
        }
      };
      if (dom::IsInTopLayerOrBackdrop(renderElement.Style(), renderElement.GetElement()))
      {
        updateRepaintContainerSkippedFlagIfApplicable();
        return &renderElement.View();
      }
      auto position = renderElement.Style().GetPosition();
      if (position == PositionType::Static || position == PositionType::Relative
          || position == PositionType::Sticky)
      {
        return renderElement.Parent();
      }
      auto *parent = renderElement.Parent();
      if (position == PositionType::Absolute)
      {
        for (; parent && !parent->CanContainAbsolutelyPositionedObjects(); parent = parent->Parent())
        {
          if (repaintContainerSkipped && repaintContainer == parent)
          {
            *repaintContainerSkipped = true;
          }
        }
        return parent;
      }
      for (; parent && !parent->CanContainFixedPositionObjects(); parent = parent->Parent())
      {
        if (dom::IsInTopLayerOrBackdrop(parent->Style(), parent->GetElement()))
        {
          updateRepaintContainerSkippedFlagIfApplicable();
          return &renderElement.View();
        }
        if (repaintContainerSkipped && repaintContainer == parent)
        {
          *repaintContainerSkipped = true;
        }
      }
      return parent;
    }
  }

  RenderElement *RenderObject::Container() const noexcept
  {
    return ContainerForElement(*this, nullptr, nullptr);
  }

  RenderElement *RenderObject::Container(const RenderLayerModelObject *repaintContainer,
                                         bool &repaintContainerSkipped) const noexcept
  {
    repaintContainerSkipped = false;
    return ContainerForElement(*this, repaintContainer, &repaintContainerSkipped);
  }

  FloatPoint RenderObject::LocalToAbsolute(const FloatPoint &localPoint, EnumSet<MapCoordinatesMode> mode,
                                           bool *wasFixed) const noexcept
  {
    TransformState transformState(TransformState::TransformDirection::ApplyTransform, localPoint);
    mode.Add(MapCoordinatesMode::ApplyContainerFlip);
    MapLocalToContainer(nullptr, transformState, mode, wasFixed);

    return transformState.MappedPoint();
  }

  FloatPoint RenderObject::LocalToAbsolute() const noexcept
  {
    return LocalToAbsolute(FloatPoint());
  }

  // FIXME: Should this exist? It is lossy but not obvious when called.
  FloatPoint RenderObject::AbsoluteToLocal(const FloatPoint &containerPoint,
                                           EnumSet<MapCoordinatesMode> mode) const noexcept
  {
    TransformState transformState(TransformState::TransformDirection::UnapplyInverseTransform,
                                  containerPoint);
    MapAbsoluteToLocalPoint(mode, transformState);

    return transformState.MappedPoint();
  }

  FloatQuad RenderObject::AbsoluteToLocalQuad(const FloatQuad &quad,
                                              EnumSet<MapCoordinatesMode> mode) const noexcept
  {
    TransformState transformState(TransformState::TransformDirection::UnapplyInverseTransform,
                                  quad.BoundingBox().Center(), quad);
    MapAbsoluteToLocalPoint(mode, transformState);
    return transformState.MappedQuad();
  }

  void RenderObject::MapLocalToContainer(const RenderLayerModelObject *ancestorContainer,
                                         TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                         bool *wasFixed) const noexcept
  {
    if (ancestorContainer == this)
    {
      return;
    }

    auto *parent = Parent();
    if (!parent)
    {
      return;
    }

    // FIXME: this should call OffsetFromContainer to share code, but I'm not sure it's ever called.
    auto centerPoint = LayoutPoint(transformState.MappedPoint());
    if (auto *parentAsBox = DynamicDowncast<RenderBox>(*parent))
    {
      if (mode.Contains(MapCoordinatesMode::ApplyContainerFlip))
      {
        if (parentAsBox->GetWritingMode().IsBlockFlipped())
        {
          transformState.Move(parentAsBox->FlipForWritingMode(LayoutPoint(transformState.MappedPoint()))
                              - centerPoint);
        }
        mode.Remove(MapCoordinatesMode::ApplyContainerFlip);
      }
      transformState.Move(-ToLayoutSize(LayoutPoint(parentAsBox->GetScrollPosition())));
    }

    parent->MapLocalToContainer(ancestorContainer, transformState, mode, wasFixed);
  }

  void RenderObject::MapAbsoluteToLocalPoint(EnumSet<MapCoordinatesMode> mode,
                                             TransformState &transformState) const noexcept
  {
    if (auto *parent = Parent())
    {
      parent->MapAbsoluteToLocalPoint(mode, transformState);
      if (auto *box = DynamicDowncast<RenderBox>(*parent))
      {
        transformState.Move(ToLayoutSize(LayoutPoint(box->GetScrollPosition())));
      }
    }
  }

  bool RenderObject::ShouldUseTransformFromContainer(const RenderElement *containerObject) const noexcept
  {
    if (IsTransformed())
    {
      return true;
    }
    if (HasLayer() && Downcast<RenderLayerModelObject>(*this).Layer()->AnchorScrollAdjustment())
    {
      return true;
    }
    if (containerObject && containerObject->Style().HasPerspective())
    {
      return containerObject == Parent();
    }
    return false;
  }

  // FIXME: Now that it's no longer passed a container maybe this should be renamed?
  void RenderObject::GetTransformFromContainer(const LayoutSize &offsetInContainer,
                                               TransformationMatrix &transform) const noexcept
  {
    transform.MakeIdentity();
    transform.Translate(offsetInContainer.Width().ToDouble(), offsetInContainer.Height().ToDouble());
    RenderLayer *layer = nullptr;
    if (HasLayer() && (layer = static_cast<const RenderLayerModelObject &>(*this).Layer())
        && layer->Transform())
    {
      transform *= layer->CurrentTransform();
    }

    auto *perspectiveObject = Parent();

    if (perspectiveObject && perspectiveObject->HasLayer() && perspectiveObject->Style().HasPerspective())
    {
      // Perpsective on the container affects us, so we have to factor it in here.
      auto perspectiveOrigin =
        static_cast<const RenderLayerModelObject &>(*perspectiveObject).Layer()->PerspectiveOrigin();

      TransformationMatrix perspectiveMatrix;
      perspectiveMatrix.ApplyPerspective(perspectiveObject->Style().UsedPerspective());

      transform.TranslateRight3d(-perspectiveOrigin.X(), -perspectiveOrigin.Y(), 0);
      transform = perspectiveMatrix * transform;
      transform.TranslateRight3d(perspectiveOrigin.X(), perspectiveOrigin.Y(), 0);
    }
  }

  void RenderObject::PushOntoTransformState(TransformState &transformState, EnumSet<MapCoordinatesMode> mode,
                                            const RenderLayerModelObject *repaintContainer,
                                            const RenderElement *container,
                                            const LayoutSize &offsetInContainer,
                                            bool containerSkipped) const noexcept
  {
    bool preserve3D = mode.Contains(MapCoordinatesMode::UseTransforms) && ParticipatesInPreserve3D();
    auto accumulation = preserve3D ? TransformState::TransformAccumulation::AccumulateTransform
                                   : TransformState::TransformAccumulation::FlattenTransform;
    if (mode.Contains(MapCoordinatesMode::UseTransforms) && ShouldUseTransformFromContainer(container))
    {
      TransformationMatrix matrix;
      GetTransformFromContainer(offsetInContainer, matrix);
      transformState.ApplyTransform(matrix, accumulation);
    }
    else
    {
      transformState.Move(offsetInContainer.Width(), offsetInContainer.Height(), accumulation);
    }

    if (containerSkipped)
    {
      // There can't be a transform between repaintContainer and container, because transforms create
      // containers, so it should be safe to just subtract the delta between the repaintContainer and
      // container.
      auto containerOffset = repaintContainer->OffsetFromAncestorContainer(*container);
      transformState.Move(-containerOffset.Width(), -containerOffset.Height(), accumulation);
    }
  }

  FloatQuad RenderObject::LocalToAbsoluteQuad(const FloatQuad &quad, EnumSet<MapCoordinatesMode> mode,
                                              bool *wasFixed) const noexcept
  {
    return LocalToContainerQuad(quad, nullptr, mode, wasFixed);
  }

  FloatQuad RenderObject::LocalToContainerQuad(const FloatQuad &localQuad,
                                               const RenderLayerModelObject *container,
                                               EnumSet<MapCoordinatesMode> mode,
                                               bool *wasFixed) const noexcept
  {
    // Track the point at the center of the quad's bounding box. As MapLocalToContainer() calls
    // OffsetFromContainer(), it will use that point as the reference point to decide which column's transform
    // to apply in multiple-column blocks.
    TransformState transformState(TransformState::TransformDirection::ApplyTransform,
                                  localQuad.BoundingBox().Center(), localQuad);
    mode.Add(MapCoordinatesMode::ApplyContainerFlip);
    MapLocalToContainer(container, transformState, mode, wasFixed);

    return transformState.MappedQuad();
  }

  FloatPoint RenderObject::LocalToContainerPoint(const FloatPoint &localPoint,
                                                 const RenderLayerModelObject *container,
                                                 EnumSet<MapCoordinatesMode> mode,
                                                 bool *wasFixed) const noexcept
  {
    TransformState transformState(TransformState::TransformDirection::ApplyTransform, localPoint);
    mode.Add(MapCoordinatesMode::ApplyContainerFlip);
    MapLocalToContainer(container, transformState, mode, wasFixed);

    return transformState.MappedPoint();
  }

  LayoutSize RenderObject::OffsetFromContainer(KRYS_MAYBE_UNUSED const RenderElement &container,
                                               const LayoutPoint &, bool *offsetDependsOnPoint) const noexcept
  {
    krys_debug_assert(&container == Container());

    LayoutSize offset;
    if (auto *box = DynamicDowncast<RenderBox>(container))
    {
      offset -= ToLayoutSize(LayoutPoint(box->GetScrollPosition()));
    }

    // FIXME: Fragmented flows make the offset depend on the point, once there are those.
    if (offsetDependsOnPoint)
    {
      *offsetDependsOnPoint = false;
    }

    return offset;
  }

  LayoutSize RenderObject::OffsetFromAncestorContainer(const RenderElement &container) const noexcept
  {
    LayoutSize offset;
    LayoutPoint referencePoint;
    const RenderObject *currentContainer = this;
    do
    {
      auto *nextContainer = currentContainer->Container();
      krys_debug_assert(nextContainer); // This means we reached the top without finding container.
      if (!nextContainer)
      {
        break;
      }
      krys_debug_assert(!currentContainer->IsTransformed());
      auto currentOffset = currentContainer->OffsetFromContainer(*nextContainer, referencePoint);
      offset += currentOffset;
      referencePoint.Move(currentOffset);
      currentContainer = nextContainer;
    } while (currentContainer != &container);

    return offset;
  }

  bool RenderObject::ParticipatesInPreserve3D() const noexcept
  {
    return HasLayer()
           && static_cast<const RenderLayerModelObject &>(*this).Layer()->ParticipatesInPreserve3D();
  }

  List<FloatQuad> RenderObject::AbsoluteTextQuads(const dom::SimpleRange &range,
                                                  EnumSet<BoundingRectBehavior> behavior) noexcept
  {
    List<FloatQuad> quads;
    for (auto &node : dom::IntersectingNodes(range))
    {
      auto *renderer = node.Renderer();
      if (!renderer)
      {
        continue;
      }
      if (auto *lineBreakRenderer = DynamicDowncast<RenderLineBreak>(*renderer);
          lineBreakRenderer && lineBreakRenderer->IsBR())
      {
        lineBreakRenderer->AbsoluteQuads(quads);
      }
      else if (auto *renderText = DynamicDowncast<RenderText>(*renderer))
      {
        auto offsetRange = dom::CharacterDataOffsetRange(range, node);
        auto textQuads = renderText->AbsoluteQuadsForRange(static_cast<uint32>(offsetRange.Start),
                                                           static_cast<uint32>(offsetRange.End), behavior);
        quads.insert(quads.end(), textQuads.begin(), textQuads.end());
      }
    }
    return quads;
  }

  IntRect RenderObject::AbsoluteBoundingBoxRect(bool useTransforms, bool *wasFixed) const noexcept
  {
    if (useTransforms)
    {
      List<FloatQuad> quads;
      AbsoluteQuads(quads, wasFixed);
      return EnclosingIntRect(UnitedBoundingBoxes(quads)).ToRectWithExtentsClippedToNumericLimits();
    }

    auto absPos = LocalToAbsolute(FloatPoint(), {} /* ignore transforms */, wasFixed);
    List<LayoutRect> rects;
    BoundingRects(rects, FlooredLayoutPoint(absPos));

    if (rects.empty())
    {
      return IntRect();
    }

    auto result = UnionRect(rects);
    return SnappedIntRect(result).ToRectWithExtentsClippedToNumericLimits();
  }

  IntRect RenderObject::AbsoluteBoundingBoxRectIgnoringTransforms() const noexcept
  {
    return AbsoluteBoundingBoxRect(false);
  }

  void RenderObject::AddAbsoluteRectForLayer(LayoutRect &result) noexcept
  {
    if (HasLayer())
    {
      result.Unite(LayoutRect(AbsoluteBoundingBoxRectIgnoringTransforms()));
    }

    auto *renderElement = DynamicDowncast<RenderElement>(*this);
    if (!renderElement)
    {
      return;
    }

    for (auto &child : ChildrenOfType<RenderObject>(*renderElement))
    {
      child.AddAbsoluteRectForLayer(result);
    }
  }

  LayoutRect RenderObject::PaintingRootRect(LayoutRect &topLevelRect) noexcept
  {
    LayoutRect result(AbsoluteBoundingBoxRectIgnoringTransforms());
    topLevelRect = result;
    if (auto *renderElement = DynamicDowncast<RenderElement>(*this))
    {
      for (auto &child : ChildrenOfType<RenderObject>(*renderElement))
      {
        child.AddAbsoluteRectForLayer(result);
      }
    }
    return result;
  }

  RenderView &RenderObject::View() const noexcept
  {
    return *GetDocument().GetRenderView();
  }

  LocalFrameViewLayoutContext &RenderObject::LayoutContext() const noexcept
  {
    return View().FrameView().LayoutContext();
  }

  RenderElement *RenderObject::FirstNonAnonymousAncestor() const noexcept
  {
    auto *ancestor = Parent();
    while (ancestor && ancestor->IsAnonymous())
    {
      ancestor = ancestor->Parent();
    }
    return ancestor;
  }

  bool RenderObject::IsRooted() const noexcept
  {
    return IsDescendantOf(&View());
  }

  bool RenderObject::RenderTreeBeingDestroyed() const noexcept
  {
    return GetDocument().RenderTreeBeingDestroyed();
  }

  CachedImageClient &RenderObject::GetCachedImageClient() const noexcept
  {
    if (!_cachedImageClient)
    {
      _cachedImageClient = CreateUnique<CachedImageListener>(*const_cast<RenderObject *>(this));
    }
    return *_cachedImageClient;
  }

  void RenderObject::CachedImageListener::NotifyFinished(CachedResource &resource) noexcept
  {
    _renderer.NotifyFinished(resource);
  }

  void RenderObject::CachedImageListener::ImageChanged(CachedImage *image, const IntRect *rect) noexcept
  {
    _renderer.ImageChanged(static_cast<WrappedImagePtr>(image), rect);
  }

  bool RenderObject::CachedImageListener::AllowsAnimation() const noexcept
  {
    return _renderer.AllowsAnimation();
  }

  bool RenderObject::CachedImageListener::CanDestroyDecodedData() const noexcept
  {
    return _renderer.CanDestroyDecodedData();
  }

  VisibleInViewportState RenderObject::CachedImageListener::ImageFrameAvailable(CachedImage &image,
                                                                                ImageAnimatingState state,
                                                                                const IntRect *rect) noexcept
  {
    return _renderer.ImageFrameAvailable(image, state, rect);
  }

  VisibleInViewportState
    RenderObject::CachedImageListener::ImageVisibleInViewport(const dom::Document &document) const noexcept
  {
    return _renderer.ImageVisibleInViewport(document);
  }

  void RenderObject::CachedImageListener::DidRemoveCachedImageClient(CachedImage &image) noexcept
  {
    _renderer.DidRemoveCachedImageClient(image);
  }

  void RenderObject::CachedImageListener::ImageContentChanged(CachedImage &image) noexcept
  {
    _renderer.ImageContentChanged(image);
  }

  void RenderObject::CachedImageListener::ScheduleRenderingUpdateForImage(CachedImage &image) noexcept
  {
    _renderer.ScheduleRenderingUpdateForImage(image);
  }

  VisibleInViewportState RenderObject::ImageFrameAvailable(CachedImage &image, ImageAnimatingState,
                                                           const IntRect *changeRect) noexcept
  {
    ImageChanged(static_cast<WrappedImagePtr>(&image), changeRect);
    return VisibleInViewportState::No;
  }

#pragma region Repaint

  namespace
  {
    KRYS_NODISCARD bool CanRelyOnAncestorLayerFullRepaint(const RenderObject &rendererToRepaint,
                                                          const RenderLayer &ancestorLayer) noexcept
    {
      auto *renderElement = DynamicDowncast<RenderElement>(rendererToRepaint);
      if (!renderElement || !renderElement->HasSelfPaintingLayer())
      {
        return true;
      }
      return ancestorLayer.Renderer().HasNonVisibleOverflow();
    }

    KRYS_NODISCARD bool FullRepaintIsScheduled(const RenderObject &renderer) noexcept
    {
      // FIXME: Documents in frames, once there are frames.
      if (!renderer.View().UsesCompositing())
      {
        return false;
      }
      for (auto *ancestorLayer = renderer.EnclosingLayer(); ancestorLayer;
           ancestorLayer = ancestorLayer->PaintOrderParent())
      {
        if (ancestorLayer->NeedsFullRepaint())
        {
          return CanRelyOnAncestorLayerFullRepaint(renderer, *ancestorLayer);
        }
      }
      return false;
    }
  }

  auto RenderObject::ContainerForRepaint() const noexcept -> RepaintContainerStatus
  {
    const RenderLayerModelObject *repaintContainer = nullptr;
    auto fullRepaintAlreadyScheduled = false;

    if (View().UsesCompositing())
    {
      if (auto *enclosingLayer = EnclosingLayer())
      {
        auto compLayerStatus = enclosingLayer->EnclosingCompositingLayerForRepaint();
        if (compLayerStatus.Layer)
        {
          repaintContainer = &compLayerStatus.Layer->Renderer();
          fullRepaintAlreadyScheduled = compLayerStatus.FullRepaintAlreadyScheduled
                                        && CanRelyOnAncestorLayerFullRepaint(*this, *compLayerStatus.Layer);
        }
      }
    }

    // FIXME: The enclosing fragmented flow, once there are any. Layers with filters aren't repaint
    // containers: repaints inside them grow to what the filter paints (RepaintUsingContainer).
    return {fullRepaintAlreadyScheduled, repaintContainer};
  }

  void RenderObject::PropagateRepaintToParentWithOutlineAutoIfNeeded(
    const RenderLayerModelObject &repaintContainer, const LayoutRect &repaintRect) const noexcept
  {
    if (!HasOutlineAutoAncestor())
    {
      return;
    }

    // FIXME: We should really propagate only when the child renderer sticks out.
    bool repaintRectNeedsConverting = false;
    // Issue repaint on the renderer with outline: auto.
    // FIXME: Column spanner placeholders, once there are multi-column flows.
    for (const auto *renderer = this; renderer; renderer = renderer->Parent())
    {
      bool rendererHasOutlineAutoAncestor = renderer->HasOutlineAutoAncestor();
      krys_debug_assert(rendererHasOutlineAutoAncestor
                        || renderer->OutlineStyleForRepaint().GetOutlineStyle() == OutlineStyle::Auto);
      if (renderer == &repaintContainer && rendererHasOutlineAutoAncestor)
      {
        repaintRectNeedsConverting = true;
      }
      if (rendererHasOutlineAutoAncestor)
      {
        continue;
      }
      // Issue repaint on the correct repaint container.
      LayoutRect adjustedRepaintRect = repaintRect;
      adjustedRepaintRect.Inflate(LayoutUnit {renderer->OutlineStyleForRepaint().OutlineSize()});
      if (!repaintRectNeedsConverting)
      {
        repaintContainer.RepaintRectangle(adjustedRepaintRect);
      }
      else if (auto *rendererWithOutline = DynamicDowncast<RenderLayerModelObject>(*renderer))
      {
        adjustedRepaintRect =
          LayoutRect(repaintContainer
                       .LocalToContainerQuad(FloatQuad(FloatRect(adjustedRepaintRect)), rendererWithOutline)
                       .BoundingBox());
        rendererWithOutline->RepaintRectangle(adjustedRepaintRect);
      }
      return;
    }
    krys_debug_assert(false);
  }

  void RenderObject::RepaintUsingContainer(const RenderLayerModelObject *repaintContainer,
                                           const LayoutRect &rect, bool shouldClipToLayer) const noexcept
  {
    if (rect.IsEmpty())
    {
      return;
    }

    if (!repaintContainer)
    {
      repaintContainer = &View();
    }

    // FIXME: Fragmented flows repaint in their fragments, once there are any.

    PropagateRepaintToParentWithOutlineAutoIfNeeded(*repaintContainer, rect);

    if (repaintContainer->IsRenderView() && !View().IsComposited())
    {
      krys_debug_assert(repaintContainer == &View());
      // A filter that moves pixels paints from all it filters, so a change inside it repaints all of it.
      auto repaintRect = rect;
      for (auto *layer = EnclosingLayer(); layer; layer = layer->Parent())
      {
        if (layer->FilterMovesPixels())
        {
          repaintRect.Unite(layer->AbsoluteFilterRepaintRect());
        }
      }
      View().RepaintViewRectangle(repaintRect);
      return;
    }

    if (View().UsesCompositing())
    {
      krys_debug_assert(repaintContainer->IsComposited());
      if (auto *layer = repaintContainer->Layer())
      {
        layer->SetBackingNeedsRepaintInRect(rect, shouldClipToLayer
                                                    ? GraphicsLayerShouldClipToLayer::Clip
                                                    : GraphicsLayerShouldClipToLayer::DoNotClip);
      }
    }
  }

  void RenderObject::IssueRepaint(Maybe<LayoutRect> partialRepaintRect,
                                  KRYS_MAYBE_UNUSED ClipRepaintToLayer clipRepaintToLayer,
                                  ForceRepaint forceRepaint,
                                  Maybe<LayoutBoxExtent> additionalRepaintOutsets) const noexcept
  {
    auto repaintContainer = ContainerForRepaint();
    if (!repaintContainer.Renderer)
    {
      repaintContainer = {FullRepaintIsScheduled(*this), &View()};
    }

    if (repaintContainer.FullRepaintIsScheduled && forceRepaint == ForceRepaint::No)
    {
      return;
    }

    LayoutRect repaintRect;

    if (partialRepaintRect)
    {
      repaintRect = ComputeRectForRepaint(*partialRepaintRect, repaintContainer.Renderer);
      if (additionalRepaintOutsets)
      {
        repaintRect.Expand(*additionalRepaintOutsets);
      }
    }
    else
    {
      repaintRect = ClippedOverflowRectForRepaint(repaintContainer.Renderer);
    }

    RepaintUsingContainer(repaintContainer.Renderer, repaintRect,
                          clipRepaintToLayer == ClipRepaintToLayer::Yes);
  }

  void RenderObject::Repaint(ForceRepaint forceRepaint) const noexcept
  {
    // Don't repaint if we're unrooted (note that view() still returns the view when unrooted)
    if (!IsRooted())
    {
      return;
    }

    // FIXME: Nothing repaints while printing, once there's printing.
    IssueRepaint(null, ClipRepaintToLayer::No, forceRepaint);
  }

  void RenderObject::RepaintRectangle(const LayoutRect &repaintRect, bool shouldClipToLayer) const noexcept
  {
    RepaintRectangle(repaintRect, shouldClipToLayer ? ClipRepaintToLayer::Yes : ClipRepaintToLayer::No,
                     ForceRepaint::No);
  }

  void RenderObject::RepaintRectangle(const LayoutRect &repaintRect, ClipRepaintToLayer shouldClipToLayer,
                                      ForceRepaint forceRepaint,
                                      Maybe<LayoutBoxExtent> additionalRepaintOutsets) const noexcept
  {
    if (!IsRooted())
    {
      return;
    }

    // FIXME: layoutDelta needs to be applied in parts before/after transforms and
    // repaint containers. https://bugs.webkit.org/show_bug.cgi?id=23308
    auto dirtyRect = repaintRect;
    dirtyRect.Move(View().FrameView().LayoutContext().LayoutDelta());
    IssueRepaint(dirtyRect, shouldClipToLayer, forceRepaint, additionalRepaintOutsets);
  }

  void RenderObject::RepaintSlowRepaintObject() const noexcept
  {
    if (!IsRooted())
    {
      return;
    }
    // FIXME: Nothing repaints while printing, once there's printing.

    const auto *repaintContainer = ContainerForRepaint().Renderer;

    bool shouldClipToLayer = true;
    IntRect repaintRect;
    // If this is the root background, we need to check if there is an extended background rect. If
    // there is, then we should not allow painting to clip to the layer size.
    if (IsDocumentElementRenderer() || IsBody())
    {
      shouldClipToLayer = !View().FrameView().HasExtendedBackgroundRectForPainting();
      repaintRect = SnappedIntRect(View().BackgroundRect());
    }
    else
    {
      repaintRect = SnappedIntRect(ClippedOverflowRectForRepaint(repaintContainer));
    }

    RepaintUsingContainer(repaintContainer, LayoutRect(repaintRect), shouldClipToLayer);
  }

  LayoutRect RenderObject::AbsoluteClippedOverflowRectForRepaint() const noexcept
  {
    return ClippedOverflowRect(nullptr, VisibleRectContextForRepaint());
  }

  LayoutRect RenderObject::AbsoluteClippedOverflowRectForSpatialNavigation() const noexcept
  {
    return ClippedOverflowRect(nullptr, VisibleRectContextForSpatialNavigation());
  }

  LayoutRect RenderObject::AbsoluteClippedOverflowRectForRenderTreeAsText() const noexcept
  {
    return ClippedOverflowRect(nullptr, VisibleRectContextForRenderTreeAsText());
  }

  IntRect RenderObject::PixelSnappedAbsoluteClippedOverflowRect() const noexcept
  {
    return SnappedIntRect(AbsoluteClippedOverflowRectForRepaint());
  }

  LayoutRect
    RenderObject::ClippedOverflowRectForRepaint(const RenderLayerModelObject *repaintContainer) const noexcept
  {
    return ClippedOverflowRect(repaintContainer, VisibleRectContextForRepaint());
  }

  LayoutRect RenderObject::RectWithOutlineForRepaint(const RenderLayerModelObject *repaintContainer,
                                                     LayoutUnit outlineWidth) const noexcept
  {
    LayoutRect r(ClippedOverflowRectForRepaint(repaintContainer));
    r.Inflate(outlineWidth);
    return r;
  }

  LayoutRect RenderObject::OutlineBoundsForRepaint(const RenderLayerModelObject *) const noexcept
  {
    return {};
  }

  auto RenderObject::LocalRectsForRepaint(RepaintOutlineBounds) const noexcept -> RepaintRects
  {
    krys_debug_assert(false);
    return {};
  }

  auto RenderObject::RectsForRepaintingAfterLayout(const RenderLayerModelObject *repaintContainer,
                                                   RepaintOutlineBounds repaintOutlineBounds) const noexcept
    -> RepaintRects
  {
    auto localRects = LocalRectsForRepaint(repaintOutlineBounds);
    if (localRects.ClippedOverflowRect.IsEmpty())
    {
      return {};
    }

    auto result = ComputeRects(localRects, repaintContainer, VisibleRectContextForRepaint());
    if (result.OutlineBoundsRect)
    {
      result.OutlineBoundsRect =
        LayoutRect(SnapRectToDevicePixels(*result.OutlineBoundsRect, GetDocument().DeviceScaleFactor()));
    }

    return result;
  }

  LayoutRect RenderObject::ClippedOverflowRect(const RenderLayerModelObject *repaintContainer,
                                               VisibleRectContext context) const noexcept
  {
    auto repaintRects = LocalRectsForRepaint(RepaintOutlineBounds::No);
    if (repaintRects.ClippedOverflowRect.IsEmpty())
    {
      return {};
    }

    return ComputeRects(repaintRects, repaintContainer, context).ClippedOverflowRect;
  }

  auto RenderObject::ComputeRects(const RepaintRects &rects, const RenderLayerModelObject *repaintContainer,
                                  VisibleRectContext context) const noexcept -> RepaintRects
  {
    auto result = ComputeVisibleRectsInContainer(rects, repaintContainer, context);
    krys_assert(result.has_value());
    return *result;
  }

  LayoutRect
    RenderObject::ComputeRectForRepaint(const LayoutRect &rect,
                                        const RenderLayerModelObject *repaintContainer) const noexcept
  {
    return ComputeRects(RepaintRects {rect}, repaintContainer, VisibleRectContextForRepaint())
      .ClippedOverflowRect;
  }

  FloatRect
    RenderObject::ComputeFloatRectForRepaint(const FloatRect &rect,
                                             const RenderLayerModelObject *repaintContainer) const noexcept
  {
    auto result = ComputeFloatVisibleRectInContainer(rect, repaintContainer, VisibleRectContextForRepaint());
    krys_assert(result.has_value());
    return *result;
  }

  auto RenderObject::ComputeVisibleRectsInContainer(const RepaintRects &rects,
                                                    const RenderLayerModelObject *container,
                                                    VisibleRectContext context) const noexcept
    -> Maybe<RepaintRects>
  {
    if (container == this)
    {
      return rects;
    }

    auto *parent = Parent();
    if (!parent)
    {
      return rects;
    }

    auto adjustedRects = rects;
    if (parent->HasNonVisibleOverflow())
    {
      bool isEmpty = !Downcast<RenderLayerModelObject>(*parent).ApplyCachedClipAndScrollPosition(
        adjustedRects, container, context);
      if (isEmpty)
      {
        if (context.Options.Contains(VisibleRectContext::Option::UseEdgeInclusiveIntersection))
        {
          return null;
        }
        return adjustedRects;
      }
    }
    return parent->ComputeVisibleRectsInContainer(adjustedRects, container, context);
  }

  Maybe<FloatRect> RenderObject::ComputeFloatVisibleRectInContainer(const FloatRect &,
                                                                    const RenderLayerModelObject *,
                                                                    VisibleRectContext) const noexcept
  {
    krys_debug_assert(false);
    return FloatRect();
  }

  VisibleRectContext RenderObject::VisibleRectContextForRepaint() noexcept
  {
    return {.HasPositionFixedDescendant = false,
            .DirtyRectIsFlipped = false,
            .DescendantNeedsEnclosingIntRect = false,
            .Options = {VisibleRectContext::Option::ApplyContainerClip,
                        VisibleRectContext::Option::ApplyCompositedContainerScrolls}};
  }

  VisibleRectContext RenderObject::VisibleRectContextForSpatialNavigation() noexcept
  {
    return {.HasPositionFixedDescendant = false,
            .DirtyRectIsFlipped = false,
            .DescendantNeedsEnclosingIntRect = false,
            .Options = {VisibleRectContext::Option::ApplyContainerClip,
                        VisibleRectContext::Option::ApplyCompositedContainerScrolls,
                        VisibleRectContext::Option::ApplyCompositedClips}};
  }

  VisibleRectContext RenderObject::VisibleRectContextForRenderTreeAsText() noexcept
  {
    return {.HasPositionFixedDescendant = false,
            .DirtyRectIsFlipped = false,
            .DescendantNeedsEnclosingIntRect = false,
            .Options = {VisibleRectContext::Option::ApplyContainerClip,
                        VisibleRectContext::Option::ApplyCompositedContainerScrolls,
                        VisibleRectContext::Option::ApplyCompositedClips,
                        VisibleRectContext::Option::CalculateAccurateRepaintRect}};
  }

  bool RenderObject::HasSelfPaintingLayer() const noexcept
  {
    if (!HasLayer())
    {
      return false;
    }
    auto *layer = Downcast<RenderLayerModelObject>(*this).Layer();
    return layer && layer->IsSelfPaintingLayer();
  }

  bool RenderObject::HasOutlineAutoAncestor() const noexcept
  {
    return HasRareData() && GetRareData().HasOutlineAutoAncestor;
  }

  void RenderObject::SetHasOutlineAutoAncestor(bool hasOutlineAutoAncestor) noexcept
  {
    if (hasOutlineAutoAncestor || HasRareData())
    {
      EnsureRareData().HasOutlineAutoAncestor = hasOutlineAutoAncestor;
    }
  }

  void RenderObject::SetLayerNeedsFullRepaint() noexcept
  {
    krys_debug_assert(HasLayer());
    Downcast<RenderLayerModelObject>(*this).Layer()->SetRepaintStatus(RepaintStatus::NeedsFullRepaint);
  }

  void RenderObject::SetLayerNeedsFullRepaintForOutOfFlowMovementLayout() noexcept
  {
    krys_debug_assert(HasLayer());
    Downcast<RenderLayerModelObject>(*this).Layer()->SetRepaintStatus(
      RepaintStatus::NeedsFullRepaintForOutOfFlowMovementLayout);
  }

  void RenderObject::RepaintRects::EncloseToIntRects() noexcept
  {
    ClippedOverflowRect = LayoutRect(EnclosingIntRect(ClippedOverflowRect));
    if (OutlineBoundsRect)
    {
      *OutlineBoundsRect = LayoutRect(EnclosingIntRect(*OutlineBoundsRect));
    }
  }

  void RenderObject::RepaintRects::FlipForWritingMode(LayoutSize containerSize,
                                                      bool isHorizontalWritingMode) noexcept
  {
    if (isHorizontalWritingMode)
    {
      ClippedOverflowRect.SetY(containerSize.Height() - ClippedOverflowRect.MaxY());
      if (OutlineBoundsRect)
      {
        OutlineBoundsRect->SetY(containerSize.Height() - OutlineBoundsRect->MaxY());
      }
    }
    else
    {
      ClippedOverflowRect.SetX(containerSize.Width() - ClippedOverflowRect.MaxX());
      if (OutlineBoundsRect)
      {
        OutlineBoundsRect->SetX(containerSize.Width() - OutlineBoundsRect->MaxX());
      }
    }
  }

  void RenderObject::RepaintRects::Transform(const TransformationMatrix &matrix) noexcept
  {
    ClippedOverflowRect = matrix.MapRect(ClippedOverflowRect);
    if (OutlineBoundsRect)
    {
      *OutlineBoundsRect = matrix.MapRect(*OutlineBoundsRect);
    }
  }

  void RenderObject::RepaintRects::Transform(const TransformationMatrix &matrix,
                                             float deviceScaleFactor) noexcept
  {
    bool identicalRects = OutlineBoundsRect && *OutlineBoundsRect == ClippedOverflowRect;
    ClippedOverflowRect =
      LayoutRect(EncloseRectToDevicePixels(matrix.MapRect(ClippedOverflowRect), deviceScaleFactor));
    if (identicalRects)
    {
      *OutlineBoundsRect = ClippedOverflowRect;
    }
    else if (OutlineBoundsRect)
    {
      *OutlineBoundsRect =
        LayoutRect(EncloseRectToDevicePixels(matrix.MapRect(*OutlineBoundsRect), deviceScaleFactor));
    }
  }

#pragma endregion

  void RenderObject::WillBeDestroyed() noexcept
  {
    krys_debug_assert(!_parent);

    // FIXME: The accessibility cache and view transitions, once there are any.

    RemoveRareData();

    if (auto *node = GetNode())
    {
      // FIXME: Continuations should be anonymous.
      krys_debug_assert(!node->Renderer() || node->Renderer() == this);
      if (node->Renderer() == this)
      {
        node->SetRenderer(nullptr);
      }
    }

    View().WillDestroyRenderer();
  }

  void RenderObject::InsertedIntoTree() noexcept
  {
    // FIXME: We should ASSERT(isRooted()) here but generated content makes some out-of-order insertion.
    if (!IsFloating() && Parent()->IsSVGRenderer() && Parent()->ChildrenInline())
    {
      Parent()->DirtyLineFromChangedChild();
    }
  }

  void RenderObject::WillBeRemovedFromTree() noexcept
  {
    // FIXME: We should ASSERT(isRooted()) but we have some out-of-order removals which would need to be fixed
    // first.
    // Update cached boundaries in SVG renderers, if a child is removed.
    Parent()->InvalidateCachedBoundaries();
  }

  void RenderObject::Destroy() noexcept
  {
    krys_assert(!_parent);
    krys_assert(!_next);
    krys_assert(!_previous);
    krys_assert(!_isBeingDestroyed);

    _isBeingDestroyed = true;

    WillBeDestroyed();

    // FIXME: Widget renderers are reference counted, once there are any.
    delete this;
  }

  dom::Document &RenderObject::GetDocument() const noexcept
  {
    return _node.NodeDocument();
  }

  dom::DocumentOrShadowRoot &RenderObject::TreeScopeForSVGReferences() const noexcept
  {
    return _node.TreeScopeForSVGReferences();
  }

  WritingMode RenderObject::GetWritingMode() const noexcept
  {
    return Style().GetWritingMode();
  }

  const RenderStyle &RenderObject::Style() const noexcept
  {
    if (IsRenderText())
    {
      krys_debug_assert(_parent);
      return _parent->Style();
    }
    return static_cast<const RenderElement &>(*this).Style();
  }

#pragma region Layout bits

  static bool IsLayoutBoundary(const RenderElement &renderer) noexcept
  {
    // FIXME: In future it may be possible to broaden these conditions in order to improve performance.
    if (renderer.IsRenderView())
    {
      return true;
    }

    const auto &style = renderer.Style();
    if (auto *textControl = DynamicDowncast<RenderTextControl>(renderer))
    {
      if (!textControl->IsFlexItem() && !textControl->IsGridItem()
          && style.GetFieldSizing() != FieldSizing::Content)
      {
        // Flexing type of layout systems may compute different size than what input's preferred width is
        // which won't happen unless they run their layout as well.
        return true;
      }
    }

    if (renderer.ShouldApplyLayoutContainment() && renderer.ShouldApplySizeContainment())
    {
      return true;
    }

    if (renderer.IsLegacyRenderSVGRoot())
    {
      return true;
    }

    if (!renderer.HasNonVisibleOverflow())
    {
      // While createsNewFormattingContext (a few lines below) covers this case, overflow visible is a super
      // common value so we should be able to bail out here fast.
      return false;
    }

    if (style.Width().IsIntrinsicOrLegacyIntrinsicOrAuto()
        || style.Height().IsIntrinsicOrLegacyIntrinsicOrAuto() || style.Height().IsPercentOrCalculated())
    {
      return false;
    }

    // FIXME: Layer-aware SVG renderers, once there's SVG.

    // Table parts can't be relayout roots since the table is responsible for layouting all the parts.
    if (renderer.IsTablePart())
    {
      return false;
    }

    if (auto *renderBlock = DynamicDowncast<RenderBlock>(renderer);
        !renderBlock || !renderBlock->CreatesNewFormattingContext())
    {
      return false;
    }

    return true;
  }

  void RenderObject::ClearNeedsLayout(HadSkippedLayout hadSkippedLayout) noexcept
  {
    // FIXME: Consider not setting the "ever had layout" bit to true when "hadSkippedLayout"
    SetEverHadLayout();
    SetHadSkippedLayout(hadSkippedLayout == HadSkippedLayout::Yes);

    if (HasLayer())
    {
      Downcast<RenderLayerModelObject>(*this).Layer()->SetSelfAndChildrenNeedPositionUpdate();
    }
    _stateFlags.Remove(StateFlag::NeedsLayout);
    SetOutOfFlowChildNeedsLayoutBit(false);
    SetNeedsSimplifiedNormalFlowLayoutBit(false);
    SetNormalChildNeedsLayoutBit(false);
    SetOutOfFlowChildNeedsStaticPositionLayoutBit(false);
    SetNeedsOutOfFlowMovementLayoutBit(false);
#if KRYS_DEBUG
    auto checkIfOutOfFlowDescendantsNeedLayout = [&](auto &renderBlock)
    {
      if (auto *outOfFlowDescendants = renderBlock.OutOfFlowBoxes())
      {
        for (auto *renderer : *outOfFlowDescendants)
        {
          krys_debug_assert(!renderer->NeedsLayout());
        }
      }
    };
    if (auto *renderBlock = DynamicDowncast<RenderBlock>(*this))
    {
      checkIfOutOfFlowDescendantsNeedLayout(*renderBlock);
    }
#endif
  }

  void RenderObject::ScheduleLayout(RenderElement *layoutRoot) noexcept
  {
    if (auto *renderView = DynamicDowncast<RenderView>(layoutRoot))
    {
      renderView->FrameView().LayoutContext().ScheduleLayout();
      return;
    }

    if (layoutRoot && layoutRoot->IsRooted())
    {
      layoutRoot->View().FrameView().LayoutContext().ScheduleSubtreeLayout(*layoutRoot);
    }
  }

  void RenderObject::SetNeedsLayout(MarkingBehavior markParents) noexcept
  {
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    if (SelfNeedsLayout())
    {
      return;
    }
    _stateFlags.Add(StateFlag::NeedsLayout);
    if (markParents == MarkingBehavior::MarkContainingBlockChain)
    {
      ScheduleLayout(MarkContainingBlocksForLayout());
    }
    if (HasLayer())
    {
      SetLayerNeedsFullRepaint();
    }
  }

  RenderElement *RenderObject::MarkContainingBlocksForLayout(RenderElement *layoutRoot) noexcept
  {
    krys_debug_assert(!IsSetNeedsLayoutForbidden());
    if (Is<RenderView>(*this))
    {
      return Downcast<RenderElement>(this);
    }

    RenderElement *ancestor = Container();

    bool simplifiedNormalFlowLayout =
      NeedsSimplifiedNormalFlowLayout() && !SelfNeedsLayout() && !NormalChildNeedsLayout();
    bool hasOutOfFlowPosition = IsOutOfFlowPositioned();

    while (ancestor)
    {
      // FIXME: Remove this once we remove the special cases for counters, quotes and mathml calling
      // setNeedsLayout during preferred width computation.
      SetLayoutNeededForbiddenScope layoutForbiddenScope(*ancestor, IsSetNeedsLayoutForbidden());

      // Don't mark the outermost object of an unrooted subtree. That object will be
      // marked when the subtree is added to the document.
      RenderElement *container = ancestor->Container();
      if (!container && !ancestor->IsRenderView())
      {
        // Internal render tree shuffle.
        return nullptr;
      }

      if (simplifiedNormalFlowLayout && ancestor->OverflowChangesMayAffectLayout())
      {
        simplifiedNormalFlowLayout = false;
      }

      if (hasOutOfFlowPosition)
      {
        bool willSkipRelativelyPositionedInlines = !ancestor->IsRenderBlock() || ancestor->IsAnonymousBlock();
        // Skip relatively positioned inlines and anonymous blocks to get to the enclosing RenderBlock.
        while (ancestor && (!ancestor->IsRenderBlock() || ancestor->IsAnonymousBlock()))
        {
          ancestor = ancestor->Container();
        }
        if (!ancestor || ancestor->OutOfFlowChildNeedsLayout())
        {
          return nullptr;
        }
        if (willSkipRelativelyPositionedInlines)
        {
          container = ancestor->Container();
        }
        ancestor->SetOutOfFlowChildNeedsLayoutBit(true);
        simplifiedNormalFlowLayout = true;
      }
      else if (simplifiedNormalFlowLayout)
      {
        if (ancestor->NeedsSimplifiedNormalFlowLayout())
        {
          return nullptr;
        }
        ancestor->SetNeedsSimplifiedNormalFlowLayoutBit(true);
      }
      else
      {
        if (ancestor->NormalChildNeedsLayout())
        {
          return nullptr;
        }
        ancestor->SetNormalChildNeedsLayoutBit(true);
      }
      krys_debug_assert(!ancestor->IsSetNeedsLayoutForbidden());

      if (layoutRoot)
      {
        // Having a valid layout root also mean we should not stop at layout boundaries.
        if (ancestor == layoutRoot)
        {
          return layoutRoot;
        }
      }
      else if (IsLayoutBoundary(*ancestor))
      {
        return ancestor;
      }

      if (auto *renderGrid = DynamicDowncast<RenderGrid>(container);
          renderGrid && renderGrid->IsExtrinsicallySized())
      {
        simplifiedNormalFlowLayout = true;
      }

      hasOutOfFlowPosition = ancestor->IsOutOfFlowPositioned();
      ancestor = container;
    }
    return nullptr;
  }

  void RenderObject::SetNeedsPreferredWidthsUpdate(MarkingBehavior markParents) noexcept
  {
    if (NeedsPreferredLogicalWidthsUpdate()
        && (!HasRareData() || !GetRareData().PreferredLogicalWidthsNeedUpdateIsMarkOnlyThis))
    {
      // Both this and our ancestor chain are already marked dirty.
      return;
    }

    _stateFlags.Add(StateFlag::PreferredLogicalWidthsNeedUpdate);
    if (IsOutOfFlowPositioned())
    {
      // A positioned object has no effect on the min/max width of its containing block ever. No need to mark
      // ancestor chain.
      return;
    }

    if (markParents == MarkingBehavior::MarkOnlyThis)
    {
      EnsureRareData().PreferredLogicalWidthsNeedUpdateIsMarkOnlyThis = true;
      return;
    }

    InvalidateContainerPreferredLogicalWidths();
    if (HasRareData())
    {
      EnsureRareData().PreferredLogicalWidthsNeedUpdateIsMarkOnlyThis = false;
    }
  }

  void RenderObject::InvalidateContainerPreferredLogicalWidths() noexcept
  {
    // In order to avoid pathological behavior when inlines are deeply nested, we do include them
    // in the chain that we mark dirty (even though they're kind of irrelevant).
    RenderElement *ancestor = IsRenderTableCell() ? ContainingBlock() : Container();
    while (ancestor)
    {
      if (ancestor->NeedsPreferredLogicalWidthsUpdate()
          && (!ancestor->HasRareData()
              || !ancestor->GetRareData().PreferredLogicalWidthsNeedUpdateIsMarkOnlyThis))
      {
        break;
      }
      // Don't invalidate the outermost object of an unrooted subtree. That object will be
      // invalidated when the subtree is added to the document.
      RenderElement *container =
        ancestor->IsRenderTableCell() ? ancestor->ContainingBlock() : ancestor->Container();
      if (!container && !ancestor->IsRenderView())
      {
        break;
      }

      ancestor->_stateFlags.Add(StateFlag::PreferredLogicalWidthsNeedUpdate);
      if (ancestor->Style().HasOutOfFlowPosition())
      {
        // A positioned object has no effect on the min/max width of its containing block ever.
        // We can optimize this case and not go up any further.
        break;
      }
      ancestor = container;
    }
  }

  auto RenderObject::GetRareDataMap() noexcept -> RareDataMap &
  {
    static RareDataMap map;
    return map;
  }

  auto RenderObject::GetRareData() const noexcept -> const RareData &
  {
    krys_debug_assert(HasRareData());
    return *GetRareDataMap().find(this)->second;
  }

  auto RenderObject::EnsureRareData() noexcept -> RareData &
  {
    _stateFlags.Add(StateFlag::HasRareData);
    auto &data = GetRareDataMap()[this];
    if (!data)
    {
      data = CreateUnique<RareData>();
    }
    return *data;
  }

  void RenderObject::RemoveRareData() noexcept
  {
    GetRareDataMap().erase(this);
    _stateFlags.Remove(StateFlag::HasRareData);
  }

  RenderObject::SetLayoutNeededForbiddenScope::SetLayoutNeededForbiddenScope(const RenderObject &renderObject,
                                                                             bool isForbidden) noexcept
      : _renderObject(renderObject), _preexistingForbidden(renderObject.IsSetNeedsLayoutForbidden())
  {
    _renderObject.SetNeedsLayoutIsForbidden(isForbidden);
  }

  RenderObject::SetLayoutNeededForbiddenScope::~SetLayoutNeededForbiddenScope() noexcept
  {
    _renderObject.SetNeedsLayoutIsForbidden(_preexistingForbidden);
  }

#pragma endregion

  bool RenderObject::HitTest(const HitTestRequest &request, HitTestResult &result,
                             const HitTestLocation &locationInContainer, const LayoutPoint &accumulatedOffset,
                             HitTestFilter hitTestFilter) noexcept
  {
    bool inside = false;
    if (hitTestFilter != HitTestSelf)
    {
      // First test the foreground layer (lines and inlines).
      inside = NodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestForeground);

      // Test floats next.
      if (!inside)
      {
        inside = NodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestFloat);
      }

      // Finally test to see if the mouse is in the background (within a child block's background).
      if (!inside)
      {
        inside =
          NodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestChildBlockBackgrounds);
      }
    }

    // See if the mouse is inside us but not any of our descendants
    if (hitTestFilter != HitTestDescendants && !inside)
    {
      inside = NodeAtPoint(request, result, locationInContainer, accumulatedOffset, HitTestBlockBackground);
    }

    return inside;
  }

  dom::Node *RenderObject::NodeForHitTest() const noexcept
  {
    auto *node = GetNode();
    // If we hit the anonymous renderers inside generated content we should
    // actually hit the generated content so walk up to the PseudoElement.
    if (!node && Parent() && Parent()->IsBeforeOrAfterContent())
    {
      for (auto *renderer = Parent(); renderer && !node; renderer = renderer->Parent())
      {
        node = renderer->GetElement();
      }
    }
    return node;
  }

  void RenderObject::UpdateHitTestResult(HitTestResult &result, const LayoutPoint &point) const noexcept
  {
    if (result.InnerNode())
    {
      return;
    }

    if (auto *node = NodeForHitTest())
    {
      result.SetInnerNode(node);
      if (!result.InnerNonSharedNode())
      {
        result.SetInnerNonSharedNode(node);
      }
      result.SetPseudoElementIdentifier(Style().GetPseudoElementIdentifier());
      result.SetLocalPoint(point);
    }
  }

  bool RenderObject::NodeAtPoint(const HitTestRequest &, HitTestResult &, const HitTestLocation &,
                                 const LayoutPoint &, HitTestAction) noexcept
  {
    return false;
  }
}
