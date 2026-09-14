#include "Krystal.Booey/CSS/Properties/Consumers/Inline.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD static RefPtr<Value> ConsumeTextEdge(TokenRange &range) noexcept
  {
    // <text-edge> = [ text | cap | ex | ideographic | ideographic-ink ]
    //               [ text | alphabetic | ideographic | ideographic-ink ]?
    // https://drafts.csswg.org/css-inline-3/#typedef-text-edge

    auto firstValue = ConsumeIdent<ValueId::Text, ValueId::Cap, ValueId::Ex, ValueId::Ideographic,
                                   ValueId::IdeographicInk>(range);
    if (!firstValue)
    {
      return nullptr;
    }

    auto secondValue = ConsumeIdent<ValueId::Text, ValueId::Alphabetic, ValueId::Ideographic,
                                    ValueId::IdeographicInk>(range);

    // https://drafts.csswg.org/css-inline-3/#text-edges
    // "If only one value is specified, both edges are assigned that same keyword if possible; else text is
    // assumed as the missing value."
    auto shouldSerializeSecondValue = [&]()
    {
      if (!secondValue)
      {
        return false;
      }

      if (firstValue->ValueId() == ValueId::Cap || firstValue->ValueId() == ValueId::Ex)
      {
        return secondValue->ValueId() != ValueId::Text;
      }

      return firstValue->ValueId() != secondValue->ValueId();
    }();

    if (!shouldSerializeSecondValue)
    {
      return firstValue;
    }

    return ValuePair::Create(krys::move(firstValue), krys::move(secondValue));
  }

  RefPtr<Value> ConsumeLineFitEdge(TokenRange &range, PropertyParserState &) noexcept
  {
    // <'line-fit-edge'> = leading | <text-edge>
    // https://drafts.csswg.org/css-inline-3/#propdef-line-fit-edge

    if (range.Peek().ValueId() == ValueId::Leading)
    {
      return ConsumeIdent(range);
    }

    return ConsumeTextEdge(range);
  }

  RefPtr<Value> ConsumeTextBoxEdge(TokenRange &range, PropertyParserState &) noexcept
  {
    // <'text-box-edge'> = auto | <text-edge>
    // https://drafts.csswg.org/css-inline-3/#propdef-text-box-edge

    if (range.Peek().ValueId() == ValueId::Auto)
    {
      return ConsumeIdent(range);
    }

    return ConsumeTextEdge(range);
  }

}