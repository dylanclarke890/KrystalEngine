/*
 * Copyright (C) 2019 Apple Inc. All rights reserved.
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

#include "Krystal.Booey/Graphics/Geometry/LayoutUnit.hpp"
#include "Krystal.Booey/Rendering/Line/LegacyInlineIterator.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LegacyInlineBox;
  class LegacyInlineFlowBox;
  class LegacyRootInlineBox;
  class LineInfo;
  class LocalFrameViewLayoutContext;
  class RenderBlockFlow;
  class RenderObject;
  class RenderStyle;

  /// @brief The legacy line layout, which lays out SVG text: one line of legacy inline boxes.
  class LegacyLineLayout
  {
    KRYS_NON_COPY_MOVABLE(LegacyLineLayout);

  private:
    RenderBlockFlow &_flow;
    UniquePtr<LegacyRootInlineBox> _legacyRootInlineBox;

  public:
    explicit LegacyLineLayout(RenderBlockFlow &) noexcept;
    ~LegacyLineLayout() noexcept;

    KRYS_NODISCARD LegacyRootInlineBox *LegacyRootBox() const noexcept
    {
      return _legacyRootInlineBox.get();
    }
    void DeleteLegacyRootBox(bool runCleanup = false) noexcept;

    void LayoutLineBoxes() noexcept;
    void ShiftLineBy(LayoutUnit shiftX, LayoutUnit shiftY) noexcept;

    LegacyRootInlineBox *ConstructLine(BidiRunList<BidiRun> &, const LineInfo &) noexcept;
    void AddOverflowFromInlineChildren() noexcept;

    KRYS_NODISCARD size_t LineCount() const noexcept;

    static void AppendRunsForObject(BidiRunList<BidiRun> *, int32 start, int32 end, RenderObject &,
                                    InlineBidiResolver &) noexcept;

    KRYS_NODISCARD static bool ShouldSkipCreatingRunsForObject(RenderObject &) noexcept;

  private:
    KRYS_NODISCARD UniquePtr<LegacyRootInlineBox> CreateRootInlineBox() noexcept;
    LegacyRootInlineBox *CreateAndAppendRootInlineBox() noexcept;
    LegacyInlineBox *CreateInlineBoxForRenderer(RenderObject *) noexcept;
    LegacyInlineFlowBox *CreateLineBoxes(RenderObject *, const LineInfo &, LegacyInlineBox *) noexcept;
    void RemoveInlineBox(BidiRun &, const LegacyRootInlineBox &) const noexcept;
    void RemoveEmptyTextBoxesAndUpdateVisualReordering(LegacyRootInlineBox *, BidiRun *firstRun) noexcept;
    LegacyRootInlineBox *CreateLineBoxesFromBidiRuns(uint32 bidiLevel, BidiRunList<BidiRun> &bidiRuns,
                                                     const LegacyInlineIterator &end, LineInfo &) noexcept;
    void LayoutRunsAndFloats(bool hasInlineChild) noexcept;
    void LayoutRunsAndFloatsInRange(InlineBidiResolver &) noexcept;

    KRYS_NODISCARD const RenderStyle &Style() const noexcept;
    KRYS_NODISCARD const LocalFrameViewLayoutContext &LayoutContext() const noexcept;
  };
}
