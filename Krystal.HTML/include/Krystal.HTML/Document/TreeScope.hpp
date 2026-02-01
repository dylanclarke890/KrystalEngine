#pragma once

#include "Krystal.Lib/Pointers/RawPtr.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class ShadowRoot;

  class TreeScope
  {
  private:
    RawPtr<ContainerNode> _rootNode;
    RawPtr<Document> _documentScope;
    RawPtr<TreeScope> _parentScope;

  public:
    TreeScope(Document &document) noexcept;
    TreeScope(ShadowRoot &shadowRoot, Document &document) noexcept;
  };
}