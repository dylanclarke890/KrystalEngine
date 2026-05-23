#include "Krystal.HTML/Ranges/StaticRange.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ExceptionOr<StaticRangeInit> StaticRangeInit::Create(BoundaryPoint start, BoundaryPoint end) noexcept
  {
    if (IsOneOf<DocumentType, Attr>(*start.Container) || IsOneOf<DocumentType, Attr>(*end.Container))
    {
      return Exception {ExceptionCode::InvalidNodeTypeError};
    }

    return StaticRangeInit {Krys::Move(start), Krys::Move(end)};
  }
}