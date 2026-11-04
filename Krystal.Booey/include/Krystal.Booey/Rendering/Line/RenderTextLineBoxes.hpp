#pragma once

#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"

namespace krys::boo
{
  class LegacyInlineTextBox;
  class RenderSVGInlineText;

  /// @brief The legacy text boxes an SVG text renderer has.
  class RenderTextLineBoxes
  {
    KRYS_NON_COPY_MOVABLE(RenderTextLineBoxes);

  private:
    LegacyInlineTextBox *_first {nullptr};
    LegacyInlineTextBox *_last {nullptr};

  public:
    RenderTextLineBoxes() noexcept = default;
    ~RenderTextLineBoxes() noexcept;

    KRYS_NODISCARD LegacyInlineTextBox *First() const noexcept
    {
      return _first;
    }
    KRYS_NODISCARD LegacyInlineTextBox *Last() const noexcept
    {
      return _last;
    }

    KRYS_NODISCARD LegacyInlineTextBox *CreateAndAppendLineBox(RenderSVGInlineText &) noexcept;

    void Remove(LegacyInlineTextBox &) noexcept;

    void RemoveAllFromParent(RenderSVGInlineText &) noexcept;
    void DeleteAll() noexcept;

    void DirtyForTextChange(RenderSVGInlineText &) noexcept;
  };
}
