#include "Krystal.Booey/DOM/Dicts/StaticRangeInit.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"

namespace krys::boo::dom
{
  ExceptionOr<StaticRangeInit> StaticRangeInit::Create(BoundaryPoint start, BoundaryPoint end) noexcept
  {
    // new StaticRange(init):
    // 1. If init["startContainer"] or init["endContainer"] is a DocumentType or Attr node, then throw an
    //    "InvalidNodeTypeError" DOMException.
    if (IsOneOf<DocumentType, Attr>(*start.Container) || IsOneOf<DocumentType, Attr>(*end.Container))
    {
      return Exception {ExceptionCode::InvalidNodeTypeError};
    }

    return StaticRangeInit {krys::move(start), krys::move(end)};
  }
}