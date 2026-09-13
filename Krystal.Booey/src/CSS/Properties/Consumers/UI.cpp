#include "Krystal.Booey/CSS/Properties/Consumers/UI.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CSSCursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueList.hpp"
#include "Krystal.Booey/CSS/Values/CSSValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/CSSValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeCursor(TokenRange &tokens, PropertyParserState &state) noexcept
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

        hotSpot = CSSValuePair::CreateNonCoalescing(krys::move(x), krys::move(y));
      }

      list.push_back(CSSCursorImageValue::Create(krys::move(image), krys::move(hotSpot)));
      if (!ConsumeComma(tokens))
      {
        return nullptr;
      }
    }

    ValueId id = tokens.Peek().ValueId();
    RefPtr<CSSValue> cursorType;
    if (id == ValueId::Hand)
    {
      if (state.Context.Mode != ParserMode::HTMLQuirks) // Non-standard behavior
      {
        return nullptr;
      }

      cursorType = CSSPrimitiveValue::Create(ValueId::Pointer);

      tokens.Discard();
      tokens.DiscardWhitespace();
    }
    else if ((id >= ValueId::Auto && id <= ValueId::WebkitZoomOut) || id == ValueId::Copy
             || id == ValueId::None)
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

    list.push_back(krys::move(cursorType));
    return CSSValueList::CreateCommaSeparated(krys::move(list));
  }
}