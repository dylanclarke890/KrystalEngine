#include "Krystal.HTML/CSS/Properties/Consumers/UI.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Values/CSSCursorImageValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.HTML/CSS/Values/CSSValuePair.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeCursor(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // <cursor> = [ [ <url> | <url-set> ] [<x> <y>]? ]#? [ auto | default | none | context-menu | help |
    // pointer | progress | wait | cell | crosshair | text | vertical-text | alias | copy | move | no-drop |
    // not-allowed | grab | grabbing | e-resize | n-resize | ne-resize | nw-resize | s-resize | se-resize |
    // sw-resize | w-resize | ew-resize | ns-resize | nesw-resize | nwse-resize | col-resize | row-resize |
    // all-scroll | zoom-in | zoom-out ] https://drafts.csswg.org/css-ui/#propdef-cursor

    CSSValueListBuilder list;
    while (auto image =
             ConsumeImage(tokens, state, AllowedImageType::URLFunction | AllowedImageType::ImageSet))
    {
      RefPtr<CSSValuePair> hotSpot;
      if (auto x = CSSPrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state))
      {
        auto y = CSSPrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state);
        if (!y)
        {
          return nullptr;
        }

        hotSpot = CSSValuePair::CreateNonCoalescing(Krys::Move(x), Krys::Move(y));
      }

      list.push_back(CSSCursorImageValue::Create(Krys::Move(image), Krys::Move(hotSpot)));
      if (!ConsumeComma(tokens))
      {
        return nullptr;
      }
    }

    CSSValueId id = tokens.Peek().ValueId();
    RefPtr<CSSValue> cursorType;
    if (id == CSSValueId::Hand)
    {
      if (state.Context.Mode != CSSParserMode::HTMLQuirks) // Non-standard behavior
      {
        return nullptr;
      }

      cursorType = CSSPrimitiveValue::Create(CSSValueId::Pointer);

      tokens.Discard();
      tokens.DiscardWhitespace();
    }
    else if ((id >= CSSValueId::Auto && id <= CSSValueId::WebkitZoomOut) || id == CSSValueId::Copy
             || id == CSSValueId::None)
    {
      cursorType = ConsumeIdent(tokens);
    }
    else
    {
      return nullptr;
    }

    if (list.empty())
    {
      return cursorType;
    }

    list.push_back(Krys::Move(cursorType));
    return CSSValueList::CreateCommaSeparated(Krys::Move(list));
  }
}