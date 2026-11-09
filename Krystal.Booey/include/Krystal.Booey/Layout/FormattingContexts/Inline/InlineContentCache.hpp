/*
 * Copyright (C) 2018-2024 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/LineLayoutResult.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo::layout
{
  /// @brief Content cached for subsequent layouts of an inline formatting context: its inline items, its
  /// text spacing and its intrinsic sizes.
  class InlineContentCache
  {
    KRYS_NON_COPY_MOVABLE(InlineContentCache);

  public:
    class InlineItems
    {
    public:
      struct ContentAttributes
      {
        bool RequiresVisualReordering {false};
        // Note that <span>this is text</span> returns true as inline boxes are not considered 'content' here.
        bool HasTextAndLineBreakOnlyContent {false};
        bool HasTextAutospace {false};
        size_t InlineBoxCount {0};
      };
      enum class IsPopulatedFromCache : bool
      {
        No,
        Yes
      };

    private:
      ContentAttributes _contentAttributes;
      InlineItemList _inlineItemList;
      bool _isPopulatedFromCache {false};

    public:
      KRYS_NODISCARD InlineItemList &Content() noexcept
      {
        return _inlineItemList;
      }
      KRYS_NODISCARD const InlineItemList &Content() const noexcept
      {
        return _inlineItemList;
      }

      void Set(InlineItemList &&inlineItemList, ContentAttributes contentAttributes,
               IsPopulatedFromCache isPopulatedFromCache) noexcept
      {
        _inlineItemList = krys::move(inlineItemList);
        _contentAttributes = contentAttributes;
        _isPopulatedFromCache = isPopulatedFromCache == IsPopulatedFromCache::Yes;
      }
      void Replace(size_t insertionPosition, InlineItemList &&inlineItemList,
                   ContentAttributes contentAttributes, IsPopulatedFromCache isPopulatedFromCache) noexcept
      {
        _inlineItemList.erase(_inlineItemList.begin() + static_cast<ptrdiff_t>(insertionPosition),
                              _inlineItemList.end());
        _inlineItemList.insert(_inlineItemList.end(), inlineItemList.begin(), inlineItemList.end());
        _contentAttributes = contentAttributes;
        if (isPopulatedFromCache == IsPopulatedFromCache::No)
        {
          _isPopulatedFromCache = false;
        }
      }
      void ShrinkToFit() noexcept
      {
        _inlineItemList.shrink_to_fit();
      }

      KRYS_NODISCARD bool IsEmpty() const noexcept
      {
        return Content().empty();
      }
      KRYS_NODISCARD size_t Size() const noexcept
      {
        return Content().size();
      }

      KRYS_NODISCARD bool RequiresVisualReordering() const noexcept
      {
        return _contentAttributes.RequiresVisualReordering;
      }
      KRYS_NODISCARD bool HasTextAndLineBreakOnlyContent() const noexcept
      {
        return _contentAttributes.HasTextAndLineBreakOnlyContent;
      }
      KRYS_NODISCARD bool HasTextAutospace() const noexcept
      {
        return _contentAttributes.HasTextAutospace;
      }
      KRYS_NODISCARD bool HasInlineBoxes() const noexcept
      {
        return InlineBoxCount() != 0;
      }
      KRYS_NODISCARD size_t InlineBoxCount() const noexcept
      {
        return _contentAttributes.InlineBoxCount;
      }
      KRYS_NODISCARD bool IsPopulatedFromCache() const noexcept
      {
        return _isPopulatedFromCache;
      }
    };

  private:
    InlineItems _inlineItems;
    TextSpacingContext _textSpacingContext;

    Maybe<LineLayoutResult> _maximumIntrinsicWidthLineContent {};
    Maybe<InlineLayoutUnit> _minimumContentSize {};
    Maybe<InlineLayoutUnit> _maximumContentSize {};

  public:
    InlineContentCache() noexcept = default;

    KRYS_NODISCARD const InlineItems &GetInlineItems() const noexcept
    {
      return _inlineItems;
    }
    KRYS_NODISCARD InlineItems &GetInlineItems() noexcept
    {
      return _inlineItems;
    }

    void SetMaximumIntrinsicWidthLineContent(LineLayoutResult &&lineContent) noexcept
    {
      _maximumIntrinsicWidthLineContent = krys::move(lineContent);
    }
    void ClearMaximumIntrinsicWidthLineContent() noexcept
    {
      _maximumIntrinsicWidthLineContent = {};
    }
    KRYS_NODISCARD Maybe<LineLayoutResult> &MaximumIntrinsicWidthLineContent() noexcept
    {
      return _maximumIntrinsicWidthLineContent;
    }

    void SetMinimumContentSize(InlineLayoutUnit minimumContentSize) noexcept
    {
      _minimumContentSize = minimumContentSize;
    }
    void SetMaximumContentSize(InlineLayoutUnit maximumContentSize) noexcept
    {
      _maximumContentSize = maximumContentSize;
    }
    KRYS_NODISCARD Maybe<InlineLayoutUnit> MinimumContentSize() const noexcept
    {
      return _minimumContentSize;
    }
    KRYS_NODISCARD Maybe<InlineLayoutUnit> MaximumContentSize() const noexcept
    {
      return _maximumContentSize;
    }
    void ResetMinimumMaximumContentSizes() noexcept
    {
      _minimumContentSize = {};
      _maximumContentSize = {};
      _maximumIntrinsicWidthLineContent = {};
    }

    KRYS_NODISCARD const InlineBoxBoundaryTextSpacings &GetInlineBoxBoundaryTextSpacings() const noexcept
    {
      return _textSpacingContext.InlineBoxBoundarySpacings;
    }
    void SetInlineBoxBoundaryTextSpacings(InlineBoxBoundaryTextSpacings &&spacings) noexcept
    {
      _textSpacingContext.InlineBoxBoundarySpacings = krys::move(spacings);
    }
    KRYS_NODISCARD const TrimmableTextSpacings &GetTrimmableTextSpacings() const noexcept
    {
      return _textSpacingContext.TrimmableSpacings;
    }
    void SetTrimmableTextSpacings(TrimmableTextSpacings &&spacings) noexcept
    {
      _textSpacingContext.TrimmableSpacings = krys::move(spacings);
    }

    KRYS_NODISCARD const TextSpacingContext &GetTextSpacingContext() const noexcept
    {
      return _textSpacingContext;
    }
  };
}
