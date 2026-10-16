#pragma once

#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/TypeCast.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  /// @brief The element generated for an element's `::before` or `::after` pseudo-element. It isn't in the
  /// tree: its host element owns it.
  class PseudoElement final : public Element
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(PseudoElement);

  private:
    WeakPtr<Element> _hostElement;
    PseudoElementType _pseudoElementType;

  public:
    PseudoElement(Element &host, PseudoElementType pseudoElementType) noexcept;
    ~PseudoElement() noexcept override;

    KRYS_NODISCARD static Ref<PseudoElement> Create(Element &host,
                                                    PseudoElementType pseudoElementType) noexcept;

    KRYS_NODISCARD Element *HostElement() const noexcept
    {
      return _hostElement.get();
    }

    void ClearHostElement() noexcept;

    KRYS_NODISCARD PseudoElementType GetPseudoElementType() const noexcept
    {
      return _pseudoElementType;
    }

    KRYS_NODISCARD bool RendererIsNeeded(const RenderStyle &style) noexcept override;
  };

  /// @brief The tag name of pseudo-elements.
  KRYS_NODISCARD const QualifiedName &PseudoElementTagName() noexcept;
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::PseudoElement)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsPseudoElement();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END()
