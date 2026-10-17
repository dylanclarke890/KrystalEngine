#pragma once

#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineContentCache.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineItem.hpp"
#include "Krystal.Booey/Layout/FormattingContexts/Inline/InlineLineTypes.hpp"
#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"
#include "Krystal.Core/Types/SmallList.hpp"

namespace krys::boo::layout
{
  class InlineTextBox;

  /// @brief Turns the content of an inline formatting context into its flat list of inline items, splits
  /// them at bidi boundaries, measures the text items and caches the result.
  class InlineItemsBuilder
  {
    KRYS_NON_COPY_MOVABLE(InlineItemsBuilder);

    using LayoutQueue = SmallList<const Box *, 8>;

  private:
    InlineContentCache &_inlineContentCache;
    const ElementBox &_root;

    bool _contentRequiresVisualReordering {false};
    bool _hasTextAutospace;
    Maybe<bool> _textContentPopulatedFromCache {};

  public:
    InlineItemsBuilder(InlineContentCache &inlineContentCache, const ElementBox &root) noexcept;
    void Build(InlineItemPosition startPosition) noexcept;

    static void PopulateBreakingPositionCache(const InlineItemList &inlineItemList) noexcept;

  private:
    void CollectInlineItems(InlineItemList &inlineItemList, InlineItemPosition startPosition) noexcept;
    KRYS_NODISCARD LayoutQueue InitializeLayoutQueue(InlineItemPosition startPosition) noexcept;
    KRYS_NODISCARD LayoutQueue TraverseUntilDamaged(const Box &firstDamagedLayoutBox) noexcept;
    void BreakAndComputeBidiLevels(InlineItemList &inlineItemList) noexcept;
    KRYS_NODISCARD InlineContentCache::InlineItems::ContentAttributes
      ComputeContentAttributesAndInlineTextItemWidths(InlineItemList &inlineItemList,
                                                      InlineItemPosition damagePosition,
                                                      const InlineItemList &damagedItemList) noexcept;

    void HandleTextContent(const InlineTextBox &inlineTextBox, InlineItemList &inlineItemList,
                           Maybe<size_t> partialContentOffset) noexcept;
    KRYS_NODISCARD bool
      BuildInlineItemListForTextFromBreakingPositionsCache(const InlineTextBox &inlineTextBox,
                                                           InlineItemList &inlineItemList) noexcept;
    void HandleInlineBoxStart(const Box &inlineBox, InlineItemList &inlineItemList) noexcept;
    void HandleInlineBoxEnd(const Box &inlineBox, InlineItemList &inlineItemList) noexcept;
    void HandleInlineLevelBox(const Box &layoutBox, InlineItemList &inlineItemList) noexcept;

    KRYS_NODISCARD bool ContentRequiresVisualReordering() const noexcept
    {
      return _contentRequiresVisualReordering;
    }

    void ComputeInlineBoxBoundaryTextSpacings(const InlineItemList &inlineItemList) noexcept;

    KRYS_NODISCARD const ElementBox &Root() const noexcept
    {
      return _root;
    }
    KRYS_NODISCARD InlineContentCache &GetInlineContentCache() noexcept
    {
      return _inlineContentCache;
    }
  };
}
