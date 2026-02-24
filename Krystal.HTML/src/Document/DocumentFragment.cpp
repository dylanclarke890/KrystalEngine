#include "Krystal.HTML/Document/DocumentFragment.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Element/Element.hpp"

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

  RefPtr<Element> DocumentFragment::GetElementById(const DOMStringAtom &id) const noexcept
  {
    // TODO(IMPL): looking for an element with a matching id (obviously)
    return RefPtr<Element>();
  }
}