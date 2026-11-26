#include "Krystal.Booey/Dom/Ranges/StaticRange.hpp"
#include "Krystal.Booey/Dom/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/Dom/Nodes/Attr.hpp"
#include "Krystal.Booey/Dom/Nodes/DocumentType.hpp"
#include "Krystal.Booey/Dom/Nodes/Element.hpp"
#include "Krystal.Booey/Dom/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/Html/Elements/HTMLSlotElement.hpp"

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

  bool StaticRange::IsValid() const noexcept
  {
    Node &startContainer = *StartContainer();
    Node &endContainer = *EndContainer();

    // Its start and end are in the same node tree.
    if (!TreeQueries::SameRoot(startContainer, endContainer))
    {
      return false;
    }

    // Its start offset is between 0 and its start node's length, inclusive.
    if (StartOffset() > TreeQueries::Length(startContainer))
    {
      return false;
    }

    // Its end offset is between 0 and its end node's length, inclusive.
    if (EndOffset() > TreeQueries::Length(endContainer))
    {
      return false;
    }

    // Its start is before or equal to its end.
    return !StrongOrder::IsGreaterThan(_start.ComparePositionTo(_end));
  }
}
