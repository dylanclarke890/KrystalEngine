#include "Krystal.Booey/DOM/StaticRange.hpp"
#include "Krystal.Booey/DOM/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Attr.hpp"
#include "Krystal.Booey/DOM/DocumentType.hpp"
#include "Krystal.Booey/DOM/Element.hpp"
#include "Krystal.Booey/DOM/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Booey/HTML/HTMLSlotElement.hpp"

namespace krys::boo::dom
{
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