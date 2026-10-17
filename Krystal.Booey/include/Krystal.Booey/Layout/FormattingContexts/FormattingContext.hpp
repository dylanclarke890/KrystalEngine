#pragma once

#include "Krystal.Booey/Layout/LayoutTree/LayoutElementBox.hpp"
#include "Krystal.Core/Attributes.hpp"

namespace krys::boo::layout
{
  class InitialContainingBlock;

  /// @brief Queries about where a layout box lives: its containing block and its formatting context.
  // FIXME: The formatting context itself (layout, intrinsic widths, the geometry access checks), once the
  // block, table or flex formatting contexts are ported. Only the inline formatting context exists, and it
  // isn't one of these.
  class FormattingContext
  {
  public:
    FormattingContext() = delete;

    KRYS_NODISCARD static const InitialContainingBlock &
      GetInitialContainingBlock(const Box &layoutBox) noexcept;
    KRYS_NODISCARD static const ElementBox &ContainingBlock(const Box &layoutBox) noexcept;
#if KRYS_DEBUG
    KRYS_NODISCARD static const ElementBox &FormattingContextRoot(const Box &layoutBox) noexcept;
#endif
  };
}
