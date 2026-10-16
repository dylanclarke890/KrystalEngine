#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo::dom
{
  const QualifiedName &PseudoElementTagName() noexcept
  {
    static const QualifiedName name {DOMStringAtom::Null(), DOMStringAtom::Null(), u8"<pseudo>"};
    return name;
  }

  PseudoElement::PseudoElement(Element &host, PseudoElementType pseudoElementType) noexcept
      : Element(host.NodeDocument(), PseudoElementTagName(), NodeFlags::IsPseudoElement),
        _hostElement(WeakPtr<Element>::WithRef(host)), _pseudoElementType(pseudoElementType)
  {
    SetNodeFlag(NodeFlags::IsConnected);
    krys_debug_assert(pseudoElementType == PseudoElementType::Before
                      || pseudoElementType == PseudoElementType::After);
  }

  PseudoElement::~PseudoElement() noexcept
  {
    krys_debug_assert(!_hostElement);
  }

  Ref<PseudoElement> PseudoElement::Create(Element &host, PseudoElementType pseudoElementType) noexcept
  {
    return CreateRef<PseudoElement>(host, pseudoElementType);
  }

  void PseudoElement::ClearHostElement() noexcept
  {
    // FIXME: Tell the element's animations it was removed (Styleable::elementWasRemoved), once there are
    // animations.
    _hostElement = nullptr;
  }

  bool PseudoElement::RendererIsNeeded(const RenderStyle &style) noexcept
  {
    // FIXME: Animations that need the pseudo-element (KeyframeEffectStack::requiresPseudoElement), once there
    // are animations.
    return PseudoElementRendererIsNeeded(&style);
  }
}
