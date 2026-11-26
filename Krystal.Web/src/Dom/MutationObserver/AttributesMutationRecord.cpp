#include <utility>

#include "Krystal.Core/Move.hpp"
#include "Krystal.Web/Dom/MutationObserver/AttributesMutationRecord.hpp"

namespace krys::web::dom
{
  AttributesMutationRecord::AttributesMutationRecord(Ref<Node> &&target, DOMStringAtom attributeName,
                                                     DOMStringAtom attributeNamespace,
                                                     DOMStringAtom oldValue) noexcept
      : EmptyNodeMutationRecord(krys::move(target), krys::move(oldValue)),
        _attributeName(krys::move(attributeName)), _attributeNamespace(krys::move(attributeNamespace))
  {
  }
}
