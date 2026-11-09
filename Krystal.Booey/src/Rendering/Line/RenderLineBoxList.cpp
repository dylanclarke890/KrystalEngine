/*
 * Copyright (C) 2008 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Line/RenderLineBoxList.hpp"

#include "Krystal.Booey/Rendering/Line/LegacyInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/Line/LegacyRootInlineBox.hpp"
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
