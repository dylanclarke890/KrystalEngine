#include "Krystal.Booey/CSS/Properties/Consumers/UI.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Image.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/NumberDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Values/CursorImageValue.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"
#include "Krystal.Booey/CSS/Values/ValuePair.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeCursor(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <cursor> = [ [ <url> | <url-set> ] [<x> <y>]? ]#? [ auto | default | none | context-menu | help |
    // pointer | progress | wait | cell | crosshair | text | vertical-text | alias | copy | move | no-drop |
    // not-allowed | grab | grabbing | e-resize | n-resize | ne-resize | nw-resize | s-resize | se-resize |
    // sw-resize | w-resize | ew-resize | ns-resize | nesw-resize | nwse-resize | col-resize | row-resize |
    // all-scroll | zoom-in | zoom-out ] https://drafts.csswg.org/css-ui/#propdef-cursor

    ValueListBuilder list;
    while (auto image =
             ConsumeImage(tokens, state, AllowedImageType::URLFunction | AllowedImageType::ImageSet))
    {
      RefPtr<ValuePair> hotSpot;
      if (auto x = PrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state))
      {
        auto y = PrimitiveValueResolver<Number<>>::ConsumeAndResolve(tokens, state);
        if (!y)
        {
          return nullptr;
        }

        hotSpot = ValuePair::CreateNonCoalescing(krys::move(x), krys::move(y));
      }

      list.push_back(CursorImageValue::Create(krys::move(image), krys::move(hotSpot)));
      if (!ConsumeComma(tokens))
      {
        return nullptr;
      }
    }

    ValueId id = tokens.Peek().ValueId();
    RefPtr<Value> cursorType;
    if (id == ValueId::Hand)
    {
      if (state.Context.Mode != ParserMode::HTMLQuirks) // Non-standard behavior
      {
        return nullptr;
      }

      cursorType = PrimitiveValue::Create(ValueId::Pointer);

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
    return ValueList::CreateCommaSeparated(krys::move(list));
  }
}