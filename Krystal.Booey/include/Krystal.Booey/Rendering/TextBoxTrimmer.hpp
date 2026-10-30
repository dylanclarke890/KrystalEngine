#pragma once

#include "Krystal.Booey/Page/LocalFrameViewLayoutContext.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Macros.hpp"
#include "Krystal.Core/Types/Maybe.hpp"

namespace krys::boo
{
  class RenderBlockFlow;
  class RenderBox;

  /// @brief Sets up `text-box-trim` for a block container's subtree while it's laid out: which formatted
  /// lines get their start and end trimmed. Restores the enclosing trimming state when it goes out of scope.
  class TextBoxTrimmer
  {
    KRYS_NON_COPY_MOVABLE(TextBoxTrimmer);

  private:
    const RenderBlockFlow *_blockContainer;
    Maybe<LocalFrameViewLayoutContext::TextBoxTrim> _previousTextBoxTrimStatus {};
    bool _shouldRestoreTextBoxTrimStatus {false};

  public:
    explicit TextBoxTrimmer(const RenderBlockFlow &blockContainer) noexcept;
    TextBoxTrimmer(const RenderBlockFlow &blockContainer,
                   const RenderBlockFlow &lastFormattedLineRoot) noexcept;
    ~TextBoxTrimmer() noexcept;

    KRYS_NODISCARD static RenderBlockFlow *
      LastInlineFormattingContextRootForTrimEnd(const RenderBlockFlow &blockContainer) noexcept;

  private:
    void AdjustTextBoxTrimStatusBeforeLayout(const RenderBlockFlow *lastFormattedLineRoot) noexcept;
    void AdjustTextBoxTrimStatusAfterLayout() noexcept;
    void HandleTextBoxTrimNoneBeforeLayout() noexcept;
  };

  /// @brief Stops `text-box-trim` from trimming the start of a block laid out inside inline content (a block
  /// that isn't on the first line), restoring the trimming state when it goes out of scope.
  class TextBoxTrimStartDisabler
  {
    KRYS_NON_COPY_MOVABLE(TextBoxTrimStartDisabler);

  private:
    const RenderBox *_renderBox;
    Maybe<LocalFrameViewLayoutContext::TextBoxTrim> _previousTextBoxTrimStatus {};

  public:
    explicit TextBoxTrimStartDisabler(const RenderBox &renderBox) noexcept;
    ~TextBoxTrimStartDisabler() noexcept;
  };
}
