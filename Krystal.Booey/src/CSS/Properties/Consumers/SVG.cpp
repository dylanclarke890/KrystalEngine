#include "Krystal.Booey/CSS/Properties/Consumers/SVG.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumePaintOrder(TokenRange &range, PropertyParserState &) noexcept
  {
    // <'paint-order'> = normal | [ fill || stroke || markers ]
    // https://svgwg.org/svg2-draft/painting.html#PaintOrderProperty

    if (range.Peek().ValueId() == ValueId::Normal)
    {
      return ConsumeIdent(range);
    }

    SmallList<ValueId, 3uz> paintTypeList;
    RefPtr<CSSPrimitiveValue> fill;
    RefPtr<CSSPrimitiveValue> stroke;
    RefPtr<CSSPrimitiveValue> markers;
    do
    {
      ValueId id = range.Peek().ValueId();
      if (id == ValueId::Fill && !fill)
      {
        fill = ConsumeIdent(range);
      }
      else if (id == ValueId::Stroke && !stroke)
      {
        stroke = ConsumeIdent(range);
      }
      else if (id == ValueId::Markers && !markers)
      {
        markers = ConsumeIdent(range);
      }
      else
      {
        return nullptr;
      }

      paintTypeList.push_back(id);
    } while (!range.IsAtEnd());

    // After parsing we serialize the paint-order list. Since it is not possible to
    // pop a last list items from CSSValueList without bigger cost, we create the
    // list after parsing.
    ValueId firstPaintOrderType = paintTypeList[0];
    CSSValueListBuilder paintOrderList;
    switch (firstPaintOrderType)
    {
      case ValueId::Fill:
      case ValueId::Stroke:
      {
        paintOrderList.push_back(firstPaintOrderType == ValueId::Fill ? krys::move(fill)
                                                                         : krys::move(stroke));
        if (paintTypeList.size() > 1uz && paintTypeList[1] == ValueId::Markers)
        {
          paintOrderList.push_back(krys::move(markers));
        }

        break;
      }
      case ValueId::Markers:
      {
        paintOrderList.push_back(krys::move(markers));
        if (paintTypeList.size() > 1uz && paintTypeList[1] == ValueId::Stroke)
        {
          paintOrderList.push_back(krys::move(stroke));
        }

        break;
      }
      default:
      {
        assert(false);
        return nullptr;
      }
    }

    return CSSValueList::CreateSpaceSeparated(krys::move(paintOrderList));
  }

  RefPtr<CSSValue> ConsumeStrokeDasharray(TokenRange &range, PropertyParserState &state) noexcept
  {
    // <'stroke-dasharray'> = none | [ [ <length-percentage> | <number> ]+ ]#
    // https://svgwg.org/svg2-draft/painting.html#StrokeDashing

    ValueId id = range.Peek().ValueId();
    if (id == ValueId::None)
    {
      return ConsumeIdent(range);
    }

    CSSValueListBuilder dashes;
    do
    {
      // FIXME: Figure out and document why overrideParserMode is explicitly set to HTMLStandardMode here or
      // remove the special case.
      auto dash = CSSPrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(
        range, state,
        {
          .UnitlessZeroLength = AllowUnitlessZero(false),
          .OverrideParserMode = ParserMode::HTMLStandard,
        });

      if (!dash)
      {
        dash = CSSPrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(range, state);
      }

      if (!dash || (ConsumeComma(range) && range.IsAtEnd()))
      {
        return nullptr;
      }

      dashes.push_back(krys::move(dash));
    } while (!range.IsAtEnd());

    return CSSValueList::CreateCommaSeparated(krys::move(dashes));
  }
}