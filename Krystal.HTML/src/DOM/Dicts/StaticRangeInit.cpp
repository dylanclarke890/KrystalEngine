#include "Krystal.HTML/DOM/Dicts/StaticRangeInit.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"

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