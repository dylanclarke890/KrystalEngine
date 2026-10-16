#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/RenderPtr.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"

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
