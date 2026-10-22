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
