#pragma once

#include "Krystal.Booey/Animation/CompositeOperationOrAuto.hpp"
#include "Krystal.Booey/Animation/TimelineRangeOffset.hpp"
#include "Krystal.Booey/DOM/Types/DOMString.hpp"
#include "Krystal.Core/Types/List.hpp"
#include "Krystal.Core/Types/Pair.hpp"
#include "Krystal.Core/Types/Variant.hpp"

namespace krys::boo
{
  using KeyframeOffset = Variant<std::nullptr_t, double, TimelineRangeOffset, dom::DOMString>;

  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-basepropertyindexedkeyframe
  struct BasePropertyIndexedKeyframe
  {
    Variant<std::nullptr_t, List<KeyframeOffset>, double, TimelineRangeOffset, dom::DOMString> Offset =
      List<KeyframeOffset>();
    Variant<List<dom::DOMString>, dom::DOMString> Easing = List<dom::DOMString>();
    Variant<List<CompositeOperationOrAuto>, CompositeOperationOrAuto> Composite =
      List<CompositeOperationOrAuto>();
  };

  /// @see https://drafts.csswg.org/web-animations-1/#dictdef-basekeyframe
  struct BaseKeyframe
  {
    KeyframeOffset Offset;
    dom::DOMString Easing {u8"linear"};
    CompositeOperationOrAuto Composite {CompositeOperationOrAuto::Auto};
  };

  /// @brief A keyframe in a list of keyframes, as script gives it: its offset, easing and composite
  /// operation, and its properties' values as CSS text, named by their IDL attributes (`backgroundColor`,
  /// `cssFloat`,
  /// `--custom`).
  /// @see https://drafts.csswg.org/web-animations-1/#processing-a-keyframes-argument
  struct InputKeyframe : BaseKeyframe
  {
    List<Pair<dom::DOMString, dom::DOMString>> Properties;
  };

  /// @brief Property-indexed keyframes, as script gives them: each property's list of values, named by its
  /// IDL attribute, with lists of offsets, easings and composite operations.
  /// @see https://drafts.csswg.org/web-animations-1/#processing-a-keyframes-argument
  struct InputPropertyIndexedKeyframes : BasePropertyIndexedKeyframe
  {
    List<Pair<dom::DOMString, List<dom::DOMString>>> Properties;
  };

  /// @brief The keyframes argument of `animate()`, `setKeyframes()` and the `KeyframeEffect` constructor:
  /// none, a list of keyframes, or property-indexed keyframes.
  using KeyframesInput = Variant<std::nullptr_t, List<InputKeyframe>, InputPropertyIndexedKeyframes>;
}
