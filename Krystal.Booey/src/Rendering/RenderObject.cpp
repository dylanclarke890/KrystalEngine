#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/HTML/HTMLBodyElement.hpp"
#include "Krystal.Booey/HTML/HTMLFieldSetElement.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderInline.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

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

  RenderObject::~RenderObject() noexcept = default;

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

  bool RenderObject::IsDocumentElementRenderer() const noexcept
  {
    return GetDocument().DocumentElement().get() == &_node;
  }

  bool RenderObject::IsFieldset() const noexcept
  {
    auto *element = DynamicDowncast<html::HTMLElement>(GetNode());
    return element && Is<html::HTMLFieldSetElement>(*element);
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
  RenderElement *RenderObject::Container() const noexcept
  {
    // FIXME: See https://bugs.webkit.org/show_bug.cgi?id=270977 for RenderLineBreak special treatment.
    if (!IsRenderElement() || IsRenderText() || IsRenderLineBreak())
    {
      return Parent();
    }

    auto &renderElement = static_cast<const RenderElement &>(*this);
    if (dom::IsInTopLayerOrBackdrop(renderElement.Style(), renderElement.GetElement()))
    {
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
      }
      return parent;
    }
    for (; parent && !parent->CanContainFixedPositionObjects(); parent = parent->Parent())
    {
      if (dom::IsInTopLayerOrBackdrop(parent->Style(), parent->GetElement()))
      {
        return &renderElement.View();
      }
    }
    return parent;
  }

  RenderView &RenderObject::View() const noexcept
  {
    return *GetDocument().GetRenderView();
  }

  bool RenderObject::IsRooted() const noexcept
  {
    return IsDescendantOf(&View());
  }

  bool RenderObject::RenderTreeBeingDestroyed() const noexcept
  {
    return GetDocument().RenderTreeBeingDestroyed();
  }

  void RenderObject::WillBeDestroyed() noexcept
  {
    krys_debug_assert(!_parent);

    // FIXME: The accessibility cache, view transitions and rare data, once there are any.

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
    // FIXME: Dirty the line from the changed child in SVG parents with inline children, once there are any.
  }

  void RenderObject::WillBeRemovedFromTree() noexcept
  {
    // FIXME: We should ASSERT(isRooted()) but we have some out-of-order removals which would need to be fixed
    // first.
    // FIXME: Update cached boundaries in SVG renderers, if a child is removed, once there are any.
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

  const RenderStyle &RenderObject::Style() const noexcept
  {
    if (IsRenderText())
    {
      krys_debug_assert(_parent);
      return _parent->Style();
    }
    return static_cast<const RenderElement &>(*this).Style();
  }
}
