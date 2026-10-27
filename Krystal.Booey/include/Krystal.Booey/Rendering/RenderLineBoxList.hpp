#pragma once

#include "Krystal.Booey/Core/LayoutUnit.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/UniquePtr.hpp"

namespace krys::boo
{
  class LegacyInlineFlowBox;
  class RenderBoxModelObject;

  /// @brief The legacy inline boxes an inline renderer has, one for each line it's on.
  class RenderLineBoxList
  {
    KRYS_NON_COPY_MOVABLE(RenderLineBoxList);

  private:
    // For block flows, each box represents the root inline box for a line in the paragraph.
    // For inline flows, each box represents a portion of that inline.
    LegacyInlineFlowBox *_firstLineBox {nullptr};
    LegacyInlineFlowBox *_lastLineBox {nullptr};

  public:
    RenderLineBoxList() noexcept = default;
    ~RenderLineBoxList() noexcept;

    KRYS_NODISCARD LegacyInlineFlowBox *FirstLegacyLineBox() const noexcept
    {
      return _firstLineBox;
    }
    KRYS_NODISCARD LegacyInlineFlowBox *LastLegacyLineBox() const noexcept
    {
      return _lastLineBox;
    }

    void AppendLineBox(UniquePtr<LegacyInlineFlowBox>) noexcept;

    void DeleteLineBoxTree() noexcept;
    void DeleteLineBoxes() noexcept;

    void RemoveLineBox(LegacyInlineFlowBox *) noexcept;

    void DirtyLineBoxes() noexcept;
    void DirtyLineFromChangedChild(RenderBoxModelObject &parent) noexcept;
    void ShiftLinesBy(LayoutUnit shiftX, LayoutUnit shiftY) noexcept;
  };
}
