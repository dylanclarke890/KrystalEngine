#include "Krystal.Booey/Rendering/Updating/RenderTreePosition.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/PseudoElement.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Core/Types/List.hpp"
#include <algorithm>

namespace krys::boo
{
  void RenderTreePosition::ComputeNextSibling(const dom::Node &node) noexcept
  {
    krys_debug_assert(!node.Renderer());
    if (_hasValidNextSibling)
    {
      // FIXME: Assert the cached sibling is still right (upstream checks the first 20 times, except under the
      // render view), once that's cheap enough to do here.
      return;
    }
    _nextSibling = NextSiblingRenderer(node);
    _hasValidNextSibling = true;
  }

  void RenderTreePosition::InvalidateNextSibling(const RenderObject &siblingRenderer) noexcept
  {
    if (!_hasValidNextSibling)
    {
      return;
    }
    if (_nextSibling == &siblingRenderer)
    {
      _hasValidNextSibling = false;
    }
  }

  RenderObject *RenderTreePosition::NextSiblingRenderer(const dom::Node &node) const noexcept
  {
    krys_debug_assert(!node.Renderer());

    auto *parentElement = _parent->GetElement();
    if (!parentElement)
    {
      return nullptr;
    }
    // FIXME: PlugingReplacement shadow trees are very wrong.
    if (parentElement == &node)
    {
      return nullptr;
    }

    List<dom::Element *> elementStack;

    // In the common case ancestor == parentElement immediately and this just pushes parentElement into stack.
    auto *ancestor = node.ParentElementInComposedTree();
    while (true)
    {
      elementStack.push_back(ancestor);
      if (ancestor == parentElement)
      {
        break;
      }
      ancestor = ancestor->ParentElementInComposedTree();
      krys_debug_assert(ancestor);
    }
    std::reverse(elementStack.begin(), elementStack.end());

    auto composedDescendants = dom::ComposedTreeDescendants(*parentElement);

    auto initializeIteratorConsideringPseudoElements = [&]
    {
      if (auto *pseudoElement = DynamicDowncast<dom::PseudoElement>(node))
      {
        auto *host = pseudoElement->HostElement();
        if (pseudoElement->GetPseudoElementType() == PseudoElementType::Before)
        {
          if (host != parentElement)
          {
            return composedDescendants.at(*host).TraverseNext();
          }
          return composedDescendants.begin();
        }
        krys_debug_assert(pseudoElement->GetPseudoElementType() == PseudoElementType::After);
        elementStack.pop_back();
        if (host != parentElement)
        {
          return composedDescendants.at(*host).TraverseNextSkippingChildren();
        }
        return composedDescendants.end();
      }
      return composedDescendants.at(node).TraverseNextSkippingChildren();
    };

    auto pushCheckingForAfterPseudoElementRenderer = [&](dom::Element &element) -> RenderElement *
    {
      krys_debug_assert(!Is<dom::PseudoElement>(element));
      if (auto *before = element.BeforePseudoElement())
      {
        if (auto *renderer = before->Renderer())
        {
          return renderer;
        }
      }
      elementStack.push_back(&element);
      return nullptr;
    };

    auto popCheckingForAfterPseudoElementRenderers = [&](uint32 iteratorDepthToMatch) -> RenderElement *
    {
      while (elementStack.size() > iteratorDepthToMatch)
      {
        auto &element = *elementStack.back();
        elementStack.pop_back();
        if (auto *after = element.AfterPseudoElement())
        {
          if (auto *renderer = after->Renderer())
          {
            return renderer;
          }
        }
      }
      return nullptr;
    };

    auto it = initializeIteratorConsideringPseudoElements();
    auto end = composedDescendants.end();

    while (it != end)
    {
      if (auto *renderer = popCheckingForAfterPseudoElementRenderers(it.Depth()))
      {
        return renderer;
      }

      if (auto *renderer = it->Renderer())
      {
        return renderer;
      }

      if (auto *element = DynamicDowncast<dom::Element>(*it))
      {
        if (element->HasDisplayContents())
        {
          if (auto *renderer = pushCheckingForAfterPseudoElementRenderer(*element))
          {
            return renderer;
          }
          it.TraverseNext();
          continue;
        }
      }

      it.TraverseNextSkippingChildren();
    }

    return popCheckingForAfterPseudoElementRenderers(0);
  }
}
