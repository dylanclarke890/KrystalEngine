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

#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Booey/DOM/Types/ExceptionOr.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/RefCounted.hpp"
#include "Krystal.Core/Types/RefPtr.hpp"
#include "Krystal.Core/Types/WeakPtr.hpp"

namespace krys::boo::dom
{
  class Document;
  class Node;
  class Position;
  class Range;
  class ShadowRoot;
  class StaticRange;
  struct SimpleRange;
}

namespace krys::boo
{
  class LocalFrame;

  /// @brief A document's `Selection`: the script's view of the frame's selection, which is the range it
  /// associates with it.
  /// @see https://w3c.github.io/selection-api/#selection-interface
  class DOMSelection final : public RefCounted<DOMSelection>
  {
    KRYS_NON_COPY_MOVABLE(DOMSelection);

  private:
    WeakPtr<dom::Document> _document;

  public:
    explicit DOMSelection(dom::Document &document) noexcept;

    KRYS_NODISCARD dom::DOMString Type() const noexcept;
    KRYS_NODISCARD dom::DOMString Direction() const noexcept;

    dom::ExceptionOr<void> SetBaseAndExtent(dom::Node &anchorNode, uint32 anchorOffset, dom::Node &focusNode,
                                            uint32 focusOffset) noexcept;
    dom::ExceptionOr<void> SetPosition(dom::Node *node, uint32 offset = 0) noexcept;
    /// @note Non-standard.
    void Modify(dom::DOMStringView alter, dom::DOMStringView direction,
                dom::DOMStringView granularity) noexcept;

    // The anchor and focus are the start and end of the selection, and
    // reflect the direction in which the selection was made by the user.
    KRYS_NODISCARD RefPtr<dom::Node> AnchorNode() const noexcept;
    KRYS_NODISCARD uint32 AnchorOffset() const noexcept;
    KRYS_NODISCARD RefPtr<dom::Node> FocusNode() const noexcept;
    KRYS_NODISCARD uint32 FocusOffset() const noexcept;
    KRYS_NODISCARD bool IsCollapsed() const noexcept;
    KRYS_NODISCARD uint32 RangeCount() const noexcept;

    dom::ExceptionOr<void> Collapse(dom::Node *node, uint32 offset = 0) noexcept;
    dom::ExceptionOr<void> CollapseToEnd() noexcept;
    dom::ExceptionOr<void> CollapseToStart() noexcept;
    dom::ExceptionOr<void> Extend(dom::Node &node, uint32 offset = 0) noexcept;
    KRYS_NODISCARD dom::ExceptionOr<Ref<dom::Range>> GetRangeAt(uint32 index) noexcept;
    void RemoveAllRanges() noexcept;
    void AddRange(dom::Range &liveRange) noexcept;
    dom::ExceptionOr<void> RemoveRange(dom::Range &liveRange) noexcept;
    KRYS_NODISCARD List<Ref<dom::StaticRange>>
      GetComposedRanges(const List<Ref<dom::ShadowRoot>> &shadowRoots = {}) noexcept;
    void DeleteFromDocument() noexcept;
    KRYS_NODISCARD bool ContainsNode(dom::Node &node, bool allowPartialContainment = false) const noexcept;
    dom::ExceptionOr<void> SelectAllChildren(dom::Node &node) noexcept;
    KRYS_NODISCARD dom::DOMString ToString() const noexcept;
    void Empty() noexcept;

  private:
    KRYS_NODISCARD LocalFrame *Frame() const noexcept;
    KRYS_NODISCARD Maybe<dom::SimpleRange> GetRange() const noexcept;
    KRYS_NODISCARD dom::Position AnchorPosition() const noexcept;
    KRYS_NODISCARD dom::Position FocusPosition() const noexcept;
    KRYS_NODISCARD RefPtr<dom::Node> ShadowAdjustedNode(const dom::Position &position) const noexcept;
    KRYS_NODISCARD uint32 ShadowAdjustedOffset(const dom::Position &position) const noexcept;
  };
}
