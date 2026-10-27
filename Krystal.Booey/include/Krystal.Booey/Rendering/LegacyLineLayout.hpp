#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Booey/Rendering/LegacyInlineIterator.hpp"
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
