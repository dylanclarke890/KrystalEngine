#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Breaks the mail blockquote the caret is in, leaving a line break between the two halves.
  class BreakBlockquoteCommand final : public CompositeEditCommand
  {
  public:
    explicit BreakBlockquoteCommand(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<BreakBlockquoteCommand> Create(dom::Document &document) noexcept;

  private:
    void DoApply() noexcept override;
  };
}
