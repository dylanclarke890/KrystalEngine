/*
 * Copyright (C) 2003, 2004, 2005, 2006, 2007 Apple Inc. All rights reserved.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "Krystal.Booey/Rendering/Line/LegacyInlineBox.hpp"

#include "Krystal.Booey/Graphics/Fonts/FontMetrics.hpp"
#include "Krystal.Booey/Rendering/Line/LegacyInlineFlowBox.hpp"
#include "Krystal.Booey/Rendering/Line/LegacyRootInlineBox.hpp"
#include "Krystal.Booey/Rendering/RenderBlockFlow.hpp"
#include "Krystal.Booey/Rendering/RenderBoxModelObject.hpp"
#include "Krystal.Booey/Rendering/RenderText.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle.hpp"

namespace krys::boo
{
  LegacyInlineBox::LegacyInlineBox(RenderObject &renderer) noexcept : _renderer(CreateWeakPtr(&renderer))
  {
  }

  const RenderStyle &LegacyInlineBox::LineStyle() const noexcept
  {
    return _firstLine ? Renderer().FirstLineStyle() : Renderer().Style();
  }

  const style::VerticalAlign &LegacyInlineBox::GetVerticalAlign() const noexcept
  {
    return LineStyle().GetVerticalAlign();
  }

  RenderBoxModelObject *LegacyInlineBox::BoxModelObject() const noexcept
  {
    if (!Is<RenderText>(Renderer()))
    {
      return &Downcast<RenderBoxModelObject>(Renderer());
    }
    return nullptr;
  }

  void LegacyInlineBox::RemoveFromParent() noexcept
  {
    if (Parent())
    {
      Parent()->RemoveChild(this);
    }
  }

  float LegacyInlineBox::LogicalHeight() const noexcept
  {
    if (HasVirtualLogicalHeight())
    {
      return VirtualLogicalHeight();
    }

    const RenderStyle &lineStyle = LineStyle();
    if (Renderer().IsRenderTextOrLineBreak())
    {
      return static_cast<float>(lineStyle.MetricsOfPrimaryFont().IntHeight());
    }

    krys_debug_assert(IsInlineFlowBox());
    RenderBoxModelObject *flowObject = BoxModelObject();
    const FontMetrics &fontMetrics = lineStyle.MetricsOfPrimaryFont();
    float result = static_cast<float>(fontMetrics.IntHeight());
    if (Parent())
    {
      result += flowObject->BorderAndPaddingLogicalHeight().ToFloat();
    }
    return result;
  }

  int32 LegacyInlineBox::CaretMinOffset() const noexcept
  {
    return Renderer().CaretMinOffset();
  }

  int32 LegacyInlineBox::CaretMaxOffset() const noexcept
  {
    return Renderer().CaretMaxOffset();
  }

  void LegacyInlineBox::DirtyLineBoxes() noexcept
  {
    MarkDirty();
    for (auto *curr = Parent(); curr && !curr->IsDirty(); curr = curr->Parent())
    {
      curr->MarkDirty();
    }
  }

  void LegacyInlineBox::AdjustPosition(float dx, float dy) noexcept
  {
    _topLeft.Move(dx, dy);
  }

  const LegacyRootInlineBox &LegacyInlineBox::Root() const noexcept
  {
    if (Parent())
    {
      return Parent()->Root();
    }
    return Downcast<LegacyRootInlineBox>(*this);
  }

  LegacyRootInlineBox &LegacyInlineBox::Root() noexcept
  {
    if (Parent())
    {
      return Parent()->Root();
    }
    return Downcast<LegacyRootInlineBox>(*this);
  }

  bool LegacyInlineBox::NextOnLineExists() const noexcept
  {
    if (!_determinedIfNextOnLineExists)
    {
      _determinedIfNextOnLineExists = true;

      if (!Parent())
      {
        _nextOnLineExists = false;
      }
      else if (NextOnLine())
      {
        _nextOnLineExists = true;
      }
      else
      {
        _nextOnLineExists = Parent()->NextOnLineExists();
      }
    }
    return _nextOnLineExists;
  }

  bool LegacyInlineBox::PreviousOnLineExists() const noexcept
  {
    if (!Parent())
    {
      return false;
    }
    if (PreviousOnLine())
    {
      return true;
    }
    return Parent()->PreviousOnLineExists();
  }

  LegacyInlineBox *LegacyInlineBox::NextLeafOnLine() const noexcept
  {
    LegacyInlineBox *leaf = nullptr;
    for (auto *box = NextOnLine(); box && !leaf; box = box->NextOnLine())
    {
      leaf = box->IsLeaf() ? box : Downcast<LegacyInlineFlowBox>(*box).FirstLeafDescendant();
    }
    if (!leaf && Parent())
    {
      leaf = Parent()->NextLeafOnLine();
    }
    return leaf;
  }

  LegacyInlineBox *LegacyInlineBox::PreviousLeafOnLine() const noexcept
  {
    LegacyInlineBox *leaf = nullptr;
    for (auto *box = PreviousOnLine(); box && !leaf; box = box->PreviousOnLine())
    {
      leaf = box->IsLeaf() ? box : Downcast<LegacyInlineFlowBox>(*box).LastLeafDescendant();
    }
    if (!leaf && Parent())
    {
      leaf = Parent()->PreviousLeafOnLine();
    }
    return leaf;
  }

  RenderObject::HighlightState LegacyInlineBox::SelectionState() const noexcept
  {
    return Renderer().SelectionState();
  }

  void LegacyInlineBox::ClearKnownToHaveNoOverflow() noexcept
  {
    _knownToHaveNoOverflow = false;
    if (Parent() && Parent()->KnownToHaveNoOverflow())
    {
      Parent()->ClearKnownToHaveNoOverflow();
    }
  }

  FloatPoint LegacyInlineBox::LocationIncludingFlipping() const noexcept
  {
    auto &rootContainer = Root().BlockFlow();
    auto writingMode = rootContainer.GetWritingMode();
    if (!writingMode.IsBlockFlipped())
    {
      return TopLeft();
    }
    if (writingMode.IsHorizontal())
    {
      return {X(), rootContainer.Height().ToFloat() - Height() - Y()};
    }
    return {rootContainer.Width().ToFloat() - Width() - X(), Y()};
  }

  void LegacyInlineBox::FlipForWritingMode(FloatRect &rect) const noexcept
  {
    auto &rootContainer = Root().BlockFlow();
    auto writingMode = rootContainer.GetWritingMode();
    if (!writingMode.IsBlockFlipped())
    {
      return;
    }
    rootContainer.FlipForWritingMode(rect);
  }

  FloatPoint LegacyInlineBox::FlipForWritingMode(const FloatPoint &point) const noexcept
  {
    auto &rootContainer = Root().BlockFlow();
    auto writingMode = rootContainer.GetWritingMode();
    if (!writingMode.IsBlockFlipped())
    {
      return point;
    }
    return rootContainer.FlipForWritingMode(point);
  }

  void LegacyInlineBox::FlipForWritingMode(LayoutRect &rect) const noexcept
  {
    auto &rootContainer = Root().BlockFlow();
    auto writingMode = rootContainer.GetWritingMode();
    if (!writingMode.IsBlockFlipped())
    {
      return;
    }
    rootContainer.FlipForWritingMode(rect);
  }

  LayoutPoint LegacyInlineBox::FlipForWritingMode(const LayoutPoint &point) const noexcept
  {
    auto &rootContainer = Root().BlockFlow();
    auto writingMode = rootContainer.GetWritingMode();
    if (!writingMode.IsBlockFlipped())
    {
      return point;
    }
    return rootContainer.FlipForWritingMode(point);
  }
}
