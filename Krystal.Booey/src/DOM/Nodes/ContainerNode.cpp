/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2025 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/DOM/Nodes/ContainerNode.hpp"
#include "Krystal.Booey/DOM/Abort/AbortSignal.hpp"
#include "Krystal.Booey/DOM/Nodes/Document.hpp"
#include "Krystal.Booey/HTML/CustomElement/CustomElementRegistry.hpp"
#include "Krystal.Core/Types/List.hpp"

namespace krys::boo::dom
{
  namespace
  {
    /// @brief Nodes whose tree references are being dropped, and whether they're being dropped now: a node
    /// deleted while dropping them adds its children here rather than recursing, so deep trees don't overflow
    /// the stack.
    struct DetachedChildrenQueue
    {
      List<Node *> Nodes;
      bool IsDraining {false};
    };

    DetachedChildrenQueue &GetDetachedChildrenQueue() noexcept
    {
      static DetachedChildrenQueue queue;
      return queue;
    }
  }

  ContainerNode::ContainerNode(Document &document, dom::NodeType type, NodeFlags flags) noexcept
      : Node(document, type, flags | NodeFlags::IsContainerNode), _firstChild(nullptr), _lastChild(nullptr)
  {
  }

  ContainerNode::~ContainerNode() noexcept
  {
    RemoveDetachedChildren();
  }

  void ContainerNode::RemoveDetachedChildren() noexcept
  {
    auto &queue = GetDetachedChildrenQueue();
    _lastChild = nullptr;
    while (auto *child = _firstChild.get())
    {
      _firstChild = child->_nextSibling;
      child->_nextSibling = nullptr;
      child->_previousSibling = nullptr;
      child->_parentNode = nullptr;
      child->ClearNodeFlag(NodeFlags::IsConnected);
      queue.Nodes.push_back(child);
    }

    if (queue.IsDraining)
    {
      return;
    }
    queue.IsDraining = true;
    while (!queue.Nodes.empty())
    {
      auto *node = queue.Nodes.back();
      queue.Nodes.pop_back();
      // The parent's tree reference (which may delete the node, adding its own children).
      node->SubRef();
    }
    queue.IsDraining = false;
  }
}
