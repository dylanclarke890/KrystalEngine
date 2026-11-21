#include <utility>

#include "Krystal.Booey/DOM/MutationObservers/AttributesMutationRecord.hpp"
#include "Krystal.Core/Utils/Move.hpp"

namespace krys::boo::dom
{
  AttributesMutationRecord::AttributesMutationRecord(Ref<Node> &&target, DOMStringAtom attributeName,
                                                     DOMStringAtom attributeNamespace,
                                                     DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), krys::move(oldValue)),
        _attributeName(krys::move(attributeName)), _attributeNamespace(krys::move(attributeNamespace))
  {
  }
}
