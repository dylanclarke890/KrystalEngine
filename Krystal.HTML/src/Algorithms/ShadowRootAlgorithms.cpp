#include "Krystal.HTML/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/Abort/AbortSignal.hpp"
#include "Krystal.HTML/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.HTML/HTMLElement/HTMLSlotElement.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"
#include "Krystal.HTML/Node/Attr.hpp"
#include "Krystal.HTML/Node/Document.hpp"
#include "Krystal.HTML/Node/Element.hpp"
#include "Krystal.HTML/Node/Node.hpp"
#include "Krystal.HTML/Node/NodeList.hpp"
#include "Krystal.HTML/Node/ShadowRoot.hpp"

namespace Krys::HTML
{
  ExceptionOr<void> ShadowRootAlgorithms::AttachShadowRoot(Element &element, ShadowRootMode mode,
                                                           Clonable clonable, Serializable serializable,
                                                           DelegatesFocus delegatesFocus,
                                                           SlotAssignmentMode slotAssignment,
                                                           RawPtr<CustomElementRegistry> registry) noexcept
  {
    if (element.NamespaceURI() != Namespaces::HTML)
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    if (!NameValidation::IsValidShadowHostName(element.LocalName().View()))
    {
      return Exception {ExceptionCode::NotSupportedError};
    }

    // TODO(impl): CUSTOM-ELEMENTS
    // If element’s local name is a valid custom element name, or element’s is value is non-null:
    //   Let definition be the result of looking up a custom element definition given element’s custom element
    //   registry, its namespace, its local name, and its is value.
    //
    //   If definition is non-null and definition’s disable shadow is true, then throw a "NotSupportedError"
    //   DOMException.

    if (IsShadowHost(element))
    {
      auto &currentShadowRoot = element._shadowRoot;
      if (!currentShadowRoot->_declarative || currentShadowRoot->Mode() != mode)
      {
        return Exception {ExceptionCode::NotSupportedError};
      }

      while (currentShadowRoot->FirstChild())
      {
        auto result = currentShadowRoot->RemoveChild(*currentShadowRoot->FirstChild());
        if (result.HasException())
        {
          return result.ReleaseException();
        }
      }

      currentShadowRoot->_declarative = false;
      return {};
    }

    auto shadow = CreateRef<ShadowRoot>(element.NodeDocument(), ShareRefPtr(registry));
    shadow->_host = CreateWeakPtr(&element);
    shadow->_delegatesFocus = delegatesFocus;

    if (element._customElementState == CustomElementState::Precustomized
        || element._customElementState == CustomElementState::Custom)
    {
      shadow->_availableToElementInternals = true;
    }

    shadow->_slotAssignment = slotAssignment;
    shadow->_declarative = false;
    shadow->_clonable = clonable;
    shadow->_serializable = serializable;

    element._shadowRoot = shadow;

    return {};
  }

  const Node &ShadowRootAlgorithms::ShadowIncludingRoot(const Node &node) noexcept
  {
    auto &root = TreeQueries::Root(node);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? ShadowIncludingRoot(*host) : root;
    }

    return root;
  }

  Node &ShadowRootAlgorithms::ShadowIncludingRoot(Node &node) noexcept
  {
    auto &root = TreeQueries::Root(node);
    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(root))
    {
      auto *host = shadowRoot->Host();
      return host ? ShadowIncludingRoot(*host) : root;
    }

    return root;
  }

  bool ShadowRootAlgorithms::IsShadowIncludingDescendant(const Node &a, const Node &b) noexcept
  {
    if (TreeQueries::IsDescendant(a, b))
    {
      return true;
    }

    if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(TreeQueries::Root(a)))
    {
      if (auto *host = shadowRoot->Host())
      {
        return IsShadowIncludingDescendant(*host, b);
      }
    }

    return false;
  }

  bool ShadowRootAlgorithms::IsShadowIncludingInclusiveDescendant(const Node &a, const Node &b) noexcept
  {
    if (&a == &b)
    {
      return true;
    }

    return IsShadowIncludingDescendant(a, b);
  }

  bool ShadowRootAlgorithms::IsShadowIncludingAncestor(const Node &a, const Node &b) noexcept
  {
    return IsShadowIncludingDescendant(b, a);
  }

  bool ShadowRootAlgorithms::IsShadowIncludingInclusiveAncestor(const Node &a, const Node &b) noexcept
  {
    return IsShadowIncludingInclusiveDescendant(b, a);
  }

  bool ShadowRootAlgorithms::IsClosedShadowHidden(const Node &a, const Node &b) noexcept
  {
    auto &aRoot = TreeQueries::Root(a);
    auto *aShadowRoot = DynamicDowncast<ShadowRoot>(aRoot);
    if (aShadowRoot == nullptr)
    {
      return false;
    }

    if (IsShadowIncludingInclusiveAncestor(aRoot, b))
    {
      return false;
    }

    if (aShadowRoot->Mode() == ShadowRootMode::Closed)
    {
      return true;
    }

    if (aShadowRoot->Host() && IsClosedShadowHidden(*aShadowRoot->Host(), b))
    {
      return true;
    }

    return false;
  }

  RawPtr<EventTarget> ShadowRootAlgorithms::Retarget(RawPtr<EventTarget> a, EventTarget &b) noexcept
  {
    auto *current = a;
    auto *bNode = DynamicDowncast<Node>(b);
    while (true)
    {
      if (current == nullptr)
      {
        return nullptr;
      }

      if (!Is<Node>(current))
      {
        return current;
      }

      auto *currentNode = Downcast<Node>(current);
      auto &currentRoot = TreeQueries::Root(*currentNode);
      if (!Is<ShadowRoot>(currentNode))
      {
        return current;
      }

      if (bNode != nullptr && !IsShadowIncludingInclusiveAncestor(currentRoot, *bNode))
      {
        return current;
      }

      auto *host = Downcast<ShadowRoot>(&currentRoot)->Host();
      current = host;
    }

    // The spec handles the necessary cases to ensure we never get here, but we need this to satisfy the
    // compiler that current is always valid.
    std::unreachable();
    return current;
  }

  bool ShadowRootAlgorithms::IsShadowHost(const Node &node) noexcept
  {
    if (!Is<Element>(node))
    {
      return false;
    }

    return IsShadowHost(Downcast<Element>(node));
  }

  bool ShadowRootAlgorithms::IsShadowHost(const Element &node) noexcept
  {
    return node._shadowRoot != nullptr;
  }

  bool ShadowRootAlgorithms::HasSameShadowIncludingRoot(const Node &a, const Node &b) noexcept
  {
    return &ShadowIncludingRoot(a) == &ShadowIncludingRoot(b);
  }

  RefPtr<ShadowRoot> ShadowRootAlgorithms::GetShadowRoot(const Node &node) noexcept
  {
    if (auto *element = DynamicDowncast<Element>(node))
    {
      return element->ShadowRoot();
    }

    return nullptr;
  }
}