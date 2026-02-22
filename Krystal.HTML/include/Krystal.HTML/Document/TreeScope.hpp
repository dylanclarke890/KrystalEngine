#pragma once

#include "Krystal.Lib/Core/Attributes.hpp"
#include "Krystal.Lib/Pointers/RawPtr.hpp"
#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Pointers/ReferenceWrapper.hpp"

namespace Krys::HTML
{
  class ContainerNode;
  class Document;
  class ShadowRoot;
  class CustomElementRegistry;

  class TreeScope
  {
  private:
    RawPtr<ContainerNode> _rootNode;
    ReferenceWrapper<Document> _documentScope;
    RawPtr<TreeScope> _parentTreeScope;
    RefPtr<CustomElementRegistry> _customElementRegistry;

  public:
    KRYS_NODISCARD const ContainerNode &RootNode() const noexcept
    {
      return *_rootNode;
    }

    KRYS_NODISCARD ContainerNode &RootNode() noexcept
    {
      return *_rootNode;
    }

    KRYS_NODISCARD Document &DocumentScope() const noexcept
    {
      return _documentScope;
    }

    KRYS_NODISCARD RawPtr<TreeScope> ParentTreeScope() const noexcept
    {
      return _parentTreeScope;
    }

    void SetParentTreeScope(TreeScope &newParentScope) noexcept;

    KRYS_NODISCARD RawPtr<CustomElementRegistry> GetCustomElementRegistry() const noexcept
    {
      return _customElementRegistry.get();
    }

    void SetCustomElementRegistry(RefPtr<CustomElementRegistry> &&registry) noexcept;

  protected:
    TreeScope(Document &document) noexcept;
    TreeScope(ShadowRoot &shadowRoot, Document &document, RefPtr<CustomElementRegistry> &&registry) noexcept;

    void SetDocumentScope(Document &newDocumentScope) noexcept;
  };
}