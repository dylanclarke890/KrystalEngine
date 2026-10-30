#pragma once

#include "Krystal.Booey/Editing/CompositeEditCommand.hpp"

namespace krys::boo
{
  /// @brief Replaces the selection with a line break: a `br`, or a newline where newlines are preserved.
  class InsertLineBreakCommand final : public CompositeEditCommand
  {
  public:
    explicit InsertLineBreakCommand(dom::Document &document) noexcept;

    KRYS_NODISCARD static Ref<InsertLineBreakCommand> Create(dom::Document &document) noexcept;

  private:
    void DoApply() noexcept override;
    KRYS_NODISCARD bool PreservesTypingStyle() const noexcept override;
    KRYS_NODISCARD bool ShouldUseBreakElement(const dom::Position &position) noexcept;
  };
}
