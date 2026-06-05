#include "Krystal.HTML/DOM/Internals/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.HTML/DOM/NodeList.hpp"

namespace Krys::HTML
{
  AttributesMutationRecord::AttributesMutationRecord(Ref<Node> &&target, DOMStringAtom attributeName,
                                                     DOMStringAtom attributeNamespace,
                                                     DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(Krys::Move(target), oldValue), _attributeName(attributeName),
        _attributeNamespace(attributeNamespace)
  {
  }
}
