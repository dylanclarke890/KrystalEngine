#include "Krystal.HTML/DOM/Algorithms/ShadowRootAlgorithms.hpp"
#include "Krystal.HTML/DOM/Algorithms/NameValidation.hpp"
#include "Krystal.HTML/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.HTML/DOM/Element.hpp"
#include "Krystal.HTML/DOM/ShadowRoot.hpp"
#include "Krystal.HTML/Infra/Namespaces.hpp"

namespace Krys::HTML
{
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