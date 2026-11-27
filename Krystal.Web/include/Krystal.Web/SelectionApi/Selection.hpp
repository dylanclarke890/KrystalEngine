/*
 * Copyright (C) 2007-2025 Apple Inc. All rights reserved.
 * Copyright (C) 2012 Google Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1.  Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 * 2.  Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 * 3.  Neither the name of Apple Inc. ("Apple") nor the names of
 *     its contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE AND ITS CONTRIBUTORS "AS IS" AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL APPLE OR ITS CONTRIBUTORS BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"
#include "Krystal.Core/Memory/WeakPtr.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Web/Dom/Types/DOMString.hpp"
#include "Krystal.Web/Dom/Types/ExceptionOr.hpp"

namespace krys::web::dom
{
  class Range;
  struct SimpleRange;
  class StaticRange;
}

namespace krys::web
{
  class Document;
  class Frame;
  class Node;
  class Position;
  class ShadowRoot;

  /// @see https://w3c.github.io/selection-api/#selection-interface
  class Selection final : public RefCounted<Selection>
  {
    KRYS_NON_COPY_MOVABLE(Selection);

  private:
    WeakPtr<Document> _document;

  public:
    explicit Selection(Document &document) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-type
    KRYS_NODISCARD DOMString Type() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-direction
    KRYS_NODISCARD DOMString Direction() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-setbaseandextent
    ExceptionOr<void> SetBaseAndExtent(Node &anchorNode, uint32 anchorOffset, Node &focusNode,
                                       uint32 focusOffset) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-setposition
    ExceptionOr<void> SetPosition(Node *node, uint32 offset = 0) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-modify
    void Modify(DOMStringView alter, DOMStringView direction, DOMStringView granularity) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-anchornode
    KRYS_NODISCARD RefPtr<Node> AnchorNode() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-anchoroffset
    KRYS_NODISCARD uint32 AnchorOffset() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-focusnode
    KRYS_NODISCARD RefPtr<Node> FocusNode() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-focusoffset
    KRYS_NODISCARD uint32 FocusOffset() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-iscollapsed
    KRYS_NODISCARD bool IsCollapsed() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-rangecount
    KRYS_NODISCARD uint32 RangeCount() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-collapse
    ExceptionOr<void> Collapse(Node *node, uint32 offset = 0) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-collapsetoend
    ExceptionOr<void> CollapseToEnd() noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-collapsetostart
    ExceptionOr<void> CollapseToStart() noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-extend
    ExceptionOr<void> Extend(Node &node, uint32 offset = 0) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-getrangeat
    KRYS_NODISCARD ExceptionOr<Ref<dom::Range>> GetRangeAt(uint32 index) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-removeallranges
    void RemoveAllRanges() noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-addrange
    void AddRange(dom::Range &liveRange) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-removerange
    ExceptionOr<void> RemoveRange(dom::Range &liveRange) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-getcomposedranges
    KRYS_NODISCARD List<Ref<dom::StaticRange>>
      GetComposedRanges(const List<Ref<ShadowRoot>> &shadowRoots = {}) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-deletefromdocument
    void DeleteFromDocument() noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-containsnode
    KRYS_NODISCARD bool ContainsNode(Node &node, bool allowPartialContainment = false) const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-selectallchildren
    ExceptionOr<void> SelectAllChildren(Node &node) noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-stringifier
    KRYS_NODISCARD DOMString ToString() const noexcept;

    /// @see https://w3c.github.io/selection-api/#dom-selection-empty
    void Empty() noexcept;

  private:
    KRYS_NODISCARD web::Frame *Frame() const noexcept;

    KRYS_NODISCARD Maybe<dom::SimpleRange> GetRange() const noexcept;

    KRYS_NODISCARD Position AnchorPosition() const noexcept;

    KRYS_NODISCARD Position FocusPosition() const noexcept;

    /// @brief Whether @p node is in the document tree: its root is the document.
    KRYS_NODISCARD bool IsInDocumentTree(const Node &node) const noexcept;

    /// @brief Whether @p position is non-null and in the document tree.
    KRYS_NODISCARD bool IsInDocumentTree(const Position &position) const noexcept;

    KRYS_NODISCARD bool AreAnchorAndFocusInDocumentTree() const noexcept;
  };
}
