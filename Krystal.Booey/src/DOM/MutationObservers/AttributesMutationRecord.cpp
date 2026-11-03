#include "Krystal.Booey/DOM/MutationObservers/AttributesMutationRecord.hpp"
#include "Krystal.Booey/DOM/Collections/NodeList.hpp"

namespace krys::boo::dom
{
  AttributesMutationRecord::AttributesMutationRecord(Ref<Node> &&target, DOMStringAtom attributeName,
                                                     DOMStringAtom attributeNamespace,
                                                     DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), oldValue), _attributeName(attributeName),
        _attributeNamespace(attributeNamespace)
  {
  }
}
