/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Peter Kelly (pmk@post.com)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2003-2025 Apple Inc. All rights reserved.
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
 *
 */

#pragma once

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::dom
{
  class ContainerNode;
  class Document;
  class Element;
  class Node;

  /// @see https://dom.spec.whatwg.org/#other-applicable-specifications
  class ExtensibilityHooks
  {
  public:
    /// @param isSubtreeRoot Whether `insertedNode` is the node that was inserted, rather than one of its
    /// shadow-including descendants.
    /// @see https://dom.spec.whatwg.org/#concept-node-insert-ext
    static void NodeInserted(Node &insertedNode, bool isSubtreeRoot) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-post-connection-ext
    static void NodePostConnection(Node &connectedNode) noexcept;

    /// @param onlyReplacedElementsInserted Whether the change inserted replaced elements and nothing else,
    /// which is neutral to the auto directionality of the node.
    /// @see https://dom.spec.whatwg.org/#concept-node-children-changed-ext
    static void NodeChildrenChanged(ContainerNode &node, bool onlyReplacedElementsInserted = false) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-move-ext
    static void NodeMoved(Node &movedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-remove-ext
    static void NodeRemoved(Node &removedNode, bool isSubtreeRoot, ContainerNode &oldAncestor) noexcept;

    /// @brief Destroys the renderers of @p child, which is about to be removed or moved. It's still in the
    /// tree, so renderers that depend on their place (list items, counters) can update their neighbours.
    static void DestroyRenderTreeIfNeeded(Node &child) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-clone-ext
    static void NodeCloned(Node &node, Node &copy, bool subtree) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-node-adopt-ext
    static void NodeAdopted(Node &node, Document &oldDocument) noexcept;

    /// @brief Adds the IDs of @p node and its descendants to @p parent's tree scope, if it's in one. Done
    /// before their insertion steps, each of which may look up an ID.
    static void SubtreeInserted(Node &node, ContainerNode &parent) noexcept;
    /// @brief Removes the IDs of @p node and its descendants from @p parent's tree scope, if it's in one.
    /// Done before their removing steps, each of which may look up an ID.
    static void SubtreeRemoved(Node &node, ContainerNode &parent) noexcept;
    /// @brief Moves the IDs of @p node and its descendants to their new tree scope, if it changed.
    static void SubtreeMoved(Node &node, ContainerNode &oldParent, ContainerNode &newParent) noexcept;

    /// @see https://dom.spec.whatwg.org/#concept-element-attributes-change-ext
    static void ElementAttributeChanged(Element &element, DOMStringAtom localName,
                                        Maybe<DOMStringView> oldValue, Maybe<DOMStringView> value,
                                        DOMStringAtom namespaceURI) noexcept;
  };
}