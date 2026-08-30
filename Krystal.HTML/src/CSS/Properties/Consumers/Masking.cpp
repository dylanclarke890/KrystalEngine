#include "Krystal.HTML/CSS/Properties/Consumers/Masking.hpp"
#include "Krystal.HTML/CSS/Parser/CSSTokenRange.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/Shapes.hpp"
#include "Krystal.HTML/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParserState.hpp"
#include "Krystal.HTML/CSS/Properties/CSSPropertyParsing.hpp"
#include "Krystal.HTML/CSS/Values/CSSPrimitiveValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSRectValue.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueList.hpp"
#include "Krystal.HTML/CSS/Values/CSSValueListBuilder.hpp"

namespace Krys::HTML::CSSPropertyParserHelpers
{
  RefPtr<CSSValue> ConsumeClipRectFunction(CSSTokenRange &tokens, CSSPropertyParserState &state) noexcept
  {
    // rect() = rect( <top>, <right>, <bottom>, <left> )
    // "<top>, <right>, <bottom>, and <left> may either have a <length> value or auto."
    // https://drafts.fxtf.org/css-masking/#funcdef-clip-rect

    if (tokens.Peek().FunctionId() != CSSValueId::Rect)
    {
      return nullptr;
    }

    CSSTokenRange args = ConsumeFunction(tokens);

    auto ConsumeClipComponent = [&] -> RefPtr<CSSPrimitiveValue>
    {
      if (args.Peek().ValueId() == CSSValueId::Auto)
      {
        return ConsumeIdent(args);
      }

      return CSSPrimitiveValueResolver<Length<>>::ConsumeAndResolve(args, state);
    };

    // Support both rect(t, r, b, l) and rect(t r b l).
    //
    // "User agents must support separation with commas, but may also support
    //  separation without commas (but not a combination), because a previous
    //  revision of this specification was ambiguous in this respect"
    auto top = ConsumeClipComponent();
    if (!top)
    {
      return nullptr;
    }

    bool needsComma = ConsumeComma(args);

    auto right = ConsumeClipComponent();
    if (!right || (needsComma && !ConsumeComma(args)))
    {
      return nullptr;
    }

    auto bottom = ConsumeClipComponent();
    if (!bottom || (needsComma && !ConsumeComma(args)))
    {
      return nullptr;
    }

    auto left = ConsumeClipComponent();
    if (!left || !args.IsAtEnd())
    {
      return nullptr;
    }

    return CSSRectValue::Create(
      Rect {Krys::Move(top), Krys::Move(right), Krys::Move(bottom), Krys::Move(left)});
  }

  RefPtr<CSSValue> Krys::HTML::CSSPropertyParserHelpers::ConsumeClipPath(CSSTokenRange &tokens,
                                                                       CSSPropertyParserState &state) noexcept
  {
    // <'clip-path'> = none | <clip-source> | [ <basic-shape> || <geometry-box> ]
    // <clip-source> = <url>
    // https://drafts.fxtf.org/css-masking/#propdef-clip-path

    if (tokens.Peek().ValueId() == CSSValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    if (auto url = ConsumeURL(tokens, state, {}))
    {
      return url;
    }

    RefPtr<CSSValue> shape;
    RefPtr<CSSValue> box;

    auto ConsumeShape = [&]() -> bool
    {
      if (shape)
      {
        return false;
      }

      shape = ConsumeBasicShape(tokens, state, {});
      return !!shape;
    };

    auto ConsumeBox = [&]() -> bool
    {
      if (box)
      {
        return false;
      }

      box = CSSPropertyParsing::ConsumeGeometryBox(tokens);
      return !!box;
    };

    while (!tokens.IsAtEnd())
    {
      if (ConsumeShape() || ConsumeBox())
      {
        continue;
      }

      break;
    }

    bool hasShape = !!shape;

    CSSValueListBuilder list;
    if (shape)
    {
      list.push_back(Krys::Move(shape));
    }
    // Default value is border-box.
    if (box && (box->ValueId() != CSSValueId::BorderBox || !hasShape))
    {
      list.push_back(Krys::Move(box));
    }

    if (list.empty())
    {
      return nullptr;
    }

    return CSSValueList::CreateSpaceSeparated(Krys::Move(list));
  }
}