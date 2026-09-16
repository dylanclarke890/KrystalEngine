#include "Krystal.Booey/CSS/Properties/Consumers/Masking.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/LengthDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Shapes.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RectValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"
#include "Krystal.Booey/CSS/Values/ValueListBuilder.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  RefPtr<Value> ConsumeClipRectFunction(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // rect() = rect( <top>, <right>, <bottom>, <left> )
    // "<top>, <right>, <bottom>, and <left> may either have a <length> value or auto."
    // https://drafts.fxtf.org/css-masking/#funcdef-clip-rect

    if (tokens.Peek().FunctionId() != ValueId::Rect)
    {
      return nullptr;
    }

    TokenRange args = ConsumeFunction(tokens);

    auto ConsumeClipComponent = [&] -> RefPtr<PrimitiveValue>
    {
      if (args.Peek().ValueId() == ValueId::Auto)
      {
        return ConsumeIdent(args);
      }

      return PrimitiveValueResolver<Length<>>::ConsumeAndResolve(args, state);
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

    return RectValue::Create(
      Rect {krys::move(top), krys::move(right), krys::move(bottom), krys::move(left)});
  }

  RefPtr<Value>
    krys::boo::css::PropertyParserHelpers::ConsumeClipPath(TokenRange &tokens,
                                                              PropertyParserState &state) noexcept
  {
    // <'clip-path'> = none | <clip-source> | [ <basic-shape> || <geometry-box> ]
    // <clip-source> = <url>
    // https://drafts.fxtf.org/css-masking/#propdef-clip-path

    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    if (auto url = ConsumeURL(tokens, state, {}))
    {
      return url;
    }

    RefPtr<Value> shape;
    RefPtr<Value> box;

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

      box = PropertyParsing::ConsumeGeometryBox(tokens);
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

    ValueListBuilder list;
    if (shape)
    {
      list.push_back(krys::move(shape));
    }
    // Default value is border-box.
    if (box && (box->ValueId() != ValueId::BorderBox || !hasShape))
    {
      list.push_back(krys::move(box));
    }

    if (list.empty())
    {
      return nullptr;
    }

    return ValueList::CreateSpaceSeparated(krys::move(list));
  }
}