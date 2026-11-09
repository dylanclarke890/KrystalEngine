/**
 * Copyright (C) 2024 Apple Inc. All rights reserved.
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
