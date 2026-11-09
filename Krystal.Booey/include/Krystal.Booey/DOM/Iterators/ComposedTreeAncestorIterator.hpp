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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Booey/DOM/Algorithms/SlotAlgorithms.hpp"
#include "Krystal.Booey/DOM/Algorithms/TreeQueries.hpp"
#include "Krystal.Booey/DOM/Nodes/Element.hpp"
#include "Krystal.Booey/DOM/Nodes/ShadowRoot.hpp"
#include "Krystal.Booey/HTML/Elements/HTMLSlotElement.hpp"
#include "Krystal.Core/Types/CheckedPtr.hpp"

namespace krys::boo::dom
{
  /// @brief Iterates over the ancestor elements of a node in the composed (flat) tree.
  class ComposedTreeAncestorIterator
  {
  private:
    CheckedPtr<Element> _current;

  public:
    ComposedTreeAncestorIterator() noexcept = default;

    explicit ComposedTreeAncestorIterator(Node &current) noexcept
        : _current(ShareCheckedPtr(TraverseParent(&current)))
    {
      krys_debug_assert(!Is<ShadowRoot>(current));
    }

    explicit ComposedTreeAncestorIterator(Element &current) noexcept : _current(ShareCheckedPtr(&current))
    {
    }

    KRYS_NODISCARD Element &operator*() const noexcept
    {
      return *_current;
    }

    KRYS_NODISCARD Element *operator->() const noexcept
    {
      return _current.get();
    }

    KRYS_NODISCARD bool operator==(const ComposedTreeAncestorIterator &other) const noexcept
    {
      return _current == other._current;
    }

    ComposedTreeAncestorIterator &operator++() noexcept
    {
      _current = ShareCheckedPtr(TraverseParent(_current.get()));
      return *this;
    }

    KRYS_NODISCARD Element &Get() const noexcept
    {
      return *_current;
    }

  private:
    KRYS_NODISCARD static Element *TraverseParent(Node *current) noexcept
    {
      auto *parent = current->ParentNode();
      if (!parent)
      {
        return nullptr;
      }
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*parent))
      {
        return shadowRoot->Host();
      }
      auto *parentElement = DynamicDowncast<Element>(*parent);
      if (!parentElement)
      {
        return nullptr;
      }
      if (TreeQueries::GetShadowRoot(*parentElement))
      {
        return SlotAlgorithms::GetAssignedSlot(*current);
      }
      return parentElement;
    }
  };

  /// @brief The ancestor elements of a node in the composed tree.
  class ComposedTreeAncestorAdapter
  {
  public:
    using iterator = ComposedTreeAncestorIterator;

  private:
    Ref<Node> _node;

  public:
    explicit ComposedTreeAncestorAdapter(Node &node) noexcept : _node(ShareRef(node))
    {
    }

    KRYS_NODISCARD iterator begin() const noexcept
    {
      if (auto *shadowRoot = DynamicDowncast<ShadowRoot>(*_node))
      {
        return iterator(*shadowRoot->Host());
      }
      return iterator(*_node);
    }

    KRYS_NODISCARD iterator end() const noexcept
    {
      return {};
    }

    KRYS_NODISCARD Element *First() const noexcept
    {
      auto it = begin();
      if (it == end())
      {
        return nullptr;
      }
      return &it.Get();
    }
  };

  KRYS_NODISCARD inline ComposedTreeAncestorAdapter ComposedTreeAncestors(Node &node) noexcept
  {
    return ComposedTreeAncestorAdapter(node);
  }
}
