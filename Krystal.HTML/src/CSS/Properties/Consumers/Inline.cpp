#include "Krystal.HTML/CSS/Properties/Consumers/Inline.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeTextEdge(CSSTokenRange &range) noexcept
  {
    // <text-edge> = [ text | cap | ex | ideographic | ideographic-ink ]
    //               [ text | alphabetic | ideographic | ideographic-ink ]?
    // https://drafts.csswg.org/css-inline-3/#typedef-text-edge

    auto firstValue = ConsumeIdent<CSSValueId::Text, CSSValueId::Cap, CSSValueId::Ex, CSSValueId::Ideographic,
                                   CSSValueId::IdeographicInk>(range);
    if (!firstValue)
    {
      return nullptr;
    }

    auto secondValue = ConsumeIdent<CSSValueId::Text, CSSValueId::Alphabetic, CSSValueId::Ideographic,
                                    CSSValueId::IdeographicInk>(range);

    // https://drafts.csswg.org/css-inline-3/#text-edges
    // "If only one value is specified, both edges are assigned that same keyword if possible; else text is
    // assumed as the missing value."
    auto shouldSerializeSecondValue = [&]()
    {
      if (!secondValue)
      {
        return false;
      }

      if (firstValue->ValueId() == CSSValueId::Cap || firstValue->ValueId() == CSSValueId::Ex)
      {
        return secondValue->ValueId() != CSSValueId::Text;
      }

      return firstValue->ValueId() != secondValue->ValueId();
    }();

    if (!shouldSerializeSecondValue)
    {
      return firstValue;
    }

    return CSSValuePair::Create(Krys::Move(firstValue), Krys::Move(secondValue));
  }

  RefPtr<CSSValue> ConsumeLineFitEdge(CSSTokenRange &range, CSSPropertyParserState &) noexcept
  {
    // <'line-fit-edge'> = leading | <text-edge>
    // https://drafts.csswg.org/css-inline-3/#propdef-line-fit-edge

    if (range.Peek().ValueId() == CSSValueId::Leading)
    {
      return ConsumeIdent(range);
    }

    return ConsumeTextEdge(range);
  }

  RefPtr<CSSValue> ConsumeTextBoxEdge(CSSTokenRange &range, CSSPropertyParserState &) noexcept
  {
    // <'text-box-edge'> = auto | <text-edge>
    // https://drafts.csswg.org/css-inline-3/#propdef-text-box-edge

    if (range.Peek().ValueId() == CSSValueId::Auto)
    {
      return ConsumeIdent(range);
    }

    return ConsumeTextEdge(range);
  }

}