#include "Krystal.HTML/DOM/StaticRange.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Attr.hpp"
#include "Krystal.HTML/DOM/DocumentType.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTML/HTMLSlotElement.hpp"

namespace Krys::HTML
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