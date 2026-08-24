#pragma once

#include "Krystal.Lib/Pointers/RefPtr.hpp"
#include "Krystal.Lib/Types/Array.hpp"
#include "Krystal.Lib/Types/Maybe.hpp"
#include <cassert>

namespace Krys::HTML
{
  class CSSTokenRange;
  class CSSValue;
  struct CSSPropertyParserState;
  struct BorderImageComponents;
  struct BorderRadius;

  namespace CSSPropertyParserHelpers
  {
    // Default value of the `fill` parameter for `border-image-slice`.
    enum class BorderImageSliceFillDefault : bool
    {
      No,
      Yes
    };

    // <'border-radius'> = <length-percentage [0,∞]>{1,4} [ / <length-percentage [0,∞]>{1,4} ]?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-radius
    KRYS_NODISCARD Maybe<BorderRadius> ConsumeUnresolvedBorderRadius(CSSTokenRange &tokens,
                                                                     CSSPropertyParserState &state) noexcept;
    // <'border-image-slice'> = [<number [0,∞]> | <percentage [0,∞]>]{1,4} && fill?
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-slice
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeBorderImageSlice(
      CSSTokenRange &tokens, CSSPropertyParserState &state,
      BorderImageSliceFillDefault fillDefault = BorderImageSliceFillDefault::No) noexcept;

    // <'border-image-width'> = [ <length-percentage [0,∞]> | <number [0,∞]> | auto ]{1,4}
    // https://drafts.csswg.org/css-backgrounds/#propdef-border-image-width
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeBorderImageWidth(CSSTokenRange &tokens,
                                                            CSSPropertyParserState &state) noexcept;

    // https://drafts.csswg.org/css-backgrounds/#border-image
    KRYS_NODISCARD Maybe<BorderImageComponents>
      ConsumeBorderImageComponents(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept;

    // <bg-size> = [ <length-percentage [0,∞]> | auto ]{1,2} | cover | contain
    // https://drafts.csswg.org/css-backgrounds/#background-size
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleBackgroundSize(CSSTokenRange &tokens,
                                                                CSSPropertyParserState &state) noexcept;

    // <single-mask-size> = <bg-size>
    // https://drafts.fxtf.org/css-masking/#the-mask-size
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeSingleMaskSize(CSSTokenRange &tokens,
                                                          CSSPropertyParserState &state) noexcept;

    // <repeat-style> = repeat-x | repeat-y | [repeat | space | round | no-repeat]{1,2}
    // https://drafts.csswg.org/css-backgrounds/#typedef-repeat-style
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeRepeatStyle(CSSTokenRange &tokens,
                                                       CSSPropertyParserState &state) noexcept;

    // <'box-shadow'> = none | <shadow>#
    // https://drafts.csswg.org/css-backgrounds/#propdef-box-shadow
    KRYS_NODISCARD RefPtr<CSSValue> ConsumeBoxShadow(CSSTokenRange &tokens,
                                                     CSSPropertyParserState &state) noexcept;

    // MARK: Utilities for filling in rects / quads in the "margin" form.

    // - if only 1 value, `a`, is provided, set top, bottom, right & left to `a`.
    // - if only 2 values, `a` and `b` are provided, set top & bottom to `a`, right & left to `b`.
    // - if only 3 values, `a`, `b`, and `c` are provided, set top to `a`, right to `b`, bottom to `c`, & left
    // to `b`.

    template <typename Container, typename T>
    KRYS_NODISCARD Container CompleteQuad(T a) noexcept
    {
      return Container {a, a, a, a};
    }

    template <typename Container, typename T>
    KRYS_NODISCARD Container CompleteQuad(T a, T b) noexcept
    {
      return Container {a, b, a, b};
    }

    template <typename Container, typename T>
    KRYS_NODISCARD Container CompleteQuad(T a, T b, T c) noexcept
    {
      return Container {a, b, c, b};
    }

    template <typename Container, typename T>
    KRYS_NODISCARD Container CompleteQuadFromArray(Array<Maybe<T>, 4uz> optionals) noexcept
    {
      assert(optionals[0].has_value());

      if (!optionals[1])
      {
        return CompleteQuad<Container>(Krys::Move(*optionals[0]));
      }

      if (!optionals[2])
      {
        return CompleteQuad<Container>(Krys::Move(*optionals[0]), Krys::Move(*optionals[1]));
      }

      if (!optionals[3])
      {
        return CompleteQuad<Container>(Krys::Move(*optionals[0]), Krys::Move(*optionals[1]),
                                       Krys::Move(*optionals[2]));
      }

      return Container {Krys::Move(*optionals[0]), Krys::Move(*optionals[1]), Krys::Move(*optionals[2]),
                        Krys::Move(*optionals[3])};
    }
  }
}