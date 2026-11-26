#pragma once

#include "Krystal.Booey/Animation/Keyframes/CompositeOperationOrAuto.hpp"
#include "Krystal.Booey/Animation/Timelines/TimelineRangeOffset.hpp"
#include "Krystal.Booey/Dom/Types/DOMString.hpp"
#include "Krystal.Core/Containers/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  using KeyframeOffset = Variant<std::nullptr_t, double, TimelineRangeOffset, DOMString>;

  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-basepropertyindexedkeyframe
  struct BasePropertyIndexedKeyframe
  {
    // The list comes first so that the default is an empty list without an initialiser, which would need
    // CSSNumericValue's definition wherever this header is included.
    Variant<List<KeyframeOffset>, std::nullptr_t, double, TimelineRangeOffset, DOMString> Offset;
    Variant<List<DOMString>, DOMString> Easing = List<DOMString>();
    Variant<List<CompositeOperationOrAuto>, CompositeOperationOrAuto> Composite =
      List<CompositeOperationOrAuto>();
  };

  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-basekeyframe
  struct BaseKeyframe
  {
    KeyframeOffset Offset;
    DOMString Easing {u8"linear"};
    CompositeOperationOrAuto Composite {CompositeOperationOrAuto::Auto};
  };

  /// @brief A keyframe in a list of keyframes, as script gives it: its offset, easing and composite
  /// operation, and its properties' values as CSS text, named by their IDL attributes (`backgroundColor`,
  /// `cssFloat`,
  /// `--custom`).
  /// @see https://drafts.csswg.org/web-animations-1/#processing-a-keyframes-argument
  struct InputKeyframe : BaseKeyframe
  {
    List<Pair<DOMString, DOMString>> Properties;
  };

  /// @brief Property-indexed keyframes, as script gives them: each property's list of values, named by its
  /// IDL attribute, with lists of offsets, easings and composite operations.
  /// @see https://drafts.csswg.org/web-animations-1/#processing-a-keyframes-argument
  struct InputPropertyIndexedKeyframes : BasePropertyIndexedKeyframe
  {
    List<Pair<DOMString, List<DOMString>>> Properties;
  };

  /// @brief The keyframes argument of `animate()`, `setKeyframes()` and the `KeyframeEffect` constructor:
  /// none, a list of keyframes, or property-indexed keyframes.
  using KeyframesInput = Variant<std::nullptr_t, List<InputKeyframe>, InputPropertyIndexedKeyframes>;
}
