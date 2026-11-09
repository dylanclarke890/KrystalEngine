/*
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

#include "Krystal.Booey/Rendering/Style/RenderStyleConstants.hpp"
#include "Krystal.Booey/Text/TextFlags.hpp"
#include "Krystal.Core/Attributes.hpp"
#include "Krystal.Core/Hash.hpp"
#include "Krystal.Core/Types/StringAtom.hpp"

namespace krys::boo
{
  class RenderStyle;
}

namespace krys::boo::layout
{
  enum class WhiteSpaceCollapseBehavior : uint8
  {
    CollapseAll,
    CollapseWhitespaceSequence,
    Preserve
  };

  KRYS_NODISCARD constexpr WhiteSpaceCollapseBehavior
    BehaviorForWhiteSpaceCollapse(WhiteSpaceCollapse collapse) noexcept
  {
    switch (collapse)
    {
      case WhiteSpaceCollapse::Collapse:       return WhiteSpaceCollapseBehavior::CollapseAll;
      case WhiteSpaceCollapse::PreserveBreaks: return WhiteSpaceCollapseBehavior::CollapseWhitespaceSequence;
      case WhiteSpaceCollapse::Preserve:
      case WhiteSpaceCollapse::BreakSpaces:
        // FIXME: WhiteSpaceCollapse::PreserveSpaces should also be handled this way.
        // WhiteSpaceCollapse::Preserve and WhiteSpaceCollapse::BreakSpaces
        // have the same text breaking positions so we don't need to recalculate
        // breaking points when switching between these behaviors.
        return WhiteSpaceCollapseBehavior::Preserve;
    }
    krys_unreachable();
  }

  /// @brief The style that decides where text can break, which breaking positions are cached under.
  struct TextBreakingPositionContext
  {
    WhiteSpaceCollapseBehavior WhitespaceCollapseBehavior {WhiteSpaceCollapseBehavior::CollapseAll};
    boo::OverflowWrap OverflowWrap {boo::OverflowWrap::Normal};
    boo::LineBreak LineBreak {boo::LineBreak::Normal};
    boo::WordBreak WordBreak {boo::WordBreak::Normal};
    StringAtom Locale {StringAtom::Null()};

    explicit TextBreakingPositionContext(const RenderStyle &style) noexcept;
    TextBreakingPositionContext() noexcept = default;

    KRYS_NODISCARD friend bool operator==(const TextBreakingPositionContext &,
                                          const TextBreakingPositionContext &) noexcept = default;
  };
}

namespace krys
{
  template <>
  struct DefaultHash<boo::layout::TextBreakingPositionContext>
  {
    KRYS_NODISCARD static size_t Hash(const boo::layout::TextBreakingPositionContext &context) noexcept
    {
      return Hash::Combine(context.WhitespaceCollapseBehavior, context.OverflowWrap, context.LineBreak,
                           context.WordBreak, context.Locale);
    }
  };
}
