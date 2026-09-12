#include "Krystal.Booey/DOM/Internals/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.Booey/DOM/NodeList.hpp"

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
