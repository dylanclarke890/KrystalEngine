#include "Krystal.Booey/CSS/Properties/Consumers/Inline.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"

namespace krys::boo::css::CSSPropertyParserHelpers
{
  KRYS_NODISCARD static RefPtr<CSSValue> ConsumeTextEdge(TokenRange &range) noexcept
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

    return CSSValuePair::Create(krys::move(firstValue), krys::move(secondValue));
  }

  RefPtr<CSSValue> ConsumeLineFitEdge(TokenRange &range, CSSPropertyParserState &) noexcept
  {
    // <'line-fit-edge'> = leading | <text-edge>
    // https://drafts.csswg.org/css-inline-3/#propdef-line-fit-edge

    if (range.Peek().ValueId() == CSSValueId::Leading)
    {
      return ConsumeIdent(range);
    }

    return ConsumeTextEdge(range);
  }

  RefPtr<CSSValue> ConsumeTextBoxEdge(TokenRange &range, CSSPropertyParserState &) noexcept
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