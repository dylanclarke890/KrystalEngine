/*
 * Copyright (C) 2015 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Rendering/Updating/RenderTreePosition.hpp"
#include "Krystal.Booey/DOM/Iterators/ComposedTreeIterator.hpp"
#include "Krystal.Booey/DOM/Nodes/PseudoElement.hpp"
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
