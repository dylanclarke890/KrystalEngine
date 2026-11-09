/*
 * Copyright (C) 1999 Lars Knoll (knoll@kde.org)
 *           (C) 1999 Antti Koivisto (koivisto@kde.org)
 *           (C) 2001 Dirk Mueller (mueller@kde.org)
 * Copyright (C) 2004-2018 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/DOM/Nodes/Node.hpp"

namespace krys::boo::dom
{
  class Element;

  /// @brief A ContainerNode is a Node that can have child nodes. It provides the common implementation for
  /// Element, Document, and DocumentFragment.
  /// @note The DOM specification does not define a ContainerNode interface, but we use it internally to save
  /// memory.
  class ContainerNode : public Node
  {
    KRYS_OVERRIDE_DELETE_FOR_CHECKED_PTR(ContainerNode);

    friend class ExtensibilityHooks;
    friend class MutationAlgorithms;

  public:
    /// @brief A change to a container's children, as style invalidation sees it.
    struct ChildChange
    {
      enum class Type : uint8
      {
        ElementInserted,
        ElementRemoved,
        TextInserted,
        TextRemoved,
        TextChanged,
        AllChildrenRemoved,
        NonContentsChildRemoved,
        NonContentsChildInserted,
        AllChildrenReplaced
      };

      /// @brief The parser appends to elements whose children it's still parsing; everything else is the API.
      enum class Source : uint8
      {
        Parser,
        API,
        Clone
      };

      enum class AffectsElements : uint8
      {
        Unknown,
        No,
        Yes
      };

      ChildChange::Type ChangeType;
      Element *SiblingChanged;
      Element *PreviousSiblingElement;
      Element *NextSiblingElement;
      ChildChange::Source ChangeSource;
      ChildChange::AffectsElements ChangeAffectsElements;

      KRYS_NODISCARD bool IsInsertion() const noexcept
      {
        switch (ChangeType)
        {
          case ChildChange::Type::ElementInserted:
          case ChildChange::Type::TextInserted:
          case ChildChange::Type::NonContentsChildInserted:
          case ChildChange::Type::AllChildrenReplaced:      return true;
          case ChildChange::Type::ElementRemoved:
          case ChildChange::Type::TextRemoved:
          case ChildChange::Type::TextChanged:
          case ChildChange::Type::AllChildrenRemoved:
          case ChildChange::Type::NonContentsChildRemoved:  return false;
        }
        return false;
      }
    };

  private:
    CheckedPtr<Node> _firstChild;
    CheckedPtr<Node> _lastChild;

  protected:
    ContainerNode(Document &document, dom::NodeType type, NodeFlags flags) noexcept;

  public:
    ~ContainerNode() noexcept override;

  protected:
    /// @brief Unlinks the children, without the removal steps (the container is going), and drops the tree
    /// references they hold.
    void RemoveDetachedChildren() noexcept;

  public:
#pragma region Node - https://dom.spec.whatwg.org/#node

    /// @see https://dom.spec.whatwg.org/#dom-node-firstchild
    KRYS_NODISCARD Node *FirstChild() const noexcept
    {
      return _firstChild.get();
    }

    /// @see https://dom.spec.whatwg.org/#dom-node-lastchild
    KRYS_NODISCARD Node *LastChild() const noexcept
    {
      return _lastChild.get();
    }

#pragma endregion

  protected:
#pragma region Extension Hooks

    virtual void OnChildrenChanged() noexcept
    {
    }

#pragma endregion

#pragma region Relationships

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetFirstChild(Node *child) noexcept
    {
      _firstChild = ShareCheckedPtr(child);
    }

    /// @warn Be careful when modifying node relationships. Node constraints are not checked.
    void SetLastChild(Node *child) noexcept
    {
      _lastChild = ShareCheckedPtr(child);
    }

#pragma endregion
  };
}

KRYS_SPECIALIZE_TYPE_CAST_TRAITS_BEGIN(krys::boo::dom::ContainerNode)
  KRYS_NODISCARD static bool IsType(const krys::boo::dom::Node &node) noexcept
  {
    return node.IsContainerNode();
  }
KRYS_SPECIALIZE_TYPE_CAST_TRAITS_END();