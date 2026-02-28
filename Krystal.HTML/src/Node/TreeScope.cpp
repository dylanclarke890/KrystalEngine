#include "Krystal.HTML/Node/TreeScope.hpp"
#include "Krystal.HTML/Node/CustomElementRegistry.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"
#include "Krystal.Lib/Core/Move.hpp"
#include <cassert>

namespace Krys::HTML
{
  TreeScope::TreeScope(Document &document) noexcept
      : _rootNode(&document), _documentScope(document), _parentTreeScope(nullptr),
        _customElementRegistry(nullptr)
  {
    document.SetTreeScope(*this);
  }

  TreeScope::TreeScope(ShadowRoot &shadowRoot, Document &document,
                       RefPtr<CustomElementRegistry> &&registry) noexcept
      : _rootNode(&shadowRoot), _documentScope(document), _parentTreeScope(&document),
        _customElementRegistry(Krys::Move(registry))
  {
    shadowRoot.SetTreeScope(*this);
  }

  void TreeScope::SetParentTreeScope(TreeScope &newParentScope) noexcept
  {
    // A document node cannot be re-parented.
    assert(!_rootNode->IsDocumentNode());

    _parentTreeScope = &newParentScope;
    SetDocumentScope(newParentScope.DocumentScope());
  }

  void TreeScope::SetCustomElementRegistry(RefPtr<CustomElementRegistry> &&registry) noexcept
  {
    _customElementRegistry = Krys::Move(registry);
  }

  void TreeScope::SetDocumentScope(Document &document) noexcept
  {
    _documentScope = document;
  }
}