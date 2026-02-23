#include "Krystal.HTML/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Element/Element.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/NodeList/NodeList.hpp"
#include "Krystal.HTML/QualifiedName.hpp"

namespace Krys::HTML
{
  AttributesMutationRecord::AttributesMutationRecord(Element &target, const QualifiedName &name,
                                                     const DOMString &oldValue) noexcept
      : EmptyNodeMutationRecord(target, oldValue), _attributeName(name.LocalName),
        _attributeNamespace(name.NamespaceURI)
  {
  }
}
