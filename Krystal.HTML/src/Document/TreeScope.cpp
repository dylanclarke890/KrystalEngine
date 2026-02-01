#include "Krystal.HTML/Document/TreeScope.hpp"
#include "Krystal.HTML/Document/Document.hpp"
#include "Krystal.HTML/Document/ShadowRoot.hpp"

namespace Krys::HTML
{
  TreeScope::TreeScope(Document &document) noexcept
      : _rootNode(&document), _documentScope(&document), _parentScope(nullptr)
  {
    document.SetTreeScope(*this);
  }

  TreeScope::TreeScope(ShadowRoot &shadowRoot, Document &document) noexcept
      : _rootNode(&shadowRoot), _documentScope(&document), _parentScope(&document)
  {
    shadowRoot.SetTreeScope(*this);
  }
}