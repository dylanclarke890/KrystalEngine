#include "Krystal.Booey/Rendering/RenderLineBoxList.hpp"

#include "Krystal.Booey/Rendering/LegacyInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInline.hpp"

namespace krys::boo
{
  RenderLineBoxList::~RenderLineBoxList() noexcept
  {
    krys_debug_assert(!_firstLineBox);
    krys_debug_assert(!_lastLineBox);
  }

  void RenderLineBoxList::AppendLineBox(UniquePtr<LegacyInlineFlowBox> box) noexcept
  {
    LegacyInlineFlowBox *boxPtr = box.release();

    if (!_firstLineBox)
    {
      _firstLineBox = boxPtr;
      _lastLineBox = boxPtr;
    }
    else
    {
      _lastLineBox->SetNextLineBox(boxPtr);
      boxPtr->SetPreviousLineBox(_lastLineBox);
      _lastLineBox = boxPtr;
    }
  }

  void RenderLineBoxList::DeleteLineBoxTree() noexcept
  {
    LegacyInlineFlowBox *line = _firstLineBox;
    LegacyInlineFlowBox *nextLine;
    while (line)
    {
      nextLine = line->NextLineBox();
      line->DeleteLine();
      line = nextLine;
    }
    _firstLineBox = _lastLineBox = nullptr;
  }

  void RenderLineBoxList::RemoveLineBox(LegacyInlineFlowBox *box) noexcept
  {
    if (box == _firstLineBox)
    {
      _firstLineBox = box->NextLineBox();
    }
    if (box == _lastLineBox)
    {
      _lastLineBox = box->PrevLineBox();
    }
    if (box->NextLineBox())
    {
      box->NextLineBox()->SetPreviousLineBox(box->PrevLineBox());
    }
    if (box->PrevLineBox())
    {
      box->PrevLineBox()->SetNextLineBox(box->NextLineBox());
    }
  }

  void RenderLineBoxList::DeleteLineBoxes() noexcept
  {
    if (_firstLineBox)
    {
      LegacyInlineFlowBox *next;
      for (auto *curr = _firstLineBox; curr; curr = next)
      {
        next = curr->NextLineBox();
        delete curr;
      }
      _firstLineBox = nullptr;
      _lastLineBox = nullptr;
    }
  }

  void RenderLineBoxList::DirtyLineBoxes() noexcept
  {
    for (auto *curr = FirstLegacyLineBox(); curr; curr = curr->NextLineBox())
    {
      curr->DirtyLineBoxes();
    }
  }

  void RenderLineBoxList::ShiftLinesBy(LayoutUnit shiftX, LayoutUnit shiftY) noexcept
  {
    for (auto *box = FirstLegacyLineBox(); box; box = box->NextLineBox())
    {
      box->AdjustPosition(shiftX.ToFloat(), shiftY.ToFloat());
    }
  }

  void RenderLineBoxList::DirtyLineFromChangedChild(RenderBoxModelObject &container) noexcept
  {
    krys_debug_assert(Is<RenderInline>(container) || Is<RenderBlockFlow>(container));
    if (!container.IsSVGRenderer())
    {
      return;
    }

    if (!container.Parent() || (Is<RenderBlockFlow>(container) && container.SelfNeedsLayout()))
    {
      return;
    }

    auto *inlineContainer = DynamicDowncast<RenderSVGInline>(container);
    if (auto *lineBox = inlineContainer ? inlineContainer->FirstLegacyInlineBox() : FirstLegacyLineBox())
    {
      lineBox->Root().MarkDirty();
      return;
    }
    // For an empty inline, propagate the check up to our parent.
    if (inlineContainer && inlineContainer->EverHadLayout())
    {
      auto *parent = inlineContainer->Parent();
      parent->DirtyLineFromChangedChild();
      parent->SetNeedsLayout();
    }
  }
}
