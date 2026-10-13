#include "Krystal.Booey/CSS/Properties/Consumers/Motion.hpp"
#include "Krystal.Booey/CSS/Parser/TokenRange.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/AngleDefinitions.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Ident.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/MetaConsumer.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Position.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Primitives.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/Shapes.hpp"
#include "Krystal.Booey/CSS/Properties/Consumers/URL.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParserState.hpp"
#include "Krystal.Booey/CSS/Properties/PropertyParsing.hpp"
#include "Krystal.Booey/CSS/Values/Motion/RayFunction.hpp"
#include "Krystal.Booey/CSS/Values/Primitives/Position.hpp"
#include "Krystal.Booey/CSS/Values/PrimitiveValue.hpp"
#include "Krystal.Booey/CSS/Values/RayValue.hpp"
#include "Krystal.Booey/CSS/Values/ValueList.hpp"

namespace krys::boo::css::PropertyParserHelpers
{
  KRYS_NODISCARD static Maybe<RaySize> RaySizeFromValueId(ValueId id) noexcept
  {
    switch (id)
    {
      case ValueId::ClosestSide:
      {
        return RaySize {keywords::ClosestSide {}};
      }
      case ValueId::ClosestCorner:
      {
        return RaySize {keywords::ClosestCorner {}};
      }
      case ValueId::FarthestSide:
      {
        return RaySize {keywords::FarthestSide {}};
      }
      case ValueId::FarthestCorner:
      {
        return RaySize {keywords::FarthestCorner {}};
      }
      case ValueId::Sides:
      {
        return RaySize {keywords::Sides {}};
      }
      default:
      {
        return null;
      }
    }
  }

  KRYS_NODISCARD static RefPtr<Value> ConsumeRayFunction(TokenRange &tokens,
                                                         PropertyParserState &state) noexcept
  {
    // ray( <angle> && <ray-size>? && contain? && [at <position>]? )
    // <ray-size> = closest-side | closest-corner | farthest-side | farthest-corner | sides
    // https://drafts.fxtf.org/motion-1/#ray-function

    if (tokens.Peek().Type() != TokenType::Function || tokens.Peek().FunctionId() != ValueId::Ray)
    {
      return {};
    }

    auto args = ConsumeFunction(tokens);

    Maybe<Angle<>> angle;
    Maybe<RaySize> size;
    Maybe<keywords::Contain> contain;
    Maybe<Position> position;

    auto ConsumeAngle = [&] -> bool
    {
      if (angle)
      {
        return false;
      }

      angle = MetaConsumer<Angle<>>::Consume(args, state);
      return angle.has_value();
    };

    auto ConsumeSize = [&] -> bool
    {
      if (size)
      {
        return false;
      }

      size = ConsumeIdentUsingMapping(args, RaySizeFromValueId);
      return size.has_value();
    };

    auto ConsumeContain = [&] -> bool
    {
      if (contain || !ConsumeIdentRaw<ValueId::Contain>(args).has_value())
      {
        return false;
      }

      contain = keywords::Contain {};
      return true;
    };

    auto ConsumeAtPosition = [&] -> bool
    {
      if (position || !ConsumeIdentRaw<ValueId::At>(args).has_value())
      {
        return false;
      }

      position = ConsumePositionUnresolved(args, state);
      return position.has_value();
    };

    while (!args.IsAtEnd())
    {
      if (ConsumeAngle() || ConsumeSize() || ConsumeContain() || ConsumeAtPosition())
      {
        continue;
      }

      return {};
    }

    // The <angle> argument is the only one that is required.
    if (!angle)
    {
      return {};
    }

    return RayValue::Create(
      RayFunction {.Parameters = Ray {krys::move(*angle), size.value_or(RaySize {keywords::ClosestSide {}}),
                                      krys::move(contain), krys::move(position)}});
  }

  RefPtr<Value> ConsumeOffsetPath(TokenRange &tokens, PropertyParserState &state) noexcept
  {
    // <'offset-path'> = none | <offset-path> || <coord-box>
    //
    // NOTE: The sub-production, <offset-path> (without the quotation marks) is distinct and defined as:
    //    <offset-path> = <ray()> | <url> | <basic-shape>
    //
    // So, this expands out to a grammar of:
    //
    // <'offset-path'> = none | [ <ray()> | <url> | <basic-shape> || <coord-box> ]
    //
    // which is almost the same as <'clip-path'> above, with the following differences:
    //
    // 1. <'clip-path'> does not support `ray()`.
    // 2. <'clip-path'> does not allow a `box` to be provided with `<url>`.
    // 3. <'clip-path'> specifies `<geometry-box>` rather than `<coord-box>`.
    //
    // https://drafts.fxtf.org/motion-1/#propdef-offset-path

    if (tokens.Peek().ValueId() == ValueId::None)
    {
      return ConsumeIdent(tokens);
    }

    // FIXME: It should be possible to consume both a <url> and <coord-box>.
    if (auto url = ConsumeURL(tokens, state, {}))
    {
      return url;
    }

    RefPtr<Value> shapeOrRay;
    RefPtr<Value> box;

    auto ConsumeRay = [&]() -> bool
    {
      if (shapeOrRay)
      {
        return false;
      }

      shapeOrRay = ConsumeRayFunction(tokens, state);
      return !!shapeOrRay;
    };

    auto ConsumeShape = [&]() -> bool
    {
      if (shapeOrRay)
      {
        return false;
      }

      shapeOrRay = ConsumeBasicShape(tokens, state, PathParsingOption::RejectPathFillRule);
      return !!shapeOrRay;
    };

    auto ConsumeBox = [&]() -> bool
    {
      if (box)
      {
        return false;
      }

      // FIXME: The Motion Path spec calls for this to be a <coord-box>, not a <geometry-box>, the difference
      // being that the former does not contain "margin-box" as a valid term. However, the spec also has a few
      // examples using "margin-box", so there seems to be some abiguity to be resolved. See:
      // https://github.com/w3c/fxtf-drafts/issues/481.
      box = PropertyParsing::ConsumeGeometryBox(tokens);
      return !!box;
    };

    while (!tokens.IsAtEnd())
    {
      if (ConsumeRay() || ConsumeShape() || ConsumeBox())
      {
        continue;
      }

      break;
    }

    bool hasShapeOrRay = !!shapeOrRay;

    ValueListBuilder list;
    if (shapeOrRay)
    {
      list.push_back(krys::move(shapeOrRay));
    }

    // Default value is border-box.
    if (box && (box->ValueId() != ValueId::BorderBox || !hasShapeOrRay))
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