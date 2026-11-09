/*
 * Copyright (C) 2016-2023 Apple Inc. All rights reserved.
 * Copyright (C) 2024 Samuel Weinig <sam@webkit.org>
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

#include "Krystal.Booey/CSS/Properties/Consumers/SVG.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthPercentageDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumePaintOrder(TokenRange &range, PropertyParserState &) noexcept
  {
    // <'paint-order'> = normal | [ fill || stroke || markers ]
    // https://svgwg.org/svg2-draft/painting.html#PaintOrderProperty

    if (range.Peek().ValueId() == ValueId::Normal)
    {
      return ConsumeIdent(range);
    }

    SmallList<ValueId, 3uz> paintTypeList;
    RefPtr<PrimitiveValue> fill;
    RefPtr<PrimitiveValue> stroke;
    RefPtr<PrimitiveValue> markers;
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
    // pop a last list items from ValueList without bigger cost, we create the
    // list after parsing.
    ValueId firstPaintOrderType = paintTypeList[0];
    ValueListBuilder paintOrderList;
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
        krys_debug_assert(false);
        return nullptr;
      }
    }

    return ValueList::CreateSpaceSeparated(krys::move(paintOrderList));
  }

  RefPtr<Value> ConsumeStrokeDasharray(TokenRange &range, PropertyParserState &state) noexcept
  {
    // <'stroke-dasharray'> = none | [ [ <length-percentage> | <number> ]+ ]#
    // https://svgwg.org/svg2-draft/painting.html#StrokeDashing

    ValueId id = range.Peek().ValueId();
    if (id == ValueId::None)
    {
      return ConsumeIdent(range);
    }

    ValueListBuilder dashes;
    do
    {
      // FIXME: Figure out and document why overrideParserMode is explicitly set to HTMLStandardMode here or
      // remove the special case.
      auto dash = PrimitiveValueResolver<LengthPercentage<NonNegative>>::ConsumeAndResolve(
        range, state,
        {
          .UnitlessZeroLength = AllowUnitlessZero(false),
          .OverrideParserMode = ParserMode::HTMLStandard,
        });

      if (!dash)
      {
        dash = PrimitiveValueResolver<Number<NonNegative>>::ConsumeAndResolve(range, state);
      }

      if (!dash || (ConsumeComma(range) && range.IsAtEnd()))
      {
        return nullptr;
      }

      dashes.push_back(krys::move(dash));
    } while (!range.IsAtEnd());

    return ValueList::CreateCommaSeparated(krys::move(dashes));
  }
}