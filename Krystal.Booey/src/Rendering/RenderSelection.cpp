#include "Krystal.Booey/Rendering/RenderSelection.hpp"
#include "Krystal.Booey/DOM/Document.hpp"
#include "Krystal.Booey/Editing/FrameSelection.hpp"
#include "Krystal.Booey/Rendering/RenderBlock.hpp"
#include "Krystal.Booey/Rendering/RenderLayer.hpp"
#include "Krystal.Booey/Rendering/RenderLayerModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderSelectionGeometry.hpp"
#include "Krystal.Booey/Rendering/RenderView.hpp"
#include "Krystal.Core/Types/HashMap.hpp"
#include "Krystal.Core/Types/HashSet.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  namespace
  {
    struct SelectionContext
    {
      using RendererMap = HashMap<RenderObject *, UniquePtr<RenderSelectionGeometry>>;
      using RenderBlockMap = HashMap<const RenderBlock *, UniquePtr<RenderBlockSelectionGeometry>>;

      uint32 StartOffset;
      uint32 EndOffset;
      RendererMap Renderers;
      RenderBlockMap Blocks;
    };

    KRYS_NODISCARD bool IsValidRendererForSelection(const RenderObject &renderer,
                                                    const RenderRange &selection) noexcept
    {
      if (!renderer.ContainingBlock())
      {
        return false;
      }

      if (renderer.IsSkippedContent())
      {
        return false;
      }

      if (renderer.SelectionState() == RenderObject::HighlightState::None)
      {
        return false;
      }

      return renderer.CanBeSelectionLeaf() || &renderer == selection.Start() || &renderer == selection.End();
    }

    KRYS_NODISCARD RenderBlock *ContainingBlockBelowView(const RenderObject &renderer) noexcept
    {
      auto *containingBlock = renderer.ContainingBlock();
      return Is<RenderView>(containingBlock) ? nullptr : containingBlock;
    }

    KRYS_NODISCARD SelectionContext CollectSelectionData(const RenderRange &selection,
                                                         bool repaintDifference) noexcept
    {
      SelectionContext oldSelectionData {selection.StartOffset(), selection.EndOffset(), {}, {}};
      // Blocks contain selected objects and fill gaps between them, either on the left, right, or in between
      // lines and blocks. In order to get the repaint rect right, we have to examine left, middle, and right
      // rects individually, since otherwise the union of those rects might remain the same even when changes
      // have occurred.
      auto *start = selection.Start();
      RenderObject *stop = nullptr;
      if (auto *selectionEnd = selection.End())
      {
        stop = RendererAfterOffset(*selectionEnd, selection.EndOffset());
      }
      RenderRangeIterator selectionIterator(start);
      while (start && start != stop)
      {
        if (IsValidRendererForSelection(*start, selection))
        {
          // Blocks are responsible for painting line gaps and margin gaps. They must be examined as well.
          oldSelectionData.Renderers.insert_or_assign(start,
                                                      CreateUnique<RenderSelectionGeometry>(*start, true));
          if (repaintDifference)
          {
            for (auto *block = ContainingBlockBelowView(*start); block;
                 block = ContainingBlockBelowView(*block))
            {
              auto &blockInfo = oldSelectionData.Blocks[block];
              if (blockInfo)
              {
                break;
              }
              blockInfo = CreateUnique<RenderBlockSelectionGeometry>(*block);
            }
          }
        }
        start = selectionIterator.Next();
      }
      return oldSelectionData;
    }
  }

  RenderSelection::RenderSelection(RenderView &view) noexcept
      : RenderHighlight(IsSelection), _renderView(view)
  {
  }

  void RenderSelection::Set(const RenderRange &selection, RepaintMode blockRepaintMode) noexcept
  {
    if ((selection.Start() && !selection.End()) || (selection.End() && !selection.Start()))
    {
      return;
    }
    // Just return if the selection hasn't changed.
    auto isCaret = _renderView.GetDocument().Selection().IsCaret();
    if (selection == _renderRange && _selectionWasCaret == isCaret)
    {
      return;
    }
    _selectionWasCaret = isCaret;
    Apply(selection, blockRepaintMode);
  }

  void RenderSelection::Clear() noexcept
  {
    if (!_selectionWasCaret)
    {
      _renderView.Layer()->RepaintBlockSelectionGaps();
    }
    Set({}, RepaintMode::NewMinusOld);
  }

  void RenderSelection::Repaint() const noexcept
  {
    HashSet<RenderBlock *> processedBlocks;
    RenderObject *end = nullptr;
    if (auto *rangeEnd = _renderRange.End())
    {
      end = RendererAfterOffset(*rangeEnd, _renderRange.EndOffset());
    }
    RenderRangeIterator highlightIterator(_renderRange.Start());
    for (auto *renderer = highlightIterator.Current(); renderer && renderer != end;
         renderer = highlightIterator.Next())
    {
      if (!IsValidRendererForSelection(*renderer, _renderRange))
      {
        continue;
      }
      RenderSelectionGeometry(*renderer, true).Repaint();
      // Blocks are responsible for painting line gaps and margin gaps. They must be examined as well.
      for (auto *block = ContainingBlockBelowView(*renderer); block; block = ContainingBlockBelowView(*block))
      {
        if (!processedBlocks.insert(block).second)
        {
          break;
        }
        RenderSelectionGeometry(*block, true).Repaint();
      }
    }
  }

  IntRect RenderSelection::CollectBounds(ClipToVisibleContent clipToVisibleContent) const noexcept
  {
    SelectionContext::RendererMap renderers;
    auto *start = _renderRange.Start();
    RenderObject *stop = nullptr;
    if (auto *rangeEnd = _renderRange.End())
    {
      stop = RendererAfterOffset(*rangeEnd, _renderRange.EndOffset());
    }

    RenderRangeIterator selectionIterator(start);
    while (start && start != stop)
    {
      if (IsValidRendererForSelection(*start, _renderRange))
      {
        // Blocks are responsible for painting line gaps and margin gaps. They must be examined as well.
        renderers.insert_or_assign(start, CreateUnique<RenderSelectionGeometry>(
                                            *start, clipToVisibleContent == ClipToVisibleContent::Yes));

        auto *block = start->ContainingBlock();
        while (block && !Is<RenderView>(*block))
        {
          auto &blockSelectionGeometry = renderers[block];
          if (blockSelectionGeometry)
          {
            break;
          }
          blockSelectionGeometry =
            CreateUnique<RenderSelectionGeometry>(*block, clipToVisibleContent == ClipToVisibleContent::Yes);
          block = block->ContainingBlock();
        }
      }
      start = selectionIterator.Next();
    }

    // Now create a single bounding box rect that encloses the whole selection.
    LayoutRect selectionRect;
    for (const auto &[renderer, selectionGeometry] : renderers)
    {
      // RenderSelectionGeometry::Rect() is in the coordinates of the repaintContainer, so map to page
      // coordinates.
      LayoutRect currentRect = selectionGeometry->Rect();
      if (currentRect.IsEmpty())
      {
        continue;
      }

      if (auto *repaintContainer = selectionGeometry->RepaintContainer())
      {
        FloatRect localRect = currentRect;
        FloatQuad absQuad = repaintContainer->LocalToAbsoluteQuad(localRect);
        currentRect = LayoutRect(absQuad.EnclosingBoundingBox());
      }
      selectionRect.Unite(currentRect);
    }

    return SnappedIntRect(selectionRect);
  }

  void RenderSelection::Apply(const RenderRange &newSelection, RepaintMode blockRepaintMode) noexcept
  {
    auto oldSelectionData = CollectSelectionData(_renderRange, blockRepaintMode == RepaintMode::NewXOROld);
    // Remove current selection.
    for (const auto &[renderer, geometry] : oldSelectionData.Renderers)
    {
      renderer->SetSelectionStateIfNeeded(RenderObject::HighlightState::None);
    }
    _renderRange = newSelection;
    auto *selectionStart = _renderRange.Start();
    // Update the selection status of all objects between selectionStart and selectionEnd
    if (selectionStart && selectionStart == _renderRange.End())
    {
      selectionStart->SetSelectionStateIfNeeded(RenderObject::HighlightState::Both);
    }
    else
    {
      if (selectionStart)
      {
        selectionStart->SetSelectionStateIfNeeded(RenderObject::HighlightState::Start);
      }
      if (auto *end = _renderRange.End())
      {
        end->SetSelectionStateIfNeeded(RenderObject::HighlightState::End);
      }
    }

    RenderObject *selectionEnd = nullptr;
    if (auto *selectionDataEnd = _renderRange.End())
    {
      selectionEnd = RendererAfterOffset(*selectionDataEnd, _renderRange.EndOffset());
    }
    RenderRangeIterator selectionIterator(selectionStart);
    for (auto *currentRenderer = selectionStart; currentRenderer && currentRenderer != selectionEnd;
         currentRenderer = selectionIterator.Next())
    {
      if (currentRenderer == selectionStart || currentRenderer == _renderRange.End())
      {
        continue;
      }
      if (!currentRenderer->CanBeSelectionLeaf() || currentRenderer->IsSkippedContent())
      {
        continue;
      }
      currentRenderer->SetSelectionStateIfNeeded(RenderObject::HighlightState::Inside);
    }

    if (blockRepaintMode != RepaintMode::Nothing)
    {
      _renderView.Layer()->ClearBlockSelectionGapsBounds();
    }

    // Now that the selection state has been updated for the new objects, walk them again and
    // put them in the new objects list.
    SelectionContext::RendererMap newSelectedRenderers;
    SelectionContext::RenderBlockMap newSelectedBlocks;
    selectionIterator = RenderRangeIterator(selectionStart);
    for (auto *currentRenderer = selectionStart; currentRenderer && currentRenderer != selectionEnd;
         currentRenderer = selectionIterator.Next())
    {
      if (IsValidRendererForSelection(*currentRenderer, _renderRange))
      {
        newSelectedRenderers.insert_or_assign(currentRenderer,
                                              CreateUnique<RenderSelectionGeometry>(*currentRenderer, true));
        auto *containingBlock = currentRenderer->ContainingBlock();
        while (containingBlock && !Is<RenderView>(*containingBlock))
        {
          auto &blockSelectionGeometry = newSelectedBlocks[containingBlock];
          if (blockSelectionGeometry)
          {
            break;
          }
          blockSelectionGeometry = CreateUnique<RenderBlockSelectionGeometry>(*containingBlock);
          containingBlock = containingBlock->ContainingBlock();
        }
      }
    }

    if (blockRepaintMode == RepaintMode::Nothing)
    {
      return;
    }

    // Have any of the old selected objects changed compared to the new selection?
    for (const auto &[renderer, oldInfo] : oldSelectionData.Renderers)
    {
      auto newIt = newSelectedRenderers.find(renderer);
      auto *newInfo = newIt != newSelectedRenderers.end() ? newIt->second.get() : nullptr;
      if (!newInfo || oldInfo->Rect() != newInfo->Rect() || oldInfo->State() != newInfo->State()
          || (_renderRange.Start() == renderer && oldSelectionData.StartOffset != _renderRange.StartOffset())
          || (_renderRange.End() == renderer && oldSelectionData.EndOffset != _renderRange.EndOffset()))
      {
        oldInfo->Repaint();
        if (newInfo)
        {
          newInfo->Repaint();
          newSelectedRenderers.erase(newIt);
        }
      }
    }

    // Any new objects that remain were not found in the old objects dict, and so they need to be updated.
    for (const auto &[renderer, newInfo] : newSelectedRenderers)
    {
      newInfo->Repaint();
    }

    // Have any of the old blocks changed?
    for (const auto &[block, oldInfo] : oldSelectionData.Blocks)
    {
      auto newIt = newSelectedBlocks.find(block);
      auto *newInfo = newIt != newSelectedBlocks.end() ? newIt->second.get() : nullptr;
      if (!newInfo || oldInfo->Rects() != newInfo->Rects() || oldInfo->State() != newInfo->State())
      {
        oldInfo->Repaint();
        if (newInfo)
        {
          newInfo->Repaint();
          newSelectedBlocks.erase(newIt);
        }
      }
    }

    // Any new blocks that remain were not found in the old blocks dict, and so they need to be updated.
    for (const auto &[block, newInfo] : newSelectedBlocks)
    {
      newInfo->Repaint();
    }
  }
}
