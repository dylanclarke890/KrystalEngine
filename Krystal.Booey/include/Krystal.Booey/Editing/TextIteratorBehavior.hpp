/*
 * Copyright (C) 2004, 2006, 2009, 2014 Apple Inc. All rights reserved.
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
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL APPLE INC. OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#pragma once

#include "Krystal.Core/Enum.hpp"
#include "Krystal.Core/Numeric.hpp"
#include "Krystal.Core/Types/EnumSet.hpp"

namespace krys::boo
{
  /// @brief How a text iterator walks a range and what it emits.
  enum class TextIteratorBehavior : uint8
  {
    // Used by selection preservation code. There should be one character emitted between every
    // VisiblePosition in the Range used to create the TextIterator.
    // FIXME <rdar://problem/6028818>: This functionality should eventually be phased out when we rewrite
    // moveParagraphs to not clone/destroy moved content.
    EmitsCharactersBetweenAllVisiblePositions,
    EntersTextControls,
    // Used when we want text for copying, pasting, and transposing.
    EmitsTextsWithoutTranscoding,
    // Used when the visibility of the style should not affect text gathering.
    IgnoresStyleVisibility,
    // Used when emitting the special 0xFFFC character is required. Children for replaced objects will be
    // ignored.
    EmitsObjectReplacementCharacters,
    // Used when pasting inside password field.
    EmitsOriginalText,
    EmitsImageAltText,
    BehavesAsIfNodesFollowing,
    // Makes visiblity test take into account the visibility of the frame.
    // FIXME: This should probably be always on unless TextIteratorBehavior::IgnoresStyleVisibility is set.
    ClipsToFrameAncestors,
    TraversesFlatTree,
    EntersImageOverlays,
    IgnoresUserSelectNone,
    EmitsObjectReplacementCharactersForImages,
    // Used by accessibility to expose untransformed kana text.
    IgnoresFullSizeKana,
    // Used when we want to make 'content-visibility: auto', auto-expanding `<details>` or
    // `hidden=until-found` content discoverable.
    EntersSkippedContentRelevantToUser
  };

  using TextIteratorBehaviors = EnumSet<TextIteratorBehavior>;
}

KRYS_DEFINE_CONTIGUOUS_ENUM_TRAITS(krys::boo::TextIteratorBehavior, 15u);
