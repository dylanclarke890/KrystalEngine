#include "Krystal.Booey/Rendering/RenderObject.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"

namespace krys::boo
{
  RenderObject::RenderObject(Type type, dom::Node &node, EnumSet<TypeFlag> typeFlags,
                             TypeSpecificFlags typeSpecificFlags) noexcept
      : _node(node), _typeFlags(Is<dom::Document>(node) ? (typeFlags | TypeFlag::IsAnonymous) : typeFlags),
        _type(type), _typeSpecificFlags(typeSpecificFlags)
  {
    krys_debug_assert(!typeFlags.Contains(TypeFlag::IsAnonymous));
    // FIXME: Tell the render view a renderer was created (for the visually non-empty milestone).
  }

  RenderObject::~RenderObject() noexcept = default;

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
