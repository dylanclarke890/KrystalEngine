#pragma once

#include "Krystal.Booey/Rendering/LogicalSelectionOffsetCaches.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"

namespace krys::boo
{
  void LogicalSelectionOffsetCaches::ContainingBlockInfo::SetBlock(
    RenderBlock *block, const LogicalSelectionOffsetCaches *cache,
    bool parentCacheHasFloatsOrFragmentedFlows) noexcept
  {
    _block = block;
    // FIXME: Fragmented flows (enclosingFragmentedFlow), once there are any.
    bool blockHasFloatsOrFragmentedFlows = _block ? _block->ContainsFloats() : false;
    _hasFloatsOrFragmentedFlows =
      parentCacheHasFloatsOrFragmentedFlows || _hasFloatsOrFragmentedFlows || blockHasFloatsOrFragmentedFlows;
    _cache = cache;
    _cachedLogicalLeftSelectionOffset = false;
    _cachedLogicalRightSelectionOffset = false;
  }

  LayoutUnit LogicalSelectionOffsetCaches::ContainingBlockInfo::LogicalLeftSelectionOffset(
    RenderBlock &rootBlock, LayoutUnit position) const noexcept
  {
    krys_debug_assert(_cache);
    if (_hasFloatsOrFragmentedFlows || !_cachedLogicalLeftSelectionOffset)
    {
      _cachedLogicalLeftSelectionOffset = true;
      _logicalLeftSelectionOffset =
        _block ? _block->LogicalLeftSelectionOffset(rootBlock, position, *_cache) : 0_lu;
    }
    else
    {
      krys_debug_assert(
        _logicalLeftSelectionOffset
        == (_block ? _block->LogicalLeftSelectionOffset(rootBlock, position, *_cache) : 0_lu));
    }
    return _logicalLeftSelectionOffset;
  }

  LayoutUnit LogicalSelectionOffsetCaches::ContainingBlockInfo::LogicalRightSelectionOffset(
    RenderBlock &rootBlock, LayoutUnit position) const noexcept
  {
    krys_debug_assert(_cache);
    if (_hasFloatsOrFragmentedFlows || !_cachedLogicalRightSelectionOffset)
    {
      _cachedLogicalRightSelectionOffset = true;
      _logicalRightSelectionOffset =
        _block ? _block->LogicalRightSelectionOffset(rootBlock, position, *_cache) : 0_lu;
    }
    else
    {
      krys_debug_assert(
        _logicalRightSelectionOffset
        == (_block ? _block->LogicalRightSelectionOffset(rootBlock, position, *_cache) : 0_lu));
    }
    return _logicalRightSelectionOffset;
  }

  LogicalSelectionOffsetCaches::LogicalSelectionOffsetCaches(RenderBlock &rootBlock) noexcept
  {
    // FIXME: We should either move this assertion to the caller (if applicable) or structure the code
    // such that we can remove this assertion.
    krys_debug_assert(rootBlock.IsSelectionRoot());
    // LogicalSelectionOffsetCaches should not be used on an orphaned tree.
    _containingBlockForFixedPosition.SetBlock(
      RenderObject::ContainingBlockForPositionType(PositionType::Fixed, rootBlock), nullptr);
    _containingBlockForAbsolutePosition.SetBlock(
      RenderObject::ContainingBlockForPositionType(PositionType::Absolute, rootBlock), nullptr);
    _containingBlockForInflowPosition.SetBlock(
      RenderObject::ContainingBlockForPositionType(PositionType::Static, rootBlock), nullptr);
  }

  LogicalSelectionOffsetCaches::LogicalSelectionOffsetCaches(
    RenderBlock &block, const LogicalSelectionOffsetCaches &cache) noexcept
      : _containingBlockForFixedPosition(cache._containingBlockForFixedPosition),
        _containingBlockForAbsolutePosition(cache._containingBlockForAbsolutePosition)
  {
    if (block.CanContainFixedPositionObjects())
    {
      _containingBlockForFixedPosition.SetBlock(
        &block, &cache, cache._containingBlockForFixedPosition.HasFloatsOrFragmentedFlows());
    }

    if (block.CanContainAbsolutelyPositionedObjects() && !block.IsRenderInline() && !block.IsAnonymousBlock())
    {
      _containingBlockForAbsolutePosition.SetBlock(
        &block, &cache, cache._containingBlockForAbsolutePosition.HasFloatsOrFragmentedFlows());
    }

    _containingBlockForInflowPosition.SetBlock(
      &block, &cache, cache._containingBlockForInflowPosition.HasFloatsOrFragmentedFlows());
  }

  auto LogicalSelectionOffsetCaches::GetContainingBlockInfo(RenderBlock &block) const noexcept
    -> const ContainingBlockInfo &
  {
    auto position = block.Style().GetPosition();
    if (position == PositionType::Fixed)
    {
      krys_debug_assert(block.ContainingBlock() == _containingBlockForFixedPosition.Block());
      return _containingBlockForFixedPosition;
    }
    if (position == PositionType::Absolute)
    {
      krys_debug_assert(block.ContainingBlock() == _containingBlockForAbsolutePosition.Block());
      return _containingBlockForAbsolutePosition;
    }
    krys_debug_assert(block.ContainingBlock() == _containingBlockForInflowPosition.Block());
    return _containingBlockForInflowPosition;
  }
}
