#include "Krystal.HTML/DOM/MutationRecord/AttributesMutationRecord.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/DOM/AbortSignal.hpp"
#include "Krystal.HTML/DOM/Collections/StaticNodeList.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/CharacterData.hpp"
#include "Krystal.HTML/Node/ContainerNode.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  AttributesMutationRecord::AttributesMutationRecord(Element &target, const QualifiedName &name,
                                                     DOMString &&oldValue) noexcept
      : EmptyNodeMutationRecord(MutationRecordType::Attributes, target, Krys::Move(oldValue)),
        _attributeName(name.LocalName), _attributeNamespace(name.NamespaceURI)
  {
  }
}
