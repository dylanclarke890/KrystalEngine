/*
 * Copyright (C) 2025 Samuel Weinig <sam@webkit.org>
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
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "Krystal.Booey/Style/Values/TextDecoration/TextEmphasisStyle.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+GettersInlines.hpp"
#include "Krystal.Booey/Rendering/Style/RenderStyle+SettersInlines.hpp"

#include "Krystal.Booey/CSS/Values/PrimitiveValueMappings.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/Style/Builder/BuilderChecking.hpp"
#include "Krystal.Booey/Style/Builder/BuilderState.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordCSSValueCreation.hpp"
#include "Krystal.Booey/Style/Values/Primitives/KeywordSerialisation.hpp"
#include "Krystal.Booey/Text/WritingMode.hpp"

namespace krys::boo::style
{
  KRYS_NODISCARD static TextEmphasisMark DefaultTextEmphasisMark(WritingMode mode) noexcept
  {
    // "If only filled or open is specified, the shape keyword computes to circle in horizontal typographic
    // modes and sesame in vertical typographic modes." -
    // https://drafts.csswg.org/css-text-decor/#propdef-text-emphasis-style

    return mode.IsVerticalTypographic() ? TextEmphasisMark::Sesame : TextEmphasisMark::Circle;
  }

  KRYS_NODISCARD static TextEmphasisStyle::Shape DefaultTextEmphasisShape(WritingMode writingMode,
                                                                          TextEmphasisFill fill) noexcept
  {
    return {.Fill = fill, .Mark = DefaultTextEmphasisMark(writingMode)};
  }

  namespace
  {
    // The emphasis marks, filled and open.
    constexpr utf8_stringview Bullet = u8"•";
    constexpr utf8_stringview WhiteBullet = u8"◦";
    constexpr utf8_stringview BlackCircle = u8"●";
    constexpr utf8_stringview WhiteCircle = u8"○";
    constexpr utf8_stringview Fisheye = u8"◉";
    constexpr utf8_stringview Bullseye = u8"◎";
    constexpr utf8_stringview BlackUpPointingTriangle = u8"▲";
    constexpr utf8_stringview WhiteUpPointingTriangle = u8"△";
    constexpr utf8_stringview SesameDot = u8"﹅";
    constexpr utf8_stringview WhiteSesameDot = u8"﹆";

    KRYS_NODISCARD const css::CSSOMStringAtom &
      MarkStringFromShape(const TextEmphasisStyle::Shape &markShape) noexcept
    {
      auto pick = [&](const css::CSSOMStringAtom &filled,
                      const css::CSSOMStringAtom &open) -> const css::CSSOMStringAtom &
      {
        return markShape.Fill == TextEmphasisFill::Filled ? filled : open;
      };

      switch (markShape.Mark)
      {
        case TextEmphasisMark::Dot:
        {
          static const css::CSSOMStringAtom filledDotString {Bullet};
          static const css::CSSOMStringAtom openDotString {WhiteBullet};
          return pick(filledDotString, openDotString);
        }
        case TextEmphasisMark::Circle:
        {
          static const css::CSSOMStringAtom filledCircleString {BlackCircle};
          static const css::CSSOMStringAtom openCircleString {WhiteCircle};
          return pick(filledCircleString, openCircleString);
        }
        case TextEmphasisMark::DoubleCircle:
        {
          static const css::CSSOMStringAtom filledDoubleCircleString {Fisheye};
          static const css::CSSOMStringAtom openDoubleCircleString {Bullseye};
          return pick(filledDoubleCircleString, openDoubleCircleString);
        }
        case TextEmphasisMark::Triangle:
        {
          static const css::CSSOMStringAtom filledTriangleString {BlackUpPointingTriangle};
          static const css::CSSOMStringAtom openTriangleString {WhiteUpPointingTriangle};
          return pick(filledTriangleString, openTriangleString);
        }
        case TextEmphasisMark::Sesame:
        {
          static const css::CSSOMStringAtom filledSesameString {SesameDot};
          static const css::CSSOMStringAtom openSesameString {WhiteSesameDot};
          return pick(filledSesameString, openSesameString);
        }
      }

      krys_debug_assert_msg(false, "Unreachable");
      return css::CSSOMStringAtom::Empty();
    }
  }

  const css::CSSOMStringAtom &TextEmphasisStyle::MarkString() const noexcept
  {
    return krys::SwitchOn(
      Value, [&](const css::keywords::None &) -> const css::CSSOMStringAtom &
      { return css::CSSOMStringAtom::Null(); },
      [&](const Shape &shape) -> const css::CSSOMStringAtom & { return MarkStringFromShape(shape); },
      [&](const css::CSSOMStringAtom &customMark) -> const css::CSSOMStringAtom & { return customMark; });
  }

  // MARK: - Conversion

  TextEmphasisStyle CSSValueConversion<TextEmphasisStyle>::operator()(BuilderState &state,
                                                                      const css::Value &value) noexcept
  {
    if (auto *list = DynamicDowncast<css::ValueList>(value))
    {
      if (list->size() != 2) KRYS_UNLIKELY
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::None {};
      }

      Maybe<TextEmphasisFill> fill;
      Maybe<TextEmphasisMark> mark;
      for (const auto &item : *list)
      {
        auto valueID = item.ValueId();
        if (valueID == css::ValueId::Filled || valueID == css::ValueId::Open)
        {
          fill = FromValueId<TextEmphasisFill>(valueID);
        }
        else
        {
          mark = FromValueId<TextEmphasisMark>(valueID);
        }
      }
      if (!fill || !mark) KRYS_UNLIKELY
      {
        state.SetCurrentPropertyInvalidAtComputedValueTime();
        return css::keywords::None {};
      }
      return TextEmphasisStyle::Shape {.Fill = *fill, .Mark = *mark};
    }

    auto *primitiveValue = RequiredDowncast<css::PrimitiveValue>(state, value);
    if (!primitiveValue)
    {
      return css::keywords::None {};
    }

    switch (primitiveValue->ValueId())
    {
      case css::ValueId::Invalid: break;

      case css::ValueId::None:    return css::keywords::None {};

      case css::ValueId::Filled:
      case css::ValueId::Open:
        return DefaultTextEmphasisShape(state.Style().GetWritingMode(),
                                        FromCSSValue<TextEmphasisFill>(*primitiveValue));

      default: return TextEmphasisStyle::Shape {.Mark = FromCSSValue<TextEmphasisMark>(*primitiveValue)};
    }

    if (primitiveValue->IsString())
    {
      return css::CSSOMStringAtom {primitiveValue->StringValue()};
    }

    state.SetCurrentPropertyInvalidAtComputedValueTime();
    return css::keywords::None {};
  }
}
