#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo
{
  class RenderBlock;

  /// @brief The containing blocks of a selection root's descendants (for fixed, absolute and in-flow
  /// positioning), and the selection offsets they extend to, cached while walking the selection gaps.
  class LogicalSelectionOffsetCaches
  {
  public:
    /// @brief A containing block, and its cached left and right selection offsets.
    class ContainingBlockInfo
    {
    private:
      RenderBlock *_block {nullptr};
      const LogicalSelectionOffsetCaches *_cache {nullptr};
      bool _hasFloatsOrFragmentedFlows {false};
      mutable bool _cachedLogicalLeftSelectionOffset {false};
      mutable bool _cachedLogicalRightSelectionOffset {false};
      mutable LayoutUnit _logicalLeftSelectionOffset;
      mutable LayoutUnit _logicalRightSelectionOffset;

    public:
      ContainingBlockInfo() noexcept = default;

      inline void SetBlock(RenderBlock *block, const LogicalSelectionOffsetCaches *cache,
                           bool parentCacheHasFloatsOrFragmentedFlows = false) noexcept;
      KRYS_NODISCARD inline LayoutUnit LogicalLeftSelectionOffset(RenderBlock &rootBlock,
                                                                  LayoutUnit position) const noexcept;
      KRYS_NODISCARD inline LayoutUnit LogicalRightSelectionOffset(RenderBlock &rootBlock,
                                                                   LayoutUnit position) const noexcept;

      KRYS_NODISCARD RenderBlock *Block() const noexcept
      {
        return _block;
      }
      KRYS_NODISCARD const LogicalSelectionOffsetCaches *Cache() const noexcept
      {
        return _cache;
      }
      KRYS_NODISCARD bool HasFloatsOrFragmentedFlows() const noexcept
      {
        return _hasFloatsOrFragmentedFlows;
      }
    };

  private:
    ContainingBlockInfo _containingBlockForFixedPosition;
    ContainingBlockInfo _containingBlockForAbsolutePosition;
    ContainingBlockInfo _containingBlockForInflowPosition;

  public:
    inline explicit LogicalSelectionOffsetCaches(RenderBlock &rootBlock) noexcept;
    inline LogicalSelectionOffsetCaches(RenderBlock &block,
                                        const LogicalSelectionOffsetCaches &cache) noexcept;

    KRYS_NODISCARD inline const ContainingBlockInfo &
      GetContainingBlockInfo(RenderBlock &block) const noexcept;
  };
}
