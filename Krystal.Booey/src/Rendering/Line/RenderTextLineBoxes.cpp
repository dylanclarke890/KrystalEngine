/*
 * Copyright (C) 2013 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Rendering/Line/RenderTextLineBoxes.hpp"

#include "Krystal.Booey/Rendering/Line/LegacyInlineTextBox.hpp"
#include "Krystal.Booey/Rendering/RenderElement.hpp"
#include "Krystal.Booey/Rendering/SVG/RenderSVGInlineText.hpp"

namespace krys::boo
{
  RenderTextLineBoxes::~RenderTextLineBoxes() noexcept
  {
    krys_debug_assert(!_first);
    krys_debug_assert(!_last);
  }

  LegacyInlineTextBox *RenderTextLineBoxes::CreateAndAppendLineBox(RenderSVGInlineText &renderText) noexcept
  {
    auto textBox = renderText.CreateTextBox();
    if (!_first)
    {
      _first = textBox.get();
      _last = textBox.get();
    }
    else
    {
      _last->SetNextTextBox(textBox.get());
      textBox->SetPreviousTextBox(_last);
      _last = textBox.get();
    }
    return textBox.release();
  }

  void RenderTextLineBoxes::Remove(LegacyInlineTextBox &box) noexcept
  {
    if (&box == _first)
    {
      _first = box.NextTextBox();
    }
    if (&box == _last)
    {
      _last = box.PrevTextBox();
    }
    if (box.NextTextBox())
    {
      box.NextTextBox()->SetPreviousTextBox(box.PrevTextBox());
    }
    if (box.PrevTextBox())
    {
      box.PrevTextBox()->SetNextTextBox(box.NextTextBox());
    }
  }

  void RenderTextLineBoxes::RemoveAllFromParent(RenderSVGInlineText &renderer) noexcept
  {
    if (!_first)
    {
      if (renderer.Parent())
      {
        renderer.Parent()->DirtyLineFromChangedChild();
      }
      return;
    }
    for (auto *box = _first; box; box = box->NextTextBox())
    {
      box->RemoveFromParent();
    }
  }

  void RenderTextLineBoxes::DeleteAll() noexcept
  {
    if (!_first)
    {
      return;
    }
    LegacyInlineTextBox *next;
    for (auto *current = _first; current; current = next)
    {
      next = current->NextTextBox();
      delete current;
    }
    _first = nullptr;
    _last = nullptr;
  }

  void RenderTextLineBoxes::DirtyForTextChange(RenderSVGInlineText &renderer) noexcept
  {
    for (auto *box = _first; box; box = box->NextTextBox())
    {
      box->DirtyLineBoxes();
    }

    if (!_first && renderer.Parent())
    {
      renderer.Parent()->DirtyLineFromChangedChild();
    }
  }
}
