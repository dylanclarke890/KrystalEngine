#include "Krystal.Booey/Rendering/RenderTextLineBoxes.hpp"

#include "Krystal.Booey/Rendering/LegacyInlineTextBox.hpp"
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
