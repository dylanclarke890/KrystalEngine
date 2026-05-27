#include "Krystal.HTML/Ranges/StaticRange.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/DocumentType.hpp"
#include "Krystal.HTML/Node/Element.hpp"
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