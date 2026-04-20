#include "Krystal.HTML/MutationObserver/AttributesMutationRecord.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
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
