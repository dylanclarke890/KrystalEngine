/*
 * Copyright (C) 2019-2025 Apple Inc. All rights reserved.
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

#pragma once

#include "Krystal.Booey/DOM/Ranges/AbstractRange.hpp"
#include "Krystal.Booey/Editing/Position.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Memory/CanMakeWeakPtr.hpp"
#include "Krystal.Core/Memory/RefCounted.hpp"
#include "Krystal.Core/Memory/RefPtr.hpp"

namespace krys::boo::css
{
  /// @brief A range in a highlight, with the positions its painting starts and ends at (worked out at each
  /// rendering update).
  class HighlightRange : public RefCounted<HighlightRange>, public CanMakeWeakPtr<HighlightRange>
  {
  private:
    const Ref<dom::AbstractRange> _range;
    boo::Position _startPosition;
    boo::Position _endPosition;

    explicit HighlightRange(Ref<dom::AbstractRange> range) noexcept : _range(krys::move(range))
    {
    }

  public:
    KRYS_NODISCARD static Ref<HighlightRange> Create(Ref<dom::AbstractRange> range) noexcept
    {
      return AdoptRef(*new HighlightRange(krys::move(range)));
    }

    KRYS_NODISCARD dom::AbstractRange &Range() const noexcept
    {
      return *_range;
    }

    KRYS_NODISCARD const boo::Position &StartPosition() const noexcept
    {
      return _startPosition;
    }

    void SetStartPosition(boo::Position startPosition) noexcept
    {
      _startPosition = krys::move(startPosition);
    }

    KRYS_NODISCARD const boo::Position &EndPosition() const noexcept
    {
      return _endPosition;
    }

    void SetEndPosition(boo::Position endPosition) noexcept
    {
      _endPosition = krys::move(endPosition);
    }
  };
}

namespace krys::boo
{
  /// @brief A custom highlight: a set of ranges painted in their own way, named in a `HighlightRegistry` and
  /// styled by `::highlight(name)`.
  /// @see https://drafts.csswg.org/css-highlight-api-1/#custom-highlight
  class Highlight : public RefCounted<Highlight>
  {
  public:
    /// @brief The highlight's meaning, for assistive technologies.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#enumdef-highlighttype
    /// @see https://drafts.csswg.org/css-highlight-api-1/#highlight-types
    enum class Type : uint8
    {
      Highlight,
      SpellingError,
      GrammarError,
    };

  private:
    List<Ref<css::HighlightRange>> _highlightRanges;
    Type _type {Type::Highlight};
    int32 _priority {0};

    Highlight() noexcept = default;

  public:
#pragma region Highlight - https://drafts.csswg.org/css-highlight-api-1/#highlight

    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-highlight
    KRYS_NODISCARD static Ref<Highlight> Create(List<Ref<dom::AbstractRange>> initialRanges = {}) noexcept;

    /// @brief The number of ranges.
    /// @see https://webidl.spec.whatwg.org/#js-setlike
    KRYS_NODISCARD size_t Size() const noexcept
    {
      return _highlightRanges.size();
    }

    /// @brief Whether @p range is one of the highlight's ranges.
    /// @see https://webidl.spec.whatwg.org/#js-setlike
    KRYS_NODISCARD bool Has(const dom::AbstractRange &range) const noexcept;

    /// @brief Adds @p range at the end, unless it's there already.
    /// @return Whether it was added.
    /// @see https://webidl.spec.whatwg.org/#js-set-add
    bool Add(dom::AbstractRange &range) noexcept;

    /// @brief Removes @p range.
    /// @return Whether it was there.
    /// @see https://webidl.spec.whatwg.org/#js-set-delete
    bool Delete(const dom::AbstractRange &range) noexcept;

    /// @brief Removes every range.
    /// @see https://webidl.spec.whatwg.org/#js-set-clear
    void Clear() noexcept;

    // FIXME: Expose the type to assistive technologies.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-type
    KRYS_NODISCARD Type GetType() const noexcept
    {
      return _type;
    }

    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-type
    void SetType(Type type) noexcept
    {
      _type = type;
    }

    /// @brief Which of overlapping highlights is painted on top: the higher priority, or of equal ones, the
    /// one registered last.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#priority
    KRYS_NODISCARD int32 Priority() const noexcept
    {
      return _priority;
    }

    /// @see https://drafts.csswg.org/css-highlight-api-1/#dom-highlight-priority
    void SetPriority(int32 priority) noexcept;

#pragma endregion

    /// @brief Repaints the renderers of the nodes @p range intersects.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#repaint
    static void RepaintRange(const dom::AbstractRange &range) noexcept;

    /// @brief Repaints each of the highlight's ranges.
    /// @see https://drafts.csswg.org/css-highlight-api-1/#repaint
    void Repaint() noexcept;

    KRYS_NODISCARD const List<Ref<css::HighlightRange>> &HighlightRanges() const noexcept
    {
      return _highlightRanges;
    }
  };
}
