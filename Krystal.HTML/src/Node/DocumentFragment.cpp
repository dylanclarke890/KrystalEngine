#include "Krystal.HTML/Node/DocumentFragment.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.HTML/NodeList/HTMLCollection.hpp"
#include "Krystal.HTML/Tree/TreeQueries.hpp"
#include "Krystal.HTML/Tree/TreeTraversal.hpp"

namespace Krys::HTML
{
  DocumentFragment::DocumentFragment(Document &document, NodeFlag flags) noexcept
      : ContainerNode(document, NodeType::DOCUMENT_FRAGMENT_NODE, flags)
  {
  }

  utf8_string DocumentFragment::NodeName() const noexcept
  {
    return u8"#document-fragment";
  }

  DOMString DocumentFragment::TextContent() const noexcept
  {
    return TreeQueries::DescendantTextContent(*this);
  }

#pragma region ParentNode

  Ref<HTMLCollection> DocumentFragment::Children() const noexcept
  {
    return _documentFragmentRareData->Children(*this);
  }

  RawPtr<Element> DocumentFragment::FirstElementChild() const noexcept
  {
    return TreeTraversal::FirstElementChild(*this);
  }

  RawPtr<Element> DocumentFragment::LastElementChild() const noexcept
  {
    return TreeTraversal::LastElementChild(*this);
  }

  size_t DocumentFragment::ChildElementCount() const noexcept
  {
    return TreeQueries::ChildElementCount(*this);
  }

#pragma endregion

#pragma region NonElementParentNode

  RefPtr<Element> DocumentFragment::GetElementById(const DOMStringAtom &id) const noexcept
  {
    // TODO(IMPL): looking for an element with a matching id (obviously)
    return RefPtr<Element>();
  }

#pragma endregion
}