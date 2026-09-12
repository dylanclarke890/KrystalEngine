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
    Node &startContainer = *this->StartContainer();
    Node &endContainer = *this->EndContainer();

    if (!TreeQueries::SameRoot(*this->StartContainer(), *this->EndContainer()))
    {
      return false;
    }

    if (StartOffset() > TreeQueries::Length(startContainer))
    {
      return false;
    }

    if (EndOffset() > TreeQueries::Length(endContainer))
    {
      return false;
    }

    if (&startContainer == &endContainer)
    {
      return EndOffset() > StartOffset();
    }

    return !StrongOrder::IsGreaterThan(_start.ComparePositionTo(_end));
  }
}